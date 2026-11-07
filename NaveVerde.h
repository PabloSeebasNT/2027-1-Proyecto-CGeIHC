/*
NaveVerde.h - Proyecto final CGeIHC 2027-1
Nave verde de Ricardo (diseño propio): se dibuja estacionada en Pyros y volando entre los planetas (cámara 3).
También tiene la ruta cerrada (spline de Catmull-Rom) que usan los dos recorridos de la nave.
*/
#pragma once
#include <vector>
#include <glm.hpp>

//Ruta cerrada que pasa por una lista de puntos. Se recorre por distancia (s), así la velocidad es constante aunque
//los puntos no estén a la misma distancia entre sí
struct RutaCerrada
{
	std::vector<glm::vec3> muestras;	//puntos de la curva, muy juntos
	std::vector<float> acumulado;		//distancia desde el inicio hasta cada muestra
	float largo = 0.0f;
	void Preparar(const glm::vec3* puntos, int n, int muestrasPorTramo);
	glm::vec3 Punto(float s) const;
	//frente: dirección del movimiento; techo: se ladea hacia el centro de las curvas (inclinacion es cuánto)
	void Pose(float s, float inclinacion, glm::vec3& pos, glm::vec3& frente, glm::vec3& techo) const;
};

void CargarNaveVerde();
//tren: 0 guardado, 1 abajo. potencia: 0 apagada, 1 a todo (largo de las llamas). tiempo: luces que parpadean y radar
void DibujarNaveVerde(const glm::mat4& base, float tren, float potencia, float tiempo, bool morada = false);
//Matriz del modelo con el frente (+Z del modelo) y el techo (+Y) dados
glm::mat4 MatrizDesdeEjes(glm::vec3 pos, glm::vec3 frente, glm::vec3 techo, float escala);
//Con el tren abajo los pies quedan a esta distancia bajo el origen del modelo (escala 1)
const float ALTURA_TREN_NAVE = 2.2f;

//Recorrido en la zona del espacio: pasa junto a los planetas del croquis. Depende solo del tiempo de la escena
void PoseNaveEspacio(float tiempo, glm::vec3& pos, glm::vec3& frente, glm::vec3& techo);
const float ESCALA_NAVE_ESPACIO = 2.0f;

