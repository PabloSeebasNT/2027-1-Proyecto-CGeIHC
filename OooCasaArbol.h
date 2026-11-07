#pragma once
#include <glm.hpp>

// La casa se carga una sola vez. Su posicion es relativa al centro de Ooo.
void CargarCasaArbolOoo();
void DibujarCasaArbolOoo();
glm::vec2 EntradaCasaArbolOoo();
// Lleva un punto del modelo de la casa a coordenadas del escenario.
glm::vec3 PuntoCasaArbolOoo(glm::vec3 punto);

// Colision sencilla con el tronco y la construccion; el sendero queda libre.
glm::vec2 EvitarCasaArbolOoo(glm::vec2 posicion, float radioPersonaje);
