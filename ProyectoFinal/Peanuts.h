#pragma once
// Planeta Peanuts (Snoopy y sus amigos) - Yadira
#include <glm.hpp>
#include "PointLight.h"
#include "SpotLight.h"

//centro de su piso (el mismo que tiene PLANETAS[PLANETA_PEANUTS].centroSuperficie en el main)
const glm::vec3 CENTRO_PEANUTS(10000.0f, 0.0f, 0.0f);

void CargarPeanuts();																//modelos, texturas y mallas
void LucesPeanuts(float tiempo, bool deNoche, PointLight* puntuales, unsigned int& np);	//lamparas y focos de las casas
void AlternarLamparasPeanuts();														//tecla N: prende o apaga las lamparas
void DibujarSuperficiePeanuts(float t, bool deNoche);								//todo lo que se ve en el planeta
glm::vec3 ResolverColisionPeanuts(glm::vec3 p, float radio);						//para que el avatar no atraviese nada