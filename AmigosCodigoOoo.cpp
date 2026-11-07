#include "AmigosCodigoOoo.h"
#include "Geometria.h"
#include <gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

namespace
{
    Texture* blanca;
    MeshModel *caja,*sonrisa,*triangulo;
    const glm::vec3 ORO(.98f,.64f,.06f),VERDE(.12f,.60f,.46f),NEGRO(.018f,.026f,.027f);
    Material mate(.15f,16);

    void Ovo(const glm::mat4& padre,glm::vec3 pos,glm::vec3 escala,glm::vec3 color,float emision=.035f)
    {
        Dibujar(mEsfera,glm::scale(glm::translate(padre,pos),escala),color,emision);
    }
    void Caja(const glm::mat4& padre,glm::vec3 pos,glm::vec3 escala,glm::vec3 color)
    {
        Dibujar(caja,glm::scale(glm::translate(padre,pos),escala),color,.035f);
    }
    void Hueso(const glm::mat4& padre,float largo,float radio,glm::vec3 color)
    {
        Dibujar(mCilindro,glm::scale(glm::translate(padre,{0,-largo*.5f,0}),{radio,largo,radio}),color,.035f);
        Ovo(padre,{0,0,0},glm::vec3(radio),color);
        Ovo(padre,{0,-largo,0},glm::vec3(radio),color);
    }

    MeshModel* CajaRedondeada()
    {
        std::vector<GLfloat> v;std::vector<unsigned int> indices;
        const glm::vec3 caras[]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
        for(const glm::vec3& normal:caras)
        {
            glm::vec3 a=std::abs(normal.y)>.5f?glm::vec3(1,0,0):glm::vec3(0,1,0);
            glm::vec3 b=glm::cross(normal,a);unsigned int inicio=unsigned(v.size()/8);
            for(int y=0;y<=12;++y)for(int x=0;x<=12;++x)
            {
                glm::vec3 p=normal*.5f+a*(x/12.0f-.5f)+b*(y/12.0f-.5f);
                glm::vec3 centro=glm::clamp(p,glm::vec3(-.40f),glm::vec3(.40f));
                glm::vec3 n=glm::normalize(p-centro);p=centro+n*.10f;
                AgregarVertice(v,p,x/12.0f,y/12.0f,n);
                if(x<12&&y<12){unsigned int k=inicio+y*13+x;indices.insert(indices.end(),{k,k+1,k+13,k+1,k+14,k+13});}
            }
        }
        return GuardarMalla(v,indices);
    }

    MeshModel* CrearSonrisa()
    {
        std::vector<GLfloat> v;std::vector<unsigned int> idx;
        for(int i=0;i<=20;++i)
        {
            float t=i/20.0f;glm::vec3 p(-.13f+.26f*t,-.055f*std::sin(t*3.14159265f),0);
            for(int j=0;j<=8;++j)
            {
                float a=j*6.2831853f/8;glm::vec3 n(0,std::cos(a),std::sin(a));
                AgregarVertice(v,p+n*.012f,0,0,n);
                if(i<20&&j<8){unsigned int k=i*9+j;idx.insert(idx.end(),{k,k+1,k+9,k+1,k+10,k+9});}
            }
        }
        return GuardarMalla(v,idx);
    }

    struct Pierna { glm::mat4 cadera,rodilla,pie; };
    Pierna PosePierna(bool jake,int lado,float fase,float actividad,const glm::mat4& torso)
    {
        float paso=std::sin(fase+(lado?3.14159265f:0))*actividad;
        float alto=jake?.78f:.55f,ancho=jake?.28f:.22f;
        float muslo=jake?.32f:.22f,pantorrilla=jake?.30f:.22f;
        Pierna p;p.cadera=glm::translate(torso,{lado?ancho:-ancho,alto,0});
        p.cadera=glm::rotate(p.cadera,glm::radians(25.0f*paso),{1,0,0});
        p.rodilla=glm::translate(p.cadera,{0,-muslo,0});
        p.rodilla=glm::rotate(p.rodilla,glm::radians(-15.0f*std::max(paso,0.0f)),{1,0,0});
        p.pie=glm::translate(p.rodilla,{0,-pantorrilla,.07f});
        return p;
    }

    void Brazos(const glm::mat4& torso,bool jake,float fase,float actividad)
    {
        glm::vec3 color=jake?ORO:VERDE;float superior=jake?.36f:.24f,inferior=jake?.32f:.22f;
        for(int i=0;i<2;++i)
        {
            float signo=i?1.0f:-1.0f,paso=std::sin(fase+(i?3.14159265f:0))*actividad;
            glm::mat4 hombro=glm::translate(torso,{signo*(jake?.61f:.45f),jake?1.62f:1.24f,0});
            hombro=glm::rotate(hombro,glm::radians(signo*12.0f),{0,0,1});
            hombro=glm::rotate(hombro,glm::radians(-18.0f*paso),{1,0,0});
            Hueso(hombro,superior,jake?.065f:.038f,color);
            glm::mat4 codo=glm::translate(hombro,{0,-superior,0});
            codo=glm::rotate(codo,glm::radians(-8.0f-8.0f*std::max(-paso,0.0f)),{1,0,0});
            Hueso(codo,inferior,jake?.058f:.034f,color);
            Ovo(codo,{0,-inferior,0},jake?glm::vec3(.085f,.10f,.065f):glm::vec3(.048f,.055f,.043f),color);
            if(jake)for(int j=0;j<3;++j)Ovo(codo,{(j-1)*.047f,-inferior-.06f,.025f},{.025f,.065f,.025f},color);
        }
    }
}

void CargarAmigosCodigoOoo()
{
    blanca=CargarTextura("Textures/plain.png");caja=CajaRedondeada();sonrisa=CrearSonrisa();
    std::vector<GLfloat> v;std::vector<unsigned int> idx={0,1,2};
    AgregarVertice(v,{-.065f,-.055f,0},0,0,{0,0,1});AgregarVertice(v,{.065f,-.055f,0},0,0,{0,0,1});AgregarVertice(v,{0,.075f,0},0,0,{0,0,1});
    triangulo=GuardarMalla(v,idx);
}

void DibujarAmigoCodigoOoo(bool jake,const glm::mat4& raiz,float fase,float actividad,float mirada,float tiempo)
{
    blanca->UseTexture();UsarMaterial(mate);
    glm::vec3 color=jake?ORO:VERDE;
    float cadera=jake?.78f:.55f;
    glm::mat4 torso=glm::translate(glm::mat4(1),{0,cadera,0});
    torso=glm::rotate(torso,glm::radians(1.2f*std::sin(fase)*actividad),{0,0,1});
    torso=glm::rotate(torso,glm::radians(mirada),{0,1,0});
    torso=glm::translate(torso,{0,-cadera,0});
    Pierna piernas[2]={PosePierna(jake,0,fase,actividad,torso),PosePierna(jake,1,fase,actividad,torso)};
    glm::vec3 tamPie=jake?glm::vec3(.13f,.10f,.18f):glm::vec3(.062f,.065f,.12f);
    float minimo=1000;
    for(const Pierna& p:piernas)
    {
        // Minimo exacto de una esfera escalada y rotada: mantiene el pie sobre el piso.
        float extension=glm::length(glm::vec3(p.pie[0][1]*tamPie.x,p.pie[1][1]*tamPie.y,p.pie[2][1]*tamPie.z));
        minimo=std::min(minimo,p.pie[3][1]-extension);
    }
    glm::mat4 base=glm::translate(raiz,{0,-minimo-.003f,0});
    for(const Pierna& p:piernas)
    {
        Hueso(base*p.cadera,jake?.32f:.22f,jake?.075f:.042f,color);
        Hueso(base*p.rodilla,jake?.30f:.22f,jake?.068f:.039f,color);
        Ovo(base*p.pie,{0,0,0},tamPie,color);
    }
    glm::mat4 padre=base*torso;
    Brazos(padre,jake,fase,actividad);
    float t=std::fmod(tiempo+(jake?0:1.7f),5.7f),ojo=1;
    if(t>5.44f)ojo=std::max(.12f,std::abs(t-5.57f)/.13f);
    if(jake)
    {
        Ovo(padre,{0,1.53f,0},{.67f,.94f,.45f},ORO,.055f);
        for(float s:{-1.0f,1.0f})
        {
            glm::mat4 oreja=glm::translate(padre,{s*.585f,1.94f,.02f});
            oreja=glm::rotate(oreja,glm::radians(s*25.0f),{0,0,1});
            Ovo(oreja,{0,0,0},{.075f,.20f,.10f},ORO);
            Ovo(padre,{s*.25f,2.01f,.405f},{.225f,.26f*ojo,.08f},NEGRO);
            Ovo(padre,{s*.25f,2.01f,.469f},{.19f,.222f*ojo,.033f},{.98f,.98f,.93f},.07f);
        }
        Ovo(padre,{0,1.64f,.485f},{.16f,.12f,.055f},NEGRO);
        Ovo(padre,{0,1.595f,.535f},{.075f,.034f,.015f},{.80f,.28f,.35f});
        Ovo(padre,{0,1.83f,.49f},{.29f,.19f,.15f},ORO);
        for(float x:{-.19f,.19f})Ovo(padre,{x,1.69f,.56f},{.11f,.22f,.10f},ORO);
        Ovo(padre,{0,1.86f,.642f},{.13f,.075f,.06f},NEGRO);
        Ovo(padre,{-.035f,1.88f,.698f},{.037f,.014f,.007f},{.40f,.41f,.36f});
        // Cola corta, unida al cuerpo por detras.
        glm::mat4 cola=glm::translate(padre,{0,1.03f,-.39f});
        cola=glm::rotate(cola,glm::radians(22*std::sin(tiempo*1.4f)),{0,0,1});
        Ovo(cola,{0,.09f,-.11f},{.085f,.19f,.17f},ORO);
    }
    else
    {
        Caja(padre,{0,1.10f,0},{.92f,1.10f,.50f},VERDE);
        Caja(padre,{0,1.31f,.256f},{.79f,.56f,.04f},{.055f,.24f,.18f});
        Caja(padre,{0,1.31f,.281f},{.72f,.49f,.025f},{.62f,.88f,.71f});
        for(float x:{-.145f,.145f})Ovo(padre,{x,1.35f,.302f},{.022f,.032f*ojo,.01f},NEGRO);
        Dibujar(sonrisa,glm::translate(padre,{0,1.25f,.306f}),NEGRO);
        Caja(padre,{-.14f,1.005f,.257f},{.34f,.055f,.02f},NEGRO);
        Caja(padre,{-.23f,.81f,.267f},{.055f,.18f,.035f},{.96f,.75f,.04f});
        Caja(padre,{-.23f,.81f,.268f},{.18f,.055f,.035f},{.96f,.75f,.04f});
        Ovo(padre,{.25f,.76f,.272f},{.078f,.078f,.028f},{.87f,.04f,.06f});
        Ovo(padre,{.29f,.94f,.272f},{.036f,.036f,.022f},{.07f,.18f,.76f});
        Dibujar(triangulo,glm::translate(padre,{.09f,.86f,.28f}),{.04f,.56f,.77f},.04f);
        for(float x:{-.23f,-.10f})Caja(padre,{x,.64f,.257f},{.09f,.025f,.02f},NEGRO);
        for(float lado:{-1.0f,1.0f})for(int fila=0;fila<3;++fila)for(int col=0;col<3;++col)
            Ovo(padre,{lado*.461f,1.45f-fila*.08f,-.12f+col*.09f},{.007f,.017f,.017f},NEGRO);
    }
}
