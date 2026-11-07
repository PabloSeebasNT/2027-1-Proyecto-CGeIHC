#pragma once
#include <glm.hpp>
#include "PointLight.h"

void CargarEstacionEspacial(glm::vec3 centro);
void DibujarEstacionEspacial(float tiempo);
void LucesEstacionEspacial(bool noche, PointLight* luces, unsigned int& cantidad);
