#include "EstacionEspacial.h"
#include "Geometria.h"
#include "NaveVerde.h"
#include "CommonValues.h"
#include <gtc/matrix_transform.hpp>
#include <cmath>

namespace {
    glm::vec3 centro;
    Texture *metal, *lisa;
    float Suave(float u) {
        u=glm::clamp(u,0.0f,1.0f);
        return u*u*u*(u*(6*u-15)+10);
    }
    void Caja(glm::vec3 p, glm::vec3 tam, glm::vec3 color, float emision=0) {
        Dibujar(mCubo,glm::scale(glm::translate(glm::mat4(1),centro+p),tam),color,emision);
    }
    void Cilindro(glm::vec3 p, glm::vec3 tam, glm::vec3 color) {
        Dibujar(mCilindro,glm::scale(glm::translate(glm::mat4(1),centro+p),tam),color);
    }
    void NaveHangar(float tiempo, int numero) {
        // Dos turnos separados 25 segundos: mientras una llega, la otra sale.
        float t=std::fmod(tiempo+25.0f*numero,50.0f);
        if(t<0)t+=50;
        float x=20, y=2.58f, giro=-1.57079633f, tren=1, potencia=0;
        if(t<14) {
            float u=Suave(t/14);
            x=glm::mix(180.0f,20.0f,u);y=glm::mix(14.0f,6.0f,u);
            tren=Suave((t-10)/4);potencia=.75f*std::sin(3.14159265f*u);
        } else if(t<18) {
            y=glm::mix(6.0f,2.58f,Suave((t-14)/4));
        } else if(t<22) {
            // Descanso en la plataforma con los apoyos extendidos.
        } else if(t<26) {
            giro=glm::mix(-1.57079633f,1.57079633f,Suave((t-22)/4));
            y=2.58f+1.0f*Suave((t-22)/4);tren=1-Suave((t-22)/4);
        } else if(t<30) {
            giro=1.57079633f;tren=0;
            y=glm::mix(3.58f,6.0f,Suave((t-26)/4));potencia=.75f*Suave((t-26)/4);
        } else if(t<44) {
            float u=Suave((t-30)/14);
            giro=1.57079633f;tren=0;
            x=glm::mix(20.0f,180.0f,u);y=glm::mix(6.0f,14.0f,u);potencia=.75f;
        } else {
            // Espera exterior y giro para comenzar otra aproximacion, sin teletransportarse.
            x=180;y=14;tren=0;potencia=.75f*(1-Suave((t-44)/6));
            giro=glm::mix(1.57079633f,4.71238898f,Suave((t-44)/6));
        }
        auto p=centro+glm::vec3(x,y,numero==0?-11.0f:11.0f);
        auto frente=glm::vec3(std::sin(giro),0,std::cos(giro));
        DibujarNaveVerde(MatrizDesdeEjes(p,frente,{0,1,0},.9f),tren,potencia,tiempo,numero==1);
    }
}

void CargarEstacionEspacial(glm::vec3 origen) {
    centro=origen;metal=CargarTextura("Textures/metal.png");lisa=CargarTextura("Textures/plain.png");
}

void DibujarEstacionEspacial(float tiempo) {
    metal->UseTexture();UsarMaterial(matMetal);
    // Modulo central y dos collares estructurales.
    Cilindro({-8,3,0},{13,24,13},{.62f,.67f,.73f});
    for(float y:{-7.0f,13.0f})Cilindro({-8,y,0},{16,2,16},{.32f,.39f,.48f});
    Cilindro({-8,17,0},{9,6,9},{.40f,.49f,.58f});
    Cilindro({-8,23,0},{.3f,8,.3f},{.8f,.8f,.8f});
    // Plataforma de los hangares: cubierta parcial, acceso abierto hacia +X.
    Caja({31,-.4f,0},{54,2,46},{.37f,.43f,.51f});
    Caja({7,6,0},{2,13,44},{.48f,.54f,.62f});
    Caja({20,13,0},{28,1,46},{.45f,.51f,.60f});
    for(float z:{-22.0f,0.0f,22.0f}) {
        Caja({20,6,z},{27,12,1},{.43f,.49f,.57f});
        Caja({47,1,z},{21,.8f,.6f},{.63f,.67f,.72f});
    }
    // Paneles solares con bastidor y divisiones visibles.
    for(float lado:{-1.0f,1.0f}) {
        Caja({-12,0,lado*32},{2,2,42},{.6f,.65f,.7f});
        Caja({-12,0,lado*42},{28,.8f,24},{.45f,.50f,.6f});
        lisa->UseTexture();
        for(int x=0;x<6;++x)for(int z=0;z<5;++z)
            Caja({-23.5f+4.6f*x,.48f,lado*42-9.2f+4.6f*z},{4.25f,.12f,4.25f},{.10f,.22f,.43f});
        metal->UseTexture();
    }
    lisa->UseTexture();UsarMaterial(matOpaco);
    // Ventanas de observacion alrededor de la torre.
    for(int i=0;i<12;++i) {
        float a=i*6.2831853f/12;
        auto m=glm::translate(glm::mat4(1),centro+glm::vec3(-8+9.05f*std::cos(a),17,9.05f*std::sin(a)));
        m=glm::rotate(m,-a,{0,1,0});
        Dibujar(mCubo,glm::scale(m,{.15f,2.0f,2.4f}),{.12f,.60f,.80f},.35f);
    }
    for(float z:{-11.0f,11.0f}) {
        // Marcas de atraque y luces que guian hacia el hangar.
        glm::vec3 color=z<0?glm::vec3(.2f,.8f,.6f):glm::vec3(.68f,.32f,.95f);
        for(float lado:{-1.0f,1.0f}) {
            Caja({31,.63f,z+lado*7},{46,.06f,.25f},color);
            for(int i=0;i<7;++i)Caja({12+7.0f*i,.85f,z+lado*8},{1,.3f,.5f},color,.65f);
        }
        Caja({20,.64f,z},{.3f,.08f,8},{.9f,.9f,.8f});
        for(float lado:{-1.0f,1.0f})Caja({20,.64f,z+lado*4},{8,.08f,.3f},{.9f,.9f,.8f});
        Caja({33,11.6f,z},{.5f,.35f,17},color,.5f);
    }
    NaveHangar(tiempo,0);NaveHangar(tiempo,1);
}

void LucesEstacionEspacial(bool noche, PointLight* luces, unsigned int& cantidad) {
    if(!noche)return;
    for(float z:{-11.0f,11.0f})if(cantidad<MAX_POINT_LIGHTS) {
        auto p=centro+glm::vec3(27,10,z);
        luces[cantidad++]=PointLight(.65f,.78f,1,.03f,3,p.x,p.y,p.z,1,.025f,.001f);
    }
}

