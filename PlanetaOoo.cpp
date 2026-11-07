#include "PlanetaOoo.h"
#include "Finn.h"
#include "Geometria.h"
#include <gtc/matrix_transform.hpp>

namespace
{
    Texture* texturaPiso = nullptr;
    Texture* texturaPlaneta = nullptr;
    MeshModel* piso = nullptr;
}

void CargarOoo()
{
    CargarFinn();
    // Recursos provisionales de la base; la ambientacion de Ooo vendra despues.
    texturaPiso = CargarTextura("Textures/piso_generico.png");
    texturaPlaneta = CargarTextura("Textures/planeta_generico.png");

    // Piso de 400 x 400 unidades. UV repetidas para conservar detalle al caminar.
    std::vector<GLfloat> vertices;
    std::vector<unsigned int> indices = { 0, 2, 1, 1, 2, 3 };
    const glm::vec3 normal(0.0f, 1.0f, 0.0f);
    AgregarVertice(vertices, glm::vec3(-200, 0, -200), 0, 40, normal);
    AgregarVertice(vertices, glm::vec3(200, 0, -200), 40, 40, normal);
    AgregarVertice(vertices, glm::vec3(-200, 0, 200), 0, 0, normal);
    AgregarVertice(vertices, glm::vec3(200, 0, 200), 40, 0, normal);
    piso = GuardarMalla(vertices, indices);
}

void DibujarSuperficieOoo(float tiempo)
{
    UsarMaterial(matOpaco);
    texturaPiso->UseTexture();
    Dibujar(piso, glm::translate(glm::mat4(1.0f), CENTRO_OOO), COLOR_OOO);
    // A la derecha y delante de la llegada del avatar, mirando hacia +Z.
    DibujarFinn(CENTRO_OOO + glm::vec3(4.0f, 0.0f, 22.0f), tiempo);
}

void DibujarOooEspacio(float tiempo)
{
    UsarMaterial(matOpaco);
    texturaPlaneta->UseTexture();
    glm::mat4 modelo = glm::translate(glm::mat4(1.0f), POS_OOO_ESPACIO);
    modelo = glm::rotate(modelo, tiempo * 0.05f, glm::vec3(0, 1, 0));
    Dibujar(mEsfera, glm::scale(modelo, glm::vec3(RADIO_OOO_ESPACIO)), COLOR_OOO);
}
