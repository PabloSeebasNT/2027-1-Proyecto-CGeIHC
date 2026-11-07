/*
Peanuts.cpp - Planeta Peanuts (Snoopy y sus amigos)
Yadira

Todas las posiciones son relativas a CENTRO_PEANUTS (función Lugar). Vista desde arriba (norte = -Z):
	cuadrícula de calles: 3 de norte a sur (x = -45, 0, 45) y 2 de oeste a este (z = -20, 35)
	casas pegadas a las calles, mirando hacia ellas (4 al norte y 2 al sur)
	pista de hielo, muñecos de nieve y NPCs dentro de las manzanas
	lámparas a los lados de las calles (tecla N: prender y apagar)
	árboles repartidos y lomas de nieve al fondo (después de r = 110)
*/
#include "Peanuts.h"
#include <stdio.h>
#include <cmath>
#include <vector>
#include <algorithm>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "Geometria.h"
#include "Avatar.h"
#include "Model.h"
#include "CommonValues.h"

static Texture* texNieve, * texBlanco, * texCalle;
static MeshModel* mPisoNieve;
static MeshModel* mCalles;		//todas las calles juntas en una sola malla

//modelos 
static Model modeloCasaSnoopy;
static Model modeloCasa[2];		//dos tipos de casa
static Model modeloArbol1;
static Model modeloLampara1;
static Model modeloLampara2;
static Model modeloLampara3;

static const int NUM_NPC = 6;
static Model modeloNpc[NUM_NPC];

const float PI_N = 3.14159265f;

//---------- CALLES 
const int NUM_CALLES_X = 3;
const float CALLES_X[NUM_CALLES_X] = { -45.0f, 0.0f, 45.0f };	//calles que van de norte a sur 
const int NUM_CALLES_Z = 2;
const float CALLES_Z[NUM_CALLES_Z] = { -20.0f, 35.0f };			//calles que van de oeste a este 
const float LIMITE_CALLES = 95.0f;								//hasta dónde llegan (el avatar camina hasta 105)
const float ANCHO_CALLE = 6.0f;

//----------------------- CASAS
//GIRO_CASAS: hacia dónde mira la puerta. 0 = mira hacia +Z (sur), 180 = mira hacia -Z (norte), 90 = hacia +X, -90 = hacia -X
const glm::vec3 CASA_SNOOPY(-22.0f, 0.0f, -31.0f);
const float GIRO_CASA_SNOOPY = 0.0f;
const int NUM_CASAS = 5;
const glm::vec3 CASAS[NUM_CASAS] = {
	glm::vec3(22.0f, 0.0f, -31.0f), glm::vec3(-68.0f, 0.0f, -31.0f), glm::vec3(68.0f, 0.0f, -31.0f),
	glm::vec3(-22.0f, 0.0f, 46.0f), glm::vec3(22.0f, 0.0f, 46.0f) };
const float GIRO_CASAS[NUM_CASAS] = { 0.0f, 0.0f, 0.0f, 180.0f, 180.0f };

const glm::vec3 PISTA_HIELO(22.0f, 0.0f, 7.0f);
const float RADIO_PISTA = 11.0f;
const int NUM_MUNECOS = 3;
const glm::vec3 MUNECOS[NUM_MUNECOS] = { glm::vec3(-20.0f, 0.0f, 10.0f), glm::vec3(-32.0f, 0.0f, -5.0f), glm::vec3(30.0f, 0.0f, 28.0f) };

//tamaño de los modelos (no sé cuánto miden tus .obj: si salen enormes o diminutos, cambia SOLO estos números)
const float ESCALA_CASA_SNOOPY = 2.5f;
const float ESCALA_CASA = 0.8f;
const float GIRO_MODELO_CASA = 0.0f;	//grados: si la puerta no mira a la calle, prueba 90, -90, 180

//ÁRBOLES
const float ESCALA_ARBOL = 1.25f;
const float ALTURA_ARBOL = 3.25f;		//

//------------------------------------------LAMPARAS
//escala y altura de cada tipo; el giro corrige hacia dónde mira el modelo
const float ESCALA_LAMPARA[3] = { 1.5f, 0.8f, 3.0f };
const float ALTURA_LAMPARA[3] = { 1.5f, 4.5f, 2.5f };		//
const float GIRO_LAMPARA[3] = { 0.0f, 0.0f, 0.0f };			//grados
//focos: la lámpara 1 tiene uno y la 2 tiene dos (de la 3 no sé: la puse con uno, cámbialo)
const int NUM_FOCOS[3] = { 1, 2, 1 };
const float ALTO_FOCO[3] = { 3.0f, -0.8f, 1.8f };			//altura del foco sobre la base de la lámpara (ya con la escala)
const float SEPARACION_FOCOS[3] = { 0.0f, 3.6f, 0.0f };		//distancia entre los dos focos (de lado a lado de la lámpara)
const float TAMANO_FOCO = 0.15f;							//esfera brillante que se ve cuando la lámpara está prendida
const float SEPARACION_LAMPARAS = 16.0f;					//cada cuantas unidades hay una lámpara en la calle

//------------------------------------------------------------------ NPCs
//archivo, escala, altura, giro_x , giro_y , centro de su paseo, radio y rapidez
const char* ARCHIVO_NPC[NUM_NPC] = { "Models/marcie.obj", "Models/Franklin.obj", "Models/pelirroja.obj", "Models/sally.obj",
	"Models/npc2.obj", "Models/snoopy_completo2.obj" };
//                                        lucy  franklin pelirroja sally  npc2  snoopy
const float ESCALA_NPC[NUM_NPC] = { 2.0f,  2.0f,    0.05f,    2.0f,  2.0f,  3.0f };
const float ALTURA_NPC[NUM_NPC] = { 0.5f,  0.8f,    1.5f,    1.8f,  1.0f,  1.0f };
const float GIRO_X_NPC[NUM_NPC] = { 0.0f,  0.0f,   90.0f,    0.0f,  0.0f,  0.0f };
const float GIRO_NPC[NUM_NPC] = { 0.0f,  0.0f,    0.0f,    0.0f,  0.0f,  0.0f };
const glm::vec3 CENTRO_NPC[NUM_NPC] = { glm::vec3(12.0f, 0.0f, -8.0f), glm::vec3(-12.0f, 0.0f, 8.0f), glm::vec3(-30.0f, 0.0f, 20.0f),
	glm::vec3(-22.0f, 0.0f, -6.0f), glm::vec3(32.0f, 0.0f, 20.0f), glm::vec3(-8.0f, 0.0f, -48.0f) };
const float RADIO_NPC[NUM_NPC] = { 3.0f, 4.0f, 3.5f, 4.0f, 3.0f, 3.0f };
const float RAPIDEZ_NPC[NUM_NPC] = { 0.35f, -0.3f, 0.4f, -0.25f, 0.3f, -0.4f };
//
const float ESCALA_SNOOPY = 1.0f;

//árboles repartidos con números pseudoaleatorios fijos (siempre salen igual)
struct Arbol { glm::vec3 pos; float escala, giro; };
static std::vector<Arbol> arboles;

//calles: segmentos rectos (inicio y fin)
struct Tramo { glm::vec3 a, b; };
static std::vector<Tramo> calles;
//lamparas a los lados de las calles
struct Lampara { glm::vec3 pos; float giro; int tipo; };
static std::vector<Lampara> lamparas;
static bool lamparasEncendidas = true;		//tecla N

static glm::mat4 Lugar(glm::vec3 p) { return glm::translate(glm::mat4(1.0f), CENTRO_PEANUTS + p); }
static float Aleatorio(unsigned int& semilla)
{
	semilla =   semilla * 1664525u + 1013904223u;
	return (semilla >> 8) / 16777216.0f;
}

//distancia (en XZ) de un punto a un segmento
static float DistanciaASegmento(glm::vec3 p, glm::vec3 a, glm::vec3 b)
{
	glm::vec2 pa(p.x - a.x, p.z - a.z), ba(b.x - a.x, b.z - a.z);
	float h = glm::clamp(glm::dot(pa, ba) / glm::dot(ba, ba), 0.0f, 1.0f);
	return glm::length(pa - ba * h);
}

//agrega a la malla un rectangulo de a hasta b (u va a lo ancho, v a lo largo: la imagen se repite cada 8 unidades)
static void AgregarCalle(std::vector<GLfloat>& v, std::vector<unsigned int>& idx, glm::vec3 a, glm::vec3 b, float altura)
{
	glm::vec3 d = glm::normalize(b - a);
	glm::vec3 lado(d.z, 0.0f, -d.x);
	glm::vec3 sube(0.0f, altura, 0.0f), arriba(0.0f, 1.0f, 0.0f);
	float repite = glm::length(b - a) / 8.0f;
	unsigned int ini = (unsigned int)(v.size() / 8);
	AgregarVertice(v, a - lado * (ANCHO_CALLE / 2.0f) + sube, 0.0f, 0.0f, arriba);
	AgregarVertice(v, a + lado * (ANCHO_CALLE / 2.0f) + sube, 1.0f, 0.0f, arriba);
	AgregarVertice(v, b - lado * (ANCHO_CALLE / 2.0f) + sube, 0.0f, repite, arriba);
	AgregarVertice(v, b + lado * (ANCHO_CALLE / 2.0f) + sube, 1.0f, repite, arriba);
	idx.insert(idx.end(), { ini, ini + 2, ini + 1, ini + 1, ini + 2, ini + 3 });
}

//Dibuja un modelo .obj (Assimp) con la matriz dada; sus texturas vienen del .mtl
static void DibujarModelo(Model& modelo, const glm::mat4& m, float emision)
{
	glUniformMatrix4fv(U.model, 1, GL_FALSE, glm::value_ptr(m));
	glUniform3f(U.color, 1.0f, 1.0f, 1.0f);
	glUniform1f(U.emision, emision);
	modelo.RenderModel();
}

//posiciones de los focos de una lampara (relativas a CENTRO_PEANUTS); regresa cuantos son
static int FocosDeLampara(const Lampara& l, glm::vec3* salida)
{
	int n = NUM_FOCOS[l.tipo];
	float g = glm::radians(l.giro + GIRO_LAMPARA[l.tipo]);
	glm::vec3 ladoLocal(cosf(g), 0.0f, -sinf(g));		//el eje X de la lampara, ya girado
	for (int k = 0; k < n; k++)
	{
		float desplazamiento = (k - (n - 1) / 2.0f) * SEPARACION_FOCOS[l.tipo];
		salida[k] = l.pos + glm::vec3(0.0f, ALTURA_LAMPARA[l.tipo] + ALTO_FOCO[l.tipo], 0.0f) + ladoLocal * desplazamiento;
	}
	return n;
}

//Foco de una puerta: al frente de la casa, hacia donde mira
static glm::vec3 PosFoco(glm::vec3 casa, float giro)
{
	float g = glm::radians(giro);
	return casa + glm::vec3(sinf(g), 0.0f, cosf(g)) * 7.0f + glm::vec3(0.0f, 3.5f, 0.0f);
}


//------------------------------------------------------------------ carga
void CargarPeanuts()
{
	texNieve = CargarTextura("Textures/nieve.png");
	texBlanco = CargarTextura("Textures/plain.png");
	texCalle = CargarTextura("Textures/calle.png");

	//piso de nieve: plano de 1 x 1 con la textura repetida 120 veces (después se escala)
	std::vector<GLfloat> v;
	std::vector<unsigned int> idx = { 0, 2, 1, 1, 2, 3 };
	glm::vec3 arriba(0.0f, 1.0f, 0.0f);
	AgregarVertice(v, glm::vec3(-0.5f, 0.0f, -0.5f), 0.0f, 120.0f, arriba);
	AgregarVertice(v, glm::vec3(0.5f, 0.0f, -0.5f), 120.0f, 120.0f, arriba);
	AgregarVertice(v, glm::vec3(-0.5f, 0.0f, 0.5f), 0.0f, 0.0f, arriba);
	AgregarVertice(v, glm::vec3(0.5f, 0.0f, 0.5f), 120.0f, 0.0f, arriba);
	mPisoNieve = GuardarMalla(v, idx);

	modeloCasaSnoopy = Model();
	modeloCasaSnoopy.LoadModel("Models/casaSnoopy.obj");
	modeloCasa[0] = Model();
	modeloCasa[0].LoadModel("Models/casa.obj");
	modeloCasa[1] = Model();
	modeloCasa[1].LoadModel("Models/casa2.obj");
	modeloArbol1 = Model();
	modeloArbol1.LoadModel("Models/arbol1.obj");
	modeloLampara1 = Model();
	modeloLampara1.LoadModel("Models/lamparasnoopy1.obj");
	modeloLampara2 = Model();
	modeloLampara2.LoadModel("Models/lamparasnoopy2.obj");
	modeloLampara3 = Model();
	modeloLampara3.LoadModel("Models/lamparasnoopy3.obj");
	for (int i = 0; i < NUM_NPC; i++)
	{
		modeloNpc[i] = Model();
		modeloNpc[i].LoadModel(ARCHIVO_NPC[i]);
	}

	//calles 
	for (int i = 0; i < NUM_CALLES_X; i++)
		calles.push_back({ glm::vec3(CALLES_X[i], 0.0f, -LIMITE_CALLES), glm::vec3(CALLES_X[i], 0.0f, LIMITE_CALLES) });
	for (int j = 0; j < NUM_CALLES_Z; j++)
		calles.push_back({ glm::vec3(-LIMITE_CALLES, 0.0f, CALLES_Z[j]), glm::vec3(LIMITE_CALLES, 0.0f, CALLES_Z[j]) });
	{
		std::vector<GLfloat> vc;
		std::vector<unsigned int> ic;
		for (size_t k = 0; k < calles.size(); k++)
			AgregarCalle(vc, ic, calles[k].a, calles[k].b, 0.03f + 0.004f * k);	//cada una un poquito mas alta para que no parpadeen
		mCalles = GuardarMalla(vc, ic);
	}

	//lamparas: a lo largo de cada calle,
	auto enCruce = [](float valor, const float* lista, int n) {
		for (int i = 0; i < n; i++) if (fabsf(valor - lista[i]) < ANCHO_CALLE / 2.0f + 3.0f) return true;
		return false;
		};
	int cont = 0;
	float lado = ANCHO_CALLE / 2.0f + 1.2f;		//que tan lejos del centro de la calle
	for (int j = 0; j < NUM_CALLES_Z; j++)		//calles de oeste a este
		for (float x = -LIMITE_CALLES + 8.0f; x <= LIMITE_CALLES - 8.0f; x += SEPARACION_LAMPARAS)
		{
			if (enCruce(x, CALLES_X, NUM_CALLES_X)) continue;
			float signo = (cont % 2 == 0) ? 1.0f : -1.0f;
			lamparas.push_back({ glm::vec3(x, 0.0f, CALLES_Z[j] + signo * lado), signo > 0.0f ? 180.0f : 0.0f, cont % 3 });	//miran hacia la calle
			cont++;
		}
	for (int i = 0; i < NUM_CALLES_X; i++)		//calles de norte a sur
		for (float z = -LIMITE_CALLES + 8.0f; z <= LIMITE_CALLES - 8.0f; z += SEPARACION_LAMPARAS)
		{
			if (enCruce(z, CALLES_Z, NUM_CALLES_Z)) continue;
			float signo = (cont % 2 == 0) ? 1.0f : -1.0f;
			lamparas.push_back({ glm::vec3(CALLES_X[i] + signo * lado, 0.0f, z), signo > 0.0f ? -90.0f : 90.0f, cont % 3 });
			cont++;
		}

	//árboles: se descartan los lugares que caen sobre una calle, una casa, la pista o un muñeco
	unsigned int semilla = 1962;
	auto libre = [](glm::vec3 p) {
		for (const Tramo& c : calles) if (DistanciaASegmento(p, c.a, c.b) < ANCHO_CALLE / 2.0f + 2.0f) return false;
		if (glm::length(p - CASA_SNOOPY) < 14.0f) return false;
		for (int i = 0; i < NUM_CASAS; i++) if (glm::length(p - CASAS[i]) < 14.0f) return false;
		for (int i = 0; i < NUM_MUNECOS; i++) if (glm::length(p - MUNECOS[i]) < 5.0f) return false;
		if (glm::length(p - PISTA_HIELO) < RADIO_PISTA + 5.0f) return false;
		return true;
		};
	while (arboles.size() < 45)
	{
		float a = 2.0f * PI_N * Aleatorio(semilla), r = 10.0f + 95.0f * Aleatorio(semilla);
		glm::vec3 p(r * cosf(a), 0.0f, r * sinf(a));
		float escala = 0.8f + 0.6f * Aleatorio(semilla), giro = 360.0f * Aleatorio(semilla);
		if (libre(p)) arboles.push_back({ p, escala, giro });
	}
}


//---------------------------- luces
void AlternarLamparasPeanuts()  
{
	lamparasEncendidas = !lamparasEncendidas;
	printf("Lamparas de Peanuts: %s\n", lamparasEncendidas ? "prendidas" : "apagadas");
}

//Hay pocas luces puntuales (MAX_POINT_LIGHTS) y muchos focos: se mandan al shader los que estan mas cerca del avatar
struct Foco { glm::vec3 pos, color; float lineal, cuadratica, dist; };

void LucesPeanuts(float tiempo, bool deNoche, PointLight* puntuales, unsigned int& np)
{
	std::vector<Foco> focos;
	glm::vec3 p, salida[4];
	if (lamparasEncendidas)
		for (const Lampara& l : lamparas)
		{
			int n = FocosDeLampara(l, salida);
			for (int k = 0; k < n; k++)
			{
				p = CENTRO_PEANUTS + salida[k];
				focos.push_back({ p, glm::vec3(1.0f, 0.9f, 0.6f), 0.09f, 0.032f, glm::length(p - avatar.posicion) });
			}
		}
	if (deNoche)		//focos de las puertas de las casas
		for (int i = 0; i <= NUM_CASAS; i++)
		{
			p = CENTRO_PEANUTS + (i == NUM_CASAS ? PosFoco(CASA_SNOOPY, GIRO_CASA_SNOOPY) : PosFoco(CASAS[i], GIRO_CASAS[i]));
			focos.push_back({ p, glm::vec3(1.0f, 0.75f, 0.4f), 0.06f, 0.008f, glm::length(p - avatar.posicion) });
		}
	std::sort(focos.begin(), focos.end(), [](const Foco& a, const Foco& b) { return a.dist < b.dist; });
	for (const Foco& f : focos)
	{
		if (np >= MAX_POINT_LIGHTS) break;
		puntuales[np++] = PointLight(f.color.r, f.color.g, f.color.b, 0.05f, 3.0f, f.pos.x, f.pos.y, f.pos.z, 1.0f, f.lineal, f.cuadratica);
	}
}


//------------------------------------------------------------------ colisiones
glm::vec3 ResolverColisionPeanuts(glm::vec3 p, float radio)
{
	struct Obstaculo { glm::vec3 c; float r; };
	std::vector<Obstaculo> obs;
	obs.push_back({ CASA_SNOOPY, 6.0f });
	for (int i = 0; i < NUM_CASAS; i++) obs.push_back({ CASAS[i], 7.0f });
	for (int i = 0; i < NUM_MUNECOS; i++) obs.push_back({ MUNECOS[i], 1.3f });
	for (const Arbol& a : arboles) obs.push_back({ a.pos, 1.0f * a.escala });
	obs.push_back({ PISTA_HIELO, RADIO_PISTA });
	for (const Lampara& l : lamparas) obs.push_back({ l.pos, 0.5f });
	glm::vec3 local = p - CENTRO_PEANUTS;
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
	return CENTRO_PEANUTS + local;
}


//------------------------------------------------------------------ muñeco de nieve
//Jerarquía: base
//Cada pieza se acomoda con la matriz de su padre, así todo se mueve junto
static void DibujarMuneco(glm::vec3 lugar, float giro, float t, int i)
{
	glm::mat4 base = glm::rotate(Lugar(lugar), glm::radians(giro), glm::vec3(0.0f, 1.0f, 0.0f)), cuerpo, cabeza, m;
	UsarMaterial(matOpaco);
	texNieve->UseTexture();
	Dibujar(mEsfera, glm::scale(glm::translate(base, glm::vec3(0.0f, 0.9f, 0.0f)), glm::vec3(1.1f)), glm::vec3(1.0f));
	cuerpo = glm::translate(base, glm::vec3(0.0f, 2.1f, 0.0f));
	Dibujar(mEsfera, glm::scale(cuerpo, glm::vec3(0.85f)), glm::vec3(1.0f));
	//la cabeza se mueve un poquito de lado a lado
	cabeza = glm::translate(cuerpo, glm::vec3(0.0f, 1.25f, 0.0f));
	cabeza = glm::rotate(cabeza, glm::radians(12.0f * sinf(t * 0.8f + i)), glm::vec3(0.0f, 1.0f, 0.0f));
	Dibujar(mEsfera, glm::scale(cabeza, glm::vec3(0.62f)), glm::vec3(1.0f));
	texBlanco->UseTexture();
	//nariz de zanahoria
	m = glm::rotate(glm::translate(cabeza, glm::vec3(0.0f, -0.05f, 0.55f)), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	Dibujar(mCono, glm::scale(m, glm::vec3(0.1f, 0.5f, 0.1f)), glm::vec3(1.0f, 0.5f, 0.1f));
	//ojos y botones de carbón
	for (int k = 0; k < 2; k++)
		Dibujar(mEsferaBaja, glm::scale(glm::translate(cabeza, glm::vec3(k == 0 ? 0.2f : -0.2f, 0.2f, 0.5f)), glm::vec3(0.07f)), glm::vec3(0.05f));
	for (int k = 0; k < 3; k++)
		Dibujar(mEsferaBaja, glm::scale(glm::translate(cuerpo, glm::vec3(0.0f, 0.35f - 0.35f * k, 0.8f)), glm::vec3(0.09f)), glm::vec3(0.05f));
	//brazos de ramas
	for (int k = 0; k < 2; k++)
	{
		float lado = (k == 0) ? 1.0f : -1.0f;
		m = glm::translate(cuerpo, glm::vec3(lado * 0.8f, 0.3f, 0.0f));
		m = glm::rotate(m, glm::radians(-lado * (55.0f + 10.0f * sinf(t * 1.5f + k))), glm::vec3(0.0f, 0.0f, 1.0f));
		Dibujar(mCilindro, glm::scale(glm::translate(m, glm::vec3(0.0f, 0.6f, 0.0f)), glm::vec3(0.05f, 1.2f, 0.05f)), glm::vec3(0.3f, 0.2f, 0.1f));
	}
	//sombrero
	Dibujar(mCilindro, glm::scale(glm::translate(cabeza, glm::vec3(0.0f, 0.6f, 0.0f)), glm::vec3(0.8f, 0.05f, 0.8f)), glm::vec3(0.1f, 0.1f, 0.3f));
	Dibujar(mCilindro, glm::scale(glm::translate(cabeza, glm::vec3(0.0f, 0.85f, 0.0f)), glm::vec3(0.5f, 0.5f, 0.5f)), glm::vec3(0.1f, 0.1f, 0.3f));
}

//------------------------------------------------------------------ SNOOPY JERÁRQUICO ANIMADO


static void DibujarSnoopyJerarquico(glm::vec3 lugar, float giro, float t, float escala, float velocidadPaso)
{
	UsarMaterial(matOpaco);
	texBlanco->UseTexture();		// textura blanca: el color lo pone Dibujar

	// Matriz base: posición en el mundo, hacia dónde mira (giro) y tamaño (escala). Todo lo demás cuelga de ella
	glm::mat4 base = glm::rotate(Lugar(lugar), glm::radians(giro), glm::vec3(0.0f, 1.0f, 0.0f));
	base = glm::scale(base, glm::vec3(escala));

	// Ciclo de la caminata: la pierna 0 se mueve con sin(ciclo) y la pierna 1 al revés (una va adelante y la otra atrás)
	float ciclo = t * velocidadPaso;
	float anguloPierna0 = glm::radians(35.0f * sinf(ciclo));

	// Altura del cuerpo: se calcula para que la pierna que está más abajo SIEMPRE toque la nieve (así no se hunde ni flota)
	// (el pie está a 0.42 abajo y 0.08 adelante de la cadera; al girar la pierna ese punto sube o baja)
	float bajo0 = 0.42f * cosf(anguloPierna0) + 0.08f * sinf(anguloPierna0);	// pierna 0
	float bajo1 = 0.42f * cosf(anguloPierna0) - 0.08f * sinf(anguloPierna0);	// pierna 1 (ángulo contrario)
	float altura = 0.57f + fmaxf(bajo0, bajo1);

	// 1. CUERPO (raíz): una esfera alargada
	glm::mat4 cuerpo = glm::translate(base, glm::vec3(0.0f, altura, 0.0f));
	Dibujar(mEsfera, glm::scale(cuerpo, glm::vec3(0.38f, 0.55f, 0.34f)), glm::vec3(0.95f));

	// Panza (sobresale un poco al frente)
	glm::mat4 panza = glm::translate(cuerpo, glm::vec3(0.0f, -0.05f, 0.18f));
	Dibujar(mEsfera, glm::scale(panza, glm::vec3(0.34f, 0.45f, 0.3f)), glm::vec3(1.0f));

	// 2. CABEZA (hija del cuerpo): se inclina un poquito al caminar
	glm::mat4 cabeza = glm::translate(cuerpo, glm::vec3(0.0f, 0.75f, 0.1f));
	cabeza = glm::rotate(cabeza, glm::radians(4.0f * sinf(ciclo * 2.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
	cabeza = glm::rotate(cabeza, glm::radians(5.0f * sinf(t * 2.0f)), glm::vec3(0.0f, 1.0f, 0.0f));
	Dibujar(mEsfera, glm::scale(cabeza, glm::vec3(0.6f, 0.55f, 0.65f)), glm::vec3(1.0f));

	// Hocico alargado (hijo de la cabeza): una esfera estirada hacia el frente (+Z)
	glm::mat4 hocico = glm::translate(cabeza, glm::vec3(0.0f, -0.1f, 0.5f));
	Dibujar(mEsfera, glm::scale(hocico, glm::vec3(0.22f, 0.2f, 0.42f)), glm::vec3(0.98f));

	// Nariz negra (hija del hocico, en la punta)
	glm::mat4 nariz = glm::translate(hocico, glm::vec3(0.0f, 0.06f, 0.4f));
	Dibujar(mEsferaBaja, glm::scale(nariz, glm::vec3(0.12f)), glm::vec3(0.05f));

	// Ojos (hijos de la cabeza): quedan sobre la superficie, antes estaban adentro de la cabeza y no se veían
	for (int k = 0; k < 2; k++)
	{
		float lado = (k == 0) ? 1.0f : -1.0f;
		glm::mat4 ojo = glm::translate(cabeza, glm::vec3(lado * 0.16f, 0.2f, 0.6f));
		Dibujar(mEsferaBaja, glm::scale(ojo, glm::vec3(0.07f)), glm::vec3(0.05f));
	}

	// Orejas negras caídas (hijas de la cabeza): se cuelgan de arriba y se mecen al caminar
	for (int k = 0; k < 2; k++)
	{
		float lado = (k == 0) ? 1.0f : -1.0f;
		glm::mat4 oreja = glm::translate(cabeza, glm::vec3(lado * 0.52f, 0.3f, 0.0f));						// donde se sujeta
		oreja = glm::rotate(oreja, glm::radians(lado * (15.0f + 8.0f * sinf(ciclo + k))), glm::vec3(0.0f, 0.0f, 1.0f));	// se mece
		oreja = glm::translate(oreja, glm::vec3(0.0f, -0.22f, 0.0f));										// la mitad del largo hacia abajo
		Dibujar(mCilindro, glm::scale(oreja, glm::vec3(0.15f, 0.45f, 0.08f)), glm::vec3(0.08f));
	}

	// 3. BRAZOS (hijos del cuerpo): se mueven al contrario que la pierna de su mismo lado
	for (int k = 0; k < 2; k++)
	{
		float lado = (k == 0) ? 1.0f : -1.0f;
		float anguloBrazo = (k == 0) ? -anguloPierna0 : anguloPierna0;
		glm::mat4 brazo = glm::translate(cuerpo, glm::vec3(lado * 0.38f, 0.25f, 0.0f));	// hombro
		brazo = glm::rotate(brazo, anguloBrazo, glm::vec3(1.0f, 0.0f, 0.0f));
		Dibujar(mCilindro, glm::scale(glm::translate(brazo, glm::vec3(0.0f, -0.25f, 0.0f)), glm::vec3(0.12f, 0.5f, 0.12f)), glm::vec3(0.95f));
	}

	// 4. PIERNAS (hijas del cuerpo) y PIES (hijos de la pierna)
	for (int k = 0; k < 2; k++)
	{
		float lado = (k == 0) ? 1.0f : -1.0f;
		float anguloPierna = (k == 0) ? anguloPierna0 : -anguloPierna0;
		glm::mat4 pierna = glm::translate(cuerpo, glm::vec3(lado * 0.2f, -0.45f, 0.0f));		// cadera
		pierna = glm::rotate(pierna, anguloPierna, glm::vec3(1.0f, 0.0f, 0.0f));				// gira sobre la cadera
		glm::mat4 muslo = glm::translate(pierna, glm::vec3(0.0f, -0.2f, 0.0f));
		Dibujar(mCilindro, glm::scale(muslo, glm::vec3(0.14f, 0.4f, 0.14f)), glm::vec3(0.95f));
		glm::mat4 pie = glm::translate(muslo, glm::vec3(0.0f, -0.22f, 0.08f));
		Dibujar(mEsferaBaja, glm::scale(pie, glm::vec3(0.16f, 0.12f, 0.25f)), glm::vec3(0.1f));
	}
}



//------------------------------------------------------------------ superficie
void DibujarSuperficiePeanuts(float t, bool deNoche)
{
	glm::mat4 m;
	UsarMaterial(matOpaco);

	//piso de nieve (muy grande para que no se vea la orilla)
	texNieve->UseTexture();
	Dibujar(mPisoNieve, glm::scale(Lugar(glm::vec3(0.0f)), glm::vec3(700.0f, 1.0f, 700.0f)), glm::vec3(1.0f));

	//calles (una sola imagen repetida)
	texCalle->UseTexture();
	Dibujar(mCalles, Lugar(glm::vec3(0.0f)), glm::vec3(1.0f));

	//lomas de nieve al fondo (después de la zona donde camina el avatar)
	texNieve->UseTexture();		//OJO: hay que volver a poner la textura de nieve, si no se queda la de la calle
	unsigned int semilla = 4242;
	for (int i = 0; i < 26; i++)
	{
		float a = 2.0f * PI_N * i / 26.0f + 0.1f * Aleatorio(semilla), r = 118.0f + 25.0f * Aleatorio(semilla);
		float ancho = 14.0f + 10.0f * Aleatorio(semilla), alto = 5.0f + 9.0f * Aleatorio(semilla);
		Dibujar(mEsfera, glm::scale(Lugar(glm::vec3(r * cosf(a), -1.5f, r * sinf(a))), glm::vec3(ancho, alto, ancho)), glm::vec3(0.95f));
	}

	//pista de hielo con un borde de nieve
	texBlanco->UseTexture();
	UsarMaterial(matBrillante);
	Dibujar(mCilindro, glm::scale(Lugar(PISTA_HIELO + glm::vec3(0.0f, 0.06f, 0.0f)), glm::vec3(RADIO_PISTA, 0.12f, RADIO_PISTA)),
		glm::vec3(0.55f, 0.78f, 0.95f), 0.15f);
	UsarMaterial(matOpaco);
	texNieve->UseTexture();
	for (int k = 0; k < 18; k++)
	{
		float a = 2.0f * PI_N * k / 18.0f;
		Dibujar(mEsfera, glm::scale(Lugar(PISTA_HIELO + glm::vec3((RADIO_PISTA + 0.5f) * cosf(a), 0.0f, (RADIO_PISTA + 0.5f) * sinf(a))),
			glm::vec3(1.8f, 0.9f, 1.8f)), glm::vec3(1.0f));
	}

	//casa de Snoopy y casas: la puerta mira hacia la calle (GIRO_CASAS)
	m = glm::rotate(Lugar(CASA_SNOOPY), glm::radians(GIRO_CASA_SNOOPY + GIRO_MODELO_CASA), glm::vec3(0.0f, 1.0f, 0.0f));
	DibujarModelo(modeloCasaSnoopy, glm::scale(m, glm::vec3(ESCALA_CASA_SNOOPY)), 0.0f);
	for (int i = 0; i < NUM_CASAS; i++)
	{
		m = glm::rotate(Lugar(CASAS[i]), glm::radians(GIRO_CASAS[i] + GIRO_MODELO_CASA), glm::vec3(0.0f, 1.0f, 0.0f));
		DibujarModelo(modeloCasa[i % 2], glm::scale(m, glm::vec3(ESCALA_CASA)), 0.0f);
	}

	//árboles
	for (const Arbol& a : arboles)
	{
		m = glm::rotate(Lugar(a.pos + glm::vec3(0.0f, ALTURA_ARBOL * a.escala, 0.0f)), glm::radians(a.giro), glm::vec3(0.0f, 1.0f, 0.0f));
		DibujarModelo(modeloArbol1, glm::scale(m, glm::vec3(ESCALA_ARBOL * a.escala)), 0.0f);
	}

	//lamparas (prendidas brillan un poco) y la esfera de luz de cada foco
	Model* modelosLampara[3] = { &modeloLampara1, &modeloLampara2, &modeloLampara3 };
	for (const Lampara& l : lamparas)
	{
		m = glm::rotate(Lugar(l.pos + glm::vec3(0.0f, ALTURA_LAMPARA[l.tipo], 0.0f)), glm::radians(l.giro + GIRO_LAMPARA[l.tipo]), glm::vec3(0.0f, 1.0f, 0.0f));
		DibujarModelo(*modelosLampara[l.tipo], glm::scale(m, glm::vec3(ESCALA_LAMPARA[l.tipo])), lamparasEncendidas ? 0.3f : 0.0f);
	}
	if (lamparasEncendidas)
	{
		texBlanco->UseTexture();
		glm::vec3 focosLampara[4];
		for (const Lampara& l : lamparas)
		{
			int n = FocosDeLampara(l, focosLampara);
			for (int k = 0; k < n; k++)
				Dibujar(mEsferaBaja, glm::scale(Lugar(focosLampara[k]), glm::vec3(TAMANO_FOCO)), glm::vec3(1.0f, 0.9f, 0.6f), 2.5f);
		}
	}

	//muñecos de nieve
	for (int i = 0; i < NUM_MUNECOS; i++)
		DibujarMuneco(MUNECOS[i], atan2f(-MUNECOS[i].x, -MUNECOS[i].z) * 180.0f / PI_N, t, i);

	//NPCs: cada uno camina en un círculo pequeño; su frente sigue la derivada de la trayectoria
	for (int i = 0; i < NUM_NPC; i++)
	{
		float a = RAPIDEZ_NPC[i] * t + i * 1.7f;
		glm::vec3 p = CENTRO_NPC[i] + glm::vec3(RADIO_NPC[i] * cosf(a), 0.0f, RADIO_NPC[i] * sinf(a));
		float sentido = RAPIDEZ_NPC[i] > 0.0f ? 1.0f : -1.0f;
		glm::vec3 v(-sentido * sinf(a), 0.0f, sentido * cosf(a));
		if (i == 5)		//el NPC 5 es Snoopy: se dibuja el jerárquico en lugar del .obj
		{
			DibujarSnoopyJerarquico(p, atan2f(v.x, v.z) * 180.0f / PI_N + GIRO_NPC[i], t, ESCALA_SNOOPY, 6.0f);
			continue;
		}
		m = glm::rotate(Lugar(p), atan2f(v.x, v.z) + glm::radians(GIRO_NPC[i]), glm::vec3(0.0f, 1.0f, 0.0f));
		m = glm::translate(m, glm::vec3(0.0f, ALTURA_NPC[i] + 0.05f * fabsf(sinf(t * 6.0f + i)), 0.0f));	//altura de cada uno + rebotecito al caminar
		m = glm::rotate(m, glm::radians(GIRO_X_NPC[i]), glm::vec3(1.0f, 0.0f, 0.0f));						//para pararlos si el modelo viene acostado
		DibujarModelo(modeloNpc[i], glm::scale(m, glm::vec3(ESCALA_NPC[i])), 0.0f);
	}

	//focos de las puertas (de noche se ven encendidos)
	texBlanco->UseTexture();
	for (int i = 0; i <= NUM_CASAS; i++)
		Dibujar(mEsferaBaja, glm::scale(Lugar(i == NUM_CASAS ? PosFoco(CASA_SNOOPY, GIRO_CASA_SNOOPY) : PosFoco(CASAS[i], GIRO_CASAS[i])), glm::vec3(0.4f)),
			glm::vec3(1.0f, 0.85f, 0.5f), deNoche ? 2.5f : 0.2f);

	//nieve cayendo: copos fijos en el mundo que se repiten en un cubo de 70 x 70 alrededor del avatar
	UsarMaterial(matOpaco); 
	for (int i = 0; i < 160; i++)
	{
		unsigned int s = 100u + i * 17u;
		float x0 = 70.0f * Aleatorio(s), z0 = 70.0f * Aleatorio(s), h0 = 38.0f * Aleatorio(s), vel = 2.5f + 1.5f * Aleatorio(s);
		float x = avatar.posicion.x + fmodf(fmodf(x0 - avatar.posicion.x + 35.0f, 70.0f) + 70.0f, 70.0f) - 35.0f + 0.8f * sinf(t + i);
		float z = avatar.posicion.z + fmodf(fmodf(z0 - avatar.posicion.z + 35.0f, 70.0f) + 70.0f, 70.0f) - 35.0f;
		float y = 38.0f - fmodf(h0 + t * vel, 38.0f);
		Dibujar(mEsferaBaja, glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z)), glm::vec3(0.09f)), glm::vec3(1.0f), 0.7f);
	}
}