#include "MentaOoo.h"
#include "Geometria.h"
#include "PlanetaOoo.h"
#include "PersonajesOoo.h"
#include "OooPaisaje.h"
#include <algorithm>
#include <gtc/matrix_transform.hpp>
#include <cmath>

namespace
{
    const float PI=3.14159265f;

    const glm::vec3 AZUL(.12f,.24f,.42f),ROJO(.87f,.055f,.13f),BLANCO(.97f,.96f,.90f),TINTA(.025f,.035f,.045f);
    Texture* blanca;
    MeshModel *franjas,*traje,*brazoI,*brazoD,*pierna,*sonrisa,*mono,*camisa,*faldones;
    Material mate(.18f,18);

    void Ovalo(const glm::mat4& padre,glm::vec3 p,glm::vec3 escala,glm::vec3 color,float luz=.03f)
    {
        Dibujar(mEsfera,glm::scale(glm::translate(padre,p),escala),color,luz);
    }

    glm::vec3 Caramelo(float r,float a,float lado)
    {
        return {1.02f*r*std::cos(a),1.85f+1.10f*r*std::sin(a),
            lado*(.34f*std::sqrt(std::max(0.0f,1-r*r))+.004f)};
    }

    MeshModel* CrearFranjas()
    {
        std::vector<GLfloat> v;std::vector<unsigned int> idx;
        // Cada gajo se estrecha hacia el centro. Se proyecta sobre ambas caras del caramelo.
        for(float lado:{-1.0f,1.0f})for(int g=0;g<10;++g)
        {
            unsigned int inicio=unsigned(v.size()/8);
            float centro=.18f+2*PI*g/10;
            for(int i=0;i<=16;++i)for(int j=0;j<=12;++j)
            {
                float u=i/16.0f,w=j/12.0f;
                float r=.70f+.30f*u;
                float ancho=.018f+.135f*std::sin(u*PI*.5f);
                float a=centro+(w*2-1)*ancho+.13f*(1-u);
                glm::vec3 p=Caramelo(r,a,lado);
                glm::vec3 n=glm::normalize(glm::vec3(p.x/(1.02f*1.02f),(p.y-1.85f)/(1.10f*1.10f),p.z/(.34f*.34f)));
                AgregarVertice(v,p,w,u,n);
                if(i<16&&j<12){unsigned int k=inicio+i*13+j;idx.insert(idx.end(),{k,k+1,k+13,k+1,k+14,k+13});}
            }
        }
        return GuardarMalla(v,idx);
    }

    MeshModel* CrearTraje()
    {
        std::vector<GLfloat> v;std::vector<unsigned int> idx;
        for(int i=0;i<=20;++i)for(int j=0;j<=80;++j)
        {
            float y=-1+.64f*i/20,a=2*PI*j/80,r=std::sqrt(std::max(0.0f,1-y*y));
            glm::vec3 p(1.027f*r*std::cos(a),1.85f+1.107f*y,.349f*r*std::sin(a));
            glm::vec3 n=glm::normalize(glm::vec3(p.x/(1.027f*1.027f),y/1.107f,p.z/(.349f*.349f)));
            AgregarVertice(v,p,float(j)/80,float(i)/20,n);
            if(i<20&&j<80){unsigned int k=i*81+j;idx.insert(idx.end(),{k,k+1,k+81,k+1,k+82,k+81});}
        }
        return GuardarMalla(v,idx);
    }

    MeshModel* Tubo(glm::vec3 a,glm::vec3 b,glm::vec3 c,float radio)
    {
        std::vector<GLfloat> v;std::vector<unsigned int> idx;
        for(int i=0;i<=24;++i)
        {
            float t=i/24.0f;glm::vec3 p=(1-t)*(1-t)*a+2*t*(1-t)*b+t*t*c;
            glm::vec3 eje=glm::normalize((1-t)*(b-a)+t*(c-b));
            glm::vec3 lateral=glm::normalize(glm::cross(eje,glm::vec3(0,0,1))),otro=glm::cross(eje,lateral);
            for(int j=0;j<=10;++j)
            {
                float ang=2*PI*j/10;glm::vec3 n=std::cos(ang)*lateral+std::sin(ang)*otro;
                AgregarVertice(v,p+radio*n,0,0,n);
                if(i<24&&j<10){unsigned int k=i*11+j;idx.insert(idx.end(),{k,k+1,k+11,k+1,k+12,k+11});}
            }
        }
        return GuardarMalla(v,idx);
    }

    MeshModel* Triangulos(const std::vector<glm::vec3>& puntos)
    {
        std::vector<GLfloat> v;std::vector<unsigned int> idx;
        for(size_t i=0;i<puntos.size();i+=3)
        {
            glm::vec3 n=glm::normalize(glm::cross(puntos[i+1]-puntos[i],puntos[i+2]-puntos[i]));
            for(int j=0;j<3;++j){idx.push_back(unsigned(v.size()/8));AgregarVertice(v,puntos[i+j],0,0,n);}
        }
        return GuardarMalla(v,idx);
    }
}

void CargarMentaOoo()
{
    blanca=CargarTextura("Textures/plain.png");franjas=CrearFranjas();traje=CrearTraje();
    brazoI=Tubo({-.92f,1.35f,0},{-1.28f,1.15f,.04f},{-1.17f,.60f,.12f},.052f);
    brazoD=Tubo({.92f,1.35f,0},{1.30f,1.12f,.04f},{1.13f,.62f,.14f},.052f);
    pierna=Tubo({0,.88f,0},{-.08f,.42f,.01f},{.015f,.13f,.09f},.062f);
    sonrisa=Tubo({-.115f,1.72f,.342f},{0,1.60f,.354f},{.12f,1.73f,.342f},.012f);
    mono=Triangulos({{-.18f,1.49f,.354f},{-.17f,1.31f,.354f},{-.025f,1.40f,.385f},
        {.025f,1.40f,.385f},{.17f,1.31f,.354f},{.18f,1.49f,.354f}});
    camisa=Triangulos({{-.105f,1.36f,.357f},{0,1.08f,.277f},{.105f,1.36f,.357f}});
    faldones=Triangulos({{-.25f,1.1f,-.27f},{-.26f,.59f,-.27f},{-.02f,.77f,-.29f},
        {.02f,.77f,-.29f},{.26f,.59f,-.27f},{.25f,1.1f,-.27f}});
}

void DibujarMentaOoo(float tiempo)
{
    blanca->UseTexture();UsarMaterial(mate);
    const auto& paseo=PaseoMentaOoo();
    glm::mat4 piernas[2];
    float apoyo=100.0f;
    for(int i=0;i<2;++i)
    {
        float lado=i==0?-1.0f:1.0f;
        float paso=std::sin(paseo.fase+(i==0?0:PI))*paseo.actividad;
        piernas[i]=glm::translate(glm::mat4(1),{lado*.23f,.88f,0});
        piernas[i]=glm::rotate(piernas[i],glm::radians(23.0f*paso),{1,0,0});
        piernas[i]=glm::translate(piernas[i],{0,-.88f,0});
        // El zapato sigue a su pierna; apoyar el mas bajo evita hundirlo.
        glm::vec4 pie=piernas[i]*glm::vec4(.02f,.065f,.16f,1);
        float a=piernas[i][1][1]*.065f,b=piernas[i][2][1]*.17f;
        apoyo=std::min(apoyo,pie.y-std::sqrt(a*a+b*b));
    }
    glm::mat4 raiz=glm::translate(glm::mat4(1),CENTRO_OOO+
        glm::vec3(paseo.posicion.x,AlturaPaseoOoo(paseo.posicion)-apoyo,paseo.posicion.y));
    raiz=glm::rotate(raiz,paseo.rumbo+glm::radians(paseo.mirada),glm::vec3(0,1,0));
    for(int i=0;i<2;++i)
    {
        glm::mat4 pata=raiz*piernas[i];Dibujar(pierna,pata,AZUL);
        Ovalo(pata,{.02f,.065f,.16f},{.095f,.065f,.17f},TINTA);
        Ovalo(pata,{.02f,.105f,.19f},{.075f,.055f,.14f},ROJO);
    }
    glm::mat4 cuerpo=glm::translate(raiz,{0,.006f*std::sin(tiempo*1.5f),0});
    Ovalo(cuerpo,{0,1.85f,0},{1.02f,1.10f,.34f},BLANCO,.08f);
    Dibujar(franjas,cuerpo,ROJO,.05f);Dibujar(traje,cuerpo,AZUL,.04f);Dibujar(faldones,cuerpo,AZUL);
    Dibujar(camisa,cuerpo,BLANCO,.08f);Dibujar(mono,cuerpo,ROJO,.06f);
    Ovalo(cuerpo,{0,1.40f,.38f},{.046f,.06f,.028f},ROJO);
    for(int i=0;i<3;++i)Ovalo(cuerpo,{0,1.30f-i*.065f,.36f-i*.02f},{.012f,.014f,.012f},TINTA);
    float fase=std::fmod(tiempo,6.1f),abierto=1;
    if(fase>5.84f)abierto=std::max(.13f,std::abs(fase-5.97f)/.13f);
    for(float x:{-.34f,.34f})
    {
        Ovalo(cuerpo,{x,1.86f,.327f},{.047f,.058f*abierto,.023f},TINTA);
        Ovalo(cuerpo,{x,1.86f,.349f},{.029f,.040f*abierto,.008f},BLANCO,.15f);
    }
    Dibujar(sonrisa,cuerpo,TINTA);
    for(int i=0;i<2;++i)
    {
        float signo=i==0?-1.0f:1.0f;glm::vec3 hombro(signo*.92f,1.35f,0);
        glm::mat4 brazo=glm::translate(cuerpo,hombro);
        float paso=std::sin(paseo.fase+(i==0?0:PI))*paseo.actividad;
        brazo=glm::rotate(brazo,glm::radians(-18.0f*paso),glm::vec3(1,0,0));
        // El saludo se reserva para las pausas del paseo.
        float t=std::fmod(tiempo,9.0f),gesto=t<2.0f?std::sin(PI*t/2.0f):0;
        brazo=glm::rotate(brazo,glm::radians((1-paseo.actividad)*(i==1?12.0f*gesto:2.0f*std::sin(tiempo*.8f))),glm::vec3(0,0,1));
        brazo=glm::translate(brazo,-hombro);Dibujar(i==0?brazoI:brazoD,brazo,AZUL);
        glm::vec3 mano=i==0?glm::vec3(-1.17f,.60f,.12f):glm::vec3(1.13f,.62f,.14f);
        Ovalo(brazo,mano,{.073f,.091f,.066f},ROJO);
        for(int j=0;j<3;++j)Ovalo(brazo,mano+glm::vec3((j-1)*.03f,-.035f,.052f),{.012f,.027f,.014f},BLANCO);
    }
}

glm::vec2 EvitarMentaOoo(glm::vec2 p,float radio)
{
    const glm::vec2 POS=PaseoMentaOoo().posicion;
    glm::vec2 d=p-POS;float dist=glm::length(d),limite=1.15f+radio;
    if(dist<limite)return POS+(dist>.0001f?d/dist:glm::vec2(0,1))*limite;
    return p;
}
