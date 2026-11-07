#pragma once
#include <glm.hpp>

// Finn esta dividido en 10 piezas; cada articulacion conserva a sus hijos.
void CargarFinn();
void ActualizarFinn(const bool* teclas, float dt, float radioCaminable);
void IniciarSaludoFinn(float tiempo);
void DibujarFinn(const glm::vec3& posicion, float tiempo);
