/*
Pyros.h - Proyecto final CGeIHC 2027-1
Planeta de Ricardo (universo de Ben 10, temática de fuego y lava). Todo lo de la superficie se dibuja alrededor de
CENTRO_PYROS; en la zona del espacio, Pyros es una esfera en POS_PYROS_ESPACIO con su luna, un satélite y un anillo
de asteroides.

Evolución de especies en Pyros (diseños propios): larva de magma -> escarabajo de lava -> coloso de basalto.
Animaciones que se inician con teclado: E erupción del volcán, T despegue / aterrizaje de la nave verde,
V evolución en el altar. Animaciones continuas: ríos de lava, burbujas del lago, humo del volcán, larvas, escarabajos
(marcha en trípode), coloso, vuelo de la nave por una spline, luna, satélite y anillo de asteroides.
*/
#pragma once
#include <glm.hpp>
#include "PointLight.h"
#include "SpotLight.h"

const glm::vec3 CENTRO_PYROS(4000.0f, 0.0f, 0.0f);			//centro del piso de Pyros (su zona de superficie)
const glm::vec3 POS_PYROS_ESPACIO(-23.0f, 0.0f, 309.0f);	//centro del planeta en la zona del espacio (croquis)
const float RADIO_PYROS_ESPACIO = 45.0f;
const float RADIO_CAMINABLE_PYROS = 105.0f;
const glm::vec3 INICIO_AVATAR_PYROS(0.0f, 0.0f, 30.0f);		//respecto a CENTRO_PYROS, mirando hacia el altar

void CargarPyros();

//Teclas (main las llama con Window::getPulsada)
void IniciarErupcion(float tiempo);		//E
void AlternarNavePyros(float tiempo);	//T: despega si está estacionada, regresa y aterriza si está volando
void IniciarEvolucion(float tiempo);	//V: el ser del altar evoluciona a su siguiente etapa
//Termina las animaciones de teclado cuando se acaba su tiempo; se llama en cada cuadro
void ActualizarPyros(float tiempo);
//Para las capturas: deja las animaciones de teclado en un estado conocido
void FijarEstadoPyros(float inicioErupcion, float inicioEvolucion, int etapaAltar, int estadoNave, float inicioNave);

//Agrega al final de los arreglos las luces de Pyros que estén encendidas: luminarias (solo de noche), cráter, lago,
//destello de la evolución y los dos reflectores de la plataforma (tecla R)
void LucesPyros(float tiempo, bool deNoche, bool reflectores, PointLight* puntuales, unsigned int& numPuntuales,
	SpotLight* spots, unsigned int& numSpots);
//Saca un punto de los obstáculos (volcán, lago, altar, pedestales, luminarias, coloso y torres)
glm::vec3 ResolverColisionPyros(glm::vec3 p, float radio);

void DibujarSuperficiePyros(float tiempo, bool deNoche);
//Humo, columna de luz y chispas de la evolución: transparentes, se dibujan después de todo lo opaco
void DibujarEfectosPyros(float tiempo, const glm::mat4& vista, glm::vec3 ojo);

void DibujarPyrosEspacio(float tiempo);
void DibujarHaloPyros(float tiempo);	//atmósfera brillante, transparente
