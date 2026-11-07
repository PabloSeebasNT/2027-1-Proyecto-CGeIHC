#pragma once
#include <glm.hpp>

// Las dos naves usan la misma curva de la pista, con carriles distintos.
void PoseCorredor(int corredor, float tiempo, glm::vec3& posicion,
    glm::vec3& frente, glm::vec3& techo);
void DibujarCarreraEspacial(float tiempo);
