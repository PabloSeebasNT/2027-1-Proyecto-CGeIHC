#include "PersonajesOoo.h"
#include "PlanetaOoo.h"
#include "OooPaisaje.h"
#include "Geometria.h"
#include "AmigosCodigoOoo.h"
#include <gtc/matrix_transform.hpp>
#include <cmath>
#include <vector>
#include <algorithm>

namespace
{
    const float PI=3.14159265f;
    struct Ruta { std::vector<glm::vec2> puntos; std::vector<float> distancia; float largo=0; };
    using Paseo = PaseoNpcOoo;
    Ruta rutaJake,rutaBmo,rutaGrumosa,rutaMenta;
    Paseo paseoJake,paseoBmo,paseoGrumosa,paseoMenta;
    float reloj=0;

    Ruta CrearRuta(const std::vector<glm::vec2>& controles,float margen)
    {
        Ruta r; int n=int(controles.size());
        for(int k=0;k<n;++k) for(int j=0;j<100;++j)
        {
            glm::vec2 a=controles[(k+n-1)%n],b=controles[k],c=controles[(k+1)%n],d=controles[(k+2)%n];
            float t=j/100.0f;
            glm::vec2 p=.5f*(2.0f*b+(-a+c)*t+(2.0f*a-5.0f*b+4.0f*c-d)*t*t+(-a+3.0f*b-3.0f*c+d)*t*t*t);
            // Sin consultar las colisiones de los propios personajes.
            p=CorregirPosicionLocalOoo(p,margen,false);
            if(!r.puntos.empty())r.largo+=glm::length(p-r.puntos.back());
            r.puntos.push_back(p);r.distancia.push_back(r.largo);
        }
        r.largo+=glm::length(r.puntos.front()-r.puntos.back());
        r.puntos.push_back(r.puntos.front());r.distancia.push_back(r.largo);
        return r;
    }

    glm::vec2 Punto(const Ruta& r,float distancia)
    {
        distancia=std::fmod(distancia,r.largo);if(distancia<0)distancia+=r.largo;
        size_t i=std::upper_bound(r.distancia.begin(),r.distancia.end(),distancia)-r.distancia.begin();
        i=std::max(size_t(1),std::min(i,r.puntos.size()-1));
        float tramo=r.distancia[i]-r.distancia[i-1];
        return glm::mix(r.puntos[i-1],r.puntos[i],tramo>.0001f?(distancia-r.distancia[i-1])/tramo:0.0f);
    }

    Paseo Evaluar(const Ruta& r,float tiempo,float velocidad,float zancada,float pausa,float desfase)
    {
        const float aceleracion=1.2f;
        float mitad=r.largo*.5f,duracion=mitad/velocidad+aceleracion,periodo=duracion+pausa;
        float t=std::fmod(std::max(0.0f,tiempo)+desfase,periodo*2.0f);
        int tramo=int(t/periodo);t-=tramo*periodo;
        float avance=mitad,v=0;
        if(t<aceleracion){v=velocidad*t/aceleracion;avance=.5f*v*t;}
        else if(t<duracion-aceleracion){v=velocidad;avance=velocidad*(t-.5f*aceleracion);}
        else if(t<duracion){float falta=duracion-t;v=velocidad*falta/aceleracion;avance=mitad-.5f*v*falta;}
        float distancia=tramo*mitad+avance;
        Paseo p;p.posicion=Punto(r,distancia);
        glm::vec2 frente=Punto(r,distancia+.8f)-Punto(r,distancia-.8f);
        p.rumbo=std::atan2(frente.x,frente.y);
        p.actividad=v/velocidad;p.fase=distancia*2.0f*PI/zancada;
        // Durante la pausa mira alrededor; vuelve al centro antes de salir.
        if(t>duracion)p.mirada=12.0f*std::sin((t-duracion)/pausa*2.0f*PI);
        return p;
    }


}

void CargarPersonajesOoo()
{
    CargarAmigosCodigoOoo();
    rutaJake=CrearRuta({{-32,16},{-61,11},{-79,-9},{-78,-42},{-57,-63},{-28,-60},{-8,-42},{-3,-17},{-24,0}},1.9f);
    // El circuito delantero queda separado del de Jake para que no se crucen.
    rutaBmo=CrearRuta({{-33,25},{-45,23},{-53,26},{-54,34},{-42,37},{-28,32},{-24,26}},.9f);
    // Circuitos separados del lago y del paseo de Jake y BMO.
    rutaGrumosa=CrearRuta({{26,22},{39,28},{57,25},{65,12},{53,0},{36,4}},2.0f);
    rutaMenta=CrearRuta({{18,42},{18,56},{36,62},{47,54},{37,45},{26,39}},1.5f);
    ActualizarPersonajesOoo(0);
}

void ActualizarPersonajesOoo(float tiempo)
{
    if(rutaJake.puntos.empty())return;
    paseoJake=Evaluar(rutaJake,tiempo,1.3f,1.35f,3.2f,0);
    paseoBmo=Evaluar(rutaBmo,tiempo,.85f,.85f,4.0f,8.0f);
    paseoGrumosa=Evaluar(rutaGrumosa,tiempo,.70f,2.4f,4.5f,0);
    paseoMenta=Evaluar(rutaMenta,tiempo,.90f,1.15f,3.5f,3.0f);
    reloj=tiempo;
}

void DibujarPersonajesOoo()
{
    UsarMaterial(matOpaco);
    const Paseo paseos[]={paseoJake,paseoBmo};
    for(int i=0;i<2;++i)
    {
        const Paseo& p=paseos[i];
        glm::mat4 raiz=glm::translate(glm::mat4(1),CENTRO_OOO+glm::vec3(p.posicion.x,AlturaPaseoOoo(p.posicion),p.posicion.y));
        raiz=glm::rotate(raiz,p.rumbo,glm::vec3(0,1,0));
        DibujarAmigoCodigoOoo(i==0,raiz,p.fase,p.actividad,p.mirada,reloj);
    }
}

glm::vec2 EvitarPersonajesOoo(glm::vec2 p,float radioPersonaje)
{
    const glm::vec2 centros[]={paseoJake.posicion,paseoBmo.posicion};
    const float radios[]={.70f,.46f};
    for(int i=0;i<2;++i)
    {
        glm::vec2 d=p-centros[i];float distancia=glm::length(d),limite=radios[i]+radioPersonaje;
        if(distancia<limite)
            p=centros[i]+(distancia>.0001f ? d/distancia : glm::vec2(0,1))*limite;
    }
    return p;
}

const PaseoNpcOoo& PaseoGrumosaOoo(){return paseoGrumosa;}
const PaseoNpcOoo& PaseoMentaOoo(){return paseoMenta;}
