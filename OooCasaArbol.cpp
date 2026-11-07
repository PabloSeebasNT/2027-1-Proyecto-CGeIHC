#include "OooCasaArbol.h"
#include "PlanetaOoo.h"
#include "Geometria.h"
#include <gtc/matrix_transform.hpp>
#include <cmath>

namespace
{
    Model casaArbol;

    // Centro del claro al final del sendero de la etapa 1.
    const glm::vec2 POS_CASA(-35.0f, -27.0f);
    const float GIRO_CASA = 42.0f;
    const float ESCALA_CASA = 3.5f; // 25% mayor; Finn conserva sus 3 unidades.

    bool ApoyarCasaEnSuelo(aiMesh* malla, const aiScene* escena)
    {
        aiString material;
        escena->mMaterials[malla->mMaterialIndex]->Get(AI_MATKEY_NAME, material);
        bool piedra = std::string(material.C_Str()) == "CasaOoo_stone1";
        if (malla->mNumVertices == 0) return false;

        float minimo = malla->mVertices[0].y;
        for (unsigned int i = 1; i < malla->mNumVertices; ++i)
            minimo = std::min(minimo, malla->mVertices[i].y);

        bool cambio = false;
        for (unsigned int i = 0; i < malla->mNumVertices; ++i)
        {
            float& y = malla->mVertices[i].y;
            if (piedra)
            {
                // Cada piedra es una malla: bajar o subir toda la pieza conserva su grosor.
                y -= minimo;
                cambio = true;
            }
            else if (y < .70f)
            {
                // El borde irregular de las raices llega hasta Y=.664 en el OBJ.
                // Llevar esos vertices y los pies de las paredes al mismo plano.
                y = 0.0f;
                cambio = true;
            }
        }
        return cambio;
    }

    // El modelo original tiene un centro distinto al del tronco.
    // La colision se calcula en coordenadas locales del OBJ para ajustarse mejor.
    glm::vec2 ALocalCasa(glm::vec2 p)
    {
        glm::vec2 d = p - POS_CASA;
        float a = glm::radians(GIRO_CASA);
        float c = std::cos(a), s = std::sin(a);
        return glm::vec2(c * d.x - s * d.y, s * d.x + c * d.y) / ESCALA_CASA;
    }

    glm::vec2 ALocalOoo(glm::vec2 p)
    {
        float a = glm::radians(GIRO_CASA);
        float c = std::cos(a), s = std::sin(a);
        return POS_CASA + glm::vec2(c * p.x + s * p.y, -s * p.x + c * p.y) * ESCALA_CASA;
    }

    glm::vec2 SacarDeCirculo(glm::vec2 p, glm::vec2 centro, float radio)
    {
        glm::vec2 d = p - centro;
        float dist2 = glm::dot(d, d);
        if (dist2 >= radio * radio) return p;
        if (dist2 < 0.000001f) return centro + glm::vec2(radio, 0.0f);
        return centro + d * (radio / std::sqrt(dist2));
    }
}

void CargarCasaArbolOoo()
{
    casaArbol.LoadModel("Models/Ooo/CasaArbol_Ooo.obj", ApoyarCasaEnSuelo);
}

void DibujarCasaArbolOoo()
{
    UsarMaterial(matOpaco);
    glm::mat4 m(1.0f);
    // La base corregida queda ligeramente enterrada en el terreno (Y=.06).
    m = glm::translate(m, CENTRO_OOO + glm::vec3(POS_CASA.x, 0.055f, POS_CASA.y));
    m = glm::rotate(m, glm::radians(GIRO_CASA), glm::vec3(0, 1, 0));
    m = glm::scale(m, glm::vec3(ESCALA_CASA));

    DibujarModelo(casaArbol, m);
}

glm::vec2 EntradaCasaArbolOoo()
{
    // Umbral exterior de la puerta; el camino pasa bajo las piedras de acceso.
    return ALocalOoo(glm::vec2(-6.3f, 3.2f));
}

glm::vec3 PuntoCasaArbolOoo(glm::vec3 punto)
{
    glm::vec2 p = ALocalOoo(glm::vec2(punto.x, punto.z));
    return CENTRO_OOO + glm::vec3(p.x, .055f + punto.y*ESCALA_CASA, p.y);
}

glm::vec2 EvitarCasaArbolOoo(glm::vec2 posicion, float radioPersonaje)
{
    glm::vec2 p = ALocalCasa(posicion);
    float r = radioPersonaje / ESCALA_CASA;
    // La casa baja y el tronco tienen volumen. La escalera y las piedras de entrada no.
    for (int i = 0; i < 2; ++i)
    {
        p = SacarDeCirculo(p, glm::vec2(-3.5f, -2.6f), 3.8f + r);
        p = SacarDeCirculo(p, glm::vec2(-6.6f, 0.0f), 2.3f + r);
    }
    return ALocalOoo(p);
}
