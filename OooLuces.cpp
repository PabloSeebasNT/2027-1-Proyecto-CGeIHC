#include "OooLuces.h"
#include "OooCasaArbol.h"
#include "PlanetaOoo.h"
#include "Geometria.h"
#include "CommonValues.h"
#include <gtc/matrix_transform.hpp>
#include <cmath>

namespace
{
    Texture* blanca = nullptr;
    const glm::vec3 CALIDO(1.0f, .64f, .25f);
    const glm::vec3 BICHO(.78f, 1.0f, .28f);
    // Centros junto a arboles ya presentes en OooPaisaje, separados del camino.
    const glm::vec2 grupos[] = {
        {-60,17}, {-43,43}, {47,36}, {-11,-68}, {25,-68}, {63,-11}, {-76,65}
    };
    const int POR_GRUPO = 8;

    float Encendido(float hora)
    {
        float sol = std::sin(hora*6.2831853f);
        float t = glm::clamp((sol+.10f)/.28f, 0.0f, 1.0f);
        return 1.0f-t*t*(3.0f-2.0f*t);
    }

    glm::vec3 Farol(int i)
    {
        if (i == 0) return PuntoCasaArbolOoo(glm::vec3(-7.55f,1.65f,3.10f));
        if (i == 1) return PuntoCasaArbolOoo(glm::vec3(-5.35f,1.65f,3.62f));
        return CENTRO_OOO+glm::vec3(-19,3.6f,19);
    }

    glm::vec3 PosicionBicho(int grupo, int i, float tiempo)
    {
        float fase = i*2.39996f + grupo*.83f;
        float radio = 2.1f + .43f*(i%4);
        float a = fase + tiempo*(.12f+.012f*(i%3));
        return CENTRO_OOO + glm::vec3(grupos[grupo].x + radio*std::cos(a),
            1.2f + .43f*(i%5) + .35f*std::sin(tiempo*.7f+fase),
            grupos[grupo].y + radio*std::sin(a) + .35f*std::sin(tiempo*.4f+fase));
    }

    float Pulso(int grupo, int i, float tiempo)
    {
        float t = .5f+.5f*std::sin(tiempo*(1.1f+.06f*(i%3))+i*1.7f+grupo);
        return .25f+.75f*t*t;
    }

    glm::mat4 Colocar(glm::vec3 p, glm::vec3 escala)
    {
        return glm::scale(glm::translate(glm::mat4(1), p), escala);
    }
}

void CargarLucesOoo()
{
    blanca = CargarTextura("Textures/plain.png");
}

void LucesOoo(float tiempo, float hora, PointLight* luces, unsigned int& cantidad)
{
    float noche = Encendido(hora);
    if (noche < .001f) return;
    // Igual que en practica 9: color, ambiente, difusa, posicion y atenuacion.
    for (int i=0; i<3 && cantidad<MAX_POINT_LIGHTS; ++i)
    {
        glm::vec3 p = Farol(i);
        luces[cantidad++] = PointLight(CALIDO.r,CALIDO.g,CALIDO.b, .045f*noche, 3.5f*noche,
            p.x,p.y,p.z, 1.0f,.09f,.025f);
    }
    // Ocho bichitos comparten una luz suave: siete grupos + tres faroles = diez luces.
    for (int g=0; g<7 && cantidad<MAX_POINT_LIGHTS; ++g)
    {
        glm::vec3 p(0); float brillo=0;
        for (int i=0; i<POR_GRUPO; ++i)
        {
            p += PosicionBicho(g,i,tiempo);
            brillo += Pulso(g,i,tiempo);
        }
        p /= float(POR_GRUPO); brillo /= POR_GRUPO;
        luces[cantidad++] = PointLight(BICHO.r,BICHO.g,BICHO.b, .025f*noche, 1.8f*brillo*noche,
            p.x,p.y,p.z, 1.0f,.15f,.065f);
    }
}

void DibujarFarolesOoo(float hora)
{
    blanca->UseTexture(); UsarMaterial(matOpaco);
    float noche = Encendido(hora);
    const glm::vec3 hierro(.16f,.13f,.09f);
    for (int i=0; i<3; ++i)
    {
        glm::vec3 p=Farol(i);
        if (i==2)
            Dibujar(mCilindro,Colocar(p+glm::vec3(0,-1.8f,0),glm::vec3(.13f,3.6f,.13f)),hierro);
        else
        {
            // Brazo corto desde la pared hasta el farol, con el giro de la casa.
            glm::vec3 pared=PuntoCasaArbolOoo(glm::vec3(i==0?-7.55f:-5.35f,1.65f,i==0?2.70f:3.22f));
            glm::mat4 brazo=glm::translate(glm::mat4(1),(pared+p)*.5f+glm::vec3(0,.55f,0));
            brazo=glm::rotate(brazo,glm::radians(42.0f),glm::vec3(0,1,0));
            Dibujar(mCubo,glm::scale(brazo,glm::vec3(.12f,.12f,glm::length(p-pared))),hierro);
        }
        Dibujar(mCubo,Colocar(p,glm::vec3(.46f,.65f,.46f)),CALIDO,.12f+1.8f*noche);
        for (float y : {-.42f,.42f})
            Dibujar(mCubo,Colocar(p+glm::vec3(0,y,0),glm::vec3(.76f,.12f,.76f)),hierro);
        for (float x : {-.29f,.29f}) for (float z : {-.29f,.29f})
            Dibujar(mCubo,Colocar(p+glm::vec3(x,0,z),glm::vec3(.07f,.80f,.07f)),hierro);
        Dibujar(mCono,Colocar(p+glm::vec3(0,.48f,0),glm::vec3(.50f,.30f,.50f)),hierro);
    }
}

void DibujarLuciernagasOoo(float tiempo, float hora)
{
    float noche=Encendido(hora);
    if (noche<.001f) return;
    blanca->UseTexture(); UsarMaterial(matOpaco);
    // Al final de la escena: halos aditivos, con profundidad pero sin escribirla.
    GLboolean mezcla=glIsEnabled(GL_BLEND), profundidad;
    GLint srcRGB,dstRGB,srcA,dstA;
    glGetBooleanv(GL_DEPTH_WRITEMASK,&profundidad);
    glGetIntegerv(GL_BLEND_SRC_RGB,&srcRGB); glGetIntegerv(GL_BLEND_DST_RGB,&dstRGB);
    glGetIntegerv(GL_BLEND_SRC_ALPHA,&srcA); glGetIntegerv(GL_BLEND_DST_ALPHA,&dstA);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE); glDepthMask(GL_FALSE);
    for(int g=0;g<7;++g) for(int i=0;i<POR_GRUPO;++i)
    {
        glm::vec3 p=PosicionBicho(g,i,tiempo);
        float luz=noche*Pulso(g,i,tiempo), radio=.10f+.025f*(i%3);
        glUniform1f(U.transparencia,1.0f-luz);
        Dibujar(mEsferaBaja,Colocar(p,glm::vec3(radio)),BICHO,2.0f);
        glUniform1f(U.transparencia,1.0f-.09f*luz);
        Dibujar(mEsferaBaja,Colocar(p,glm::vec3(radio*3.5f)),BICHO,1.0f);
    }
    glUniform1f(U.transparencia,0); glUniform1f(U.emision,0);
    glDepthMask(profundidad); glBlendFuncSeparate(srcRGB,dstRGB,srcA,dstA);
    if(!mezcla) glDisable(GL_BLEND);
}
