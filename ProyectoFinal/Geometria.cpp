/*
Geometria.cpp - Proyecto final CGeIHC 2027-1
Ricardo Emmanuel Galicia Tequianes - 118001740
*/
#include "Geometria.h"
#include <cmath>
#include <gtc/type_ptr.hpp>

UniformesEscena U = { 0, 0, 0, 0, 0, 0, 0 };

MeshModel* mCubo = nullptr;
MeshModel* mCilindro = nullptr;
MeshModel* mCono = nullptr;
MeshModel* mEsfera = nullptr;
MeshModel* mEsferaBaja = nullptr;
MeshModel* mPlano = nullptr;

Material matBrillante;
Material matOpaco;
Material matMetal;

static const float PI = 3.14159265f;


//La normal se recibe hacia afuera y se guarda hacia adentro: el shader calcula dot(normal, dirección de la luz) con
//la dirección que va de la luz al fragmento, y ese producto solo es positivo si la normal apunta al interior
void AgregarVertice(std::vector<GLfloat>& v, glm::vec3 p, float u, float t, glm::vec3 normalExterior)
{
	v.insert(v.end(), { p.x, p.y, p.z, u, t, -normalExterior.x, -normalExterior.y, -normalExterior.z });
}

MeshModel* GuardarMalla(std::vector<GLfloat>& v, std::vector<unsigned int>& idx)
{
	MeshModel* malla = new MeshModel();
	malla->CreateMeshModel(v.data(), idx.data(), (unsigned int)v.size(), (unsigned int)idx.size());
	return malla;
}


//Cubo de lado 1: 4 vértices por cara para que cada cara tenga su normal y la textura completa
static MeshModel* CrearCubo()
{
	std::vector<GLfloat> v;
	std::vector<unsigned int> idx;
	const glm::vec3 normales[6] = { {0, 0, 1}, {0, 0, -1}, {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0} };
	for (const glm::vec3& n : normales)
	{
		glm::vec3 e1 = (fabsf(n.y) > 0.5f) ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);	//dos ejes sobre la cara
		glm::vec3 e2 = glm::cross(n, e1);
		unsigned int b = (unsigned int)(v.size() / 8);
		AgregarVertice(v, 0.5f * (n - e1 - e2), 0, 0, n);
		AgregarVertice(v, 0.5f * (n + e1 - e2), 1, 0, n);
		AgregarVertice(v, 0.5f * (n + e1 + e2), 1, 1, n);
		AgregarVertice(v, 0.5f * (n - e1 + e2), 0, 1, n);
		idx.insert(idx.end(), { b, b + 1, b + 2, b, b + 2, b + 3 });
	}
	return GuardarMalla(v, idx);
}


//Tapa circular horizontal de radio r a la altura y; normal hacia arriba (+1) o hacia abajo (-1)
static void AgregarTapa(std::vector<GLfloat>& v, std::vector<unsigned int>& idx, float y, float r, float sentido, int lados)
{
	unsigned int b = (unsigned int)(v.size() / 8);	//centro
	AgregarVertice(v, glm::vec3(0.0f, y, 0.0f), 0.5f, 0.5f, glm::vec3(0.0f, sentido, 0.0f));
	for (int k = 0; k <= lados; k++)
	{
		float a = 2.0f * PI * k / lados;
		AgregarVertice(v, glm::vec3(r * cosf(a), y, r * sinf(a)), 0.5f + 0.5f * cosf(a), 0.5f + 0.5f * sinf(a), glm::vec3(0.0f, sentido, 0.0f));
		if (k > 0) idx.insert(idx.end(), { b, b + k, b + k + 1 });
	}
}


//Cilindro de radio 1 y alto 1 centrado en el origen: pared (u da la vuelta, v sube) y dos tapas
static MeshModel* CrearCilindro(int lados)
{
	std::vector<GLfloat> v;
	std::vector<unsigned int> idx;
	for (int k = 0; k <= lados; k++)
	{
		float a = 2.0f * PI * k / lados;
		glm::vec3 n(cosf(a), 0.0f, sinf(a));
		AgregarVertice(v, glm::vec3(n.x, -0.5f, n.z), (float)k / lados, 0, n);
		AgregarVertice(v, glm::vec3(n.x, 0.5f, n.z), (float)k / lados, 1, n);
		if (k > 0)
		{
			unsigned int b = 2 * (k - 1);
			idx.insert(idx.end(), { b, b + 1, b + 3, b, b + 3, b + 2 });
		}
	}
	AgregarTapa(v, idx, -0.5f, 1.0f, -1.0f, lados);
	AgregarTapa(v, idx, 0.5f, 1.0f, 1.0f, lados);
	return GuardarMalla(v, idx);
}


//Tronco de cono: base de radio 1 en y = 0 y tapa de radio r en y = 1 (con r = 0 es un cono).
//La pared va de (1, 0) a (r, 1) en el plano radio-altura; su normal hacia afuera es (1, 1 - r) en ese plano
MeshModel* CrearTroncoCono(float radioSuperior, int lados)
{
	std::vector<GLfloat> v;
	std::vector<unsigned int> idx;
	for (int k = 0; k <= lados; k++)
	{
		float a = 2.0f * PI * k / lados;
		glm::vec3 n = glm::normalize(glm::vec3(cosf(a), 1.0f - radioSuperior, sinf(a)));
		AgregarVertice(v, glm::vec3(cosf(a), 0.0f, sinf(a)), (float)k / lados, 0, n);
		AgregarVertice(v, glm::vec3(radioSuperior * cosf(a), 1.0f, radioSuperior * sinf(a)), (float)k / lados, 1, n);
		if (k > 0)
		{
			unsigned int b = 2 * (k - 1);
			idx.insert(idx.end(), { b, b + 1, b + 3, b, b + 3, b + 2 });
		}
	}
	AgregarTapa(v, idx, 0.0f, 1.0f, -1.0f, lados);
	if (radioSuperior > 0.0f) AgregarTapa(v, idx, 1.0f, radioSuperior, 1.0f, lados);
	return GuardarMalla(v, idx);
}


//Esfera de radio 1: en una esfera la normal es la misma posición. u da la vuelta y v va de 1 (polo norte) a 0,
//así una textura equirectangular (mapa de planeta) queda derecha
MeshModel* CrearEsfera(int pisos, int lados)
{
	std::vector<GLfloat> v;
	std::vector<unsigned int> idx;
	for (int i = 0; i <= pisos; i++)
	{
		float fi = PI * i / pisos;	//de 0 (polo de arriba) a pi
		for (int k = 0; k <= lados; k++)
		{
			float a = 2.0f * PI * k / lados;
			glm::vec3 p(sinf(fi) * cosf(a), cosf(fi), -sinf(fi) * sinf(a));
			AgregarVertice(v, p, (float)k / lados, 1.0f - (float)i / pisos, p);
			if (i > 0 && k > 0)
			{
				unsigned int actual = i * (lados + 1) + k, arriba = actual - (lados + 1);
				idx.insert(idx.end(), { arriba - 1, arriba, actual, arriba - 1, actual, actual - 1 });
			}
		}
	}
	return GuardarMalla(v, idx);
}


//Plano de 1 x 1 sobre XZ, centrado y con la cara hacia +Y
static MeshModel* CrearPlano()
{
	std::vector<GLfloat> v;
	std::vector<unsigned int> idx = { 0, 2, 1, 1, 2, 3 };
	glm::vec3 arriba(0.0f, 1.0f, 0.0f);
	AgregarVertice(v, glm::vec3(-0.5f, 0.0f, -0.5f), 0, 1, arriba);
	AgregarVertice(v, glm::vec3(0.5f, 0.0f, -0.5f), 1, 1, arriba);
	AgregarVertice(v, glm::vec3(-0.5f, 0.0f, 0.5f), 0, 0, arriba);
	AgregarVertice(v, glm::vec3(0.5f, 0.0f, 0.5f), 1, 0, arriba);
	return GuardarMalla(v, idx);
}


//Ruido de valor: un número pseudoaleatorio en cada punto entero de la rejilla y una interpolación suave entre ellos
static float Aleatorio3(int x, int y, int z)
{
	unsigned int h = (unsigned int)x * 374761393u + (unsigned int)y * 668265263u + (unsigned int)z * 2246822519u;
	h = (h ^ (h >> 13)) * 1274126177u;
	h ^= h >> 16;
	return (h & 0xFFFFFF) / (float)0xFFFFFF * 2.0f - 1.0f;
}

float Ruido3(glm::vec3 p)
{
	glm::vec3 base = glm::floor(p);
	glm::vec3 f = p - base;
	glm::vec3 s = f * f * (3.0f - 2.0f * f);	//curva suave para que no se noten las orillas de la rejilla
	int x = (int)base.x, y = (int)base.y, z = (int)base.z;
	float c000 = Aleatorio3(x, y, z), c100 = Aleatorio3(x + 1, y, z);
	float c010 = Aleatorio3(x, y + 1, z), c110 = Aleatorio3(x + 1, y + 1, z);
	float c001 = Aleatorio3(x, y, z + 1), c101 = Aleatorio3(x + 1, y, z + 1);
	float c011 = Aleatorio3(x, y + 1, z + 1), c111 = Aleatorio3(x + 1, y + 1, z + 1);
	float x00 = c000 + (c100 - c000) * s.x, x10 = c010 + (c110 - c010) * s.x;
	float x01 = c001 + (c101 - c001) * s.x, x11 = c011 + (c111 - c011) * s.x;
	float y0 = x00 + (x10 - x00) * s.y, y1 = x01 + (x11 - x01) * s.y;
	return y0 + (y1 - y0) * s.z;
}

float Fbm3(glm::vec3 p, int octavas)
{
	float suma = 0.0f, amplitud = 1.0f, total = 0.0f;
	for (int i = 0; i < octavas; i++)
	{
		suma += amplitud * Ruido3(p);
		total += amplitud;
		amplitud *= 0.5f;
		p *= 2.03f;
	}
	return suma / total;
}


//Roca: esfera cuyo radio cambia con el ruido; un poco aplastada para que asiente en el piso. Como la superficie ya no
//es una esfera, las normales se calculan promediando las de los triángulos que tocan a cada vértice
MeshModel* CrearRoca(int semilla)
{
	const int PISOS = 10, LADOS = 16;
	std::vector<GLfloat> v;
	std::vector<unsigned int> idx;
	glm::vec3 desfase(semilla * 7.31f, semilla * 3.17f, semilla * 5.43f);
	for (int i = 0; i <= PISOS; i++)
	{
		float fi = PI * i / PISOS;
		for (int k = 0; k <= LADOS; k++)
		{
			float a = 2.0f * PI * (k % LADOS) / LADOS;	//la última columna repite la primera para cerrar sin grieta
			glm::vec3 d(sinf(fi) * cosf(a), cosf(fi), -sinf(fi) * sinf(a));
			float r = 1.0f + 0.35f * Fbm3(d * 1.8f + desfase, 3);
			glm::vec3 p = d * r * glm::vec3(1.0f, 0.75f, 1.0f);
			AgregarVertice(v, p, 2.0f * k / LADOS, 1.0f - (float)i / PISOS, glm::vec3(0.0f));
			if (i > 0 && k > 0)
			{
				unsigned int actual = i * (LADOS + 1) + k, arriba = actual - (LADOS + 1);
				idx.insert(idx.end(), { arriba - 1, arriba, actual, arriba - 1, actual, actual - 1 });
			}
		}
	}
	//normales: suma de las normales de cada triángulo; se orienta hacia afuera (mismo lado que la posición)
	std::vector<glm::vec3> n(v.size() / 8, glm::vec3(0.0f));
	for (size_t t = 0; t < idx.size(); t += 3)
	{
		glm::vec3 p0(v[idx[t] * 8], v[idx[t] * 8 + 1], v[idx[t] * 8 + 2]);
		glm::vec3 p1(v[idx[t + 1] * 8], v[idx[t + 1] * 8 + 1], v[idx[t + 1] * 8 + 2]);
		glm::vec3 p2(v[idx[t + 2] * 8], v[idx[t + 2] * 8 + 1], v[idx[t + 2] * 8 + 2]);
		glm::vec3 c = glm::cross(p1 - p0, p2 - p0);
		if (glm::dot(c, p0 + p1 + p2) < 0.0f) c = -c;
		n[idx[t]] += c; n[idx[t + 1]] += c; n[idx[t + 2]] += c;
	}
	for (size_t i = 0; i < n.size(); i++)
	{
		glm::vec3 m = glm::length(n[i]) > 0.0f ? glm::normalize(n[i]) : glm::vec3(0.0f, 1.0f, 0.0f);
		v[i * 8 + 5] = -m.x; v[i * 8 + 6] = -m.y; v[i * 8 + 7] = -m.z;	//hacia el interior, como AgregarVertice
	}
	return GuardarMalla(v, idx);
}


void CrearPrimitivas()
{
	mCubo = CrearCubo();
	mCilindro = CrearCilindro(24);
	mCono = CrearTroncoCono(0.0f, 24);
	mEsfera = CrearEsfera(24, 36);
	mEsferaBaja = CrearEsfera(8, 12);
	mPlano = CrearPlano();
	matBrillante = Material(1.2f, 48);
	matOpaco = Material(0.25f, 4);
	matMetal = Material(2.5f, 128);
}

void UsarMaterial(Material& m)
{
	m.UseMaterial(U.especular, U.brillo);
}

Texture* CargarTextura(const char* ruta)
{
	Texture* t = new Texture(ruta);
	t->LoadTextureA();
	return t;
}


void Dibujar(MeshModel* malla, const glm::mat4& model, glm::vec3 color, float emision)
{
	glUniformMatrix4fv(U.model, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(U.color, 1, glm::value_ptr(color));
	glUniform1f(U.emision, emision);
	malla->RenderMeshModel();
}

void DibujarModelo(Model& modelo, const glm::mat4& model, float emision)
{
	glUniformMatrix4fv(U.model, 1, GL_FALSE, glm::value_ptr(model));
	glUniform1f(U.emision, emision);
	modelo.RenderModel(U.color);
}
