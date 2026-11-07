#pragma once
#include <glm.hpp>

void CargarPersonajesOoo();
void ActualizarPersonajesOoo(float tiempo);
void DibujarPersonajesOoo();
glm::vec2 EvitarPersonajesOoo(glm::vec2 posicion, float radioPersonaje);

// Estado compartido por el dibujo y las colisiones del jardin.
struct PaseoNpcOoo {
    glm::vec2 posicion{0};
    float rumbo=0, fase=0, actividad=0, mirada=0;
};
const PaseoNpcOoo& PaseoGrumosaOoo();
const PaseoNpcOoo& PaseoMentaOoo();