// Ooo - Etapa 1. Paisaje, senderos y vegetacion.
// Se dibuja junto con PlanetaOoo, sin reemplazar la implementacion de Finn.
#pragma once
#include <glm.hpp>

void CargarPaisajeOoo();
void DibujarPaisajeOoo();

// Reciben posiciones del planeta: local para Finn, global para el avatar.
glm::vec2 CorregirPosicionLocalOoo(glm::vec2 posicion, float radioPersonaje, bool evitarAmigos = true);
float AlturaPaseoOoo(glm::vec2 posicion);
glm::vec3 CorregirPosicionAvatarOoo(glm::vec3 posicion, float radioPersonaje);
