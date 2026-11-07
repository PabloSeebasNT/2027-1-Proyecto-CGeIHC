/*
Avatar.h - Proyecto final CGeIHC 2027-1
Avatar del equipo: "Tizón", criatura original de Pyros de cuatro patas, cola y ojos en tallos (no humanoide).
Está armado por jerarquía con las figuras de Geometria; ver el árbol en Avatar.cpp.
*/
#pragma once
#include <glm.hpp>

struct Avatar
{
	glm::vec3 posicion;			//punto del piso entre las cuatro patas (coordenadas del mundo)
	float rumbo;				//grados alrededor de Y; con 0 mira hacia +Z
	float faseCaminata;			//radianes; solo avanza mientras camina
	float intensidadCaminata;	//0 quieto, 1 caminando; se suaviza para que las patas regresen a su lugar
	float tiempo;				//segundos; mueve la cola, los ojos y la llama aunque esté quieto
};
extern Avatar avatar;

void CargarAvatar();
//W y S avanzan y retroceden, A y D giran; giroRaton es el movimiento horizontal del ratón.
//El avatar no sale del círculo de radio radioCaminable alrededor de centro (el piso de su planeta)
void ActualizarAvatar(bool* keys, float giroRaton, float dt, glm::vec3 centro, float radioCaminable);
void DibujarAvatar();
glm::vec3 FrenteAvatar();
//Linterna (spotlight de la tecla L): sale de la cabeza y apunta hacia adelante y un poco al piso
void LinternaAvatar(glm::vec3& posicion, glm::vec3& direccion);
