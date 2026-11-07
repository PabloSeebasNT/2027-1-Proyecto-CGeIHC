#include "ArcoirisOoo.h"
#include "Geometria.h"
#include "PlanetaOoo.h"
#include <gtc/matrix_transform.hpp>
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>
#include <gtc/type_ptr.hpp>

namespace
{
    enum Parte { CUERPO1, CUERPO2, CUERPO3, CUERPO4, CUERPO5,
        CABEZA, CARA, COLA, PATA_DEL_D, PATA_DEL_I, PATA_TRAS_D, PATA_TRAS_I, TOTAL };

    struct Articulacion
    {
        const char* archivo;
        int padre;
        glm::vec3 pivote;
    };

    // Coordenadas originales de Blender. Los pivotes del cuerpo se midieron
    // en los vertices compartidos por las dos piezas de cada union.
    const Articulacion articulaciones[TOTAL] = {
        {"cuerpo1", -1, {0,.380161f,0}},
        {"cuerpo2", CUERPO1, {0,.380161f,-.321203f}},
        {"cuerpo3", CUERPO2, {0,.387012f,-1.106684f}},
        {"cuerpo4", CUERPO3, {0,.359407f,-1.894075f}},
        {"cuerpo5", CUERPO4, {0,.359407f,-2.680866f}},
        {"caebza", CUERPO1, {0,.396621f,.101391f}},
        {"ojosboca", CABEZA, {0,.396621f,.101391f}},
        {"cola", CUERPO5, {0,.401f,-3.275f}},
        {"pataenfrenteder", CUERPO1, {-.047f,.325f,.070f}},
        {"pataenfrenteizq", CUERPO1, {.047f,.325f,.070f}},
        {"pataatrasder", CUERPO5, {-.047f,.325f,-3.214f}},
        {"pataatrasizq", CUERPO5, {.047f,.325f,-3.214f}}
    };
    Model piezas[TOTAL];
    Material mate(.12f,12);

    const float PI=3.14159265f;
    glm::mat4 huesos[TOTAL];
    // Solo el cuerpo necesita una franja flexible entre articulaciones.
    struct MallaFlexible {
        int parte;
        GLuint vao=0,vbo=0,ibo=0;
        GLsizei indices=0;
        Texture* textura=nullptr;
        std::vector<float> reposo,vertices;
        std::vector<int> articulacion;
    };
    std::vector<MallaFlexible> cuerpo;

    void CargarCuerpo(int parte,const std::string& ruta)
    {
        Assimp::Importer lector;
        const aiScene* escena=lector.ReadFile(ruta,aiProcess_Triangulate|aiProcess_GenSmoothNormals|aiProcess_JoinIdenticalVertices);
        if(!escena){printf("Error cargando Arcoiris: %s\n",lector.GetErrorString());return;}
        for(unsigned int k=0;k<escena->mNumMeshes;++k)
        {
            auto m=escena->mMeshes[k]; MallaFlexible f;f.parte=parte;
            // Algunos trozos de pata se exportaron dentro de cuerpo4/cuerpo5.
            // Detectar componentes separados y hacer que hereden SU pata.
            // No se borran caras: pueden completar una abertura del OBJ de la pata.
            std::vector<unsigned int> grupos(m->mNumVertices);
            for(unsigned int i=0;i<m->mNumVertices;++i)grupos[i]=i;
            auto grupo=[&](unsigned int i){while(grupos[i]!=i)i=grupos[i];return i;};
            for(unsigned int i=0;i<m->mNumFaces;++i){
                const auto& cara=m->mFaces[i];
                for(unsigned int j=1;j<cara.mNumIndices;++j)
                    grupos[grupo(cara.mIndices[j])]=grupo(cara.mIndices[0]);
            }
            f.articulacion.assign(m->mNumVertices,parte);
            if(parte==CUERPO4||parte==CUERPO5){
                for(unsigned int g=0;g<m->mNumVertices;++g){
                    if(grupo(g)!=g)continue;
                    glm::vec3 minimo(1000),maximo(-1000);
                    for(unsigned int i=0;i<m->mNumVertices;++i)if(grupo(i)==g){
                        auto v=m->mVertices[i];glm::vec3 p(v.x,v.y,v.z);
                        minimo=glm::min(minimo,p);maximo=glm::max(maximo,p);
                    }
                    bool fragmento=maximo.z<-3.1f&&maximo.y<.33f&&
                        minimo.x>-.061f&&maximo.x<.061f&&minimo.x*maximo.x>0;
                    if(fragmento){
                        int pata=maximo.x<0?PATA_TRAS_D:PATA_TRAS_I;
                        int cantidad=0;
                        for(unsigned int i=0;i<m->mNumVertices;++i)if(grupo(i)==g){f.articulacion[i]=pata;++cantidad;}
                        printf("Arcoiris: %d vertices de cuerpo%d vinculados a pata %s\n",cantidad,parte+1,pata==PATA_TRAS_D?"trasera derecha":"trasera izquierda");
                    }
                }
            }
            for(unsigned int i=0;i<m->mNumVertices;++i){
                auto p=m->mVertices[i],n=m->mNormals[i];
                aiVector3D uv=m->HasTextureCoords(0)?m->mTextureCoords[0][i]:aiVector3D();
                f.reposo.insert(f.reposo.end(),{p.x,p.y,p.z,uv.x,uv.y,-n.x,-n.y,-n.z});
            }
            std::vector<unsigned int> indices;
            for(unsigned int i=0;i<m->mNumFaces;++i)
                for(unsigned int j=0;j<m->mFaces[i].mNumIndices;++j)indices.push_back(m->mFaces[i].mIndices[j]);
            aiString nombre; escena->mMaterials[m->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE,0,&nombre);
            std::string textura=nombre.C_Str();auto separador=textura.find_last_of("/\\");
            if(separador!=std::string::npos)textura=textura.substr(separador+1);
            f.textura=CargarTextura(("Textures/"+textura).c_str());
            f.vertices=f.reposo;f.indices=GLsizei(indices.size());
            glGenVertexArrays(1,&f.vao);glBindVertexArray(f.vao);
            glGenBuffers(1,&f.ibo);glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,f.ibo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,indices.size()*sizeof(unsigned int),indices.data(),GL_STATIC_DRAW);
            glGenBuffers(1,&f.vbo);glBindBuffer(GL_ARRAY_BUFFER,f.vbo);
            glBufferData(GL_ARRAY_BUFFER,f.vertices.size()*sizeof(float),f.vertices.data(),GL_DYNAMIC_DRAW);
            for(int a=0;a<3;++a){
                glVertexAttribPointer(a,a==1?2:3,GL_FLOAT,GL_FALSE,8*sizeof(float),reinterpret_cast<void*>((a==0?0:a==1?3:5)*sizeof(float)));
                glEnableVertexAttribArray(a);
            }
            glBindVertexArray(0);glBindBuffer(GL_ARRAY_BUFFER,0);
            cuerpo.push_back(std::move(f));
        }
    }

    void DibujarCuerpo(const glm::mat4& raiz)
    {
        glUniformMatrix4fv(U.model,1,GL_FALSE,glm::value_ptr(raiz));
        glUniform3f(U.color,1,1,1);glUniform1f(U.emision,.12f);
        for(auto& m:cuerpo){
            for(size_t i=0;i<m.reposo.size();i+=8){
                glm::vec3 p(m.reposo[i],m.reposo[i+1],m.reposo[i+2]);
                int enlace=m.articulacion[i/8];
                glm::mat4 matriz=huesos[enlace];
                // Ambos lados del corte usan los mismos pesos y matrices.
                // Asi los vertices coincidentes siguen coincidiendo al doblar.
                for(int unionCuerpo=1;enlace<=CUERPO5&&unionCuerpo<=CUERPO5;++unionCuerpo){
                    float dz=p.z-articulaciones[unionCuerpo].pivote.z;
                    if(std::abs(dz)<.14f){
                        float peso=glm::smoothstep(-.14f,.14f,dz);
                        matriz=huesos[unionCuerpo]*(1-peso)+huesos[unionCuerpo-1]*peso;
                        break;
                    }
                }
                glm::vec3 v=glm::vec3(matriz*glm::vec4(p,1));
                glm::vec3 n=glm::normalize(glm::mat3(matriz)*glm::vec3(m.reposo[i+5],m.reposo[i+6],m.reposo[i+7]));
                for(int a=0;a<3;++a){m.vertices[i+a]=v[a];m.vertices[i+5+a]=n[a];}
            }
            m.textura->UseTexture();glBindVertexArray(m.vao);glBindBuffer(GL_ARRAY_BUFFER,m.vbo);
            glBufferSubData(GL_ARRAY_BUFFER,0,m.vertices.size()*sizeof(float),m.vertices.data());
            glDrawElements(GL_TRIANGLES,m.indices,GL_UNSIGNED_INT,nullptr);
        }
        glBindVertexArray(0);glBindBuffer(GL_ARRAY_BUFFER,0);
    }

    // Onda viajera: el desfase aumenta hacia la cola, no oscilan todos a la vez.
    glm::vec3 Giro(int indice,float fase,float recogida)
    {
        if(indice<=CUERPO5){
            float amplitud=10.0f-3.0f*recogida;
            float actual=amplitud*std::sin(fase-indice*.85f);
            float anterior=indice==0?0:amplitud*std::sin(fase-(indice-1)*.85f);
            // Diferencia de angulos: evita acumular giros y enrollar el cuerpo.
            return {actual-anterior,indice==0?0:2.0f*std::sin(fase*.7f-indice*.8f),0};
        }
        if(indice==CABEZA)return {-5.0f*std::sin(fase),0,0};
        if(indice==COLA)return {5.0f*std::sin(fase-4.8f),3.0f*std::sin(fase-4.4f),0};
        if(indice>=PATA_DEL_D){
            float desfase=(indice>=PATA_TRAS_D?2.1f:0)+(indice%2?PI:0);
            return {9.0f+24.0f*recogida+7.0f*(1-.5f*recogida)*std::sin(fase+desfase),0,indice%2?4.0f:-4.0f};
        }
        return glm::vec3(0);
    }

    void CalcularParte(int indice, const glm::mat4& padre, glm::vec3 pivotePadre,float fase,float recogida)
    {
        const auto& parte=articulaciones[indice];
        glm::mat4 articulacion=glm::translate(padre,parte.pivote-pivotePadre);
        glm::vec3 giro=glm::radians(Giro(indice,fase,recogida));
        articulacion=glm::rotate(articulacion,giro.x,glm::vec3(1,0,0));
        articulacion=glm::rotate(articulacion,giro.y,glm::vec3(0,1,0));
        articulacion=glm::rotate(articulacion,giro.z,glm::vec3(0,0,1));
        huesos[indice]=glm::translate(articulacion,-parte.pivote);
        for(int hijo=0;hijo<TOTAL;++hijo)
            if(articulaciones[hijo].padre==indice)
                CalcularParte(hijo,articulacion,parte.pivote,fase,recogida);
    }

    struct Acrobacia { float giro=0,subida=0,recogida=0; };
    Acrobacia EvaluarAcrobacia(float tiempo)
    {
        // Un tonel de siete segundos cada 32: preparar, girar y recuperar.
        float reloj=std::fmod(std::max(0.0f,tiempo),32.0f);
        Acrobacia a;
        if(reloj<18.0f||reloj>25.0f)return a;
        float u=(reloj-18.0f)/7.0f;
        float suave=u*u*u*(u*(u*6.0f-15.0f)+10.0f);
        float arco=std::sin(PI*u);
        a.giro=2*PI*suave;
        a.subida=1.6f*arco*arco;
        a.recogida=arco*arco;
        return a;
    }

    glm::vec3 Recorrido(float t)
    {
        // Circuito cerrado sobre el lago y el jardin, lejos de la casa y el borde.
        float a=t*.055f;
        return {12.0f+26.0f*std::sin(a),7.4f+.45f*std::sin(2*a)+.18f*std::sin(t*1.45f)+EvaluarAcrobacia(t).subida,26.0f-31.0f*std::cos(a)};
    }
}

void CargarArcoirisOoo()
{
    for(int i=0;i<TOTAL;++i){
        std::string ruta=std::string("Models/Arcoiris/")+articulaciones[i].archivo+".obj";
        if(i<=CUERPO5)CargarCuerpo(i,ruta);else piezas[i].LoadModel(ruta);
    }
}

void DibujarArcoirisOoo(float tiempo)
{
    UsarMaterial(mate);
    Acrobacia acrobacia=EvaluarAcrobacia(tiempo);
    glm::vec3 posicion=Recorrido(tiempo);
    glm::vec3 frente=glm::normalize(Recorrido(tiempo+.3f)-Recorrido(tiempo-.3f));
    float rumbo=std::atan2(frente.x,frente.z);
    float elevacion=-std::atan2(frente.y,glm::length(glm::vec2(frente.x,frente.z)));
    glm::mat4 raiz=glm::translate(glm::mat4(1),CENTRO_OOO+posicion);
    raiz=glm::rotate(raiz,rumbo,glm::vec3(0,1,0));
    raiz=glm::rotate(raiz,elevacion,glm::vec3(1,0,0));
    raiz=glm::rotate(raiz,acrobacia.giro+glm::radians(5.0f+1.5f*std::sin(tiempo*.11f)),glm::vec3(0,0,1));
    raiz=glm::scale(raiz,glm::vec3(4.0f));
    CalcularParte(CUERPO1,glm::mat4(1),glm::vec3(0),tiempo*1.45f,acrobacia.recogida);
    DibujarCuerpo(raiz);
    for(int i=CABEZA;i<TOTAL;++i)DibujarModelo(piezas[i],raiz*huesos[i],.12f);
}
