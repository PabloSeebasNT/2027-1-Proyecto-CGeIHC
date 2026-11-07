#include "CircuitoEspacial.h"
#include "Geometria.h"
#include "CommonValues.h"
#include <gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>

namespace {
    const float ANCHO=12.0f;
    glm::vec3 centro;
    struct Punto {glm::vec3 p,frente,lado,arriba;float distancia;};
    std::vector<Punto> ruta;
    float longitud=0;
    enum Lote { PISO, ESTRUCTURA, PINTURA, AZUL, AMBAR, OSCURO, TOTAL };
    struct GeometriaLote {std::vector<float> v;std::vector<unsigned> idx;MeshModel* m=nullptr;};
    GeometriaLote lotes[TOTAL];
    Texture *metal,*blanca;
    Material pavimento(.24f,14),acero(.55f,28);
    // Salida al sur; vuelta exterior y regreso por las curvas interiores.
    // Los puntos de la meta quedan alineados para conservar la parrilla recta.
    const glm::vec3 controles[]={
        {-20,5,80},{20,5,80},{55,5,80},{80,6,60},
        {82,7,20},{82,8,-25},{75,9,-65},{48,9,-80},
        {5,9,-75},{-40,8,-80},{-72,7,-65},{-82,6,-35},
        {-70,6,-10},{-48,6,-8},{-23,7,-23},
        {-9,8,-12},{2,9,9},{18,9,9},{27,8,-7},
        {37,7,-19},{50,7,-19},{59,6,-6},{59,6,15},{50,5,32},{36,5,42},
        {8,5,32},{-24,5,20},{-52,5,15},{-75,5,30},
        {-80,5,58},{-65,5,78},{-50,5,80}
    };

    glm::vec3 Curva(int k,float t){
        const int n=int(sizeof(controles)/sizeof(controles[0]));
        auto a=controles[(k+n-1)%n],b=controles[k],c=controles[(k+1)%n],d=controles[(k+2)%n];
        return .5f*((2.0f*b)+(-a+c)*t+(2.0f*a-5.0f*b+4.0f*c-d)*t*t+(-a+3.0f*b-3.0f*c+d)*t*t*t);
    }
    Punto Muestra(float metros){
        metros=std::fmod(metros,longitud);if(metros<0)metros+=longitud;
        auto it=std::upper_bound(ruta.begin(),ruta.end(),metros,[](float d,const Punto& p){return d<p.distancia;});
        size_t i=std::max(size_t(1),std::min(size_t(it-ruta.begin()),ruta.size()-1));
        float u=(metros-ruta[i-1].distancia)/(ruta[i].distancia-ruta[i-1].distancia);
        Punto p;p.p=glm::mix(ruta[i-1].p,ruta[i].p,u);p.frente=glm::normalize(glm::mix(ruta[i-1].frente,ruta[i].frente,u));
        p.lado=glm::normalize(glm::cross(glm::vec3(0,1,0),p.frente));p.arriba=glm::normalize(glm::cross(p.frente,p.lado));p.distancia=metros;return p;
    }
    glm::vec3 Sobre(const Punto& p,float lateral,float alto){return p.p+p.lado*lateral+p.arriba*alto;}
    void Quad(int lote,glm::vec3 a,glm::vec3 b,glm::vec3 c,glm::vec3 d,glm::vec3 normal,float u=1,float v=1){
        auto& g=lotes[lote];unsigned k=unsigned(g.v.size()/8);
        AgregarVertice(g.v,a,0,0,normal);AgregarVertice(g.v,b,u,0,normal);
        AgregarVertice(g.v,c,u,v,normal);AgregarVertice(g.v,d,0,v,normal);
        g.idx.insert(g.idx.end(),{k,k+1,k+2,k,k+2,k+3});
    }
    void Caja(int lote,const Punto& p,glm::vec3 pos,glm::vec3 tam){
        glm::vec3 c=Sobre(p,pos.x,pos.y)+p.frente*pos.z;
        glm::vec3 x=p.lado*tam.x*.5f,y=p.arriba*tam.y*.5f,z=p.frente*tam.z*.5f;
        Quad(lote,c-x+y-z,c+x+y-z,c+x+y+z,c-x+y+z,p.arriba);
        Quad(lote,c-x-y+z,c+x-y+z,c+x-y-z,c-x-y-z,-p.arriba);
        Quad(lote,c+x-y-z,c+x-y+z,c+x+y+z,c+x+y-z,p.lado);
        Quad(lote,c-x-y+z,c-x-y-z,c-x+y-z,c-x+y+z,-p.lado);
        Quad(lote,c-x-y+z,c-x+y+z,c+x+y+z,c+x-y+z,p.frente);
        Quad(lote,c+x-y-z,c+x+y-z,c-x+y-z,c-x-y-z,-p.frente);
    }
    void Franja(int lote,const Punto& a,const Punto& b,float l,float r,float altura){
        Quad(lote,Sobre(a,l,altura),Sobre(a,r,altura),Sobre(b,r,altura),Sobre(b,l,altura),glm::normalize(a.arriba+b.arriba),2,glm::length(b.p-a.p)*.25f);
    }
}

void CargarCircuitoEspacial(glm::vec3 origen){
    centro=origen;metal=CargarTextura("Textures/metal.png");blanca=CargarTextura("Textures/plain.png");
    const int n=int(sizeof(controles)/sizeof(controles[0])),porTramo=40;
    for(int k=0;k<n;++k)for(int j=0;j<porTramo;++j){
        Punto p;p.p=Curva(k,float(j)/porTramo);
        p.frente=glm::normalize(Curva(k,float(j)/porTramo+.001f)-Curva(k,float(j)/porTramo-.001f));
        p.lado=glm::normalize(glm::cross(glm::vec3(0,1,0),p.frente));p.arriba=glm::normalize(glm::cross(p.frente,p.lado));
        if(!ruta.empty())longitud+=glm::length(p.p-ruta.back().p);
        p.distancia=longitud;ruta.push_back(p);
    }
    longitud+=glm::length(ruta.front().p-ruta.back().p);Punto cierre=ruta.front();cierre.distancia=longitud;ruta.push_back(cierre);
    float pendiente=0;
    for(size_t i=0;i+1<ruta.size();++i){
        auto a=ruta[i],b=ruta[i+1];pendiente=std::max(pendiente,std::abs(a.frente.y));
        Franja(PISO,a,b,-6,6,0);Franja(ESTRUCTURA,a,b,-6.6f,6.6f,-.55f);
        for(float s:{-1.0f,1.0f}){
            float x=s*6.2f;
            Quad(ESTRUCTURA,Sobre(a,x,-.55f),Sobre(b,x,-.55f),Sobre(b,x,.85f),Sobre(a,x,.85f),a.lado*s);
            Franja(ESTRUCTURA,a,b,x-.22f,x+.22f,.85f);
            Franja(AZUL,a,b,x-.10f,x+.10f,.87f);
            Franja(PINTURA,a,b,s*5.30f-.09f,s*5.30f+.09f,.025f);
        }
        if(int(a.distancia/4)%2==0)Franja(PINTURA,a,b,-.06f,.06f,.028f);
    }
    // Apoyos estructurales y postes: tambien se agrupan en mallas estaticas.
    for(float d=0;d<longitud;d+=18){auto p=Muestra(d);
        for(float s:{-1.0f,1.0f}){
            Caja(ESTRUCTURA,p,{s*5.7f,-p.p.y*.5f,0},{.65f,p.p.y,.80f});
            Caja(ESTRUCTURA,p,{s*6.2f,.55f,0},{.50f,1.4f,.45f});
            Caja(AMBAR,p,{s*6.2f,1.30f,0},{.35f,.20f,.35f});
        }
    }
    // Flechas sobre el pavimento indican el sentido sin texto ni carteles gigantes.
    for(float d=18;d<longitud;d+=32){auto p=Muestra(d);
        for(float s:{-1.0f,1.0f}){
            glm::vec3 a=Sobre(p,s*1.1f,.04f)-p.frente*.9f,b=Sobre(p,0,.04f)+p.frente*.8f;
            glm::vec3 desplazamiento=p.frente*.35f;
            Quad(AMBAR,a,b,b+desplazamiento,a+desplazamiento,p.arriba);
        }
    }
    // Meta a cuadros y seis cajones de salida detras de la linea.
    for(int f=0;f<2;++f)for(int c=0;c<12;++c){auto p=Muestra(f*1.0f);
        Caja((f+c)%2?OSCURO:PINTURA,p,{c-5.5f,.035f,0},{1,.045f,1});
    }
    for(int f=0;f<3;++f)for(float lado:{-1.0f,1.0f}){
        auto p=Muestra(-5-f*5.0f);Caja(PINTURA,p,{lado*2.7f,.03f,-1.5f},{2.8f,.04f,.12f});
        for(float s:{-1.0f,1.0f})Caja(PINTURA,p,{lado*2.7f+s*1.4f,.03f,0},{.10f,.04f,3});
    }
    auto salida=Muestra(0);
    for(float s:{-1.0f,1.0f})Caja(ESTRUCTURA,salida,{s*7.0f,3.6f,0},{.8f,7.2f,1});
    Caja(ESTRUCTURA,salida,{0,7.0f,0},{14.8f,.9f,1.2f});
    for(float s:{-1.0f,1.0f}){
        for(int i=0;i<5;++i)Caja(AMBAR,salida,{(i-2)*1.1f,6.9f,s*.63f},{.50f,.35f,.10f});
        for(int i=0;i<4;++i)for(float lado:{-1.0f,1.0f})
            Caja(i%2?PINTURA:OSCURO,salida,{lado*(4.1f+i*.65f),7.0f,s*.63f},{.65f,.70f,.08f});
    }
    for(int i=0;i<8;++i){auto p=Muestra(longitud*i/8);
        Caja(ESTRUCTURA,p,{7.2f,2.1f,0},{.22f,4.2f,.22f});
        Caja(AZUL,p,{7.2f,4.3f,0},{1.0f,.20f,.70f});
    }
    // Comprobar cruces de bordes en XZ antes de subir las mallas.
    struct Borde {glm::vec2 a,b;};std::vector<Borde> bordes;
    for(float lado:{-6.2f,6.2f})for(size_t i=0;i+1<ruta.size();++i){
        auto a=Sobre(ruta[i],lado,0),b=Sobre(ruta[i+1],lado,0);bordes.push_back({{a.x,a.z},{b.x,b.z}});
    }
    auto orientacion=[](glm::vec2 a,glm::vec2 b,glm::vec2 c){auto u=b-a,v=c-a;return u.x*v.y-u.y*v.x;};
    int cruces=0;
    for(size_t i=0;i<bordes.size();++i)for(size_t j=i+1;j<bordes.size();++j){
        auto a=bordes[i],b=bordes[j];
        if(orientacion(a.a,a.b,b.a)*orientacion(a.a,a.b,b.b)<-.00001f&&
           orientacion(b.a,b.b,a.a)*orientacion(b.a,b.b,a.b)<-.00001f)++cruces;
    }
    if(cruces)throw std::runtime_error("El circuito tiene cruces entre bordes; revisar los controles.");
    size_t triangulos=0;for(auto& lote:lotes){
        for(float v:lote.v)if(!std::isfinite(v))throw std::runtime_error("Vertice no finito en circuito");
        for(unsigned i:lote.idx)if(i>=lote.v.size()/8)throw std::runtime_error("Indice invalido en circuito");
        lote.m=GuardarMalla(lote.v,lote.idx);triangulos+=lote.idx.size()/3;
    }
    printf("Circuito: %.2f unidades, %zu muestras, %zu triangulos, pendiente maxima %.3f, cierre %.6f, cruces %d\n",longitud,ruta.size(),triangulos,pendiente,glm::length(ruta.front().p-ruta.back().p),cruces);
}
float LongitudCircuitoEspacial(){return longitud;}
void PoseCircuitoEspacial(float distancia,glm::vec3& posicion,glm::vec3& frente){
    if(ruta.empty()){posicion=centro;frente={0,0,1};return;}
    auto p=Muestra(distancia);posicion=centro+p.p;frente=p.frente;
}
void DibujarCircuitoEspacial(){
    auto m=glm::translate(glm::mat4(1),centro);
    const glm::vec3 colores[]={{.27f,.34f,.43f},{.33f,.40f,.49f},{.86f,.91f,.94f},{.12f,.62f,.90f},{1,.65f,.18f},{.045f,.055f,.075f}};
    for(int i=0;i<TOTAL;++i){if(i<2){metal->UseTexture();UsarMaterial(i==0?pavimento:acero);}else blanca->UseTexture();
        Dibujar(lotes[i].m,m,colores[i],i==AZUL?.48f:i==AMBAR?.18f:0);
    }
}
void LucesCircuitoEspacial(float noche,PointLight* luces,unsigned int& cantidad){
    for(int i=0;i<8&&cantidad<MAX_POINT_LIGHTS;++i){auto p=Muestra(longitud*i/8);auto lugar=centro+Sobre(p,7.2f,4.1f);
        luces[cantidad++]=PointLight(.48f,.74f,1,.02f,1.5f+2*noche,lugar.x,lugar.y,lugar.z,1,.035f,.004f);
    }
}


