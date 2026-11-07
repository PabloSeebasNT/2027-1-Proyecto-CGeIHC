#pragma once
#include <glm.hpp>
#include "PointLight.h"
void CargarCircuitoEspacial(glm::vec3 centro);
void DibujarCircuitoEspacial();
void LucesCircuitoEspacial(float noche,PointLight* luces,unsigned int& cantidad);
// Distancia en unidades, con vuelta automatica al inicio. Sin estado por cuadro.
void PoseCircuitoEspacial(float distancia,glm::vec3& posicion,glm::vec3& frente);
float LongitudCircuitoEspacial();
