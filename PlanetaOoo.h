// Etapa 1 de Uri: registro y superficie base del planeta de Hora de Aventura.
// Es una zona nueva; no reemplaza los planetas reservados por el equipo.
#pragma once
#include <glm.hpp>

const glm::vec3 POS_OOO_ESPACIO(-330.0f, 0.0f, 430.0f);
const float RADIO_OOO_ESPACIO = 42.0f;
const glm::vec3 CENTRO_OOO(22000.0f, 0.0f, 0.0f);
const float RADIO_CAMINABLE_OOO = 110.0f;
const glm::vec3 INICIO_AVATAR_OOO(0.0f, 0.0f, 30.0f);
const glm::vec3 COLOR_OOO(0.45f, 0.85f, 0.35f);

void CargarOoo();
void DibujarSuperficieOoo(float tiempo);
void DibujarOooEspacio(float tiempo);
