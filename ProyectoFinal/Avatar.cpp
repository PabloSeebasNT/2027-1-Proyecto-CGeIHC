/*
Avatar.cpp - Proyecto final CGeIHC 2027-1
Ricardo Emmanuel Galicia Tequianes - 118001740

Jerarquía del avatar (cada hijo hereda la matriz de su padre):
	RAÍZ (posición y rumbo)
	└── CUERPO (sube, baja y se mece con la caminata)
		├── caparazón de obsidiana y tres púas
		├── CUELLO -> CABEZA -> MANDÍBULA
		│                    └── 2 TALLOS -> OJO
		├── 4 PATAS: CADERA -> MUSLO -> RODILLA -> ESPINILLA -> PIE -> 3 garras
		└── COLA: 5 segmentos encadenados -> LLAMA
Caminata: trote, las patas en diagonal se mueven juntas (delantera izquierda con trasera derecha y al revés).
Cada pata gira en la cadera con un seno de la fase y dobla la rodilla solo mientras va hacia adelante (en el aire).
La cola es una onda: cada segmento gira un poco respecto al anterior con un retraso, así la onda viaja hacia la punta.
*/
#include "Avatar.h"
#include <cmath>
#include "Geometria.h"	//incluye glew.h, que debe ir antes que glfw3.h
#include <glfw3.h>
#include <gtc/matrix_transform.hpp>

Avatar avatar = { glm::vec3(0.0f), 0.0f, 0.0f, 0.0f, 0.0f };

static Texture* texPiel;
static Texture* texObsidiana;
static Texture* texLava;
static Texture* texCristal;

const float VELOCIDAD_AVATAR = 7.0f;		//unidades por segundo
const float GIRO_AVATAR = 110.0f;			//grados por segundo con A y D
const float SENSIBILIDAD_RATON = 0.25f;		//grados por píxel de movimiento del ratón
const float PASOS_POR_SEGUNDO = 1.7f;		//ciclos completos de las patas por segundo a velocidad normal
const float LARGO_MUSLO = 0.55f;
const float LARGO_ESPINILLA = 0.6f;
const int SEGMENTOS_COLA = 5;
const float PI_A = 3.14159265f;

void CargarAvatar()
{
	texPiel = CargarTextura("Textures/piel_avatar.png");
	texObsidiana = CargarTextura("Textures/obsidiana.png");
	texLava = CargarTextura("Textures/lava.png");
	texCristal = CargarTextura("Textures/cristal.png");
}

glm::vec3 FrenteAvatar()
{
	float r = glm::radians(avatar.rumbo);
	return glm::vec3(sinf(r), 0.0f, cosf(r));
}

void ActualizarAvatar(bool* keys, float giroRaton, float dt, glm::vec3 centro, float radioCaminable)
{
	float avance = 0.0f;
	if (keys[GLFW_KEY_W]) avance += 1.0f;
	if (keys[GLFW_KEY_S]) avance -= 0.6f;	//hacia atrás camina más despacio
	if (keys[GLFW_KEY_A]) avatar.rumbo += GIRO_AVATAR * dt;
	if (keys[GLFW_KEY_D]) avatar.rumbo -= GIRO_AVATAR * dt;
	avatar.rumbo -= giroRaton * SENSIBILIDAD_RATON;
	avatar.rumbo = fmodf(avatar.rumbo, 360.0f);

	avatar.posicion += FrenteAvatar() * (avance * VELOCIDAD_AVATAR * dt);
	//límite del piso: si sale del círculo se regresa a la orilla
	glm::vec2 d(avatar.posicion.x - centro.x, avatar.posicion.z - centro.z);
	float dist = glm::length(d);
	if (dist > radioCaminable)
	{
		d *= radioCaminable / dist;
		avatar.posicion.x = centro.x + d.x;
		avatar.posicion.z = centro.z + d.y;
	}
	avatar.posicion.y = centro.y;	//el piso es un plano paralelo a XZ

	//la intensidad sube o baja poco a poco; la fase avanza con ella (al retroceder avanza al revés)
	float objetivo = (avance != 0.0f) ? 1.0f : 0.0f;
	avatar.intensidadCaminata += (objetivo - avatar.intensidadCaminata) * fminf(1.0f, dt * 6.0f);
	avatar.faseCaminata += dt * 2.0f * PI_A * PASOS_POR_SEGUNDO * (avance < 0.0f ? -0.7f : 1.0f) * avatar.intensidadCaminata;
	avatar.faseCaminata = fmodf(avatar.faseCaminata, 2.0f * PI_A);
	avatar.tiempo += dt;
}

void LinternaAvatar(glm::vec3& posicion, glm::vec3& direccion)
{
	glm::vec3 f = FrenteAvatar();
	posicion = avatar.posicion + glm::vec3(0.0f, 2.2f, 0.0f) + f * 2.2f;
	direccion = glm::normalize(f + glm::vec3(0.0f, -0.35f, 0.0f));
}


//Una pata completa. cadera es el punto de unión con el cuerpo, lado es +1 (izquierda, +X) o -1 (derecha) y desfase
//la diferencia de fase con la delantera izquierda
static void DibujarPata(const glm::mat4& cuerpo, glm::vec3 cadera, float lado, float desfase)
{
	float s = avatar.intensidadCaminata;
	float a = avatar.faseCaminata + desfase;
	float balanceo = 22.0f * s * sinf(a);				//positivo: el pie va hacia adelante
	float levanta = 30.0f * s * fmaxf(0.0f, cosf(a));	//dobla la rodilla mientras la pata avanza
	glm::mat4 m, pieza;

	//CADERA: se abre hacia afuera (giro en Z) y el muslo apunta hacia abajo y un poco al frente (giro en X)
	m = glm::translate(cuerpo, cadera);
	m = glm::rotate(m, glm::radians(lado * 12.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	m = glm::rotate(m, glm::radians(-(20.0f + balanceo)), glm::vec3(1.0f, 0.0f, 0.0f));
	texPiel->UseTexture();
	Dibujar(mEsfera, glm::scale(m, glm::vec3(0.2f)), glm::vec3(1.0f));
	//MUSLO
	pieza = glm::translate(m, glm::vec3(0.0f, -0.5f * LARGO_MUSLO, 0.0f));
	Dibujar(mCilindro, glm::scale(pieza, glm::vec3(0.14f, LARGO_MUSLO, 0.14f)), glm::vec3(1.0f));
	//RODILLA: la espinilla regresa hacia atrás para que el pie quede bajo la cadera
	m = glm::translate(m, glm::vec3(0.0f, -LARGO_MUSLO, 0.0f));
	m = glm::rotate(m, glm::radians(40.0f + levanta), glm::vec3(1.0f, 0.0f, 0.0f));
	Dibujar(mEsfera, glm::scale(m, glm::vec3(0.15f)), glm::vec3(1.0f));
	//ESPINILLA
	pieza = glm::translate(m, glm::vec3(0.0f, -0.5f * LARGO_ESPINILLA, 0.0f));
	Dibujar(mCilindro, glm::scale(pieza, glm::vec3(0.11f, LARGO_ESPINILLA, 0.11f)), glm::vec3(1.0f));
	//PIE: se endereza para quedar casi paralelo al piso
	m = glm::translate(m, glm::vec3(0.0f, -LARGO_ESPINILLA, 0.0f));
	m = glm::rotate(m, glm::radians(-(20.0f - balanceo + levanta) * 0.8f), glm::vec3(1.0f, 0.0f, 0.0f));
	texObsidiana->UseTexture();
	Dibujar(mEsfera, glm::scale(glm::translate(m, glm::vec3(0.0f, 0.0f, 0.08f)), glm::vec3(0.2f, 0.08f, 0.3f)), glm::vec3(1.0f));
	//3 garras: conos acostados que apuntan hacia adelante
	for (int g = -1; g <= 1; g++)
	{
		pieza = glm::translate(m, glm::vec3(g * 0.11f, -0.02f, 0.3f));
		pieza = glm::rotate(pieza, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		Dibujar(mCono, glm::scale(pieza, glm::vec3(0.05f, 0.18f, 0.05f)), glm::vec3(1.0f));
	}
}


void DibujarAvatar()
{
	float s = avatar.intensidadCaminata;
	float f = avatar.faseCaminata;
	float t = avatar.tiempo;
	glm::mat4 raiz, cuerpo, cuello, cabeza, m, pieza;

	UsarMaterial(matBrillante);	//piel con brillo especular: refleja el sol, las luminarias y la linterna

	//RAÍZ
	raiz = glm::translate(glm::mat4(1.0f), avatar.posicion);
	raiz = glm::rotate(raiz, glm::radians(avatar.rumbo), glm::vec3(0.0f, 1.0f, 0.0f));
	//CUERPO: sube dos veces por ciclo (cada vez que apoya un par de patas) y se mece de lado; quieto respira
	cuerpo = glm::translate(raiz, glm::vec3(0.0f, 1.25f + 0.06f * s * fabsf(sinf(f)) + 0.02f * sinf(t * 1.8f), 0.0f));
	cuerpo = glm::rotate(cuerpo, glm::radians(3.0f * s * sinf(f)), glm::vec3(0.0f, 0.0f, 1.0f));
	texPiel->UseTexture();
	Dibujar(mEsfera, glm::scale(cuerpo, glm::vec3(0.75f, 0.55f, 1.25f)), glm::vec3(1.0f));

	//caparazón y púas sobre el lomo
	texObsidiana->UseTexture();
	Dibujar(mEsfera, glm::scale(glm::translate(cuerpo, glm::vec3(0.0f, 0.3f, -0.1f)), glm::vec3(0.66f, 0.32f, 1.05f)), glm::vec3(1.0f));
	for (int k = 0; k < 3; k++)
	{
		pieza = glm::translate(cuerpo, glm::vec3(0.0f, 0.55f, 0.55f - 0.55f * k));
		pieza = glm::rotate(pieza, glm::radians(-20.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		Dibujar(mCono, glm::scale(pieza, glm::vec3(0.18f, 0.45f - 0.08f * k, 0.18f)), glm::vec3(1.0f));
	}

	//CUELLO: sale del frente del cuerpo inclinado hacia adelante
	cuello = glm::translate(cuerpo, glm::vec3(0.0f, 0.25f, 1.05f));
	cuello = glm::rotate(cuello, glm::radians(40.0f + 4.0f * sinf(t * 1.3f)), glm::vec3(1.0f, 0.0f, 0.0f));
	texPiel->UseTexture();
	Dibujar(mCilindro, glm::scale(glm::translate(cuello, glm::vec3(0.0f, 0.4f, 0.0f)), glm::vec3(0.2f, 0.8f, 0.2f)), glm::vec3(1.0f));
	//CABEZA: se endereza para mirar al frente y cabecea con los pasos
	cabeza = glm::translate(cuello, glm::vec3(0.0f, 0.8f, 0.0f));
	cabeza = glm::rotate(cabeza, glm::radians(-40.0f + 5.0f * s * sinf(2.0f * f)), glm::vec3(1.0f, 0.0f, 0.0f));
	Dibujar(mEsfera, glm::scale(glm::translate(cabeza, glm::vec3(0.0f, 0.0f, 0.15f)), glm::vec3(0.42f, 0.34f, 0.55f)), glm::vec3(1.0f));
	//MANDÍBULA: se abre y se cierra despacio
	m = glm::translate(cabeza, glm::vec3(0.0f, -0.12f, 0.05f));
	m = glm::rotate(m, glm::radians(8.0f + 6.0f * sinf(t * 2.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
	texObsidiana->UseTexture();
	Dibujar(mEsfera, glm::scale(glm::translate(m, glm::vec3(0.0f, -0.06f, 0.35f)), glm::vec3(0.3f, 0.11f, 0.4f)), glm::vec3(1.0f));
	//TALLOS y OJOS: se abren hacia los lados y se balancean con un desfase entre los dos
	for (int i = 0; i < 2; i++)
	{
		float lado = (i == 0) ? 1.0f : -1.0f;
		m = glm::translate(cabeza, glm::vec3(lado * 0.18f, 0.25f, 0.15f));
		m = glm::rotate(m, glm::radians(-lado * 25.0f + 6.0f * sinf(t * 3.0f + i * 1.7f)), glm::vec3(0.0f, 0.0f, 1.0f));
		texPiel->UseTexture();
		Dibujar(mCilindro, glm::scale(glm::translate(m, glm::vec3(0.0f, 0.2f, 0.0f)), glm::vec3(0.05f, 0.4f, 0.05f)), glm::vec3(1.0f));
		texCristal->UseTexture();
		Dibujar(mEsfera, glm::scale(glm::translate(m, glm::vec3(0.0f, 0.45f, 0.0f)), glm::vec3(0.12f)), glm::vec3(1.0f, 0.9f, 0.5f), 1.2f);
	}

	//PATAS: delantera izq. (+X), delantera der., trasera izq., trasera der.
	DibujarPata(cuerpo, glm::vec3(0.55f, -0.15f, 0.75f), 1.0f, 0.0f);
	DibujarPata(cuerpo, glm::vec3(-0.55f, -0.15f, 0.75f), -1.0f, PI_A);
	DibujarPata(cuerpo, glm::vec3(0.55f, -0.15f, -0.75f), 1.0f, PI_A);
	DibujarPata(cuerpo, glm::vec3(-0.55f, -0.15f, -0.75f), -1.0f, 0.0f);

	//COLA: cada segmento cuelga del anterior. El cilindro va sobre Y; se acuesta sobre -Z con un giro de 90° en X
	m = glm::translate(cuerpo, glm::vec3(0.0f, 0.0f, -1.1f));
	texPiel->UseTexture();
	for (int k = 0; k < SEGMENTOS_COLA; k++)
	{
		float largo = 0.42f;
		float radio = 0.2f - 0.028f * k;
		m = glm::rotate(m, glm::radians(18.0f * sinf(t * 2.2f + f * 0.5f - k * 0.7f)), glm::vec3(0.0f, 1.0f, 0.0f));
		m = glm::rotate(m, glm::radians(-5.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		pieza = glm::translate(m, glm::vec3(0.0f, 0.0f, -0.5f * largo));
		pieza = glm::rotate(pieza, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		Dibujar(mCilindro, glm::scale(pieza, glm::vec3(radio, largo, radio)), glm::vec3(1.0f));
		Dibujar(mEsfera, glm::scale(m, glm::vec3(radio)), glm::vec3(1.0f));	//unión entre segmentos
		m = glm::translate(m, glm::vec3(0.0f, 0.0f, -largo));
	}
	//LLAMA en la punta: dos conos (exterior naranja e interior amarillo) que cambian de largo muy rápido
	m = glm::rotate(m, glm::radians(-40.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	texLava->UseTexture();
	Dibujar(mCono, glm::scale(m, glm::vec3(0.17f, 0.55f * (1.0f + 0.18f * sinf(t * 17.0f)), 0.17f)), glm::vec3(1.0f, 0.6f, 0.3f), 1.6f);
	Dibujar(mCono, glm::scale(m, glm::vec3(0.09f, 0.4f * (1.0f + 0.25f * sinf(t * 23.0f + 1.0f)), 0.09f)), glm::vec3(1.0f, 0.95f, 0.5f), 2.2f);
}
