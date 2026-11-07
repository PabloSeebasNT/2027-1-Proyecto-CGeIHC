#include "GrumosaOoo.h"
#include "Geometria.h"
#include "PlanetaOoo.h"
#include "PersonajesOoo.h"
#include "OooPaisaje.h"
#include <gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

namespace
{
    const float PI=3.14159265f;

    const glm::vec3 MORADO(.74f,.43f,.82f);
    Texture* blanca;
    MeshModel *cuerpo,*estrella,*brazoIzq,*brazoDer;
    Material suave(.18f,20);

    // Circulos que definen el contorno. Se unen en una sola superficie redondeada.
    float Radio(float angulo)
    {
        const glm::vec3 lobulos[]={ {0,1.5f,1.02f},{0,2.55f,.70f},
            {-.60f,1.99f,.64f},{.65f,2.05f,.64f},{-.92f,1.22f,.62f},
            {.90f,1.23f,.66f},{-.72f,.60f,.45f},{.66f,.58f,.43f},
            {-.28f,.27f,.38f},{.24f,.29f,.40f} };
        glm::vec2 d(std::cos(angulo),std::sin(angulo));float radio=.8f;
        for(const glm::vec3& l:lobulos)
        {
            glm::vec2 c(l.x,l.y-1.6f);float proyeccion=glm::dot(c,d);
            float disc=proyeccion*proyeccion-glm::dot(c,c)+l.z*l.z;
            if(disc<0)continue;
            float candidato=proyeccion+std::sqrt(disc);
            // Maximo suavizado: elimina las costuras entre los bultos.
            float h=std::max(.09f-std::abs(radio-candidato),0.0f)/.09f;
            radio=std::max(radio,candidato)+h*h*.09f*.25f;
        }
        return radio;
    }

    glm::vec3 Superficie(float a,float p)
    {
        // Los bultos dominan el borde; el frente conserva una panza lisa, sin pliegues radiales.
        float seno=std::sin(p),c=std::cos(a),s=std::sin(a);
        float elipse=1.0f/std::sqrt(c*c/(1.3f*1.3f)+s*s/(1.6f*1.6f));
        float r=glm::mix(elipse,Radio(a),std::pow(seno,6.0f))*seno;
        return glm::vec3(r*std::cos(a),1.6f+r*std::sin(a),.64f*std::cos(p));
    }

    MeshModel* CrearNube()
    {
        std::vector<GLfloat> v;std::vector<unsigned int> idx;
        const int ANILLOS=36,LADOS=128;
        for(int j=0;j<=ANILLOS;++j)for(int i=0;i<=LADOS;++i)
        {
            float a=2*PI*i/LADOS,p=PI*j/ANILLOS;
            glm::vec3 punto=Superficie(a,p),n;
            if(j==0)n=glm::vec3(0,0,1);
            else if(j==ANILLOS)n=glm::vec3(0,0,-1);
            else n=glm::normalize(glm::cross(Superficie(a,p+.001f)-Superficie(a,p-.001f),
                Superficie(a+.001f,p)-Superficie(a-.001f,p)));
            AgregarVertice(v,punto,float(i)/LADOS,float(j)/ANILLOS,n);
            if(j<ANILLOS&&i<LADOS){unsigned int b=j*(LADOS+1)+i;
                idx.insert(idx.end(),{b,b+LADOS+1,b+1,b+1,b+LADOS+1,b+LADOS+2});}
        }
        return GuardarMalla(v,idx);
    }

    MeshModel* CrearEstrella()
    {
        std::vector<GLfloat> v;std::vector<unsigned int> idx;
        AgregarVertice(v,{0,0,.065f},.5f,.5f,{0,0,1});
        for(int i=0;i<10;++i)
        {
            float a=PI*.5f+i*PI/5,r=i%2?.11f:.25f;
            AgregarVertice(v,{r*std::cos(a),r*std::sin(a),0},0,0,{0,0,1});
        }
        for(unsigned int i=0;i<10;++i)idx.insert(idx.end(),{0,1+i,1+(i+1)%10});
        // Un borde con grosor; el dorso queda apoyado sobre la frente.
        for(int i=0;i<10;++i)
        {
            int siguiente=(i+1)%10;glm::vec3 a(v[(i+1)*8],v[(i+1)*8+1],0);
            glm::vec3 b(v[(siguiente+1)*8],v[(siguiente+1)*8+1],0);
            glm::vec3 n=glm::normalize(glm::cross(b-a,glm::vec3(0,0,-1)));
            unsigned int k=unsigned(v.size()/8);
            for(glm::vec3 p:{a,b,b+glm::vec3(0,0,-.055f),a+glm::vec3(0,0,-.055f)})AgregarVertice(v,p,0,0,n);
            idx.insert(idx.end(),{k,k+1,k+2,k,k+2,k+3});
        }
        return GuardarMalla(v,idx);
    }

    MeshModel* CrearBrazo(glm::vec3 a,glm::vec3 b,glm::vec3 c,glm::vec3 d)
    {
        std::vector<GLfloat> v;std::vector<unsigned int> idx;
        for(int i=0;i<=32;++i)
        {
            float t=i/32.0f,u=1-t;
            glm::vec3 p=u*u*u*a+3*u*u*t*b+3*u*t*t*c+t*t*t*d;
            glm::vec3 tangente=glm::normalize(3*u*u*(b-a)+6*u*t*(c-b)+3*t*t*(d-c));
            glm::vec3 lateral=glm::normalize(glm::cross(tangente,glm::vec3(0,0,1)));
            glm::vec3 otro=glm::normalize(glm::cross(tangente,lateral));
            float radio=.065f-.018f*t;
            for(int j=0;j<=12;++j)
            {
                float angulo=2*PI*j/12;glm::vec3 n=std::cos(angulo)*lateral+std::sin(angulo)*otro;
                AgregarVertice(v,p+radio*n,float(j)/12,t,n);
                if(i<32&&j<12){unsigned int k=i*13+j;idx.insert(idx.end(),{k,k+1,k+13,k+1,k+14,k+13});}
            }
        }
        return GuardarMalla(v,idx);
    }

    void Ovalo(const glm::mat4& padre,glm::vec3 pos,glm::vec3 tam,glm::vec3 color,float emision=0)
    {
        Dibujar(mEsfera,glm::scale(glm::translate(padre,pos),tam),color,emision);
    }

    void Mano(const glm::mat4& brazo,glm::vec3 centro,bool izquierda)
    {
        Ovalo(brazo,centro,{.105f,.06f,.07f},MORADO,.08f);
        for(int i=0;i<3;++i)
        {
            glm::vec3 p=centro+glm::vec3((izquierda?1:-1)*(.045f+i*.035f),-.025f-i*.022f,.035f);
            Ovalo(brazo,p,{.029f,.065f,.028f},MORADO,.08f);
        }
    }
}

void CargarGrumosaOoo()
{
    blanca=CargarTextura("Textures/plain.png");cuerpo=CrearNube();estrella=CrearEstrella();
    brazoIzq=CrearBrazo({-1,1.5f,0},{-1.40f,.3f,.25f},{-.85f,.35f,.78f},{-.30f,1.13f,.70f});
    brazoDer=CrearBrazo({1.03f,1.6f,0},{1.85f,1.5f,.20f},{1.92f,.55f,.45f},{1.14f,.72f,.65f});
}

void DibujarGrumosaOoo(float tiempo)
{
    blanca->UseTexture();UsarMaterial(suave);
    const auto& paseo=PaseoGrumosaOoo();
    glm::vec2 posicion=paseo.posicion;
    glm::mat4 raiz=glm::translate(glm::mat4(1),CENTRO_OOO+
        glm::vec3(posicion.x,AlturaPaseoOoo(posicion)+.49f+.10f*std::sin(tiempo*1.1f),posicion.y));
    raiz=glm::rotate(raiz,paseo.rumbo+glm::radians(paseo.mirada),glm::vec3(0,1,0));
    raiz=glm::rotate(raiz,glm::radians(1.6f*std::sin(tiempo*.73f)),glm::vec3(0,0,1));
    Dibujar(cuerpo,raiz,MORADO,.08f);
    // La cara y la estrella heredan todos los movimientos de la nube.
    glm::mat4 frente=glm::translate(raiz,{.13f,2.65f,.53f});
    frente=glm::rotate(frente,glm::radians(-25.0f),glm::vec3(1,0,0));
    Dibujar(estrella,frente,{1.0f,.79f,.23f},.22f);
    float ciclo=std::fmod(tiempo,5.4f),parpadeo=1;
    if(ciclo>5.12f)parpadeo=std::max(.12f,std::abs(ciclo-5.26f)/.14f);
    for(float x:{-.30f,.30f})
    {
        Ovalo(raiz,{x,2.02f,.64f},{.085f,.10f*parpadeo,.06f},{.055f,.035f,.07f});
        if(parpadeo>.5f)Ovalo(raiz,{x-.024f,2.05f,.694f},{.018f,.021f,.012f},{.95f,.92f,1.0f},.2f);
    }
    Ovalo(raiz,{0,1.65f,.653f},{.29f,.19f,.055f},{.20f,.055f,.26f});
    Ovalo(raiz,{.035f,1.565f,.697f},{.16f,.055f,.018f},{.72f,.28f,.67f},.08f);
    for(float x:{-.16f,0.0f,.16f})Ovalo(raiz,{x,1.76f,.697f},{.078f,.07f,.035f},{.97f,.94f,.91f},.1f);
    // Brazos como hijos con pivote en el hombro; el gesto no desplaza la union.
    glm::vec3 hombroI(-1,1.5f,0),hombroD(1.03f,1.6f,0);
    glm::mat4 izq=glm::translate(raiz,hombroI);
    izq=glm::rotate(izq,glm::radians(5*std::sin(tiempo*.8f)),glm::vec3(0,0,1));
    izq=glm::translate(izq,-hombroI);Dibujar(brazoIzq,izq,MORADO,.08f);Mano(izq,{-.30f,1.13f,.70f},true);
    glm::mat4 der=glm::translate(raiz,hombroD);
    der=glm::rotate(der,glm::radians(4*std::sin(tiempo*.6f+.7f)),glm::vec3(0,0,1));
    der=glm::translate(der,-hombroD);Dibujar(brazoDer,der,MORADO,.08f);Mano(der,{1.14f,.72f,.65f},false);
}

glm::vec2 EvitarGrumosaOoo(glm::vec2 p,float radio)
{
    const glm::vec2 POSICION=PaseoGrumosaOoo().posicion;
    glm::vec2 d=p-POSICION;float distancia=glm::length(d),limite=1.45f+radio;
    if(distancia<limite)return POSICION+(distancia>.0001f?d/distancia:glm::vec2(0,1))*limite;
    return p;
}
