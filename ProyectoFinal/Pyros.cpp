/*
Pyros.cpp - Proyecto final CGeIHC 2027-1
Ricardo Emmanuel Galicia Tequianes - 118001740

Todas las posiciones de la superficie son relativas a CENTRO_PYROS (función Lugar). Vista desde arriba:
	altar de la evolución en el centro, sobre un círculo de runas, con 4 luminarias alrededor
	museo de la evolución (3 pedestales) al norte del altar (-Z)
	volcán al noroeste, con su río de lava que baja hasta el lago de lava al noreste
	plataforma de aterrizaje de la nave verde al sureste, con 2 torres de reflectores y 2 luminarias
	montañas alrededor (el terreno sube después de r = 108); el avatar camina dentro de r = 105
*/
#include "Pyros.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <gtc/matrix_transform.hpp>
#include "Geometria.h"
#include "NaveVerde.h"
#include "CommonValues.h"

static Texture *texRoca, *texLava, *texObsidiana, *texMetal, *texPlataforma, *texRunas, *texCristal, *texLarva,
	*texCaparazon, *texColoso, *texHumo, *texPlaneta, *texLuna, *texAsteroide, *texPanel, *texBlanco;
static MeshModel *mTerreno, *mVolcan, *mFlujo, *mRio, *mHexagono, *mRocas[6];

const float PI_P = 3.14159265f;

//------------------------------------------------------------------ distribución de la superficie
const glm::vec3 VOLCAN(-60.0f, 0.0f, -70.0f);
const float RADIO_VOLCAN = 45.0f, ALTURA_VOLCAN = 38.0f, RADIO_CRATER = 8.0f;
const float ANGULO_FLUJO = 0.46f;			//dirección (en XZ, desde el centro del volcán) por donde baja la lava
const glm::vec3 LAGO(55.0f, 0.0f, -45.0f);
const float RADIO_LAGO = 13.0f;
const glm::vec3 PLATAFORMA(50.0f, 0.0f, 38.0f);
const float RADIO_PLATAFORMA = 10.0f, ALTO_PLATAFORMA = 0.5f;
const glm::vec3 TORRES[2] = { glm::vec3(35.0f, 0.0f, 38.0f), glm::vec3(65.0f, 0.0f, 38.0f) };
const float ALTO_TORRE = 7.0f;
const glm::vec3 LUMINARIAS[6] = {
	glm::vec3(14.0f, 0.0f, 14.0f), glm::vec3(-14.0f, 0.0f, 14.0f), glm::vec3(14.0f, 0.0f, -14.0f), glm::vec3(-14.0f, 0.0f, -14.0f),
	glm::vec3(38.0f, 0.0f, 24.0f), glm::vec3(62.0f, 0.0f, 24.0f) };
const float ALTO_CRISTAL_LUMINARIA = 4.6f;
const float RADIO_PLAZA = 13.0f;
const glm::vec3 PEDESTALES[3] = { glm::vec3(-11.0f, 0.0f, -18.0f), glm::vec3(0.0f, 0.0f, -21.0f), glm::vec3(11.0f, 0.0f, -18.0f) };
const glm::vec3 COLOSO(-30.0f, 0.0f, -24.0f);
const float ALTO_ALTAR = 0.8f;

//------------------------------------------------------------------ estado de las animaciones de teclado
const float DURACION_ERUPCION = 7.0f;
const float DURACION_EVOLUCION = 4.0f;
static float inicioErupcion = -1000.0f;
static float inicioEvolucion = -1000.0f;
static int etapaAltar = 0;		//0 larva, 1 escarabajo, 2 coloso

//Nave verde de Pyros: estacionada, despegando, volando en circuito, regresando al punto sobre la plataforma y aterrizando
enum EstadoNave { NAVE_PLATAFORMA, NAVE_DESPEGANDO, NAVE_VOLANDO, NAVE_REGRESANDO, NAVE_ATERRIZANDO };
static int estadoNave = NAVE_PLATAFORMA;
static float inicioNave = 0.0f;
static glm::vec3 regresoInicio, regresoVelocidad;	//dónde estaba y hacia dónde iba cuando se pidió aterrizar
const float DURACION_DESPEGUE = 4.0f, DURACION_ATERRIZAJE = 4.0f, DURACION_REGRESO = 6.0f;
const float VELOCIDAD_VUELO = 22.0f, TIEMPO_ACELERACION = 3.0f;
const float ALTURA_ESPERA = 22.0f;	//la nave sube a este punto sobre la plataforma antes de empezar el circuito
static RutaCerrada rutaPyros;
static const glm::vec3 PUNTOS_VUELO[] = {
	glm::vec3(50.0f, ALTURA_ESPERA, 38.0f),
	glm::vec3(15.0f, 28.0f, 80.0f),
	glm::vec3(-48.0f, 34.0f, 72.0f),
	glm::vec3(-95.0f, 42.0f, 5.0f),
	glm::vec3(-72.0f, 60.0f, -62.0f),
	glm::vec3(-5.0f, 45.0f, -100.0f),
	glm::vec3(62.0f, 34.0f, -82.0f),
	glm::vec3(100.0f, 28.0f, -15.0f),
	glm::vec3(88.0f, 24.0f, 30.0f)
};

//Decoración repartida con números pseudoaleatorios fijos (siempre sale igual)
struct Decoracion { glm::vec3 pos; float escala, giro, alto; int tipo; };
static std::vector<Decoracion> rocas, cristales;

static glm::mat4 Lugar(glm::vec3 p) { return glm::translate(glm::mat4(1.0f), CENTRO_PYROS + p); }
static float Suave(float a, float b, float x)
{
	float t = fminf(fmaxf((x - a) / (b - a), 0.0f), 1.0f);
	return t * t * (3.0f - 2.0f * t);
}
static float Aleatorio(unsigned int& semilla)
{
	semilla = semilla * 1664525u + 1013904223u;
	return (semilla >> 8) / 16777216.0f;
}


//------------------------------------------------------------------ alturas
//Terreno: plano en la zona donde se camina y montañas alrededor
static float AlturaTerreno(float x, float z)
{
	float r = sqrtf(x * x + z * z);
	float anillo = Suave(108.0f, 175.0f, r);
	if (anillo <= 0.0f) return 0.0f;
	float montes = anillo * anillo * (34.0f + 26.0f * Fbm3(glm::vec3(x * 0.013f, 0.5f, z * 0.013f), 4));
	float rugoso = anillo * 3.0f * Fbm3(glm::vec3(x * 0.09f, 2.5f, z * 0.09f), 3);
	return fmaxf(0.0f, montes + rugoso);
}

//Volcán: r es la distancia a su centro y a el ángulo. Dentro del cráter hay un tazón; afuera la ladera baja con un
//perfil cóncavo (1 - t)^1.7, con surcos que bajan desde la cima y ruido. En la orilla queda un poco bajo el piso
static float AlturaVolcan(float r, float a)
{
	if (r < RADIO_CRATER)
	{
		float t = r / RADIO_CRATER;
		return ALTURA_VOLCAN - 5.0f + 6.5f * t * t;
	}
	float t = (r - RADIO_CRATER) / (RADIO_VOLCAN - RADIO_CRATER);
	if (t >= 1.0f) return -0.3f;
	float h = ALTURA_VOLCAN * powf(1.0f - t, 1.7f);
	float surcos = sinf(a * 9.0f + 1.3f * sinf(a * 4.0f));
	float ruido = Fbm3(glm::vec3(cosf(a) * r * 0.07f, 4.2f, sinf(a) * r * 0.07f), 3);
	h += (surcos * 1.2f + ruido * 3.0f) * 4.0f * t * (1.0f - t);
	h += 1.5f * expf(-t * t * 60.0f);	//labio del cráter
	return fmaxf(h, -0.3f);
}

static float AlturaVolcanEn(glm::vec3 p)	//p relativo a CENTRO_PYROS
{
	glm::vec3 d = p - VOLCAN;
	return AlturaVolcan(sqrtf(d.x * d.x + d.z * d.z), atan2f(d.z, d.x));
}


//------------------------------------------------------------------ mallas propias
//Normales por promedio de triángulos, orientadas hacia arriba (sirve para terreno, volcán y tiras de lava)
static void NormalesHaciaArriba(std::vector<GLfloat>& v, const std::vector<unsigned int>& idx)
{
	std::vector<glm::vec3> n(v.size() / 8, glm::vec3(0.0f));
	for (size_t t = 0; t < idx.size(); t += 3)
	{
		glm::vec3 p[3];
		for (int k = 0; k < 3; k++) p[k] = glm::vec3(v[idx[t + k] * 8], v[idx[t + k] * 8 + 1], v[idx[t + k] * 8 + 2]);
		glm::vec3 c = glm::cross(p[1] - p[0], p[2] - p[0]);
		if (c.y < 0.0f) c = -c;
		for (int k = 0; k < 3; k++) n[idx[t + k]] += c;
	}
	for (size_t i = 0; i < n.size(); i++)
	{
		glm::vec3 m = glm::length(n[i]) > 0.0f ? glm::normalize(n[i]) : glm::vec3(0.0f, 1.0f, 0.0f);
		v[i * 8 + 5] = -m.x; v[i * 8 + 6] = -m.y; v[i * 8 + 7] = -m.z;	//hacia el interior, como en Geometria
	}
}

//Rejilla: filas x columnas de vértices ya agregados en v; dos triángulos por celda
static void IndicesRejilla(std::vector<unsigned int>& idx, int filas, int columnas)
{
	for (int i = 1; i < filas; i++)
		for (int k = 1; k < columnas; k++)
		{
			unsigned int a = (i - 1) * columnas + (k - 1), b = a + 1, c = i * columnas + (k - 1), d = c + 1;
			idx.insert(idx.end(), { a, c, b, b, c, d });
		}
}

static MeshModel* CrearTerreno()
{
	const float MEDIDA = 200.0f, PASO = 2.5f;
	const int N = (int)(2.0f * MEDIDA / PASO) + 1;
	std::vector<GLfloat> v;
	std::vector<unsigned int> idx;
	for (int i = 0; i < N; i++)
		for (int k = 0; k < N; k++)
		{
			float x = -MEDIDA + k * PASO, z = -MEDIDA + i * PASO;
			AgregarVertice(v, glm::vec3(x, AlturaTerreno(x, z), z), x / 10.0f, z / 10.0f, glm::vec3(0.0f));
		}
	IndicesRejilla(idx, N, N);
	NormalesHaciaArriba(v, idx);
	return GuardarMalla(v, idx);
}

static MeshModel* CrearVolcan()
{
	const int ANILLOS = 44, ANGULOS = 72;
	std::vector<GLfloat> v;
	std::vector<unsigned int> idx;
	for (int i = 0; i <= ANILLOS; i++)
	{
		float r = (RADIO_VOLCAN + 2.0f) * i / ANILLOS;
		for (int k = 0; k <= ANGULOS; k++)
		{
			float a = 2.0f * PI_P * (k % ANGULOS) / ANGULOS;	//la última columna repite la primera
			glm::vec3 p = VOLCAN + glm::vec3(r * cosf(a), AlturaVolcan(r, a), r * sinf(a));
			AgregarVertice(v, p, 12.0f * k / ANGULOS, r / 4.0f, glm::vec3(0.0f));
		}
	}
	IndicesRejilla(idx, ANILLOS + 1, ANGULOS + 1);
	NormalesHaciaArriba(v, idx);
	return GuardarMalla(v, idx);
}

//Tira de lava a lo largo de un camino de puntos (relativos a CENTRO_PYROS). u va de una orilla a la otra y v avanza con
//la distancia, así al mover la textura en v la lava corre en el sentido del camino. La altura de cada orilla la da
//la función alturaEn más un pequeño margen para que quede sobre la superficie
static MeshModel* CrearTiraLava(const std::vector<glm::vec3>& camino, const std::vector<float>& anchos, bool sobreVolcan)
{
	std::vector<GLfloat> v;
	std::vector<unsigned int> idx;
	float recorrido = 0.0f;
	for (size_t i = 0; i < camino.size(); i++)
	{
		glm::vec3 dir = camino[std::min(i + 1, camino.size() - 1)] - camino[i > 0 ? i - 1 : 0];
		dir.y = 0.0f;
		dir = glm::normalize(dir);
		glm::vec3 lado(-dir.z, 0.0f, dir.x);
		if (i > 0) recorrido += glm::length(camino[i] - camino[i - 1]);
		for (int k = 0; k < 2; k++)
		{
			glm::vec3 p = camino[i] + lado * ((k == 0 ? -0.5f : 0.5f) * anchos[i]);
			p.y = sobreVolcan ? fmaxf(AlturaVolcanEn(p), 0.0f) + 0.35f : 0.07f;
			AgregarVertice(v, p, (float)k, recorrido / 6.0f, glm::vec3(0.0f));
		}
	}
	IndicesRejilla(idx, (int)camino.size(), 2);
	NormalesHaciaArriba(v, idx);
	return GuardarMalla(v, idx);
}

static void CrearLava()
{
	//por la ladera: desde dentro del cráter hasta la base, con una pequeña ondulación
	std::vector<glm::vec3> camino;
	std::vector<float> anchos;
	const int N = 70;
	for (int i = 0; i <= N; i++)
	{
		float u = (float)i / N;
		float r = RADIO_CRATER * 0.9f + u * (RADIO_VOLCAN + 1.0f - RADIO_CRATER * 0.9f);
		float a = ANGULO_FLUJO + 0.1f * sinf(u * 8.0f);
		camino.push_back(VOLCAN + glm::vec3(r * cosf(a), 0.0f, r * sinf(a)));
		anchos.push_back(1.5f + 2.5f * u);
	}
	mFlujo = CrearTiraLava(camino, anchos, true);

	//río: de la base del volcán al lago, con una curva de Catmull-Rom (abierta) entre puntos de control
	glm::vec3 inicio = camino.back();
	glm::vec3 control[] = { inicio, glm::vec3(-8.0f, 0.0f, -52.0f), glm::vec3(10.0f, 0.0f, -43.0f),
		glm::vec3(28.0f, 0.0f, -52.0f), LAGO + glm::vec3(-RADIO_LAGO * 0.7f, 0.0f, 0.0f) };
	const int NC = sizeof(control) / sizeof(control[0]);
	camino.clear();
	anchos.clear();
	for (int s = 0; s < NC - 1; s++)
		for (int k = 0; k < 20; k++)
		{
			float u = k / 20.0f;
			glm::vec3 p0 = control[std::max(s - 1, 0)], p1 = control[s], p2 = control[s + 1], p3 = control[std::min(s + 2, NC - 1)];
			camino.push_back(0.5f * ((2.0f * p1) + (-p0 + p2) * u + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * u * u
				+ (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * u * u * u));
			anchos.push_back(4.0f + 2.0f * (s * 20 + k) / (20.0f * (NC - 1)));
		}
	camino.push_back(control[NC - 1]);
	anchos.push_back(6.0f);
	mRio = CrearTiraLava(camino, anchos, false);
}


//------------------------------------------------------------------ carga
void CargarPyros()
{
	texRoca = CargarTextura("Textures/roca_volcanica.png");
	texLava = CargarTextura("Textures/lava.png");
	texObsidiana = CargarTextura("Textures/obsidiana.png");
	texMetal = CargarTextura("Textures/metal.png");
	texPlataforma = CargarTextura("Textures/plataforma.png");
	texRunas = CargarTextura("Textures/runas.png");
	texCristal = CargarTextura("Textures/cristal.png");
	texLarva = CargarTextura("Textures/larva.png");
	texCaparazon = CargarTextura("Textures/caparazon_escarabajo.png");
	texColoso = CargarTextura("Textures/coloso.png");
	texHumo = CargarTextura("Textures/humo.png");
	texPlaneta = CargarTextura("Textures/pyros_planeta.png");
	texLuna = CargarTextura("Textures/luna_ceniza.png");
	texAsteroide = CargarTextura("Textures/asteroide.png");
	texPanel = CargarTextura("Textures/panel_solar.png");
	texBlanco = CargarTextura("Textures/plain.png");

	mTerreno = CrearTerreno();
	mVolcan = CrearVolcan();
	CrearLava();
	mHexagono = CrearTroncoCono(1.0f, 6);	//prisma hexagonal: con radio superior 1 el tronco de cono es un prisma
	for (int i = 0; i < 6; i++) mRocas[i] = CrearRoca(i + 1);
	rutaPyros.Preparar(PUNTOS_VUELO, sizeof(PUNTOS_VUELO) / sizeof(PUNTOS_VUELO[0]), 60);

	//rocas y grupos de cristales: se descartan los lugares que caen sobre algo
	unsigned int semilla = 2027;
	auto libre = [](glm::vec3 p, float margen) {
		if (glm::length(p - VOLCAN) < RADIO_VOLCAN + margen) return false;
		if (glm::length(p - LAGO) < RADIO_LAGO + 3.0f + margen) return false;
		if (glm::length(p - PLATAFORMA) < 17.0f + margen) return false;
		if (glm::length(p) < 26.0f + margen) return false;
		if (glm::length(p - COLOSO) < 7.0f + margen) return false;
		return true;
	};
	while (rocas.size() < 36)
	{
		float a = 2.0f * PI_P * Aleatorio(semilla), r = 25.0f + 82.0f * Aleatorio(semilla);
		glm::vec3 p(r * cosf(a), 0.0f, r * sinf(a));
		float escala = 0.6f + 1.8f * Aleatorio(semilla) * Aleatorio(semilla);
		float giro = 360.0f * Aleatorio(semilla), alto = 0.6f + 0.6f * Aleatorio(semilla);
		int tipo = (int)(Aleatorio(semilla) * 6.0f) % 6;
		if (libre(p, escala)) rocas.push_back({ p, escala, giro, alto, tipo });
	}
	while (cristales.size() < 14)
	{
		float a = 2.0f * PI_P * Aleatorio(semilla), r = 30.0f + 72.0f * Aleatorio(semilla);
		glm::vec3 p(r * cosf(a), 0.0f, r * sinf(a));
		float escala = 0.8f + 0.8f * Aleatorio(semilla), giro = 360.0f * Aleatorio(semilla);
		if (libre(p, 2.0f)) cristales.push_back({ p, escala, giro, 1.0f, 0 });
	}
}


//------------------------------------------------------------------ animaciones de teclado
void IniciarErupcion(float tiempo)
{
	if (tiempo - inicioErupcion > DURACION_ERUPCION) inicioErupcion = tiempo;
}

void IniciarEvolucion(float tiempo)
{
	if (tiempo - inicioEvolucion > DURACION_EVOLUCION) inicioEvolucion = tiempo;
}

//Intensidad de la erupción: sube rápido, se mantiene y se apaga despacio
static float IntensidadErupcion(float tiempo)
{
	float t = tiempo - inicioErupcion;
	if (t < 0.0f || t > DURACION_ERUPCION) return 0.0f;
	return Suave(0.0f, 0.4f, t) * (1.0f - Suave(4.5f, DURACION_ERUPCION, t));
}

//Pose de la nave verde de Pyros para el tiempo dado (posición relativa a CENTRO_PYROS)
static void PoseNavePyros(float tiempo, glm::vec3& pos, glm::vec3& frente, glm::vec3& techo, float& tren, float& potencia)
{
	glm::vec3 espera = PUNTOS_VUELO[0];
	glm::vec3 suelo = PLATAFORMA + glm::vec3(0.0f, ALTO_PLATAFORMA + ALTURA_TREN_NAVE, 0.0f);
	glm::vec3 frenteEspera, nada;
	rutaPyros.Pose(0.0f, 0.0f, nada, frenteEspera, techo);	//la nave se estaciona viendo hacia donde empieza el circuito
	frenteEspera.y = 0.0f;
	frenteEspera = glm::normalize(frenteEspera);
	float t = tiempo - inicioNave;
	techo = glm::vec3(0.0f, 1.0f, 0.0f);
	frente = frenteEspera;
	switch (estadoNave)
	{
	case NAVE_PLATAFORMA:
		pos = suelo;
		tren = 1.0f;
		potencia = 0.08f;	//motores en reposo: solo una llama pequeña
		break;
	case NAVE_DESPEGANDO:
		pos = glm::mix(suelo, espera, Suave(0.8f, DURACION_DESPEGUE, t));
		tren = 1.0f - Suave(1.0f, 2.5f, t);
		potencia = 0.2f + 0.8f * Suave(0.0f, 1.0f, t);
		break;
	case NAVE_VOLANDO:
	{
		//acelera los primeros segundos (s = v t^2 / 2T) y luego va a velocidad constante
		float s = (t < TIEMPO_ACELERACION) ? VELOCIDAD_VUELO * t * t / (2.0f * TIEMPO_ACELERACION)
			: VELOCIDAD_VUELO * (t - 0.5f * TIEMPO_ACELERACION);
		rutaPyros.Pose(s, 60.0f, pos, frente, techo);
		tren = 0.0f;
		potencia = 1.0f;
		break;
	}
	case NAVE_REGRESANDO:
	{
		//curva de Hermite: sale de donde estaba con su velocidad y llega al punto de espera casi detenida
		float u = fminf(t / DURACION_REGRESO, 1.0f), u2 = u * u, u3 = u2 * u;
		glm::vec3 m0 = regresoVelocidad * DURACION_REGRESO, m1 = frenteEspera * 4.0f;
		pos = (2 * u3 - 3 * u2 + 1) * regresoInicio + (u3 - 2 * u2 + u) * m0 + (-2 * u3 + 3 * u2) * espera + (u3 - u2) * m1;
		glm::vec3 d = (6 * u2 - 6 * u) * regresoInicio + (3 * u2 - 4 * u + 1) * m0 + (-6 * u2 + 6 * u) * espera + (3 * u2 - 2 * u) * m1;
		if (glm::length(d) > 0.01f) frente = glm::normalize(d);
		tren = Suave(0.7f, 1.0f, u);
		potencia = 1.0f - 0.5f * u;
		break;
	}
	case NAVE_ATERRIZANDO:
		pos = glm::mix(espera, suelo, Suave(0.0f, DURACION_ATERRIZAJE, t));
		tren = 1.0f;
		potencia = 0.5f - 0.42f * Suave(1.5f, DURACION_ATERRIZAJE, t);
		break;
	}
}

void AlternarNavePyros(float tiempo)
{
	if (estadoNave == NAVE_PLATAFORMA)
	{
		estadoNave = NAVE_DESPEGANDO;
		inicioNave = tiempo;
	}
	else if (estadoNave == NAVE_VOLANDO)
	{
		glm::vec3 pos, frente, techo;
		float tren, potencia;
		PoseNavePyros(tiempo, pos, frente, techo, tren, potencia);
		regresoInicio = pos;
		regresoVelocidad = frente * VELOCIDAD_VUELO;
		estadoNave = NAVE_REGRESANDO;
		inicioNave = tiempo;
	}
	//si está despegando, regresando o aterrizando se ignora la tecla hasta que termine
}

void ActualizarPyros(float tiempo)
{
	if (inicioEvolucion > -999.0f && tiempo - inicioEvolucion >= DURACION_EVOLUCION)
	{
		etapaAltar = (etapaAltar + 1) % 3;
		inicioEvolucion = -1000.0f;
	}
	float t = tiempo - inicioNave;
	if (estadoNave == NAVE_DESPEGANDO && t >= DURACION_DESPEGUE) { estadoNave = NAVE_VOLANDO; inicioNave += DURACION_DESPEGUE; }
	else if (estadoNave == NAVE_REGRESANDO && t >= DURACION_REGRESO) { estadoNave = NAVE_ATERRIZANDO; inicioNave += DURACION_REGRESO; }
	else if (estadoNave == NAVE_ATERRIZANDO && t >= DURACION_ATERRIZAJE) { estadoNave = NAVE_PLATAFORMA; inicioNave += DURACION_ATERRIZAJE; }
}

void FijarEstadoPyros(float erupcion, float evolucion, int etapa, int nave, float inicio)
{
	inicioErupcion = erupcion;
	inicioEvolucion = evolucion;
	etapaAltar = etapa % 3;
	estadoNave = nave;
	inicioNave = inicio;
	if (nave == NAVE_REGRESANDO)	//para las capturas: regresa desde el inicio del circuito
	{
		regresoInicio = rutaPyros.Punto(300.0f);
		regresoVelocidad = glm::normalize(rutaPyros.Punto(302.0f) - regresoInicio) * VELOCIDAD_VUELO;
	}
}


//------------------------------------------------------------------ luces
void LucesPyros(float tiempo, bool deNoche, bool reflectores, PointLight* puntuales, unsigned int& np,
	SpotLight* spots, unsigned int& ns)
{
	glm::vec3 p;
	//luminarias: se prenden todas juntas cuando empieza la noche
	if (deNoche)
		for (int i = 0; i < 6 && np < MAX_POINT_LIGHTS; i++)
		{
			p = CENTRO_PYROS + LUMINARIAS[i] + glm::vec3(0.0f, ALTO_CRISTAL_LUMINARIA, 0.0f);
			puntuales[np++] = PointLight(1.0f, 0.62f, 0.28f, 0.05f, 3.0f, p.x, p.y, p.z, 1.0f, 0.06f, 0.008f);
		}
	//cráter: siempre encendido; durante la erupción crece y parpadea
	float e = IntensidadErupcion(tiempo);
	p = CENTRO_PYROS + VOLCAN + glm::vec3(0.0f, ALTURA_VOLCAN + 3.0f, 0.0f);
	if (np < MAX_POINT_LIGHTS)
		puntuales[np++] = PointLight(1.0f, 0.4f, 0.1f, 0.0f, 1.6f + e * (5.0f + 1.5f * sinf(tiempo * 25.0f)), p.x, p.y, p.z, 1.0f, 0.012f, 0.0004f);
	//lago de lava
	p = CENTRO_PYROS + LAGO + glm::vec3(0.0f, 3.0f, 0.0f);
	if (np < MAX_POINT_LIGHTS)
		puntuales[np++] = PointLight(1.0f, 0.35f, 0.05f, 0.0f, 2.0f, p.x, p.y, p.z, 1.0f, 0.04f, 0.002f);
	//destello de la evolución: llega a su máximo cuando cambia la etapa (t = 2.4 s)
	float tv = tiempo - inicioEvolucion;
	if (tv >= 0.0f && tv <= DURACION_EVOLUCION && np < MAX_POINT_LIGHTS)
	{
		float k = Suave(0.0f, 2.4f, tv) * (1.0f - Suave(2.4f, DURACION_EVOLUCION, tv));
		p = CENTRO_PYROS + glm::vec3(0.0f, ALTO_ALTAR + 2.5f, 0.0f);
		puntuales[np++] = PointLight(0.7f, 0.85f, 1.0f, 0.0f, 6.0f * k, p.x, p.y, p.z, 1.0f, 0.08f, 0.01f);
	}
	//reflectores de la plataforma: desde la cabeza de cada torre hacia el centro de la plataforma
	if (reflectores)
		for (int i = 0; i < 2 && ns < MAX_SPOT_LIGHTS; i++)
		{
			glm::vec3 cabeza = CENTRO_PYROS + TORRES[i] + glm::vec3(0.0f, ALTO_TORRE, 0.0f);
			glm::vec3 d = glm::normalize(CENTRO_PYROS + PLATAFORMA + glm::vec3(0.0f, 1.0f, 0.0f) - cabeza);
			spots[ns++] = SpotLight(1.0f, 0.95f, 0.85f, 0.0f, 3.0f, cabeza.x, cabeza.y, cabeza.z, d.x, d.y, d.z,
				1.0f, 0.01f, 0.0008f, 24.0f);
		}
}


glm::vec3 ResolverColisionPyros(glm::vec3 p, float radio)
{
	struct Obstaculo { glm::vec3 c; float r; };
	const Obstaculo obs[] = {
		{ VOLCAN, RADIO_VOLCAN - 8.0f }, { LAGO, RADIO_LAGO }, { glm::vec3(0.0f), 4.0f }, { COLOSO, 4.5f },
		{ PEDESTALES[0], 2.4f }, { PEDESTALES[1], 2.4f }, { PEDESTALES[2], 2.4f }, { TORRES[0], 1.0f }, { TORRES[1], 1.0f },
		{ LUMINARIAS[0], 0.7f }, { LUMINARIAS[1], 0.7f }, { LUMINARIAS[2], 0.7f }, { LUMINARIAS[3], 0.7f },
		{ LUMINARIAS[4], 0.7f }, { LUMINARIAS[5], 0.7f } };
	glm::vec3 local = p - CENTRO_PYROS;
	for (const Obstaculo& o : obs)
	{
		glm::vec2 d(local.x - o.c.x, local.z - o.c.z);
		float dist = glm::length(d), minimo = o.r + radio;
		if (dist < minimo && dist > 0.001f)
		{
			d *= minimo / dist;
			local.x = o.c.x + d.x;
			local.z = o.c.z + d.y;
		}
	}
	return CENTRO_PYROS + local;
}


//------------------------------------------------------------------ seres de Pyros (diseños propios)
//En el museo se dibujan como estatuas de obsidiana: misma jerarquía, otra textura, sin brillo propio y sin moverse
static bool modoEstatua = false;
static void Tex(Texture* t) { (modoEstatua ? texObsidiana : t)->UseTexture(); }
static float Em(float e) { return modoEstatua ? 0.0f : e; }

//ETAPA 1, larva de magma: seis segmentos detrás de la cabeza. Una onda viaja de la cabeza a la cola (cada segmento
//va 0.8 rad detrás del anterior) moviéndolos de lado y levantándolos un poco: así se arrastra
static void DibujarLarva(const glm::mat4& base, float t, float brillo)
{
	const float RADIOS[6] = { 0.32f, 0.3f, 0.27f, 0.23f, 0.18f, 0.13f };
	glm::mat4 cabeza(1.0f), m;
	UsarMaterial(matBrillante);
	for (int i = 0; i < 6; i++)
	{
		float onda = t * 5.0f - i * 0.8f;
		glm::vec3 p(0.12f * sinf(onda), RADIOS[i] * 0.85f + 0.06f * fmaxf(0.0f, sinf(onda + 1.5f)), -i * 0.27f);
		m = glm::translate(base, p);
		if (i == 0) cabeza = m;
		Tex(texLarva);
		Dibujar(mEsfera, glm::scale(m, glm::vec3(RADIOS[i], RADIOS[i] * 0.85f, RADIOS[i] * 1.1f)), glm::vec3(1.0f), Em(0.5f + brillo));
	}
	for (int i = 0; i < 2; i++)
	{
		float lado = (i == 0) ? 1.0f : -1.0f;
		//antenas con punta brillante
		m = glm::translate(cabeza, glm::vec3(lado * 0.12f, 0.2f, 0.12f));
		m = glm::rotate(m, glm::radians(-lado * 30.0f + 10.0f * sinf(t * 4.0f + i)), glm::vec3(0.0f, 0.0f, 1.0f));
		m = glm::rotate(m, glm::radians(25.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		Tex(texObsidiana);
		Dibujar(mCilindro, glm::scale(glm::translate(m, glm::vec3(0.0f, 0.15f, 0.0f)), glm::vec3(0.025f, 0.3f, 0.025f)), glm::vec3(1.0f), Em(brillo));
		Tex(texCristal);
		Dibujar(mEsferaBaja, glm::scale(glm::translate(m, glm::vec3(0.0f, 0.32f, 0.0f)), glm::vec3(0.05f)), glm::vec3(1.0f, 0.9f, 0.4f), Em(1.5f + brillo));
		//ojos
		Tex(texObsidiana);
		Dibujar(mEsferaBaja, glm::scale(glm::translate(cabeza, glm::vec3(lado * 0.13f, 0.08f, 0.27f)), glm::vec3(0.06f)), glm::vec3(1.0f), Em(brillo));
	}
}

//ETAPA 2, escarabajo de lava: caparazón con manchas que brillan, mandíbulas, antenas y seis patas. Camina en trípode:
//delantera y trasera de un lado con la de en medio del otro lado se mueven juntas, y el otro trío va medio ciclo atrás
static void DibujarEscarabajo(const glm::mat4& base, float fase, float t, float brillo)
{
	glm::mat4 cuerpo, cabeza, m, pieza;
	cuerpo = glm::translate(base, glm::vec3(0.0f, 0.85f + 0.03f * sinf(2.0f * fase), 0.0f));
	UsarMaterial(matBrillante);
	Tex(texCaparazon);
	Dibujar(mEsfera, glm::scale(cuerpo, glm::vec3(0.9f, 0.55f, 1.2f)), glm::vec3(1.0f), Em(0.25f + brillo));
	Tex(texObsidiana);
	Dibujar(mEsfera, glm::scale(glm::translate(cuerpo, glm::vec3(0.0f, -0.15f, 0.0f)), glm::vec3(0.75f, 0.35f, 1.0f)), glm::vec3(1.0f), Em(brillo));
	//CABEZA -> mandíbulas y antenas
	cabeza = glm::translate(cuerpo, glm::vec3(0.0f, -0.1f, 1.15f));
	Dibujar(mEsfera, glm::scale(cabeza, glm::vec3(0.45f, 0.35f, 0.42f)), glm::vec3(1.0f), Em(brillo));
	for (int i = 0; i < 2; i++)
	{
		float lado = (i == 0) ? 1.0f : -1.0f;
		m = glm::translate(cabeza, glm::vec3(lado * 0.18f, -0.08f, 0.3f));
		m = glm::rotate(m, glm::radians(-lado * (20.0f + 12.0f * sinf(t * 3.0f))), glm::vec3(0.0f, 1.0f, 0.0f));
		m = glm::rotate(m, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		Tex(texObsidiana);
		Dibujar(mCono, glm::scale(m, glm::vec3(0.08f, 0.45f, 0.08f)), glm::vec3(1.0f), Em(brillo));
		m = glm::translate(cabeza, glm::vec3(lado * 0.15f, 0.22f, 0.22f));
		m = glm::rotate(m, glm::radians(-lado * 30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		m = glm::rotate(m, glm::radians(35.0f + 8.0f * sinf(t * 2.0f + i)), glm::vec3(1.0f, 0.0f, 0.0f));
		Dibujar(mCilindro, glm::scale(glm::translate(m, glm::vec3(0.0f, 0.3f, 0.0f)), glm::vec3(0.025f, 0.6f, 0.025f)), glm::vec3(1.0f), Em(brillo));
		Tex(texCristal);
		Dibujar(mEsferaBaja, glm::scale(glm::translate(m, glm::vec3(0.0f, 0.62f, 0.0f)), glm::vec3(0.06f)), glm::vec3(1.0f, 0.85f, 0.3f), Em(1.5f + brillo));
	}
	//PATAS: cadera -> fémur (hacia afuera y arriba) -> tibia (hacia abajo hasta el piso)
	Tex(texObsidiana);
	for (int i = 0; i < 2; i++)
	{
		float lado = (i == 0) ? 1.0f : -1.0f;
		for (int k = 0; k < 3; k++)
		{
			float grupo = ((k % 2 == 0) == (lado > 0.0f)) ? 0.0f : PI_P;
			float a = fase + grupo;
			float balanceo = 25.0f * sinf(a), levanta = 20.0f * fmaxf(0.0f, cosf(a));
			m = glm::translate(cuerpo, glm::vec3(lado * 0.7f, -0.15f, 0.55f - 0.55f * k));
			m = glm::rotate(m, glm::radians(-lado * balanceo), glm::vec3(0.0f, 1.0f, 0.0f));
			m = glm::rotate(m, glm::radians(lado * (35.0f + levanta)), glm::vec3(0.0f, 0.0f, 1.0f));
			pieza = glm::translate(m, glm::vec3(lado * 0.325f, 0.0f, 0.0f));
			pieza = glm::rotate(pieza, glm::radians(-lado * 90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			Dibujar(mCilindro, glm::scale(pieza, glm::vec3(0.07f, 0.65f, 0.07f)), glm::vec3(1.0f), Em(brillo));
			m = glm::translate(m, glm::vec3(lado * 0.65f, 0.0f, 0.0f));
			m = glm::rotate(m, glm::radians(-lado * (110.0f + 0.5f * levanta)), glm::vec3(0.0f, 0.0f, 1.0f));
			Dibujar(mEsferaBaja, glm::scale(m, glm::vec3(0.09f)), glm::vec3(1.0f), Em(brillo));
			pieza = glm::translate(m, glm::vec3(lado * 0.55f, 0.0f, 0.0f));
			pieza = glm::rotate(pieza, glm::radians(-lado * 90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			Dibujar(mCilindro, glm::scale(pieza, glm::vec3(0.055f, 1.1f, 0.055f)), glm::vec3(1.0f), Em(brillo));
		}
	}
}

//ETAPA 3, coloso de basalto: torso de roca que respira, núcleo de lava en el pecho, cristales en el lomo, cabeza que
//voltea despacio y cuatro patas gruesas que cambian el peso de una a otra
static void DibujarColoso(const glm::mat4& base, float t, float brillo)
{
	float respira = sinf(t * 1.1f);
	glm::mat4 torso, cabeza, m;
	torso = glm::translate(base, glm::vec3(0.0f, 4.3f + 0.08f * respira, 0.0f));
	UsarMaterial(matOpaco);
	Tex(texColoso);
	Dibujar(mRocas[4], glm::scale(torso, glm::vec3(2.4f, 1.9f * (1.0f + 0.02f * respira), 3.2f)), glm::vec3(1.0f), Em(brillo));
	//núcleo de lava
	Tex(texLava);
	Dibujar(mEsfera, glm::scale(glm::translate(torso, glm::vec3(0.0f, -0.2f, 2.45f)), glm::vec3(0.95f * (1.0f + 0.08f * respira))),
		glm::vec3(1.0f), Em(1.4f + 0.4f * respira + brillo));
	//cristales del lomo
	const glm::vec3 CRISTALES[5] = { {0.0f, 1.0f, 1.4f}, {0.7f, 1.0f, 0.4f}, {-0.7f, 1.0f, 0.4f}, {0.0f, 1.1f, -0.6f}, {0.5f, 0.9f, -1.6f} };
	Tex(texCristal);
	for (int i = 0; i < 5; i++)
	{
		m = glm::translate(torso, CRISTALES[i]);
		m = glm::rotate(m, glm::radians(CRISTALES[i].x * 25.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		m = glm::rotate(m, glm::radians(-15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		Dibujar(mCono, glm::scale(m, glm::vec3(0.35f, 1.6f - 0.15f * i, 0.35f)), glm::vec3(1.0f, 0.6f, 0.3f), Em(0.6f + brillo));
	}
	//CABEZA: voltea de un lado a otro y asiente
	cabeza = glm::translate(torso, glm::vec3(0.0f, 0.9f, 3.1f));
	cabeza = glm::rotate(cabeza, glm::radians(20.0f * sinf(t * 0.4f)), glm::vec3(0.0f, 1.0f, 0.0f));
	cabeza = glm::rotate(cabeza, glm::radians(5.0f * sinf(t * 0.7f)), glm::vec3(1.0f, 0.0f, 0.0f));
	Tex(texColoso);
	Dibujar(mRocas[2], glm::scale(glm::translate(cabeza, glm::vec3(0.0f, 0.0f, 0.6f)), glm::vec3(1.1f, 0.9f, 1.25f)), glm::vec3(1.0f), Em(brillo));
	Tex(texCristal);
	for (int i = 0; i < 2; i++)
		Dibujar(mEsferaBaja, glm::scale(glm::translate(cabeza, glm::vec3((i == 0 ? 0.42f : -0.42f), 0.15f, 1.65f)), glm::vec3(0.17f)),
			glm::vec3(1.0f, 0.9f, 0.4f), Em(1.8f + brillo));
	//PATAS: cadera -> pierna -> rodilla -> pierna baja -> pie
	const glm::vec3 CADERAS[4] = { {1.7f, -0.95f, 2.0f}, {-1.7f, -0.95f, 2.0f}, {1.7f, -0.95f, -2.0f}, {-1.7f, -0.95f, -2.0f} };
	for (int i = 0; i < 4; i++)
	{
		m = glm::translate(torso, CADERAS[i]);
		m = glm::rotate(m, glm::radians(5.0f * sinf(t * 0.5f + i * 1.6f)), glm::vec3(1.0f, 0.0f, 0.0f));
		Tex(texColoso);
		Dibujar(mCilindro, glm::scale(glm::translate(m, glm::vec3(0.0f, -0.9f, 0.0f)), glm::vec3(0.75f, 1.8f, 0.75f)), glm::vec3(1.0f), Em(brillo));
		Dibujar(mRocas[1], glm::scale(glm::translate(m, glm::vec3(0.0f, -1.8f, 0.0f)), glm::vec3(0.8f)), glm::vec3(1.0f), Em(brillo));
		Dibujar(mCilindro, glm::scale(glm::translate(m, glm::vec3(0.0f, -2.7f, 0.0f)), glm::vec3(0.65f, 1.8f, 0.65f)), glm::vec3(1.0f), Em(brillo));
		Dibujar(mRocas[3], glm::scale(glm::translate(m, glm::vec3(0.0f, -3.4f, 0.2f)), glm::vec3(0.9f, 0.35f, 1.1f)), glm::vec3(1.0f), Em(brillo));
	}
}

//Dibuja la etapa pedida; las tres miden distinto, escala ajusta su tamaño
static void DibujarEtapa(int etapa, const glm::mat4& base, float t, float brillo)
{
	if (etapa == 0) DibujarLarva(glm::scale(base, glm::vec3(1.4f)), t, brillo);
	else if (etapa == 1) DibujarEscarabajo(base, t * 4.0f, t, brillo);
	else DibujarColoso(glm::scale(base, glm::vec3(0.42f)), t, brillo);
}


//------------------------------------------------------------------ superficie
static void DibujarLuminaria(glm::vec3 lugar, float t, bool deNoche, int i)
{
	glm::mat4 base = Lugar(lugar), m;
	UsarMaterial(matMetal);
	texMetal->UseTexture();
	Dibujar(mCilindro, glm::scale(glm::translate(base, glm::vec3(0.0f, 0.2f, 0.0f)), glm::vec3(0.55f, 0.4f, 0.55f)), glm::vec3(0.7f));
	texObsidiana->UseTexture();
	Dibujar(mCilindro, glm::scale(glm::translate(base, glm::vec3(0.0f, 2.1f, 0.0f)), glm::vec3(0.2f, 3.6f, 0.2f)), glm::vec3(1.0f));
	//soporte: cuatro brazos de metal que sostienen el cristal
	texMetal->UseTexture();
	for (int k = 0; k < 4; k++)
	{
		m = glm::rotate(glm::translate(base, glm::vec3(0.0f, 3.9f, 0.0f)), glm::radians(45.0f + 90.0f * k), glm::vec3(0.0f, 1.0f, 0.0f));
		m = glm::rotate(glm::translate(m, glm::vec3(0.0f, 0.0f, 0.3f)), glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		Dibujar(mCubo, glm::scale(m, glm::vec3(0.08f, 0.8f, 0.08f)), glm::vec3(0.6f));
	}
	//cristal: dos conos unidos por la base; gira y flota. De noche brilla (la luz puntual está en su centro)
	m = glm::translate(base, glm::vec3(0.0f, ALTO_CRISTAL_LUMINARIA + 0.12f * sinf(t * 1.5f + i), 0.0f));
	m = glm::rotate(m, glm::radians(t * 40.0f + i * 30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	texCristal->UseTexture();
	float brillo = deNoche ? 1.8f : 0.15f;
	Dibujar(mCono, glm::scale(m, glm::vec3(0.35f, 0.6f, 0.35f)), glm::vec3(1.0f, 0.75f, 0.4f), brillo);
	Dibujar(mCono, glm::scale(glm::rotate(m, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)), glm::vec3(0.35f, 0.45f, 0.35f)), glm::vec3(1.0f, 0.75f, 0.4f), brillo);
}

static void DibujarPlataformaYNave(float t)
{
	glm::mat4 base = Lugar(PLATAFORMA), m;
	UsarMaterial(matMetal);
	texPlataforma->UseTexture();
	Dibujar(mHexagono, glm::scale(base, glm::vec3(RADIO_PLATAFORMA, ALTO_PLATAFORMA, RADIO_PLATAFORMA)), glm::vec3(1.0f));
	//luces de las esquinas: se encienden una tras otra alrededor de la plataforma
	texCristal->UseTexture();
	for (int k = 0; k < 6; k++)
	{
		float a = 2.0f * PI_P * k / 6.0f;
		float encendida = (int)(t * 6.0f) % 6 == k ? 2.2f : 0.25f;
		Dibujar(mEsferaBaja, glm::scale(glm::translate(base, glm::vec3(RADIO_PLATAFORMA * 0.93f * cosf(a), ALTO_PLATAFORMA + 0.1f,
			RADIO_PLATAFORMA * 0.93f * sinf(a))), glm::vec3(0.25f)), glm::vec3(1.0f, 0.9f, 0.2f), encendida);
	}
	//torres de los reflectores: poste y cabeza que apunta al centro de la plataforma
	for (int i = 0; i < 2; i++)
	{
		glm::mat4 torre = Lugar(TORRES[i]);
		texMetal->UseTexture();
		Dibujar(mCilindro, glm::scale(glm::translate(torre, glm::vec3(0.0f, 0.5f * ALTO_TORRE, 0.0f)), glm::vec3(0.25f, ALTO_TORRE, 0.25f)), glm::vec3(0.75f));
		glm::vec3 d = PLATAFORMA + glm::vec3(0.0f, 1.0f, 0.0f) - (TORRES[i] + glm::vec3(0.0f, ALTO_TORRE, 0.0f));
		m = glm::translate(torre, glm::vec3(0.0f, ALTO_TORRE, 0.0f));
		m = glm::rotate(m, atan2f(d.x, d.z), glm::vec3(0.0f, 1.0f, 0.0f));
		m = glm::rotate(m, -atan2f(d.y, sqrtf(d.x * d.x + d.z * d.z)), glm::vec3(1.0f, 0.0f, 0.0f));
		Dibujar(mCubo, glm::scale(m, glm::vec3(1.2f, 0.8f, 1.4f)), glm::vec3(0.5f));
		texCristal->UseTexture();
		Dibujar(mCilindro, glm::scale(glm::rotate(glm::translate(m, glm::vec3(0.0f, 0.0f, 0.72f)), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::vec3(0.45f, 0.05f, 0.45f)), glm::vec3(1.0f, 0.95f, 0.85f), 1.5f);
	}
	//nave verde
	glm::vec3 pos, frente, techo;
	float tren, potencia;
	PoseNavePyros(t, pos, frente, techo, tren, potencia);
	DibujarNaveVerde(MatrizDesdeEjes(CENTRO_PYROS + pos, frente, techo, 1.0f), tren, potencia, t);
}

static void DibujarErupcion(float t)
{
	float te = t - inicioErupcion;
	if (te < 0.0f || te > DURACION_ERUPCION) return;
	//bombas de lava: tiro parabólico desde el cráter, con salida escalonada; desaparecen al caer bajo la ladera
	const int BOMBAS = 36;
	const float GRAVEDAD = 14.0f;
	glm::vec3 crater = VOLCAN + glm::vec3(0.0f, ALTURA_VOLCAN, 0.0f);
	texLava->UseTexture();
	UsarMaterial(matOpaco);
	for (int i = 0; i < BOMBAS; i++)
	{
		unsigned int semilla = 77u + i * 13u;
		float salida = i * 0.1f;
		float azimut = 2.0f * PI_P * Aleatorio(semilla);
		float elevacion = glm::radians(58.0f + 27.0f * Aleatorio(semilla));
		float rapidez = 20.0f + 14.0f * Aleatorio(semilla);
		float tamano = 0.8f + 0.9f * Aleatorio(semilla);
		for (int estela = 0; estela < 3; estela++)	//la bomba y dos puntos anteriores más chicos
		{
			float tv = te - salida - estela * 0.06f;
			if (tv <= 0.0f) continue;
			glm::vec3 v(rapidez * cosf(elevacion) * cosf(azimut), rapidez * sinf(elevacion), rapidez * cosf(elevacion) * sinf(azimut));
			glm::vec3 p = crater + v * tv + glm::vec3(0.0f, -0.5f * GRAVEDAD * tv * tv, 0.0f);
			if (p.y < fmaxf(AlturaVolcanEn(p), 0.0f)) continue;
			Dibujar(mEsferaBaja, glm::scale(Lugar(p), glm::vec3(tamano * (1.0f - 0.3f * estela))), glm::vec3(1.0f, 0.8f, 0.5f), 1.8f);
		}
	}
}

void DibujarSuperficiePyros(float t, bool deNoche)
{
	glm::mat4 m;
	glm::mat4 origen = Lugar(glm::vec3(0.0f));

	//terreno y volcán
	UsarMaterial(matOpaco);
	texRoca->UseTexture();
	Dibujar(mTerreno, origen, glm::vec3(1.0f));
	Dibujar(mVolcan, origen, glm::vec3(0.95f, 0.88f, 0.85f));

	//lava: misma textura movida en v con el tiempo (desplazamientoUV), así parece que corre
	float e = IntensidadErupcion(t);
	texLava->UseTexture();
	glUniform2f(U.desplazamientoUV, 0.0f, -0.25f * t);
	Dibujar(mFlujo, origen, glm::vec3(1.0f), 1.1f + e);
	Dibujar(mRio, origen, glm::vec3(1.0f), 1.1f);
	glUniform2f(U.desplazamientoUV, 0.03f * t, 0.05f * t);
	Dibujar(mCilindro, glm::scale(Lugar(LAGO), glm::vec3(RADIO_LAGO, 0.12f, RADIO_LAGO)), glm::vec3(1.0f), 1.0f);
	Dibujar(mCilindro, glm::scale(Lugar(VOLCAN + glm::vec3(0.0f, ALTURA_VOLCAN - 2.85f + 1.2f * e, 0.0f)), glm::vec3(4.7f, 0.2f, 4.7f)),
		glm::vec3(1.0f), 1.2f + 1.5f * e);
	glUniform2f(U.desplazamientoUV, 0.0f, 0.0f);
	//burbujas del lago: cada una nace, crece y revienta en 2.2 s; empiezan en momentos distintos
	for (int i = 0; i < 9; i++)
	{
		unsigned int semilla = 300u + i * 7u;
		float a = 2.0f * PI_P * Aleatorio(semilla), r = RADIO_LAGO * 0.75f * sqrtf(Aleatorio(semilla));
		float vida = fmodf(t / 2.2f + Aleatorio(semilla), 1.0f);
		float s = 0.9f * Suave(0.0f, 0.8f, vida) * (1.0f - Suave(0.85f, 1.0f, vida));
		if (s <= 0.01f) continue;
		m = Lugar(LAGO + glm::vec3(r * cosf(a), 0.06f, r * sinf(a)));
		Dibujar(mEsferaBaja, glm::scale(m, glm::vec3(s, s * 0.7f, s)), glm::vec3(1.0f), 1.3f);
	}
	//orilla del lago: rocas alrededor
	texRoca->UseTexture();
	for (int k = 0; k < 16; k++)
	{
		float a = 2.0f * PI_P * k / 16.0f;
		m = Lugar(LAGO + glm::vec3((RADIO_LAGO + 0.6f) * cosf(a), 0.2f, (RADIO_LAGO + 0.6f) * sinf(a)));
		m = glm::rotate(m, a * 3.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		Dibujar(mRocas[k % 6], glm::scale(m, glm::vec3(1.6f, 0.9f, 1.3f)), glm::vec3(0.8f));
	}

	//decoración
	for (const Decoracion& d : rocas)
	{
		m = glm::rotate(Lugar(d.pos + glm::vec3(0.0f, 0.3f * d.escala, 0.0f)), glm::radians(d.giro), glm::vec3(0.0f, 1.0f, 0.0f));
		Dibujar(mRocas[d.tipo], glm::scale(m, glm::vec3(d.escala, d.escala * d.alto, d.escala)), glm::vec3(0.85f));
	}
	UsarMaterial(matBrillante);
	texObsidiana->UseTexture();
	for (const Decoracion& c : cristales)
		for (int k = 0; k < 3; k++)
		{
			m = glm::rotate(Lugar(c.pos), glm::radians(c.giro + 120.0f * k), glm::vec3(0.0f, 1.0f, 0.0f));
			m = glm::rotate(glm::translate(m, glm::vec3(0.0f, 0.0f, 0.4f * k)), glm::radians(12.0f + 14.0f * k), glm::vec3(1.0f, 0.0f, 0.0f));
			Dibujar(mCono, glm::scale(m, glm::vec3(0.45f, 2.6f - 0.5f * k, 0.45f) * c.escala), glm::vec3(0.9f, 0.8f, 1.0f), deNoche ? 0.15f : 0.0f);
		}

	//plaza: círculo de runas (brilla un poco de noche) y altar
	UsarMaterial(matOpaco);
	texRunas->UseTexture();
	Dibujar(mCilindro, glm::scale(Lugar(glm::vec3(0.0f, 0.0f, 0.0f)), glm::vec3(RADIO_PLAZA, 0.1f, RADIO_PLAZA)), glm::vec3(1.0f), deNoche ? 0.35f : 0.0f);
	texObsidiana->UseTexture();
	Dibujar(mHexagono, glm::scale(Lugar(glm::vec3(0.0f)), glm::vec3(3.5f, ALTO_ALTAR, 3.5f)), glm::vec3(1.0f));
	texRunas->UseTexture();
	Dibujar(mCilindro, glm::scale(Lugar(glm::vec3(0.0f, ALTO_ALTAR + 0.02f, 0.0f)), glm::vec3(3.0f, 0.06f, 3.0f)), glm::vec3(1.0f), 0.6f);

	//ser del altar: gira despacio; al evolucionar gira cada vez más rápido, brilla y alterna entre su forma actual y
	//la siguiente cada vez más seguido (como un parpadeo) hasta quedarse con la nueva
	float tv = t - inicioEvolucion;
	bool evolucionando = tv >= 0.0f && tv <= DURACION_EVOLUCION;
	int etapa = etapaAltar;
	float brillo = 0.0f, giro = t * 20.0f;
	if (evolucionando)
	{
		brillo = 2.5f * Suave(0.0f, 1.2f, tv) * (1.0f - Suave(2.4f, DURACION_EVOLUCION, tv));
		giro += 200.0f * tv * tv;
		int siguiente = (etapaAltar + 1) % 3;
		if (tv >= 2.4f) etapa = siguiente;
		else if (tv >= 1.2f) etapa = sinf(tv * PI_P * (4.0f + 10.0f * (tv - 1.2f))) > 0.0f ? etapaAltar : siguiente;
	}
	m = glm::rotate(Lugar(glm::vec3(0.0f, ALTO_ALTAR + 0.08f, 0.0f)), glm::radians(giro), glm::vec3(0.0f, 1.0f, 0.0f));
	DibujarEtapa(etapa, m, t, brillo);

	//museo de la evolución: estatuas de las tres etapas sobre pedestales y flechas que brillan entre ellos
	for (int i = 0; i < 3; i++)
	{
		UsarMaterial(matMetal);
		texObsidiana->UseTexture();
		Dibujar(mCilindro, glm::scale(Lugar(PEDESTALES[i] + glm::vec3(0.0f, 0.6f, 0.0f)), glm::vec3(2.0f, 1.2f, 2.0f)), glm::vec3(1.0f));
		texMetal->UseTexture();
		Dibujar(mCilindro, glm::scale(Lugar(PEDESTALES[i] + glm::vec3(0.0f, 1.22f, 0.0f)), glm::vec3(2.1f, 0.06f, 2.1f)), glm::vec3(0.8f));
		modoEstatua = true;
		glm::vec3 haciaPlaza = -glm::normalize(PEDESTALES[i]);
		m = glm::rotate(Lugar(PEDESTALES[i] + glm::vec3(0.0f, 1.25f, 0.0f)), atan2f(haciaPlaza.x, haciaPlaza.z), glm::vec3(0.0f, 1.0f, 0.0f));
		DibujarEtapa(i, m, 0.0f, 0.0f);
		modoEstatua = false;
		if (i < 2)
		{
			glm::vec3 a = PEDESTALES[i], b = PEDESTALES[i + 1], d = glm::normalize(b - a);
			m = Lugar(0.5f * (a + b) + glm::vec3(0.0f, 0.4f, 0.0f));
			m = glm::rotate(m, atan2f(d.x, d.z), glm::vec3(0.0f, 1.0f, 0.0f));
			m = glm::rotate(m, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			texCristal->UseTexture();
			Dibujar(mCono, glm::scale(m, glm::vec3(0.4f, 1.2f, 0.15f)), glm::vec3(1.0f, 0.6f, 0.2f), 0.8f + 0.6f * sinf(t * 3.0f - i));
		}
	}

	//seres que viven en Pyros (animación continua)
	//larvas: dan vueltas alrededor del lago de lava
	for (int i = 0; i < 5; i++)
	{
		float a = -0.35f * t + i * 2.0f * PI_P / 5.0f, r = RADIO_LAGO + 3.5f;
		glm::vec3 p = LAGO + glm::vec3(r * cosf(a), 0.0f, r * sinf(a));
		glm::vec3 v(sinf(a), 0.0f, -cosf(a));	//derivada respecto al tiempo (sentido de avance)
		m = glm::rotate(Lugar(p), atan2f(v.x, v.z), glm::vec3(0.0f, 1.0f, 0.0f));
		DibujarLarva(glm::scale(m, glm::vec3(1.3f)), t + i, 0.0f);
	}
	//escarabajos: cada uno recorre una elipse; su frente sigue la derivada de la trayectoria
	const glm::vec4 ELIPSES[3] = { {-48.0f, 30.0f, 14.0f, 9.0f}, {33.0f, 0.0f, 9.0f, 7.0f}, {-5.0f, 58.0f, 20.0f, 7.0f} };
	const float RAPIDEZ[3] = { 0.18f, 0.25f, 0.14f };
	for (int i = 0; i < 3; i++)
	{
		float a = RAPIDEZ[i] * t + i * 2.0f;
		glm::vec3 p(ELIPSES[i].x + ELIPSES[i].z * sinf(a), 0.0f, ELIPSES[i].y + ELIPSES[i].w * cosf(a));
		glm::vec3 v(ELIPSES[i].z * cosf(a), 0.0f, -ELIPSES[i].w * sinf(a));
		m = glm::rotate(Lugar(p), atan2f(v.x, v.z), glm::vec3(0.0f, 1.0f, 0.0f));
		DibujarEscarabajo(glm::scale(m, glm::vec3(1.2f)), t * 7.0f + i, t + i, 0.0f);
	}
	//coloso: mira hacia la plaza
	m = glm::rotate(Lugar(COLOSO), atan2f(-COLOSO.x, -COLOSO.z), glm::vec3(0.0f, 1.0f, 0.0f));
	DibujarColoso(m, t, 0.0f);

	for (int i = 0; i < 6; i++) DibujarLuminaria(LUMINARIAS[i], t, deNoche, i);
	DibujarPlataformaYNave(t);
	DibujarErupcion(t);
}


//Cartel: plano que siempre mira a la cámara (se usa para el humo). Sus ejes salen de la matriz de vista
static glm::mat4 Cartel(glm::vec3 p, const glm::mat4& vista, float tam)
{
	glm::vec3 derecha(vista[0][0], vista[1][0], vista[2][0]);
	glm::vec3 arriba(vista[0][1], vista[1][1], vista[2][1]);
	glm::vec3 haciaOjo = glm::cross(derecha, arriba);
	return glm::mat4(glm::vec4(derecha * tam, 0.0f), glm::vec4(haciaOjo * tam, 0.0f), glm::vec4(-arriba * tam, 0.0f), glm::vec4(p, 1.0f));
}

void DibujarEfectosPyros(float t, const glm::mat4& vista, glm::vec3 ojo)
{
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);

	//humo del volcán: bocanadas que suben, se hacen grandes y se desvanecen; con la erupción salen más y más oscuras
	float e = IntensidadErupcion(t);
	const int BOCANADAS = 22;
	struct Bocanada { glm::vec3 p; float tam, alfa; };
	std::vector<Bocanada> humo;
	for (int i = 0; i < BOCANADAS; i++)
	{
		float edad = fmodf(t * (0.06f + 0.06f * e) + (float)i / BOCANADAS, 1.0f);
		glm::vec3 p = CENTRO_PYROS + VOLCAN + glm::vec3(edad * 30.0f + 2.0f * sinf(t * 0.5f + i), ALTURA_VOLCAN + 2.0f + edad * 60.0f, edad * 12.0f);
		float alfa = (0.45f + 0.3f * e) * (1.0f - edad) * Suave(0.0f, 0.08f, edad);
		humo.push_back({ p, 5.0f + 24.0f * edad, alfa });
	}
	std::sort(humo.begin(), humo.end(), [ojo](const Bocanada& a, const Bocanada& b) {
		return glm::length(a.p - ojo) > glm::length(b.p - ojo); });	//de la más lejana a la más cercana
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	UsarMaterial(matOpaco);
	texHumo->UseTexture();
	for (const Bocanada& b : humo)
	{
		glUniform1f(U.transparencia, 1.0f - b.alfa);
		Dibujar(mPlano, Cartel(b.p, vista, b.tam), glm::vec3(0.4f - 0.15f * e, 0.36f - 0.15f * e, 0.34f - 0.15f * e), 0.25f);
	}

	//evolución: columna de luz que sube desde el altar y chispas que salen en espiral (mezcla aditiva: se suman como luz)
	float tv = t - inicioEvolucion;
	if (tv >= 0.0f && tv <= DURACION_EVOLUCION)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		texBlanco->UseTexture();
		float crece = Suave(0.0f, 1.5f, tv), apaga = 1.0f - Suave(2.6f, DURACION_EVOLUCION, tv);
		glUniform1f(U.transparencia, 1.0f - 0.22f * apaga);
		Dibujar(mCilindro, glm::scale(Lugar(glm::vec3(0.0f, ALTO_ALTAR + 7.0f * crece, 0.0f)), glm::vec3(2.2f + 0.4f * sinf(tv * 9.0f), 14.0f * crece, 2.2f)),
			glm::vec3(0.45f, 0.65f, 1.0f), 1.0f);
		if (tv > 2.2f)
		{
			float u = tv - 2.2f;
			glUniform1f(U.transparencia, Suave(1.0f, 1.8f, u));
			for (int i = 0; i < 24; i++)
			{
				float a = i * 0.26f * 2.0f * PI_P + u * 4.0f, r = 0.5f + u * 3.0f;
				glm::vec3 p(r * cosf(a), ALTO_ALTAR + 1.0f + i * 0.15f + u * 1.5f, r * sinf(a));
				Dibujar(mEsferaBaja, glm::scale(Lugar(p), glm::vec3(0.12f)), glm::vec3(1.0f, 0.95f, 0.7f), 3.0f);
			}
		}
	}

	glUniform1f(U.transparencia, 0.0f);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}


//------------------------------------------------------------------ Pyros en el espacio
//Jerarquía: PYROS (inclinado 23° y girando) ; ÓRBITA DE LA LUNA -> LUNA -> ÓRBITA DEL SATÉLITE -> SATÉLITE -> paneles ;
//ANILLO -> 70 asteroides. La luna y el anillo cuelgan de la base y no del giro del planeta, así no giran con él
void DibujarPyrosEspacio(float t)
{
	glm::mat4 base = glm::translate(glm::mat4(1.0f), POS_PYROS_ESPACIO), m, luna, sat;
	UsarMaterial(matOpaco);
	m = glm::rotate(base, glm::radians(23.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	m = glm::rotate(m, t * 0.08f, glm::vec3(0.0f, 1.0f, 0.0f));
	texPlaneta->UseTexture();
	Dibujar(mEsfera, glm::scale(m, glm::vec3(RADIO_PYROS_ESPACIO)), glm::vec3(1.0f), 0.35f);	//la lava brilla también en el lado oscuro

	//LUNA (Ceniza): órbita inclinada de radio 95, una vuelta cada 40 s
	luna = glm::rotate(base, glm::radians(12.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	luna = glm::rotate(luna, t * 2.0f * PI_P / 40.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	luna = glm::translate(luna, glm::vec3(95.0f, 0.0f, 0.0f));
	texLuna->UseTexture();
	Dibujar(mEsfera, glm::scale(glm::rotate(luna, t * 0.3f, glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(9.0f)), glm::vec3(1.0f));
	//SATÉLITE: orbita la luna cada 10 s; sus paneles giran despacio para seguir al sol
	sat = glm::rotate(luna, glm::radians(30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	sat = glm::rotate(sat, t * 2.0f * PI_P / 10.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sat = glm::translate(sat, glm::vec3(17.0f, 0.0f, 0.0f));
	UsarMaterial(matMetal);
	texMetal->UseTexture();
	Dibujar(mCubo, glm::scale(sat, glm::vec3(1.6f)), glm::vec3(0.85f));
	Dibujar(mCono, glm::scale(glm::translate(sat, glm::vec3(0.0f, 0.8f, 0.0f)), glm::vec3(0.7f, 0.4f, 0.7f)), glm::vec3(0.9f));
	texPanel->UseTexture();
	for (int i = 0; i < 2; i++)
	{
		m = glm::translate(sat, glm::vec3(i == 0 ? 3.3f : -3.3f, 0.0f, 0.0f));
		m = glm::rotate(m, t * 0.4f, glm::vec3(1.0f, 0.0f, 0.0f));
		Dibujar(mCubo, glm::scale(m, glm::vec3(4.6f, 0.1f, 1.6f)), glm::vec3(1.0f));
	}

	//ANILLO DE ASTEROIDES: plano inclinado 18° que gira despacio; cada roca gira sobre su propio eje
	UsarMaterial(matOpaco);
	texAsteroide->UseTexture();
	glm::mat4 anillo = glm::rotate(base, glm::radians(18.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	anillo = glm::rotate(anillo, t * 0.05f, glm::vec3(0.0f, 1.0f, 0.0f));
	for (int i = 0; i < 70; i++)
	{
		unsigned int semilla = 900u + i * 31u;
		float a = 2.0f * PI_P * i / 70.0f + 0.08f * Aleatorio(semilla);
		float r = 62.0f + 18.0f * Aleatorio(semilla), y = -3.0f + 6.0f * Aleatorio(semilla);
		float tam = 0.8f + 2.2f * Aleatorio(semilla) * Aleatorio(semilla);
		glm::vec3 eje = glm::normalize(glm::vec3(Aleatorio(semilla) - 0.5f, Aleatorio(semilla) - 0.5f, Aleatorio(semilla) - 0.5f));
		m = glm::translate(anillo, glm::vec3(r * cosf(a), y, r * sinf(a)));
		m = glm::rotate(m, t * (0.3f + Aleatorio(semilla)), eje);
		Dibujar(mRocas[i % 6], glm::scale(m, glm::vec3(tam)), glm::vec3(1.0f));
	}
}

void DibujarHaloPyros(float t)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDepthMask(GL_FALSE);
	UsarMaterial(matOpaco);
	texBlanco->UseTexture();
	glUniform1f(U.transparencia, 0.8f);
	Dibujar(mEsfera, glm::scale(glm::translate(glm::mat4(1.0f), POS_PYROS_ESPACIO), glm::vec3(RADIO_PYROS_ESPACIO * (1.08f + 0.01f * sinf(t)))),
		glm::vec3(1.0f, 0.35f, 0.08f), 0.9f);
	glUniform1f(U.transparencia, 0.0f);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}
