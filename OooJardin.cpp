// Estanque y piedras de paso inspirados en el jardin de la casa del arbol.
// Geometria propia: contorno irregular, orilla baja y piedras individuales.
#include "OooJardin.h"
#include "OooCasaArbol.h"
#include "PlanetaOoo.h"
#include "Geometria.h"
#include <gtc/matrix_transform.hpp>
#include <cmath>

namespace
{
    const float PI = 3.14159265f;
    const glm::vec2 CENTRO_LAGO(-7.0f, 8.0f);
    const glm::vec2 RADIOS(11.0f, 6.7f);
    MeshModel* agua;
    MeshModel* orilla;
    MeshModel* reflejo;
    Texture* blanca;
    Texture* piedra;
    Texture* tierra;

    glm::vec2 Contorno(float a)
    {
        float r = 1.0f + .065f * std::sin(3*a + .4f) + .035f * std::cos(5*a);
        return glm::vec2(RADIOS.x * std::cos(a), RADIOS.y * std::sin(a)) * r;
    }

    MeshModel* CrearFranja(float interior, float exterior, float yInterior, float yExterior)
    {
        std::vector<GLfloat> v;
        std::vector<unsigned int> indices;
        for (int i=0; i<=96; ++i)
        {
            glm::vec2 c = Contorno(2*PI*i/96.0f);
            for (int j=0; j<2; ++j)
            {
                glm::vec2 p = CENTRO_LAGO + c * (j ? exterior : interior);
                AgregarVertice(v, glm::vec3(p.x, j ? yExterior : yInterior, p.y),
                    p.x*.23f, p.y*.23f, glm::vec3(0,1,0));
            }
            if (i>0)
            {
                unsigned int a=2*(i-1);
                indices.insert(indices.end(), {a,a+2,a+1, a+1,a+2,a+3});
            }
        }
        return GuardarMalla(v,indices);
    }

    glm::mat4 Lugar(glm::vec2 p, float y)
    {
        return glm::translate(glm::mat4(1), CENTRO_OOO + glm::vec3(p.x,y,p.y));
    }

    void PiedraPaso(glm::vec2 p, int i, float tamano=1.0f)
    {
        float variacion = .9f + .15f*std::sin(i*2.13f);
        glm::mat4 m=Lugar(p,.13f);
        m=glm::rotate(m, i*1.71f, glm::vec3(0,1,0));
        m=glm::scale(m,glm::vec3(.63f*variacion,.16f,.43f)*tamano);
        Dibujar(mEsferaBaja,m,glm::vec3(.77f,.68f,.49f));
    }
}

void CargarJardinOoo()
{
    blanca=CargarTextura("Textures/plain.png");
    piedra=CargarTextura("Textures/OooEtapa1/piedra.png");
    tierra=CargarTextura("Textures/OooEtapa1/sendero.png");
    agua=CrearFranja(0,1,.115f,.115f);
    orilla=CrearFranja(1,1.105f,.12f,.065f);
    reflejo=CrearFranja(.95f,.963f,.118f,.118f);
}

void DibujarJardinOoo()
{
    glm::mat4 mundo=glm::translate(glm::mat4(1),CENTRO_OOO);
    UsarMaterial(matOpaco);
    tierra->UseTexture();
    Dibujar(orilla,mundo,glm::vec3(.60f,.58f,.32f));
    blanca->UseTexture();
    UsarMaterial(matBrillante);
    Dibujar(agua,mundo,glm::vec3(.12f,.48f,.54f),.08f);
    Dibujar(reflejo,mundo,glm::vec3(.28f,.62f,.61f),.05f);

    // Hojas en pequenos grupos asimetricos; el centro queda despejado.
    UsarMaterial(matOpaco);
    const glm::vec2 hojas[]={{-13,9},{-12,9.6f},{-11.6f,8.5f},{-3,5},
        {-2.1f,5.5f},{-.9f,5},{-6,11.5f},{-5.1f,12},{-4.6f,10.8f}};
    for (int i=0;i<9;++i)
    {
        glm::mat4 m=glm::rotate(Lugar(hojas[i],.15f),i*.9f,glm::vec3(0,1,0));
        Dibujar(mCilindro,glm::scale(m,glm::vec3(.40f+.06f*(i%3),.025f,.31f)),
            i%2 ? glm::vec3(.40f,.58f,.17f) : glm::vec3(.27f,.43f,.12f));
    }

    // Camino discontinuo: desde el acceso, rodea el lado cercano del estanque.
    piedra->UseTexture();
    glm::vec2 a=EntradaCasaArbolOoo(), b(-21,27), c(10,26), d(17,8);
    for (int i=0;i<31;++i)
    {
        float t=i/30.0f, u=1-t;
        glm::vec2 p=u*u*u*a+3*u*u*t*b+3*u*t*t*c+t*t*t*d;
        p+=glm::vec2(.18f*std::sin(i*2.4f),.20f*std::cos(i*1.7f));
        PiedraPaso(p,i);
    }
    // Grupos cortos de piedras de ribera; no un collar uniforme alrededor del agua.
    const float angulos[]={.12f,.25f,.39f,1.62f,1.75f,2.75f,2.90f,3.07f,4.45f};
    for(int i=0;i<9;++i)
        PiedraPaso(CENTRO_LAGO+Contorno(angulos[i])*1.09f,40+i,.65f);
}

glm::vec2 EvitarLagoOoo(glm::vec2 p, float radioPersonaje)
{
    // No se implementa natacion: una elipse algo mayor que el agua evita cruzarla.
    glm::vec2 e=RADIOS*1.10f+glm::vec2(radioPersonaje);
    glm::vec2 q=(p-CENTRO_LAGO)/e;
    float distancia=glm::length(q);
    if(distancia>=1) return p;
    if(distancia<.0001f) q=glm::vec2(0,1);
    else q/=distancia;
    return CENTRO_LAGO+q*e;
}
