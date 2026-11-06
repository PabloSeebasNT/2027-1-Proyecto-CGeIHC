/*
Animación:
Simple o básica:Por banderas y condicionales (más de 1 transformación geométrica se ve modificada)
Compleja: Por medio de funciones y algoritmos.
Textura Animada
APUNTES PROFE
ANIMACION BASICA
- MODIFICAR  MAS DE 1 TRANSFORMACION GEOMETRICA

- CONTEXTO DE LA ANIMACION

// LA JERARQUIA DEL ROVER
SOLO MODIFICAR EL TRANSLATE Y EL ROTATE


*/
//para cargar imagen
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <math.h>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>
//para probar el importer
//#include<assimp/Importer.hpp>

#include "Window.h"
#include "Mesh_tn.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Sphere.h"
#include"Model.h"
#include "Skybox.h"

//para iluminación
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"
const float toRadians = 3.14159265f / 180.0f;

//variables para animación
bool avanza;

float toffsetflechau = 0.0f;
float toffsetflechav = 0.0f;
float toffsetnumerou = 0.0f;
float toffsetnumerov = 0.0f;
float toffsetnumerocambiau = 0.0;
float angulovaria = 0.0f;
float movCoche = 0.0f;
float movOffset = 0.01f;





Window mainWindow;
std::vector<MeshModel*> meshList;
std::vector<Shader> shaderList;

Camera camera;

Texture brickTexture;
Texture dirtTexture;
Texture plainTexture;
Texture pisoTexture;
Texture AgaveTexture;
Texture FlechaTexture;
Texture NumerosTexture;
Texture Numero1Texture;
Texture Numero2Texture;
Texture pistaTexture;



Model Rover_M;
Model nave_carrera;

Model satelite;


//avio
Model Blackhawk_M;
//ELEMENTOS MUNDO SNOOPY
Model casaSnoopy;
Model lunaSnoopy;\
Model estrellaWoodstock;


Skybox skybox;

//materiales
Material Material_brillante;
Material Material_opaco;


//Sphere cabeza = Sphere(0.5, 20, 20);
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

// luz direccional
DirectionalLight mainLight;
//para declarar varias luces de tipo pointlight
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

// Vertex Shader
static const char* vShader = "shaders/shader_light.vert";

// Fragment Shader
static const char* fShader = "shaders/shader_light.frag";



//cálculo del promedio de las normales para sombreado de Phong
void calcAverageNormals(unsigned int* indices, unsigned int indiceCount, GLfloat* vertices, unsigned int verticeCount,
	unsigned int vLength, unsigned int normalOffset)
{
	for (size_t i = 0; i < indiceCount; i += 3)
	{
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);

		in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
		vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
	}

	for (size_t i = 0; i < verticeCount / vLength; i++)
	{
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
	}
}


void CreateObjects()
{
	unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	GLfloat vertices[] = {
		//	x      y      z			u	  v			nx	  ny    nz
			-1.0f, -1.0f, -0.6f,	0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 1.0f,		0.5f, 0.0f,		0.0f, 0.0f, 0.0f,
			1.0f, -1.0f, -0.6f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,		0.5f, 1.0f,		0.0f, 0.0f, 0.0f
	};

	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat floorVertices[] = {
		-10.0f, 0.0f, -10.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-10.0f, 0.0f, 10.0f,	0.0f, 10.0f,	0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, 10.0f,		10.0f, 10.0f,	0.0f, -1.0f, 0.0f
	};
	unsigned int vegetacionIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	   4,5,6,
	   4,6,7
	};

	GLfloat vegetacionVertices[] = {
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.0f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, 0.5f, 0.0f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		-0.5f, 0.5f, 0.0f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,

		0.0f, -0.5f, -0.5f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.5f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.5f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, -0.5f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,


	};


	unsigned int flechaIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	};

	GLfloat flechaVertices[] = {
		-0.5f, 0.0f, 0.5f,		0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, 0.5f,		1.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, -0.5f,		1.0f, 1.0f,		0.0f, -1.0f, 0.0f,
		-0.5f, 0.0f, -0.5f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,

	};

	unsigned int scoreIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	};

	GLfloat scoreVertices[] = {
		-0.5f, 0.0f, 0.5f,		0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, 0.5f,		1.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, -0.5f,		1.0f, 1.0f,		0.0f, -1.0f, 0.0f,
		-0.5f, 0.0f, -0.5f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,

	};

	unsigned int numeroIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	};

	GLfloat numeroVertices[] = {
		-0.5f, 0.0f, 0.5f,		0.0f, 0.67f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, 0.5f,		0.25f, 0.67f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, -0.5f,		0.25f, 1.0f,		0.0f, -1.0f, 0.0f,
		-0.5f, 0.0f, -0.5f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,

	};

	MeshModel* obj1 = new MeshModel();
	obj1->CreateMeshModel(vertices, indices, 32, 12);
	meshList.push_back(obj1);

	MeshModel* obj2 = new MeshModel();
	obj2->CreateMeshModel(vertices, indices, 32, 12);
	meshList.push_back(obj2);

	MeshModel* obj3 = new MeshModel();
	obj3->CreateMeshModel(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);


	MeshModel* obj4 = new MeshModel();
	obj4->CreateMeshModel(vegetacionVertices, vegetacionIndices, 64, 12);
	meshList.push_back(obj4);

	MeshModel* obj5 = new MeshModel();
	obj5->CreateMeshModel(flechaVertices, flechaIndices, 32, 6);
	meshList.push_back(obj5);

	MeshModel* obj6 = new MeshModel();
	obj6->CreateMeshModel(scoreVertices, scoreIndices, 32, 6);
	meshList.push_back(obj6); // todos los números

	MeshModel* obj7 = new MeshModel();
	obj7->CreateMeshModel(numeroVertices, numeroIndices, 32, 6);
	meshList.push_back(obj7); // solo un número

}


void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}




int main()
{



	mainWindow = Window(1366, 768); // 1280, 1024 or 1024, 768
	mainWindow.Initialise();

	CreateObjects();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.5f, 0.5f);

	brickTexture = Texture("Textures/brick.png");
	brickTexture.LoadTextureA();
	dirtTexture = Texture("Textures/dirt.png");
	dirtTexture.LoadTextureA();
	plainTexture = Texture("Textures/plain.png");
	plainTexture.LoadTextureA();
	pisoTexture = Texture("Textures/piso.tga");
	pisoTexture.LoadTextureA();
	AgaveTexture = Texture("Textures/Agave.tga");
	AgaveTexture.LoadTextureA();
	FlechaTexture = Texture("Textures/flechas.tga");
	FlechaTexture.LoadTextureA();
	NumerosTexture = Texture("Textures/numerosbase.tga");
	NumerosTexture.LoadTextureA();
	Numero1Texture = Texture("Textures/numero1.tga");
	Numero1Texture.LoadTextureA();
	Numero2Texture = Texture("Textures/numero2.tga");
	Numero2Texture.LoadTextureA();
	pistaTexture = Texture("Textures/pista.png");
	pistaTexture.LoadTextureA();

	

	nave_carrera = Model();
	nave_carrera.LoadModel("Models/nave_carrera.obj");

	satelite = Model();
	satelite.LoadModel("Models/satelite.obj");
	//carga modelos snoopy
	casaSnoopy = Model();
	casaSnoopy.LoadModel("Models/casaSnoopy.obj");

	lunaSnoopy = Model();
	lunaSnoopy.LoadModel("Models/lunaSnoopy.obj");

	estrellaWoodstock = Model();
	estrellaWoodstock.LoadModel("Models/estrellaWoodstock.obj");

	//avion 


	//skybox
	//glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/sky_right.png");
	skyboxFaces.push_back("Textures/sky_left.png");
	skyboxFaces.push_back("Textures/sky_down.png");
	skyboxFaces.push_back("Textures/sky_up.png");
	skyboxFaces.push_back("Textures/sky_front.png");
	skyboxFaces.push_back("Textures/sky_back.png");

	skybox = Skybox(skyboxFaces);

	Material_brillante = Material(4.0f, 256);
	Material_opaco = Material(0.3f, 4);

	//--------------------------------------------------
	// 
	// LUCES
	// --------------------------------------
	//luz direccional, sólo 1 y siempre debe de existir
	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,
		0.3f, 0.3f,
		0.0f, 0.0f, -1.0f);
	//contador de luces puntuales
	unsigned int pointLightCount = 0;
	//Declaración de primer luz puntual
	pointLights[0] = PointLight(1.0f, 0.0f, 0.0f,
		0.0f, 1.0f,
		0.0f, 2.5f, 1.5f,
		0.3f, 0.2f, 0.1f);
	pointLightCount++;

	unsigned int spotLightCount = 0;
	//linterna
	spotLights[0] = SpotLight(1.0f, 1.0f, 1.0f,
		0.0f, 2.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		5.0f);
	spotLightCount++;

	//luz fija
	spotLights[1] = SpotLight(0.0f, 0.0f, 1.0f,
		1.0f, 2.0f,
		5.0f, 10.0f, 0.0f,
		0.0f, -5.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		15.0f);
	spotLightCount++;

	//avion ejex
	spotLights[2] = SpotLight(
		0.0f, 0.0f, 1.0f,  // color  
		0.0f,               // sin ambiente
		8.0f,               // intensidad difusa
		0.0f, 5.0f, 6.0f, // posición inicial 
		0.0f, 0.0f, -1.0f, // dirección inicial 
		1.0f, 0.0f, 0.0f, // atenua 
		20.0f               // 
	);
	spotLightCount++; // 4

	//avion ejey
	spotLights[3] = SpotLight(
		0.5f, 0.0f, 0.8f,  // color  
		0.0f,               // sin ambiente
		8.0f,               // intensidad difusa
		0.0f, 5.0f, 6.0f, // pos
		0.0f, 0.0f, -1.0f, // dir
		1.0f, 0.0f, 0.0f, // atenuac
		20.0f               // 
	);
	spotLightCount++; // 

	//-------------------------------------------------------------------
	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0, uniformTextureOffset = 0;
	GLuint uniformColor = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

	//---------------------------------
	//CIRCUITO DE LA NAVE (rectas, curvas y rampas)
	std::vector<glm::vec3> puntosPista;            // todos los puntos por donde pasa la nave
	float pasoPista = 0.25f;                       // distancia entre un punto y el siguiente
	glm::vec3 origenPista(-35.0f, 1.0f, -110.0f);  // donde esta la pista en el mundo
	float anchoPista = 9.0f;                       // ancho de la pista
	float velocidadNave = 0.3f;                    // velocidad de la nave
	float distanciaNave = 0.0f;                    // cuanto ha recorrido (solo aumenta, nunca va de reversa)
	float alturaNave = 0.6f;                       // que tan arriba del asfalto flota la nave
	float giroModelo = 90.0f;                      // para que la nave vaya de frente (prueba 90, -90, 180)

	//La pista es una lista de tramos (se recorre 2 veces: ida y regreso, asi cierra el circuito)
	// tipoTramo:   0 = recta, 1 = curva
	// medidaTramo: largo si es recta, radio si es curva
	// gradosTramo: solo curvas (+ derecha, - izquierda)
	// subidaTramo: cuanto sube (+) o baja (-) en ese tramo
	//                      recta  rampa  recta  rampa  chicana chicana recta  curva
	int tipoTramo[8] = { 0, 0, 0, 0, 1, 1, 0, 1 };
	float medidaTramo[8] = { 12.0f, 14.0f, 10.0f, 14.0f, 10.0f, 10.0f, 12.0f, 25.0f };
	float gradosTramo[8] = { 0.0f, 0.0f, 0.0f, 0.0f, 40.0f, -40.0f, 0.0f, 180.0f };
	float subidaTramo[8] = { 0.0f, 5.0f, 0.0f, -5.0f, 0.0f, 0.0f, 0.0f, 0.0f };

	//se arma la lista de puntos con una "tortuga" que camina por la pista
	float pistaX = 0.0f, pistaY = 0.0f, pistaZ = 0.0f; // donde va la tortuga
	float rumbo = 0.0f;                                // hacia donde mira (0 = hacia +X)
	for (int vuelta = 0; vuelta < 2; vuelta++)
	{
		for (int tramo = 0; tramo < 8; tramo++)
		{
			int pasos;             // cuantos puntos tiene el tramo
			float giroPaso = 0.0f; // cuanto gira en cada paso
			float cuerda;          // cuanto avanza en cada paso
			if (tipoTramo[tramo] == 0) // recta
			{
				pasos = (int)(medidaTramo[tramo] / pasoPista + 0.5f);
				cuerda = medidaTramo[tramo] / pasos;
			}
			else // curva
			{
				float giroTotal = gradosTramo[tramo] * toRadians;
				pasos = (int)(medidaTramo[tramo] * fabs(giroTotal) / pasoPista + 0.5f);
				giroPaso = giroTotal / pasos;
				cuerda = 2.0f * medidaTramo[tramo] * sin(fabs(giroPaso) / 2.0f);
			}
			float yInicial = pistaY;
			for (int k = 0; k < pasos; k++)
			{
				float t = (float)k / pasos;
				float suave = t * t * (3.0f - 2.0f * t); // para que la rampa empiece y termine suave
				puntosPista.push_back(glm::vec3(pistaX, yInicial + subidaTramo[tramo] * suave, pistaZ));
				pistaX += cos(rumbo + giroPaso / 2.0f) * cuerda;
				pistaZ += sin(rumbo + giroPaso / 2.0f) * cuerda;
				rumbo += giroPaso;
			}
			pistaY = yInicial + subidaTramo[tramo];
		}
	}
	int totalPuntos = puntosPista.size();

	//MALLA DE LA PISTA: el asfalto y los muros que la sostienen
	float sueloY = -2.0f - origenPista.y + 0.01f; // el piso del mundo esta en y = -2
	float largoPista = totalPuntos * pasoPista;
	float largoTextura = largoPista / floor(largoPista / 10.0f + 0.5f); // la textura se repite cada ~10 unidades
	std::vector<GLfloat> verticesPista;
	std::vector<unsigned int> indicesPista;
	std::vector<GLfloat> verticesMuros;
	std::vector<unsigned int> indicesMuros;
	for (int i = 0; i <= totalPuntos; i++) // hasta totalPuntos para cerrar la vuelta
	{
		glm::vec3 punto = puntosPista[i % totalPuntos];
		glm::vec3 direccion = puntosPista[(i + 1) % totalPuntos] - puntosPista[(i - 1 + totalPuntos) % totalPuntos];
		glm::vec3 derecha = glm::normalize(glm::vec3(-direccion.z, 0.0f, direccion.x)); // lado derecho de la pista
		glm::vec3 bordeIzq = punto - derecha * (anchoPista / 2.0f);
		glm::vec3 bordeDer = punto + derecha * (anchoPista / 2.0f);
		float v = i * pasoPista / largoTextura;
		float u = i * pasoPista / 4.0f;

		//asfalto: un vertice en cada borde (la normal va hacia abajo, igual que tu piso)
		GLfloat asfalto[] = {
			bordeIzq.x, bordeIzq.y, bordeIzq.z,   0.0f, v,   0.0f, -1.0f, 0.0f,
			bordeDer.x, bordeDer.y, bordeDer.z,   1.0f, v,   0.0f, -1.0f, 0.0f };
		verticesPista.insert(verticesPista.end(), asfalto, asfalto + 16);

		//muros: izquierdo (arriba y abajo) y derecho (arriba y abajo)
		GLfloat muros[] = {
			bordeIzq.x, bordeIzq.y, bordeIzq.z,   u, (bordeIzq.y - sueloY) / 4.0f,   derecha.x, 0.0f, derecha.z,
			bordeIzq.x, sueloY,     bordeIzq.z,   u, 0.0f,                           derecha.x, 0.0f, derecha.z,
			bordeDer.x, bordeDer.y, bordeDer.z,   u, (bordeDer.y - sueloY) / 4.0f,   -derecha.x, 0.0f, -derecha.z,
			bordeDer.x, sueloY,     bordeDer.z,   u, 0.0f,                           -derecha.x, 0.0f, -derecha.z };
		verticesMuros.insert(verticesMuros.end(), muros, muros + 32);
	}
	for (int i = 0; i < totalPuntos; i++)
	{
		//cada par de puntos con el siguiente forma 2 triangulos
		unsigned int a = 2 * i, b = 2 * i + 1, c = 2 * i + 2, d = 2 * i + 3;
		unsigned int triAsfalto[] = { a, b, c, b, d, c };
		indicesPista.insert(indicesPista.end(), triAsfalto, triAsfalto + 6);

		unsigned int w = 4 * i;
		unsigned int triMuros[] = { w, w + 1, w + 4, w + 1, w + 5, w + 4,       // muro izquierdo
									w + 2, w + 6, w + 3, w + 3, w + 6, w + 7 }; // muro derecho
		indicesMuros.insert(indicesMuros.end(), triMuros, triMuros + 12);
	}
	MeshModel* pistaMesh = new MeshModel();
	pistaMesh->CreateMeshModel(verticesPista.data(), indicesPista.data(), (unsigned int)verticesPista.size(), (unsigned int)indicesPista.size());
	MeshModel* murosMesh = new MeshModel();
	murosMesh->CreateMeshModel(verticesMuros.data(), indicesMuros.data(), (unsigned int)verticesMuros.size(), (unsigned int)indicesMuros.size());

	//variables que se usan en cada ciclo para mover la nave
	int actual = 0;                          // punto de la pista donde esta la nave
	int siguiente = 0;                       // el punto que sigue
	float fraccion = 0.0f;                   // que tan cerca esta del siguiente punto (0 a 1)
	glm::vec3 posNave(0.0f, 0.0f, 0.0f);     // posicion de la nave
	glm::vec3 adelante(1.0f, 0.0f, 0.0f);    // hacia donde avanza
	float giroNave = 0.0f;                   // giro para mirar hacia donde avanza
	float inclinacionNave = 0.0f;            // sube o baja la nariz en las rampas
	//TRAYECTORIA DEL SATELITE (un circulo a una altura fija)
	glm::vec3 centroOrbita(0.0f, 6.0f, -20.0f);  // centro del circulo (la altura en Y es la del vuelo)
	float radioOrbita = 8.0f;                     // radio del circulo (entre mas chico, mas corta la trayectoria)
	float anguloSatelite = 0.0f;                  // en que parte de la orbita va (solo aumenta)
	float velocidadSatelite = 0.01f;              // velocidad en la orbita
	float giroPropioSatelite = 0.0f;              // giro del satelite sobre si mismo
	glm::vec3 posSatelite(0.0f, 0.0f, 0.0f);      // posicion del satelite




	//---------------------------------
	glm::vec3 lowerLight(0.0f, 0.0f, 0.0f);

	glm::mat4 model(1.0);
	glm::mat4 modelaux(1.0);
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec2 toffset = glm::vec2(0.0f, 0.0f);

	//planeta
	Sphere planeta = Sphere(10.0f, 40, 40);   // global: radio 10, 40 cortes
	Texture planetaTexture;                   // global, junto a las otras texturas

	// en el main, antes del while 
	planetaTexture = Texture("Textures/planeta3.jpg");
	planetaTexture.LoadTextureA();
	planeta.init();
	planeta.load();


	// antes del while
	glm::vec3 centroPlaneta(-40.0f, 50.0f, -30.0f);
	float giroPlaneta = 0.0f;

	
	////Loop mientras no se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		angulovaria += 0.5f * deltaTime;


		//Recibir eventos del usuario
		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// Clear the window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		skybox.DrawSkybox(camera.calculateViewMatrix(), projection);
		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformEyePosition = shaderList[0].GetEyePositionLocation();
		uniformColor = shaderList[0].getColorLocation();
		uniformTextureOffset = shaderList[0].getOffsetLocation(); // para la textura con movimiento

		//información en el shader de intensidad especular y brillo
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		// luz ligada a la cámara de tipo flash
		lowerLight = camera.getCameraPosition();
		lowerLight.y -= 0.3f;
		spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

		//información al shader de fuentes de iluminación
		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);
		//




		//Reinicializando variables cada ciclo de reloj
		model = glm::mat4(1.0);
		modelaux = glm::mat4(1.0);
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		toffset = glm::vec2(0.0f, 0.0f);

		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		pisoTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

		meshList[2]->RenderMeshModel();





		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);




		//============pista y nave
		//PISTA DE CARRERAS
		model = glm::mat4(1.0);
		model = glm::translate(model, origenPista);
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		pistaTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		pistaMesh->RenderMeshModel(); // el asfalto
		brickTexture.UseTexture();
		murosMesh->RenderMeshModel(); // los muros que sostienen la pista

		//NAVE: avanza por la pista
		distanciaNave += velocidadNave * deltaTime; // solo suma, nunca va de reversa
		if (distanciaNave >= largoPista)
			distanciaNave -= largoPista; // termino la vuelta, empieza otra
		actual = (int)(distanciaNave / pasoPista) % totalPuntos;
		siguiente = (actual + 1) % totalPuntos;
		fraccion = distanciaNave / pasoPista - (int)(distanciaNave / pasoPista);
		posNave = glm::mix(puntosPista[actual], puntosPista[siguiente], fraccion); // entre dos puntos
		//hacia donde mira: del punto de atras al punto de adelante
		adelante = puntosPista[(actual + 2) % totalPuntos] - puntosPista[(actual - 2 + totalPuntos) % totalPuntos];
		giroNave = atan2(-adelante.z, adelante.x);                                       // gira hacia donde avanza
		inclinacionNave = atan2(adelante.y, sqrt(adelante.x * adelante.x + adelante.z * adelante.z)); // sube o baja la nariz

		model = glm::mat4(1.0);
		model = glm::translate(model, origenPista + posNave + glm::vec3(0.0f, alturaNave, 0.0f)); // donde esta en la pista
		model = glm::rotate(model, giroNave, glm::vec3(0.0f, 1.0f, 0.0f));                       // gira hacia donde avanza
		model = glm::rotate(model, inclinacionNave, glm::vec3(0.0f, 0.0f, 1.0f));                // sube/baja la nariz
		model = glm::rotate(model, giroModelo * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));         // corrige la orientacion del modelo
		model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		nave_carrera.RenderModel();


		//satelite: vuela siguiendo su orbita
		anguloSatelite += velocidadSatelite * deltaTime; // solo suma, siempre avanza en la orbita
		if (anguloSatelite > 360.0f * toRadians)
			anguloSatelite -= 360.0f * toRadians; // completo una vuelta, empieza otra
		giroPropioSatelite += 1.0f * deltaTime;       // el satelite gira sobre si mismo (grados)
		posSatelite.x = centroOrbita.x + radioOrbita * cos(anguloSatelite);
		posSatelite.z = centroOrbita.z + radioOrbita * sin(anguloSatelite);
		posSatelite.y = centroOrbita.y; // siempre a la misma altura

		model = glm::mat4(1.0);
		model = glm::translate(model, posSatelite);                                          // donde esta en la orbita
		model = glm::rotate(model, giroPropioSatelite * toRadians, glm::vec3(0.0f, 1.0f, 0.0f)); // gira sobre si mismo
		model = glm::scale(model, glm::vec3(0.8f, 0.8f, 0.8f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		satelite.RenderModel();

		//Agave ¿qué sucede si lo renderizan antes del Rover?
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 0.5f, -3.0f));
		model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		//blending: transparencia o traslucidez
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		AgaveTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[3]->RenderMeshModel();

		//textura con movimiento
		//Importantes porque la variable uniform no podemos modificarla directamente
		toffsetflechau += 0.001;
		toffsetflechav = 0.000;
		//para que no se desborde la variable
		if (toffsetflechau > 1.0)
			toffsetflechau = 0.0;
		//if (toffsetv > 1.0)
		//	toffsetv = 0;
		//printf("\ntfosset %f \n", toffsetu);
		//pasar a la variable uniform el valor actualizado
		toffset = glm::vec2(toffsetflechau, toffsetflechav);

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-2.0f, 1.0f, -6.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 0.0f, 0.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		FlechaTexture.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMeshModel();


		//luna
		model = glm::mat4(1.0);                      // la matriz del planeta
		model = glm::translate(model, glm::vec3(30.0f, 50.0f, 0.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.0f, 5.0f, 5.0f));\
		color = glm::vec3(1.0f, 1.0f, 1.0f);                       
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		lunaSnoopy.RenderModel();

		//estrella 
		model = glm::mat4(1.0);                      // la matriz del planeta
		model = glm::translate(model, glm::vec3(90.0f, 50.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.0f, 5.0f, 5.0f));\
		color = glm::vec3(1.0f, 1.0f, 0.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		estrellaWoodstock.RenderModel();
		


		giroPlaneta += 0.2f * deltaTime;
		model = glm::mat4(1.0);
		model = glm::translate(model, centroPlaneta);
		model = glm::rotate(model, giroPlaneta * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		modelaux = model;                       // padre: el planeta

		//casa snoopy
		//hijo: un elemento adentro (cambia los numeros para acomodarlo)
		model = modelaux;
		model = glm::translate(model, glm::vec3(2.0f, -4.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		casaSnoopy.RenderModel();


		//Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		//meshList[5]->RenderMeshModel();

		

		model = modelaux;       // la matriz del planeta
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		planetaTexture.UseTexture();
		color = glm::vec3(1.0f, 1.0f, 1.0f);                      
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		planeta.render();

		glDisable(GL_BLEND);

		

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}
