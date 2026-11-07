/*
NaveVerde.cpp - Proyecto final CGeIHC 2027-1
Ricardo Emmanuel Galicia Tequianes - 118001740

Jerarquía de la nave (modelo hacia +Z, techo +Y, unos 10 de largo con escala 1):
	BASE
	├── fuselaje, sonda de la nariz, cabina y aleta dorsal
	├── RADAR: mástil -> plato que gira
	├── 2 ALAS (flecha hacia atrás y diedro) -> MOTOR de la punta -> llama y luz de navegación
	├── 2 MOTORES PRINCIPALES -> llama
	└── TREN: 3 BISAGRAS -> PATA -> PIE (el pie se mantiene horizontal al plegarse)
*/
#include "NaveVerde.h"
#include <cmath>
#include <gtc/matrix_transform.hpp>
#include "Geometria.h"

static Texture* texCasco;
static Texture* texCabina;
static Texture* texMetal;
static Texture* texLava;
static Texture* texCristal;
static RutaCerrada rutaEspacio;

const float VELOCIDAD_NAVE_ESPACIO = 45.0f;	//unidades por segundo

//Ruta en el espacio (zona del espacio, coordenadas del mundo). Pasa junto a Pyros, al planeta verde, al de Peanuts,
//entre la estación y el planeta compartido y junto al planeta de lava oscuro, como en el croquis
static const glm::vec3 PUNTOS_ESPACIO[] = {
	glm::vec3(-130.0f, 15.0f, 330.0f),
	glm::vec3(-300.0f, 35.0f, 130.0f),
	glm::vec3(-300.0f, 50.0f, -60.0f),
	glm::vec3(-150.0f, 70.0f, -240.0f),
	glm::vec3(90.0f, 40.0f, -310.0f),
	glm::vec3(210.0f, 20.0f, -120.0f),
	glm::vec3(90.0f, 0.0f, 40.0f),
	glm::vec3(300.0f, -20.0f, 170.0f),
	glm::vec3(440.0f, 0.0f, 300.0f),
	glm::vec3(260.0f, 25.0f, 440.0f),
	glm::vec3(60.0f, 30.0f, 420.0f)
};


//Catmull-Rom: la curva pasa por p1 y p2 y su dirección en cada punto la dan los puntos vecinos
static glm::vec3 CatmullRom(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float u)
{
	return 0.5f * ((2.0f * p1) + (-p0 + p2) * u + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * u * u
		+ (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * u * u * u);
}

void RutaCerrada::Preparar(const glm::vec3* puntos, int n, int muestrasPorTramo)
{
	muestras.clear();
	acumulado.clear();
	for (int i = 0; i < n; i++)
		for (int k = 0; k < muestrasPorTramo; k++)
			muestras.push_back(CatmullRom(puntos[(i - 1 + n) % n], puntos[i], puntos[(i + 1) % n], puntos[(i + 2) % n],
				(float)k / muestrasPorTramo));
	muestras.push_back(muestras[0]);	//la última muestra es la primera: la ruta queda cerrada
	largo = 0.0f;
	acumulado.push_back(0.0f);
	for (size_t i = 1; i < muestras.size(); i++)
	{
		largo += glm::length(muestras[i] - muestras[i - 1]);
		acumulado.push_back(largo);
	}
}

glm::vec3 RutaCerrada::Punto(float s) const
{
	s = fmodf(s, largo);
	if (s < 0.0f) s += largo;
	//búsqueda binaria de la muestra donde empieza el tramo que contiene a s
	size_t a = 0, b = acumulado.size() - 1;
	while (b - a > 1)
	{
		size_t m = (a + b) / 2;
		if (acumulado[m] <= s) a = m;
		else b = m;
	}
	float t = (s - acumulado[a]) / fmaxf(acumulado[b] - acumulado[a], 0.0001f);
	return muestras[a] + (muestras[b] - muestras[a]) * t;
}

void RutaCerrada::Pose(float s, float inclinacion, glm::vec3& pos, glm::vec3& frente, glm::vec3& techo) const
{
	const float D = 2.0f;	//distancia para las diferencias: la dirección sale de los puntos de adelante y de atrás
	pos = Punto(s);
	glm::vec3 fAtras = glm::normalize(pos - Punto(s - D));
	glm::vec3 fAdelante = glm::normalize(Punto(s + D) - pos);
	frente = glm::normalize(fAtras + fAdelante);
	//curvatura: cuánto cambia la dirección por unidad de distancia; apunta al centro de la curva
	glm::vec3 curva = (fAdelante - fAtras) / D;
	techo = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f) + curva * inclinacion);
	glm::vec3 lado = glm::normalize(glm::cross(frente, techo));
	techo = glm::cross(lado, frente);
}


glm::mat4 MatrizDesdeEjes(glm::vec3 pos, glm::vec3 frente, glm::vec3 techo, float escala)
{
	//columnas: +X del modelo (su izquierda) = techo x frente, +Y = techo, +Z = frente
	glm::mat4 m(glm::vec4(glm::cross(techo, frente) * escala, 0.0f), glm::vec4(techo * escala, 0.0f),
		glm::vec4(frente * escala, 0.0f), glm::vec4(pos, 1.0f));
	return m;
}


void CargarNaveVerde()
{
	texCasco = CargarTextura("Textures/casco_verde.png");
	texCabina = CargarTextura("Textures/cabina.png");
	texMetal = CargarTextura("Textures/metal.png");
	texLava = CargarTextura("Textures/lava.png");
	texCristal = CargarTextura("Textures/cristal.png");
	rutaEspacio.Preparar(PUNTOS_ESPACIO, sizeof(PUNTOS_ESPACIO) / sizeof(PUNTOS_ESPACIO[0]), 80);
}

void PoseNaveEspacio(float tiempo, glm::vec3& pos, glm::vec3& frente, glm::vec3& techo)
{
	rutaEspacio.Pose(VELOCIDAD_NAVE_ESPACIO * tiempo, 140.0f, pos, frente, techo);
}


//Llama de un motor: cono que apunta hacia atrás (-Z); su largo depende de la potencia y parpadea
static void DibujarLlama(const glm::mat4& tobera, float radio, float largo, float potencia, float tiempo, float desfase)
{
	if (potencia <= 0.01f) return;
	float parpadeo = 1.0f + 0.12f * sinf(tiempo * 31.0f + desfase) + 0.06f * sinf(tiempo * 53.0f + 2.0f * desfase);
	glm::mat4 m = glm::rotate(tobera, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));	//+Y del cono -> -Z
	texLava->UseTexture();
	Dibujar(mCono, glm::scale(m, glm::vec3(radio, largo * potencia * parpadeo, radio)), glm::vec3(1.0f, 0.7f, 0.4f), 1.8f);
	Dibujar(mCono, glm::scale(m, glm::vec3(radio * 0.55f, largo * 0.7f * potencia * parpadeo, radio * 0.55f)), glm::vec3(1.0f, 1.0f, 0.7f), 2.5f);
}


void DibujarNaveVerde(const glm::mat4& base, float tren, float potencia, float tiempo)
{
	glm::mat4 m, pieza;
	UsarMaterial(matBrillante);

	//FUSELAJE: esfera alargada; la franja negra de la textura queda a media altura alrededor de la nave
	texCasco->UseTexture();
	Dibujar(mEsfera, glm::scale(base, glm::vec3(1.4f, 1.0f, 5.0f)), glm::vec3(1.0f));
	//aleta dorsal
	pieza = glm::translate(base, glm::vec3(0.0f, 1.0f, -2.8f));
	pieza = glm::rotate(pieza, glm::radians(-18.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	Dibujar(mCubo, glm::scale(pieza, glm::vec3(0.22f, 1.1f, 2.0f)), glm::vec3(1.0f));
	//sonda de la nariz
	UsarMaterial(matMetal);
	texMetal->UseTexture();
	pieza = glm::translate(base, glm::vec3(0.0f, 0.0f, 4.7f));
	pieza = glm::rotate(pieza, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	Dibujar(mCono, glm::scale(pieza, glm::vec3(0.25f, 1.3f, 0.25f)), glm::vec3(0.6f, 0.65f, 0.6f));
	//CABINA: vidrio con un poco de brillo propio
	texCabina->UseTexture();
	Dibujar(mEsfera, glm::scale(glm::translate(base, glm::vec3(0.0f, 0.65f, 1.9f)), glm::vec3(0.75f, 0.55f, 1.5f)), glm::vec3(1.0f), 0.2f);

	//RADAR: mástil y plato que gira sobre Y
	texMetal->UseTexture();
	m = glm::translate(base, glm::vec3(0.0f, 0.95f, -0.6f));
	Dibujar(mCilindro, glm::scale(glm::translate(m, glm::vec3(0.0f, 0.2f, 0.0f)), glm::vec3(0.06f, 0.4f, 0.06f)), glm::vec3(0.8f));
	m = glm::translate(m, glm::vec3(0.0f, 0.4f, 0.0f));
	m = glm::rotate(m, glm::radians(tiempo * 120.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	m = glm::rotate(m, glm::radians(-35.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	Dibujar(mCono, glm::scale(m, glm::vec3(0.45f, 0.15f, 0.45f)), glm::vec3(0.9f));

	//ALAS: +X del modelo es la izquierda (lado = +1). Flecha de 28° hacia atrás y 6° de diedro
	for (int i = 0; i < 2; i++)
	{
		float lado = (i == 0) ? 1.0f : -1.0f;
		glm::mat4 ala = glm::translate(base, glm::vec3(lado * 1.0f, -0.2f, -0.6f));
		ala = glm::rotate(ala, glm::radians(lado * 28.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		ala = glm::rotate(ala, glm::radians(lado * 6.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		UsarMaterial(matBrillante);
		texCasco->UseTexture();
		Dibujar(mCubo, glm::scale(glm::translate(ala, glm::vec3(lado * 2.0f, 0.0f, 0.0f)), glm::vec3(4.0f, 0.16f, 1.8f)), glm::vec3(1.0f));
		//MOTOR de la punta: se gira de regreso para que apunte hacia el frente de la nave
		glm::mat4 motor = glm::translate(ala, glm::vec3(lado * 4.0f, 0.0f, 0.1f));
		motor = glm::rotate(motor, glm::radians(-lado * 28.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		UsarMaterial(matMetal);
		texMetal->UseTexture();
		pieza = glm::rotate(motor, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		Dibujar(mCilindro, glm::scale(pieza, glm::vec3(0.42f, 2.4f, 0.42f)), glm::vec3(0.75f, 0.8f, 0.75f));
		Dibujar(mEsfera, glm::scale(glm::translate(motor, glm::vec3(0.0f, 0.0f, 1.2f)), glm::vec3(0.42f, 0.42f, 0.35f)), glm::vec3(0.35f));
		DibujarLlama(glm::translate(motor, glm::vec3(0.0f, 0.0f, -1.2f)), 0.32f, 1.6f, potencia, tiempo, (float)i);
		//luz de navegación: roja a la izquierda y verde a la derecha; parpadea una vez por segundo
		float encendida = (fmodf(tiempo + 0.5f * i, 1.0f) < 0.5f) ? 2.0f : 0.3f;
		texCristal->UseTexture();
		Dibujar(mEsfera, glm::scale(glm::translate(motor, glm::vec3(lado * 0.5f, 0.0f, 0.4f)), glm::vec3(0.14f)),
			(i == 0) ? glm::vec3(1.0f, 0.1f, 0.1f) : glm::vec3(0.1f, 1.0f, 0.2f), encendida);
	}

	//MOTORES PRINCIPALES en la parte trasera
	for (int i = 0; i < 2; i++)
	{
		float x = (i == 0) ? 0.6f : -0.6f;
		texMetal->UseTexture();
		m = glm::translate(base, glm::vec3(x, 0.0f, -4.4f));
		pieza = glm::rotate(m, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		Dibujar(mCilindro, glm::scale(pieza, glm::vec3(0.42f, 1.4f, 0.42f)), glm::vec3(0.55f, 0.6f, 0.55f));
		DibujarLlama(glm::translate(m, glm::vec3(0.0f, 0.0f, -0.7f)), 0.38f, 2.4f, potencia, tiempo, 3.0f + i);
	}

	//TREN DE ATERRIZAJE: con tren = 1 la pata apunta hacia abajo; con 0 se pliega 90° hacia atrás bajo el casco
	const glm::vec3 BISAGRAS[3] = { glm::vec3(0.0f, -0.85f, 2.6f), glm::vec3(1.0f, -0.8f, -2.0f), glm::vec3(-1.0f, -0.8f, -2.0f) };
	float angulo = 90.0f * (1.0f - tren);
	texMetal->UseTexture();
	UsarMaterial(matMetal);
	for (int i = 0; i < 3; i++)
	{
		m = glm::translate(base, BISAGRAS[i]);
		m = glm::rotate(m, glm::radians(angulo), glm::vec3(1.0f, 0.0f, 0.0f));
		Dibujar(mEsfera, glm::scale(m, glm::vec3(0.18f)), glm::vec3(0.5f));
		Dibujar(mCilindro, glm::scale(glm::translate(m, glm::vec3(0.0f, -0.65f, 0.0f)), glm::vec3(0.12f, 1.3f, 0.12f)), glm::vec3(0.7f));
		m = glm::translate(m, glm::vec3(0.0f, -1.3f, 0.0f));
		m = glm::rotate(m, glm::radians(-angulo), glm::vec3(1.0f, 0.0f, 0.0f));
		Dibujar(mCilindro, glm::scale(m, glm::vec3(0.35f, 0.08f, 0.35f)), glm::vec3(0.4f));
	}
}
