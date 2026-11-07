/*
Proyecto Final - Computación Gráfica e Interacción Humano-Computadora 2027-1
Temáticas: "Viajes espaciales" y "Evolución de especies intergalácticas"

Base del proyecto (cámaras, ciclo de día y noche, zonas y skybox por planeta, avatar) y planeta Pyros con la nave verde:
Ricardo Emmanuel Galicia Tequianes - 118001740

ORGANIZACIÓN DEL MUNDO
- Zona del espacio: alrededor del origen. Los planetas son esferas acomodadas como en el croquis (x = píxel x - 808,
  z = píxel y - 531), con el sol, la estación y la nave verde que vuela entre ellos.
- Zona de superficie de cada planeta: su piso está en centroSuperficie, a 3000 unidades del siguiente, así desde un
  planeta no se ve otro (el plano lejano de la proyección está en 2000).
- La zona donde está la cámara decide qué skybox se dibuja y qué luces se mandan al shader.
Cada integrante agrega su planeta en la tabla PLANETAS y, como Pyros, una función para cargar, otra para sus luces y
otra para dibujar su superficie. Lo marcado como TEMPORAL solo sirve para probar las cámaras y se borra cuando cada
quien suba su parte (carrera, plataforma de lanzamiento y estación).

CONTROLES (las teclas se procesan en Window::ManejaTeclado; aquí solo se leen)
	1	cámara en 3a persona del avatar (plano paralelo a XZ): W A S D + ratón
	2	cámara en 3a persona de la nave de la carrera
	3	cámara en 3a persona de la nave verde volando entre planetas
	4	recorrido: lanzamiento del cohete, estación espacial y cuerpos celestes
	5	cámara libre (W A S D + ratón), para revisar la escena y grabar el video
	P	el avatar viaja al siguiente planeta
	H	adelanta el ciclo de día y noche 15 segundos
	L	linterna del avatar (spotlight)
	R	reflectores de la plataforma de Pyros (spotlights)
	E	erupción del volcán de Pyros
	T	la nave verde despega de Pyros / regresa y aterriza
	V	evolución en el altar de Pyros (larva -> escarabajo -> coloso)
	Esc	cerrar
*/
//para cargar imagen
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#ifdef __APPLE__
#include <filesystem>	//solo en macOS; en Visual Studio con C++14 este encabezado marca error
#include <mach-o/dyld.h>
#endif

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "Window.h"
#include "Mesh_tn.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Model.h"
#include "Skybox.h"
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"

#include "Geometria.h"
#include "Avatar.h"
#include "NaveVerde.h"
#include "Pyros.h"

Window mainWindow;
std::vector<Shader> shaderList;
Camera camera;		//solo la cámara libre (5); las demás se calculan en CalcularCamara

// Vertex Shader
static const char* vShader = "shaders/shader_light.vert";
// Fragment Shader
static const char* fShader = "shaders/shader_light.frag";

//------------------------------------------------------------------ skybox
//Las caras se cargan en el orden de OpenGL (+X, -X, +Y, -Y, +Z, -Z); herramientas/texturas_proyecto.py las genera así
enum { SKY_ESPACIO, SKY_PYROS, NUM_SKYBOX };
Skybox skyboxes[NUM_SKYBOX];

Skybox CargarSkybox(const std::string& nombre)
{
	std::vector<std::string> caras;
	for (const char* c : { "px", "nx", "py", "ny", "pz", "nz" })
		caras.push_back("Textures/Skybox/" + nombre + "_" + c + ".png");
	return Skybox(caras);
}

//------------------------------------------------------------------ planetas (croquis)
struct Planeta
{
	const char* nombre;
	const char* responsable;
	glm::vec3 posEspacio;			//centro de la esfera en la zona del espacio
	float radioEspacio;
	glm::vec3 color;				//color de la esfera y del piso temporales
	glm::vec3 centroSuperficie;		//centro de su piso
	float radioCaminable;
	int skybox;
};
const int PLANETA_PYROS = 0, PLANETA_COMPARTIDO = 1;
const Planeta PLANETAS[] = {
	{ "Pyros", "Ricardo", POS_PYROS_ESPACIO, RADIO_PYROS_ESPACIO, glm::vec3(1.0f, 0.4f, 0.1f), CENTRO_PYROS, RADIO_CAMINABLE_PYROS, SKY_PYROS },
	{ "Cuatripoke (compartido: carrera y pokemon)", "todos", glm::vec3(220.0f, 0.0f, 17.0f), 42.0f, glm::vec3(0.9f, 0.75f, 0.4f), glm::vec3(7000.0f, 0.0f, 0.0f), 110.0f, SKY_ESPACIO },
	{ "Planeta Peanuts", "Yadira", glm::vec3(155.0f, 0.0f, -219.0f), 48.0f, glm::vec3(0.45f, 0.65f, 0.9f), glm::vec3(10000.0f, 0.0f, 0.0f), 110.0f, SKY_ESPACIO },
	{ "Planeta verde", "por asignar", glm::vec3(-381.0f, 0.0f, -126.0f), 40.0f, glm::vec3(0.45f, 0.85f, 0.35f), glm::vec3(13000.0f, 0.0f, 0.0f), 110.0f, SKY_ESPACIO },
	{ "Planeta rosa", "por asignar", glm::vec3(-528.0f, 0.0f, 179.0f), 46.0f, glm::vec3(0.85f, 0.35f, 0.45f), glm::vec3(16000.0f, 0.0f, 0.0f), 110.0f, SKY_ESPACIO },
	{ "Planeta de lava oscuro", "por asignar", glm::vec3(537.0f, 0.0f, 399.0f), 44.0f, glm::vec3(0.4f, 0.33f, 0.32f), glm::vec3(19000.0f, 0.0f, 0.0f), 110.0f, SKY_ESPACIO }
};
const int NUM_PLANETAS = sizeof(PLANETAS) / sizeof(PLANETAS[0]);
const int ZONA_ESPACIO = -1;
const glm::vec3 POS_SOL(-620.0f, 0.0f, -316.0f);
const float RADIO_SOL = 60.0f;
int planetaAvatar = PLANETA_PYROS;

//Zona de un punto: el planeta cuyo piso está a menos de 1400 unidades o, si no, el espacio
int ZonaDe(glm::vec3 p)
{
	for (int i = 0; i < NUM_PLANETAS; i++)
		if (glm::length(glm::vec2(p.x - PLANETAS[i].centroSuperficie.x, p.z - PLANETAS[i].centroSuperficie.z)) < 1400.0f)
			return i;
	return ZONA_ESPACIO;
}

void ColocarAvatarEnPlaneta(int k)
{
	planetaAvatar = k;
	avatar.posicion = PLANETAS[k].centroSuperficie + INICIO_AVATAR_PYROS;
	avatar.rumbo = 180.0f;	//mirando hacia -Z
	avatar.intensidadCaminata = 0.0f;
	printf("Avatar en: %s (%s)\n", PLANETAS[k].nombre, PLANETAS[k].responsable);
}

//------------------------------------------------------------------ ciclo de día y noche
//Un día completo dura DURACION_CICLO segundos: la mitad de día y la mitad de noche. El sol sale por +X, pasa por arriba
//y se pone por -X; de noche la luz direccional es la luna, azulada y débil, desde el lado contrario
const float DURACION_CICLO = 120.0f;
const float HORA_INICIAL = 0.18f;	//0: amanecer, 0.25: mediodía, 0.5: atardecer, 0.75: medianoche
float adelantoHora = 0.0f;			//tecla H
DirectionalLight mainLight;

static float Suave(float a, float b, float x)
{
	float t = fminf(fmaxf((x - a) / (b - a), 0.0f), 1.0f);
	return t * t * (3.0f - 2.0f * t);
}

void ActualizarSol(float hora, bool& deNoche, float& brilloCielo)
{
	float th = 2.0f * 3.14159265f * hora;
	float elevacion = sinf(th);
	float dia = Suave(-0.08f, 0.12f, elevacion);
	glm::vec3 dirSol = -glm::normalize(glm::vec3(cosf(th), sinf(th), 0.35f));
	glm::vec3 dirLuna = -glm::normalize(glm::vec3(-cosf(th), -sinf(th), 0.35f));
	glm::vec3 colorSol = glm::mix(glm::vec3(1.0f, 0.55f, 0.3f), glm::vec3(1.0f, 0.96f, 0.9f), Suave(0.0f, 0.35f, elevacion));
	glm::vec3 colorLuna(0.45f, 0.55f, 1.0f);
	float alto = Suave(0.0f, 0.5f, elevacion);
	mainLight.SetDireccion(glm::normalize(glm::mix(dirLuna, dirSol, dia)));
	mainLight.SetColor(glm::mix(colorLuna, colorSol, dia));
	mainLight.SetIntensidades(glm::mix(0.12f, 0.22f + 0.13f * alto, dia), glm::mix(0.22f, 0.25f + 0.6f * alto, dia));
	deNoche = elevacion < 0.0f;	//todas las luminarias se prenden y se apagan en este mismo instante
	brilloCielo = 0.55f + 0.45f * dia;
}

//------------------------------------------------------------------ TEMPORAL: elementos de otros integrantes
//Solo para que las cámaras 2 y 4 tengan qué seguir. Se reemplazan por la carrera, la plataforma de lanzamiento y la
//estación reales; cada quien debe dejar su función de pose con el mismo nombre o cambiar la llamada en CalcularCamara
const glm::vec3 PLATAFORMA_LANZAMIENTO = PLANETAS[PLANETA_COMPARTIDO].centroSuperficie + glm::vec3(40.0f, 0.0f, -40.0f);
const glm::vec3 POS_ESTACION(-62.0f, 0.0f, 51.0f);
Texture* texPisoGenerico;
Texture* texPlanetaGenerico;
Texture* texMetalBase;
Texture* texLavaBase;
MeshModel* mPisoGenerico;

void PoseNaveCarrera(float t, glm::vec3& pos, glm::vec3& frente)	//TEMPORAL: elipse en el planeta compartido
{
	float a = t * 0.45f;
	pos = PLANETAS[PLANETA_COMPARTIDO].centroSuperficie + glm::vec3(60.0f * sinf(a), 1.0f, 35.0f * cosf(a));
	frente = glm::normalize(glm::vec3(60.0f * cosf(a), 0.0f, -35.0f * sinf(a)));
}

float AlturaCohete(float t)	//TEMPORAL: t es el tiempo desde que inicia la toma del lanzamiento
{
	return t < 2.0f ? 0.0f : 6.0f * (t - 2.0f) * (t - 2.0f);
}

void DibujarTemporalesSuperficie(int zona, float t, float tiempoRecorrido)
{
	if (zona != PLANETA_COMPARTIDO) return;
	glm::vec3 pos, frente;
	glm::mat4 m;
	UsarMaterial(matMetal);
	texMetalBase->UseTexture();
	PoseNaveCarrera(t, pos, frente);
	m = glm::rotate(glm::translate(glm::mat4(1.0f), pos), atan2f(frente.x, frente.z), glm::vec3(0.0f, 1.0f, 0.0f));
	Dibujar(mCubo, glm::scale(m, glm::vec3(2.0f, 0.8f, 4.0f)), glm::vec3(0.9f, 0.3f, 0.3f));
	Dibujar(mCilindro, glm::scale(glm::translate(glm::mat4(1.0f), PLATAFORMA_LANZAMIENTO + glm::vec3(0.0f, 0.5f, 0.0f)), glm::vec3(12.0f, 1.0f, 12.0f)), glm::vec3(0.6f));
	float tt = fmodf(tiempoRecorrido, 39.0f);
	glm::vec3 cohete = PLATAFORMA_LANZAMIENTO + glm::vec3(0.0f, 1.0f + (tt < 9.0f ? AlturaCohete(tt) : 0.0f), 0.0f);
	Dibujar(mCilindro, glm::scale(glm::translate(glm::mat4(1.0f), cohete + glm::vec3(0.0f, 5.0f, 0.0f)), glm::vec3(1.5f, 10.0f, 1.5f)), glm::vec3(0.95f));
	Dibujar(mCono, glm::scale(glm::translate(glm::mat4(1.0f), cohete + glm::vec3(0.0f, 10.0f, 0.0f)), glm::vec3(1.5f, 3.0f, 1.5f)), glm::vec3(0.9f, 0.2f, 0.2f));
}

void DibujarTemporalesEspacio(float t)
{
	glm::mat4 m = glm::rotate(glm::translate(glm::mat4(1.0f), POS_ESTACION), t * 0.05f, glm::vec3(0.0f, 1.0f, 0.0f));
	UsarMaterial(matMetal);
	texMetalBase->UseTexture();
	Dibujar(mCilindro, glm::scale(m, glm::vec3(10.0f, 14.0f, 10.0f)), glm::vec3(0.8f));
	Dibujar(mCilindro, glm::scale(m, glm::vec3(28.0f, 2.5f, 28.0f)), glm::vec3(0.6f));
	Dibujar(mCubo, glm::scale(glm::translate(m, glm::vec3(30.0f, 0.0f, 0.0f)), glm::vec3(12.0f, 4.0f, 4.0f)), glm::vec3(0.7f));
	//naves que entran y salen por el muelle (+X de la estación)
	for (int i = 0; i < 2; i++)
	{
		float u = fmodf(t / 12.0f + 0.5f * i, 1.0f);
		float x = (i == 0) ? 200.0f * (1.0f - u) + 36.0f : 36.0f + 200.0f * u;
		glm::mat4 n = glm::translate(m, glm::vec3(x, 0.0f, 0.0f));
		n = glm::rotate(n, glm::radians(i == 0 ? -90.0f : 90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		Dibujar(mCono, glm::scale(n, glm::vec3(1.5f, 5.0f, 1.5f)), i == 0 ? glm::vec3(0.3f, 0.6f, 1.0f) : glm::vec3(1.0f, 0.8f, 0.2f));
	}
}

//Pisos de los planetas que todavía no tienen su parte: plano grande del color del planeta
void DibujarPisoTemporal(int zona)
{
	UsarMaterial(matOpaco);
	texPisoGenerico->UseTexture();
	glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.0f), PLANETAS[zona].centroSuperficie), glm::vec3(400.0f, 1.0f, 400.0f));
	Dibujar(mPisoGenerico, m, PLANETAS[zona].color);
}

//------------------------------------------------------------------ zona del espacio
void DibujarEspacio(float t)
{
	//sol
	UsarMaterial(matOpaco);
	texLavaBase->UseTexture();
	glm::mat4 m = glm::rotate(glm::translate(glm::mat4(1.0f), POS_SOL), t * 0.02f, glm::vec3(0.0f, 1.0f, 0.0f));
	Dibujar(mEsfera, glm::scale(m, glm::vec3(RADIO_SOL)), glm::vec3(1.0f, 0.95f, 0.75f), 3.0f);
	//planetas de los demás integrantes (TEMPORAL hasta que cada quien ponga su textura o modelo)
	texPlanetaGenerico->UseTexture();
	for (int i = 0; i < NUM_PLANETAS; i++)
	{
		if (i == PLANETA_PYROS) continue;
		m = glm::rotate(glm::translate(glm::mat4(1.0f), PLANETAS[i].posEspacio), t * 0.05f + i, glm::vec3(0.0f, 1.0f, 0.0f));
		Dibujar(mEsfera, glm::scale(m, glm::vec3(PLANETAS[i].radioEspacio)), PLANETAS[i].color);
	}
	DibujarPyrosEspacio(t);
	DibujarTemporalesEspacio(t);
	//nave verde volando entre los planetas (cámara 3)
	glm::vec3 pos, frente, techo;
	PoseNaveEspacio(t, pos, frente, techo);
	DibujarNaveVerde(MatrizDesdeEjes(pos, frente, techo, ESCALA_NAVE_ESPACIO), 0.0f, 1.0f, t);
}

//------------------------------------------------------------------ cámaras
//Recorrido (cámara 4): cuatro tomas seguidas que se repiten
const float DURACION_RECORRIDO = 39.0f;
glm::vec3 ojoAvatar(0.0f);		//posición suavizada de la cámara del avatar
float inicioRecorrido = 0.0f;

void CalcularCamara(int modo, float t, float dt, bool suavizar, glm::vec3& ojo, glm::vec3& objetivo, glm::vec3& arriba)
{
	glm::vec3 pos, frente, techo;
	arriba = glm::vec3(0.0f, 1.0f, 0.0f);
	switch (modo)
	{
	case CAM_AVATAR:
	{
		//detrás del avatar y siempre a la misma altura sobre su piso: la cámara se mueve en un plano paralelo a XZ
		glm::vec3 f = FrenteAvatar();
		glm::vec3 deseado = avatar.posicion - f * 9.0f;
		//si el avatar cambió de planeta la cámara salta con él en lugar de cruzar el espacio
		if (!suavizar || glm::length(deseado - ojoAvatar) > 60.0f) ojoAvatar = deseado;
		else ojoAvatar = glm::mix(ojoAvatar, deseado, 1.0f - expf(-dt * 6.0f));
		ojo = glm::vec3(ojoAvatar.x, avatar.posicion.y + 4.2f, ojoAvatar.z);
		objetivo = avatar.posicion + glm::vec3(0.0f, 1.6f, 0.0f) + f * 4.0f;
		break;
	}
	case CAM_CARRERA:
		PoseNaveCarrera(t, pos, frente);
		ojo = pos - frente * 10.0f + glm::vec3(0.0f, 3.5f, 0.0f);
		objetivo = pos + frente * 6.0f + glm::vec3(0.0f, 1.0f, 0.0f);
		break;
	case CAM_NAVE:
		PoseNaveEspacio(t, pos, frente, techo);
		ojo = pos - frente * 30.0f + techo * 9.0f;
		objetivo = pos + frente * 15.0f;
		arriba = glm::normalize(glm::mix(glm::vec3(0.0f, 1.0f, 0.0f), techo, 0.6f));	//se ladea con la nave, menos que ella
		break;
	case CAM_RECORRIDO:
	{
		float r = fmodf(t - inicioRecorrido, DURACION_RECORRIDO);
		if (r < 0.0f) r += DURACION_RECORRIDO;
		if (r < 9.0f)			//toma 1: lanzamiento del cohete, la cámara sigue al cohete hacia arriba
		{
			ojo = PLATAFORMA_LANZAMIENTO + glm::vec3(45.0f - r, 6.0f + r * 0.5f, 60.0f);
			objetivo = PLATAFORMA_LANZAMIENTO + glm::vec3(0.0f, 6.0f + AlturaCohete(r), 0.0f);
		}
		else if (r < 19.0f)		//toma 2: alrededor de la estación, con naves entrando y saliendo
		{
			float a = 0.6f + (r - 9.0f) * 0.12f;
			ojo = POS_ESTACION + 110.0f * glm::vec3(cosf(a), 0.35f, sinf(a));
			objetivo = POS_ESTACION + glm::vec3(20.0f, 0.0f, 0.0f);
		}
		else if (r < 29.0f)		//toma 3: Pyros con su luna, su satélite y el anillo de asteroides
		{
			float b = -0.6f + (r - 19.0f) * 0.1f;
			ojo = POS_PYROS_ESPACIO + 210.0f * glm::vec3(cosf(b), 0.3f, sinf(b));
			objetivo = POS_PYROS_ESPACIO;
		}
		else					//toma 4: panorámica de todo el sistema
		{
			float u = (r - 29.0f) / 10.0f;
			ojo = glm::mix(glm::vec3(250.0f, 420.0f, 1000.0f), glm::vec3(-350.0f, 330.0f, 820.0f), u);
			objetivo = glm::vec3(0.0f, 0.0f, 60.0f);
		}
		break;
	}
	default:	//CAM_LIBRE
		ojo = camera.getCameraPosition();
		objetivo = ojo + camera.getCameraDirection();
		break;
	}
}

//La cámara libre empieza donde estaba la cámara anterior, mirando hacia el mismo lado
void IniciarCamaraLibre(glm::vec3 ojo, glm::vec3 objetivo)
{
	glm::vec3 d = glm::normalize(objetivo - ojo);
	float yaw = glm::degrees(atan2f(d.z, d.x)), pitch = glm::degrees(asinf(d.y));
	camera = Camera(ojo, glm::vec3(0.0f, 1.0f, 0.0f), yaw, pitch, 40.0f, 0.3f);
}

//------------------------------------------------------------------ capturas para el reporte y las pruebas
//Con --capturas lista.txt se recorre la lista dentro de la misma ventana (una línea por captura: archivo.ppm y
//opciones) y se guarda cada vista. Opciones: --modo n, --hora h (0 a 1), --tiempo t, --planeta k,
//--avatar x z rumbo (relativo al centro de su planeta), --fase f i (caminata), --libre x y z yaw pitch,
//--erupcion t0, --evolucion t0 etapa, --nave estado t0, --linterna 0|1, --reflectores 0|1, --recorrido s
struct Captura
{
	std::string archivo;
	std::vector<std::string> opciones;
};
float tiempoEscena = 0.0f;
float horaFija = -1.0f;		//en las capturas la hora sale de la lista

void EstadoInicial()
{
	mainWindow.reiniciarControles();
	ColocarAvatarEnPlaneta(PLANETA_PYROS);
	avatar.faseCaminata = 0.0f;
	avatar.tiempo = 0.0f;
	FijarEstadoPyros(-1000.0f, -1000.0f, 0, 0, 0.0f);
	horaFija = -1.0f;
	inicioRecorrido = 0.0f;
}

void AplicarOpciones(const std::vector<std::string>& op)
{
	for (size_t i = 0; i < op.size(); i++)
	{
		auto num = [&](size_t k) { return (float)atof(op[k].c_str()); };
		if (op[i] == "--modo" && i + 1 < op.size()) mainWindow.setCamara((int)num(++i));
		else if (op[i] == "--hora" && i + 1 < op.size()) horaFija = num(++i);
		else if (op[i] == "--tiempo" && i + 1 < op.size()) { tiempoEscena = num(++i); avatar.tiempo = tiempoEscena; }
		else if (op[i] == "--planeta" && i + 1 < op.size()) ColocarAvatarEnPlaneta((int)num(++i));
		else if (op[i] == "--avatar" && i + 3 < op.size())
		{
			avatar.posicion = PLANETAS[planetaAvatar].centroSuperficie + glm::vec3(num(i + 1), 0.0f, num(i + 2));
			avatar.rumbo = num(i + 3);
			i += 3;
		}
		else if (op[i] == "--fase" && i + 2 < op.size()) { avatar.faseCaminata = num(i + 1); avatar.intensidadCaminata = num(i + 2); i += 2; }
		else if (op[i] == "--libre" && i + 5 < op.size())
		{
			mainWindow.setCamara(CAM_LIBRE);
			camera = Camera(glm::vec3(num(i + 1), num(i + 2), num(i + 3)), glm::vec3(0.0f, 1.0f, 0.0f), num(i + 4), num(i + 5), 40.0f, 0.3f);
			i += 5;
		}
		else if (op[i] == "--erupcion" && i + 1 < op.size()) FijarEstadoPyros(num(++i), -1000.0f, 0, 0, 0.0f);
		else if (op[i] == "--evolucion" && i + 2 < op.size()) { FijarEstadoPyros(-1000.0f, num(i + 1), (int)num(i + 2), 0, 0.0f); i += 2; }
		else if (op[i] == "--nave" && i + 2 < op.size()) { FijarEstadoPyros(-1000.0f, -1000.0f, 0, (int)num(i + 1), num(i + 2)); i += 2; }
		else if (op[i] == "--linterna" && i + 1 < op.size()) mainWindow.setLinterna(num(++i) != 0.0f);
		else if (op[i] == "--reflectores" && i + 1 < op.size()) mainWindow.setReflectores(num(++i) != 0.0f);
		else if (op[i] == "--recorrido" && i + 1 < op.size()) inicioRecorrido = tiempoEscena - num(++i);
	}
}

std::vector<Captura> LeerListaCapturas(const char* archivo)
{
	std::vector<Captura> lista;
	std::ifstream entrada(archivo);
	std::string linea;
	while (std::getline(entrada, linea))
	{
		std::istringstream palabras(linea);
		Captura c;
		if (!(palabras >> c.archivo) || c.archivo[0] == '#') continue;
		std::string p;
		while (palabras >> p) c.opciones.push_back(p);
		lista.push_back(c);
	}
	printf("Capturas en la lista: %zu\n", lista.size());
	return lista;
}

//Guarda el cuadro actual en un archivo PPM
void GuardarCaptura(const char* archivo)
{
	int ancho = (int)mainWindow.getBufferWidth();
	int alto = (int)mainWindow.getBufferHeight();
	std::vector<unsigned char> pixeles(ancho * alto * 4);
	glReadPixels(0, 0, ancho, alto, GL_RGBA, GL_UNSIGNED_BYTE, pixeles.data());
	std::ofstream salida(archivo, std::ios::binary);
	salida << "P6\n" << ancho << " " << alto << "\n255\n";
	for (int y = alto - 1; y >= 0; y--)	//OpenGL entrega las filas de abajo hacia arriba
		for (int x = 0; x < ancho; x++)
			salida.write(reinterpret_cast<char*>(&pixeles[(y * ancho + x) * 4]), 3);
	printf("Captura %s %dx%d, glGetError = %u\n", archivo, ancho, alto, glGetError());
}


int main(int argc, char** argv)
{
#ifdef __APPLE__
	//Al abrir la .app desde Finder el directorio de trabajo no es el del programa:
	//se busca la carpeta donde están shaders, Models y Textures
	char ejecutable[4096]; uint32_t tam = sizeof(ejecutable);
	if (_NSGetExecutablePath(ejecutable, &tam) == 0 && !std::filesystem::exists(vShader))
	{
		auto base = std::filesystem::weakly_canonical(ejecutable).parent_path();
		if (std::filesystem::exists(base / "shaders")) std::filesystem::current_path(base);
		else if (std::filesystem::exists(base / "../Resources/shaders")) std::filesystem::current_path(base / "../Resources");
	}
#endif
	setvbuf(stdout, NULL, _IONBF, 0);	//los mensajes se escriben de inmediato en el registro
	mainWindow = Window(1366, 768);
	mainWindow.Initialise();
	printf("OpenGL %s / GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
	while (glGetError() != GL_NO_ERROR) {}	//GLEW deja un error de enumeración en perfil Core

	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);

	CrearPrimitivas();
	{
		//piso de los planetas temporales: plano de 1 x 1 con la textura repetida 40 veces
		std::vector<GLfloat> v;
		std::vector<unsigned int> idx = { 0, 2, 1, 1, 2, 3 };
		glm::vec3 arriba(0.0f, 1.0f, 0.0f);
		AgregarVertice(v, glm::vec3(-0.5f, 0.0f, -0.5f), 0.0f, 40.0f, arriba);
		AgregarVertice(v, glm::vec3(0.5f, 0.0f, -0.5f), 40.0f, 40.0f, arriba);
		AgregarVertice(v, glm::vec3(-0.5f, 0.0f, 0.5f), 0.0f, 0.0f, arriba);
		AgregarVertice(v, glm::vec3(0.5f, 0.0f, 0.5f), 40.0f, 0.0f, arriba);
		mPisoGenerico = GuardarMalla(v, idx);
	}
	texPisoGenerico = CargarTextura("Textures/piso_generico.png");
	texPlanetaGenerico = CargarTextura("Textures/planeta_generico.png");
	texMetalBase = CargarTextura("Textures/metal.png");
	texLavaBase = CargarTextura("Textures/lava.png");
	CargarAvatar();
	CargarNaveVerde();
	CargarPyros();
	skyboxes[SKY_ESPACIO] = CargarSkybox("espacio");
	skyboxes[SKY_PYROS] = CargarSkybox("pyros");

	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f, 0.3f, 0.3f, 0.0f, -1.0f, 0.0f);
	PointLight puntuales[MAX_POINT_LIGHTS];
	SpotLight spots[MAX_SPOT_LIGHTS];

	//capturas
	std::vector<Captura> capturas;
	bool salirAlTerminar = false;
	for (int i = 1; i < argc; i++)
	{
		if (std::string(argv[i]) == "--capturas" && i + 1 < argc) capturas = LeerListaCapturas(argv[++i]);
		else if (std::string(argv[i]) == "--salir") salirAlTerminar = true;
	}
	size_t capturaActual = 0;
	int cuadrosCaptura = 0;
	EstadoInicial();
	ColocarAvatarEnPlaneta(PLANETA_PYROS);
	if (!capturas.empty())
	{
		glfwSwapInterval(0);
		AplicarOpciones(capturas[0].opciones);
	}

	//Variables que se usan en cada cuadro: se declaran una sola vez aquí y dentro del while solo se les asigna valor
	GLfloat now = 0.0f, lastTime = (GLfloat)glfwGetTime(), dt = 0.0f;
	bool capturando = false, deNoche = false;
	bool sinTeclas[1024] = { false };
	int modo = CAM_AVATAR, modoAnterior = CAM_AVATAR, zona = 0;
	float hora = 0.0f, brilloCielo = 1.0f;
	glm::vec3 ojo, objetivo, arriba, posLinterna, dirLinterna;
	glm::mat4 vista(1.0f);
	unsigned int numPuntuales = 0, numSpots = 0;
	glm::mat4 projection = glm::perspective(glm::radians(60.0f), (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 2000.0f);

	////Loop mientras no se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		now = (GLfloat)glfwGetTime();
		dt = fminf(now - lastTime, 0.1f);	//si un cuadro tarda mucho las animaciones no dan un salto
		lastTime = now;
		capturando = capturaActual < capturas.size();
		if (capturando) dt = 0.0f;			//en las capturas el tiempo de la escena sale de la lista

		//Recibir eventos del usuario
		glfwPollEvents();
		tiempoEscena += dt;
		modo = mainWindow.getCamara();

		//animaciones de teclado (una vez por cada vez que se presiona la tecla)
		if (mainWindow.getPulsada(GLFW_KEY_E)) IniciarErupcion(tiempoEscena);
		if (mainWindow.getPulsada(GLFW_KEY_T)) AlternarNavePyros(tiempoEscena);
		if (mainWindow.getPulsada(GLFW_KEY_V)) IniciarEvolucion(tiempoEscena);
		if (mainWindow.getPulsada(GLFW_KEY_P)) ColocarAvatarEnPlaneta((planetaAvatar + 1) % NUM_PLANETAS);
		if (mainWindow.getPulsada(GLFW_KEY_H)) adelantoHora += 15.0f / DURACION_CICLO;
		ActualizarPyros(tiempoEscena);

		//el avatar solo camina con su cámara; con las demás se queda quieto pero sigue moviendo la cola y los ojos
		if (modo == CAM_AVATAR)
			ActualizarAvatar(mainWindow.getsKeys(), mainWindow.getXChange(), dt, PLANETAS[planetaAvatar].centroSuperficie, PLANETAS[planetaAvatar].radioCaminable);
		else
			ActualizarAvatar(sinTeclas, 0.0f, dt, PLANETAS[planetaAvatar].centroSuperficie, PLANETAS[planetaAvatar].radioCaminable);
		if (planetaAvatar == PLANETA_PYROS) avatar.posicion = ResolverColisionPyros(avatar.posicion, 1.2f);

		//cámara activa
		if (modo == CAM_RECORRIDO && modoAnterior != CAM_RECORRIDO && !capturando) inicioRecorrido = tiempoEscena;
		if (modo == CAM_LIBRE && modoAnterior != CAM_LIBRE && !capturando) IniciarCamaraLibre(ojo, objetivo);
		if (modo == CAM_LIBRE && !capturando)
		{
			camera.keyControl(mainWindow.getsKeys(), dt);
			camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());
		}
		CalcularCamara(modo, tiempoEscena, dt, !capturando && modo == modoAnterior, ojo, objetivo, arriba);
		modoAnterior = modo;
		vista = glm::lookAt(ojo, objetivo, arriba);
		zona = ZonaDe(ojo);

		//sol y luna
		hora = (horaFija >= 0.0f) ? horaFija : fmodf(HORA_INICIAL + adelantoHora + tiempoEscena / DURACION_CICLO, 1.0f);
		ActualizarSol(hora, deNoche, brilloCielo);

		// Clear the window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		skyboxes[zona == ZONA_ESPACIO ? SKY_ESPACIO : PLANETAS[zona].skybox].DrawSkybox(vista, projection, brilloCielo);

		shaderList[0].UseShader();
		U.model = shaderList[0].GetModelLocation();
		U.color = shaderList[0].getColorLocation();
		U.especular = shaderList[0].GetSpecularIntensityLocation();
		U.brillo = shaderList[0].GetShininessLocation();
		U.emision = shaderList[0].GetEmisionLocation();
		U.desplazamientoUV = shaderList[0].GetDesplazamientoUVLocation();
		U.transparencia = shaderList[0].GetTransparenciaLocation();
		glUniformMatrix4fv(shaderList[0].GetProjectionLocation(), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(shaderList[0].GetViewLocation(), 1, GL_FALSE, glm::value_ptr(vista));
		glUniform3f(shaderList[0].GetEyePositionLocation(), ojo.x, ojo.y, ojo.z);
		glUniform2f(U.desplazamientoUV, 0.0f, 0.0f);
		glUniform1f(U.transparencia, 0.0f);

		//luces: la direccional siempre; las puntuales y spotlight solo las de la zona donde está la cámara
		shaderList[0].SetDirectionalLight(&mainLight);
		numPuntuales = 0;
		numSpots = 0;
		if (zona == PLANETA_PYROS)
			LucesPyros(tiempoEscena, deNoche, mainWindow.getReflectores(), puntuales, numPuntuales, spots, numSpots);
		if (mainWindow.getLinterna() && zona == planetaAvatar && numSpots < MAX_SPOT_LIGHTS)
		{
			LinternaAvatar(posLinterna, dirLinterna);
			spots[numSpots++] = SpotLight(1.0f, 0.95f, 0.8f, 0.0f, 5.0f, posLinterna.x, posLinterna.y, posLinterna.z,
				dirLinterna.x, dirLinterna.y, dirLinterna.z, 1.0f, 0.02f, 0.002f, 25.0f);
		}
		shaderList[0].SetPointLights(puntuales, numPuntuales);
		shaderList[0].SetSpotLights(spots, numSpots);

		//objetos opacos de la zona
		if (zona == ZONA_ESPACIO)
			DibujarEspacio(tiempoEscena);
		else
		{
			if (zona == PLANETA_PYROS) DibujarSuperficiePyros(tiempoEscena, deNoche);
			else DibujarPisoTemporal(zona);
			DibujarTemporalesSuperficie(zona, tiempoEscena, tiempoEscena - inicioRecorrido);
			if (zona == planetaAvatar) DibujarAvatar();
		}
		//transparentes al final
		if (zona == ZONA_ESPACIO) DibujarHaloPyros(tiempoEscena);
		if (zona == PLANETA_PYROS) DibujarEfectosPyros(tiempoEscena, vista, ojo);

		//Capturas: se guarda el tercer cuadro de cada vista y se pasa a la siguiente
		if (capturando && ++cuadrosCaptura == 3)
		{
			GuardarCaptura(capturas[capturaActual].archivo.c_str());
			cuadrosCaptura = 0;
			capturaActual++;
			EstadoInicial();
			if (capturaActual < capturas.size())
				AplicarOpciones(capturas[capturaActual].opciones);
			else if (salirAlTerminar)
				break;
			else
				glfwSwapInterval(1);	//termina la lista: se vuelve al modo normal
		}

		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}
