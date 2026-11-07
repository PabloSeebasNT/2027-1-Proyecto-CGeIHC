#include "OooModelosNature.h"
#include "PlanetaOoo.h"
#include "Geometria.h"
#include <cmath>
#include <algorithm>
#include <gtc/matrix_transform.hpp>

namespace {
    constexpr float PI=3.14159265f;
    struct Arbol { float x, z, tamano; int forma; };
    const Arbol arboles[] = {
        {-88,  12, 1.15f, 0}, {-90,  43, 1.05f, 1}, {-76,  65, 1.25f, 0},
        {-53,  82, 1.10f, 0}, {-21,  91, 1.32f, 1}, { 18,  87, 1.08f, 0},
        { 56,  76, 1.18f, 0}, { 84,  47, 1.35f, 1}, { 94,  13, 1.12f, 0},
        { 88, -27, 1.28f, 0}, { 75, -69, 1.05f, 1}, { 49, -90, 1.13f, 0},
        { 15, -93, 1.22f, 1}, {-26, -92, 1.16f, 1}, {-67, -73, 1.27f, 0},
        {-88, -42, 1.06f, 1}, {-60,  17, 0.86f, 0}, {-43,  43, 0.95f, 0},
        { 47,  36, 0.93f, 1}, { 63, -11, 0.95f, 0}, {-11, -68, 1.03f, 0},
        { 25, -68, 0.97f, 1},
        // Grupos del bosque: claros entre ellos para las rutas y las vistas.
        {-95,-12,.86f,0},{-94,27,.78f,1},{-84,55,.92f,0},{-66,77,.85f,0},
        {-39,87,1.04f,1},{-6,94,.82f,0},{32,88,.96f,0},{69,68,.84f,1},
        {86,32,.78f,0},{94,-3,.91f,1},{91,-44,.83f,0},{63,-79,.94f,0},
        {34,-90,.81f,1},{-8,-88,1.02f,0},{-43,-84,.85f,0},{-77,-59,.76f,1},
        {75,12,.82f,0},{72,42,1.03f,1},{60,53,.76f,0},{-57,58,.83f,0}
    };

    struct Roca { float x, z, radio; };
    const Roca rocasGrandes[] = {
        {-72, 33, 2.1f}, {-69,-48,2.7f}, {73,27,2.0f}, {78,-43,2.3f},
        {-17,77,1.7f}, {14,-80,2.1f}, {65,64,1.8f},
        {-88,29,1.3f},{-65,70,1.1f},{-33,80,1.6f},{30,80,1.2f},
        {76,52,1.5f},{82,-18,1.2f},{59,-73,1.6f},{-43,-77,1.1f},{57,14,1.3f}
    };

    const glm::vec2 sitiosArbustos[] = {
        {-51,27},{-62,50},{-45,67},{-11,72},{28,65},{65,42},
        {75,-11},{70,-57},{31,-72},{-37,-66},{-72,-25},{-48,-9},
        {51,8},{-9,-42},{-1,50},{42,-55},
        {-91,17},{-87,32},{-81,51},{-69,73},{-62,76},{-48,78},
        {-31,87},{-16,83},{24,81},{35,83},{62,68},{70,56},
        {77,37},{82,16},{84,-9},{82,-37},{64,-74},{41,-81},
        {-2,-80},{-40,-79},{-68,-67},{-61,59}
    };

    MeshModel *copa[3], *roca[3], *tronco;
    Texture *hojas, *corteza, *piedra;

    glm::mat4 Colocar(float x,float z,float escala,float giro=0)
    {
        glm::mat4 m=glm::translate(glm::mat4(1),CENTRO_OOO+glm::vec3(x,.06f,z));
        m=glm::rotate(m,glm::radians(giro),glm::vec3(0,1,0));
        return glm::scale(m,glm::vec3(escala));
    }

    // Superficie cerrada con bultos amplios; no usa vertices de ningun OBJ.
    glm::vec3 PuntoForma(float lat,float lon,int variante,bool esRoca)
    {
        glm::vec3 d(std::sin(lat)*std::cos(lon),std::cos(lat),std::sin(lat)*std::sin(lon));
        float onda=std::sin(3.1f*d.x+variante)*std::cos(3.7f*d.z-.6f*variante);
        float r=1+.09f*onda+.05f*std::sin(5*d.y+2*d.x+variante);
        glm::vec3 p=d*r;
        if(esRoca){
            p.x+=.15f*p.y;p.z+=.08f*std::sin(3*p.y+variante);
            // Base plana apoyada en y=0; la irregularidad queda arriba.
            p.y=std::max(0.0f,(p.y+.60f)*.72f);
        }
        return p;
    }

    MeshModel* CrearForma(int variante,bool esRoca)
    {
        int pisos=esRoca?7:16,lados=esRoca?11:24;
        std::vector<GLfloat> v;std::vector<unsigned int> idx;
        auto agregar=[&](glm::vec3 p,glm::vec2 uv,glm::vec3 n){
            idx.push_back(unsigned(v.size()/8));AgregarVertice(v,p,uv.x,uv.y,n);
        };
        for(int i=0;i<pisos;++i)for(int j=0;j<lados;++j){
            float u=float(j)/lados,w=float(i)/pisos;
            glm::vec2 uv[]={{u,w},{u+1.0f/lados,w},{u,w+1.0f/pisos},{u+1.0f/lados,w+1.0f/pisos}};
            glm::vec3 p[4];for(int k=0;k<4;++k)p[k]=PuntoForma(PI*uv[k].y,2*PI*uv[k].x,variante,esRoca);
            const int caras[]={0,2,1,1,2,3};
            for(int k=0;k<6;k+=3){
                glm::vec3 n=glm::cross(p[caras[k+1]]-p[caras[k]],p[caras[k+2]]-p[caras[k]]);
                if(glm::length(n)<.00001f)continue;
                n=glm::normalize(n);
                glm::vec3 centro=(p[caras[k]]+p[caras[k+1]]+p[caras[k+2]])/3.0f;
                if(glm::dot(n,centro-glm::vec3(0,esRoca?.5f:0,0))<0)n=-n;
                for(int q=0;q<3;++q){int b=caras[k+q];glm::vec3 normal=n;
                    if(!esRoca){
                        float lat=PI*uv[b].y,lon=2*PI*uv[b].x;
                        glm::vec3 a=PuntoForma(lat+.002f,lon,variante,false)-PuntoForma(lat-.002f,lon,variante,false);
                        glm::vec3 c=PuntoForma(lat,lon+.002f,variante,false)-PuntoForma(lat,lon-.002f,variante,false);
                        normal=glm::cross(a,c);
                        normal=glm::length(normal)>.000001f?glm::normalize(normal):glm::normalize(p[b]);
                        if(glm::dot(normal,p[b])<0)normal=-normal;
                    }
                    agregar(p[b],uv[b],normal);
                }
            }
        }
        return GuardarMalla(v,idx);
    }

    MeshModel* CrearTroncoCurvo()
    {
        std::vector<GLfloat> v;std::vector<unsigned int> idx;
        const int anillos=9,lados=12;
        for(int i=0;i<=anillos;++i){float t=float(i)/anillos;
            glm::vec3 centro(.30f*t*t,5.7f*t,.18f*std::sin(PI*t));
            float radio=.46f*(1-.72f*t)+.17f*std::exp(-t*18);
            for(int j=0;j<=lados;++j){float a=2*PI*j/lados;
                glm::vec3 n(std::cos(a),.06f,std::sin(a));
                AgregarVertice(v,centro+glm::vec3(n.x*radio,0,n.z*radio),float(j)/lados,t*2,glm::normalize(n));
                if(i<anillos&&j<lados){unsigned k=i*(lados+1)+j;idx.insert(idx.end(),{k,k+1,k+lados+1,k+1,k+lados+2,k+lados+1});}
            }
        }
        return GuardarMalla(v,idx);
    }

    void Bulto(const glm::mat4& padre,glm::vec3 pos,glm::vec3 escala,int forma,glm::vec3 color)
    {
        Dibujar(copa[forma%3],glm::scale(glm::translate(padre,pos),escala),color,.035f);
    }

    void Rama(const glm::mat4& padre,glm::vec3 a,glm::vec3 b,float radio)
    {
        glm::vec3 d=b-a;float largo=glm::length(d);glm::vec3 eje=glm::normalize(d);
        glm::mat4 m=glm::translate(padre,(a+b)*.5f);
        glm::vec3 cruz=glm::cross(glm::vec3(0,1,0),eje);
        if(glm::length(cruz)>.001f)m=glm::rotate(m,std::acos(glm::clamp(eje.y,-1.0f,1.0f)),glm::normalize(cruz));
        Dibujar(mCilindro,glm::scale(m,{radio,largo,radio}),{.58f,.38f,.20f});
    }

    void ArbolCodigo(const Arbol& a,int i)
    {
        glm::mat4 base=Colocar(a.x,a.z,a.tamano,float((i*47)%360));
        corteza->UseTexture();Dibujar(tronco,base,{.66f,.44f,.25f});
        Rama(base,{.05f,3.1f,0},{-1.25f,5.0f,.30f},.19f);
        Rama(base,{.12f,3.8f,0},{1.35f,5.5f,-.35f},.16f);
        // Tres raices bajas: el tronco no termina como un cilindro sobre el piso.
        for(int r=0;r<3;++r){float ang=2*PI*r/3;Rama(base,{0,.30f,0},{.80f*std::cos(ang),.10f,.80f*std::sin(ang)},.14f);}
        hojas->UseTexture();
        glm::vec3 verde= i%5==0?glm::vec3(.48f,.72f,.27f):glm::vec3(.31f,.64f,.28f);
        if(a.forma==0){
            Bulto(base,{.15f,5.8f,0},{2.25f,2.25f,1.95f},i,verde);
            Bulto(base,{-1.45f,5.0f,.35f},{1.55f,1.55f,1.40f},i+1,verde*.92f);
            Bulto(base,{1.50f,5.55f,-.30f},{1.45f,1.70f,1.35f},i+2,verde*1.06f);
            Bulto(base,{.40f,7.0f,.12f},{1.35f,1.55f,1.30f},i+1,verde*1.10f);
        }else{
            // Arbol alto de copa colgante, inspirado en las siluetas de Ooo.
            Bulto(base,{.20f,5.7f,0},{1.75f,3.0f,1.65f},i,verde*.91f);
            Bulto(base,{-1.15f,4.5f,.22f},{.95f,1.9f,1.10f},i+1,verde*.96f);
            Bulto(base,{1.20f,5.0f,-.40f},{1.0f,2.2f,.95f},i+2,verde*1.04f);
        }
    }
}

void CargarModelosNatureOoo()
{
    // Nombre conservado para no romper la llamada del paisaje. No carga modelos.
    hojas=CargarTextura("Textures/OooEtapa1/hojas.png");
    corteza=CargarTextura("Textures/OooEtapa1/corteza.png");
    piedra=CargarTextura("Textures/OooEtapa1/piedra.png");
    tronco=CrearTroncoCurvo();
    for(int i=0;i<3;++i){copa[i]=CrearForma(i,false);roca[i]=CrearForma(i,true);}
}

void DibujarModelosNatureOoo()
{
    UsarMaterial(matOpaco);
    for(int i=0;i<int(sizeof(arboles)/sizeof(arboles[0]));++i)ArbolCodigo(arboles[i],i);
    hojas->UseTexture();
    for(int i=0;i<int(sizeof(sitiosArbustos)/sizeof(sitiosArbustos[0]));++i){
        const auto& p=sitiosArbustos[i];
        float tam=.90f+.12f*(i%4);glm::mat4 base=Colocar(p.x,p.y,tam,float((i*71)%360));
        glm::vec3 color(.37f,.69f,.30f);
        Bulto(base,{0,.65f,0},{1.2f,.85f,.95f},i,color);
        Bulto(base,{.85f,.45f,.10f},{.78f,.62f,.70f},i+1,color*1.08f);
        Bulto(base,{-.75f,.42f,-.15f},{.70f,.58f,.65f},i+2,color*.94f);
    }
    piedra->UseTexture();
    for(int i=0;i<int(sizeof(rocasGrandes)/sizeof(rocasGrandes[0]));++i){
        const auto& r=rocasGrandes[i];glm::mat4 base=Colocar(r.x,r.z,r.radio*.75f,float((i*63)%360));
        Dibujar(roca[i%3],base,{.60f,.66f,.58f});
        glm::mat4 pequena=glm::scale(glm::translate(base,{.85f,0,.35f}),{.38f,.45f,.48f});
        Dibujar(roca[(i+1)%3],pequena,{.66f,.69f,.61f});
    }
    for(int i=0;i<26;++i){
        float a=2*PI*(i+.32f)/26,r=105+9*std::sin(i*2.17f);
        Dibujar(roca[i%3],Colocar(r*std::cos(a),r*std::sin(a),.38f+.11f*(i%4),float(i*23)),{.62f,.66f,.58f});
    }
    for(int i=0;i<18;++i){
        float x=-52+6.2f*i,z=(i%2==0?52.0f:-54.0f)+3*std::sin(i*1.7f);
        auto m=glm::scale(Colocar(x,z,.23f+.03f*(i%4),float(i*53)),{1.0f,.50f,1.0f});
        Dibujar(roca[i%3],m,{.68f,.70f,.62f});
    }
}
