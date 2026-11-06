/*
Práctica 7: Iluminación 1
*/

/*
Práctica 8: Iluminación 2

a nivel shader no se hacen cambios sino aca
para cambiar , arreglo1 y arreglo2
si quieres apagar la otra luz como la verde o la roja, se tendria que crear dos arrgelos mas
en el main
if arreglo1()
	if(prendida)
	shaderList(0).setPointLights(pointhLights, pointLightCount);

	else{}
else(){
	if(predida)
	shaderLIst(0).setPointLights(arreglo2,pointlightCount);

	else
	sahderList(0).setPonitLight(arreglo2,pointLightCount-1);

despues en el main
shaderList[0].setSpotLights(SpotLights.spotLightCount);

explicacion
lo de la camra tiene que ver

para la luz del sol no se apaga
se puede poner color negro o mandar lejos de la camara



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
//#include "Sphere.h"
#include"Model.h"
#include "Skybox.h"

//para iluminación
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"
const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<MeshModel*> meshListModel; // recibe xyz uv nx ny nz
std::vector<Shader> shaderList;

Camera camera;

Texture brickTexture;
Texture dirtTexture;
Texture plainTexture;
Texture pisoTexture;
Texture AgaveTexture;
// Añadir Texturas del dado y del holocron
Texture dadoTexture;
Texture holocronTexture;

//Texture dado;////// anadir texturas del dado

Model Dado_M;
Model Holocron_M;

//Rover = Model();
//Rover_Mod.LoadModel("Models/cubo_starsw.obj");

Model Blackhawk_M;
Model Rover_M; //Sustituir por sus propios Rovers Con jerarquía
//Añadir modelo del avión.
//Model Avion_M;

//nave
Model Nave_M;
//fuego 
Model Fuego_M;

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


//función de calculo de normales por promedio de vértices 
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
	//agavee
	GLfloat vegetacionVertices[] = {
		//nz
-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,		1.0f, 0.0f, -1.0f, //nz esta cero por eso no tiene iluminacion ni colro
0.5f, -0.5f, 0.0f,		1.0f, 0.0f,		1.0f, 0.0f, -1.0f,
0.5f, 0.5f, 0.0f,		1.0f, 1.0f,		1.0f, 0.0f, -1.0f,
-0.5f, 0.5f, 0.0f,		0.0f, 1.0f,		1.0f, 0.0f, -1.0f,

0.0f, -0.5f, -0.5f,		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
0.0f, -0.5f, 0.5f,		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
0.0f, 0.5f, 0.5f,		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
0.0f, 0.5f, -0.5f,		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,


	};

	MeshModel* obj1 = new MeshModel();
	obj1->CreateMeshModel(vertices, indices, 32, 12);
	meshListModel.push_back(obj1);

	MeshModel* obj2 = new MeshModel();
	obj2->CreateMeshModel(vertices, indices, 32, 12);
	meshListModel.push_back(obj2);

	MeshModel* obj3 = new MeshModel();
	obj3->CreateMeshModel(floorVertices, floorIndices, 32, 6);
	meshListModel.push_back(obj3);

	MeshModel* obj4 = new MeshModel();
	obj4->CreateMeshModel(vegetacionVertices, vegetacionIndices, 64, 12);
	meshListModel.push_back(obj4);

	calcAverageNormals(indices, 12, vertices, 32, 8, 5);

	calcAverageNormals(vegetacionIndices, 12, vegetacionVertices, 64, 8, 5);

}


void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}


void CrearDado() //sustituir con su propio dado, para que tenga la textura de la práctica 6
{
	unsigned int cubo_indices[] = {
		// front
		0, 1, 2,
		2, 3, 0,

		// back
		8, 9, 10,
		10, 11, 8,

		// left
		12, 13, 14,
		14, 15, 12,
		// bottom
		16, 17, 18,
		18, 19, 16,
		// top
		20, 21, 22,
		22, 23, 20,

		// right
		4, 5, 6,
		6, 7, 4,

	};
	GLfloat cubo_vertices[] = {
		// front
		//x		y		z		S		T			NX		NY		NZ
		-0.5f, -0.5f,  0.5f,	0.68f,  0.4f,		0.0f,	0.0f,	-1.0f,	//0
		0.5f, -0.5f,  0.5f,		0.84f,	0.4f,		0.0f,	0.0f,	-1.0f,	//1
		0.5f,  0.5f,  0.5f,		0.84f,	0.59f,		0.0f,	0.0f,	-1.0f,	//2
		-0.5f,  0.5f,  0.5f,	0.68f,	0.59f,		0.0f,	0.0f,	-1.0f,	//3

		// right
		//x		y		z		S		T
		0.5f, -0.5f,  0.5f,	    0.34f,  0.59f,		-1.0f,	0.0f,	0.0f,// abizq
		0.5f, -0.5f,  -0.5f,	0.54f,	0.59f,		-1.0f,	0.0f,	0.0f,// abder
		0.5f,  0.5f,  -0.5f,	0.54f,	0.79f,		-1.0f,	0.0f,	0.0f,// arrder
		0.5f,  0.5f,  0.5f,	    0.34f,	0.79f,		-1.0f,	0.0f,	0.0f,// arrizq
		// back black sun
		-0.5f, -0.5f, -0.5f,	1.0f,  0.8f,		0.0f,	0.0f,	1.0f,//abder
		0.5f, -0.5f, -0.5f,		0.85f,	0.8f,		0.0f,	0.0f,	1.0f,//abizq
		0.5f,  0.5f, -0.5f,		0.85f,	0.97f,		0.0f,	0.0f,	1.0f,//arrizq
		-0.5f,  0.5f, -0.5f,	1.0f,	0.97f,		0.0f,	0.0f,	1.0f,//arrder

		// left //raziel clan
		//x		y		z		S		T
		-0.5f, -0.5f,  -0.5f,	0.8f,  0.2f,		1.0f,	0.0f,	0.0f,//abizq
		-0.5f, -0.5f,  0.5f,	1.0f,	0.2f,		1.0f,	0.0f,	0.0f,//abder
		-0.5f,  0.5f,  0.5f,	1.0f,	0.4f,		1.0f,	0.0f,	0.0f,//arrder
		-0.5f,  0.5f,  -0.5f,	0.8f,	0.4f,		1.0f,	0.0f,	0.0f,//arrizq

		// bottom
		//x		y		z		S		T
		-0.5f, -0.5f,  0.5f,	0.16f,  0.6f,		0.0f,	1.0f,	0.0f,//abiz
		0.5f,  -0.5f,  0.5f,	0.16f,	0.4f,		0.0f,	1.0f,	0.0f,//abder
		 0.5f,  -0.5f,  -0.5f,	0.0f,	0.4f,		0.0f,	1.0f,	0.0f,//arrder
		-0.5f, -0.5f,  -0.5f,	0.0f,	0.6f,		0.0f,	1.0f,	0.0f,//arrizq

		//UP
		 //x		y		z		S		T
		 -0.5f, 0.5f,  0.5f,	0.48f,  0.19f,		0.0f,	-1.0f,	0.0f,//arrizq
		 0.5f,  0.5f,  0.5f,	0.48f,	0.4f,		0.0f,	-1.0f,	0.0f,//arrder
		  0.5f, 0.5f,  -0.5f,	0.33f,	0.4f,		0.0f,	-1.0f,	0.0f,//abder
		 -0.5f, 0.5f,  -0.5f,	0.33f,	0.19f,		0.0f,	-1.0f,	0.0f,//abizq

	};

	MeshModel* dado = new MeshModel();
	dado->CreateMeshModel(cubo_vertices, cubo_indices, 192, 36);
	meshListModel.push_back(dado);

}


void CrearHolocron()
{

	//Ejercicio 1: reemplazar con sus dados de 6 caras texturizados, agregar normales
// average normals
	// Vértices y normales extraídos directamente de holocron_simple.obj
// Formato por fila: X, Y, Z,  S, T,  NX, NY, NZ

	GLfloat holocron_vertices[] = {
		// front First Order
		//x      y       z       S       T       NX    NY    NZ
		 0.0f,  3.0f,  3.0f,   0.75f,  0.59f,  0.0f, 0.0f, -1.0f,  //0  a
		 3.0f,  0.0f,  3.0f,   0.84f,  0.50f,  0.0f, 0.0f, -1.0f,  //1  b
		 0.0f, -3.0f,  3.0f,   0.75f,  0.41f,  0.0f, 0.0f, -1.0f,  //2  c
		-3.0f,  0.0f,  3.0f,   0.66f,  0.50f,  0.0f, 0.0f, -1.0f,  //3  d

		//der Galactic Republic
		//x      y       z       S       T       NX    NY    NZ
		 3.0f,  3.0f,  0.0f,   0.44f,  0.81f, -1.0f, 0.0f, 0.0f,  //4  e
		 3.0f,  0.0f, -3.0f,   0.53f,  0.72f,  -1.0f, 0.0f, 0.0f,  //5  f
		 3.0f, -3.0f,  0.0f,   0.44f,  0.63f,  -1.0f, 0.0f, 0.0f,  //6  g
		 3.0f,  0.0f,  3.0f,   0.35f,  0.72f,  -1.0f, 0.0f, 0.0f,  //7  b

		 //izq Raziel Clan
		 //x      y       z       S       T       NX    NY    NZ
		 -3.0f,  3.0f,  0.0f,   0.89f,  0.38f, 1.0f, 0.0f, 0.0f,  //8  h
		 -3.0f,  0.0f,  3.0f,   0.98f,  0.29f, 1.0f, 0.0f, 0.0f,  //9  d
		 -3.0f, -3.0f,  0.0f,   0.89f,  0.20f, 1.0f, 0.0f, 0.0f,  //10 i
		 -3.0f,  0.0f, -3.0f,   0.80f,  0.29f, 1.0f, 0.0f, 0.0f,  //11 j

		 //inf SITH Empire
		 //x      y       z       S       T       NX    NY    NZ
		  0.0f, -3.0f,  3.0f,   0.09f,  0.60f,  0.0f,1.0f, 0.0f,  //12 c
		  3.0f, -3.0f,  0.0f,   0.18f,  0.51f,  0.0f,1.0f, 0.0f,  //13 g
		  0.0f, -3.0f, -3.0f,   0.09f,  0.42f,  0.0f,1.0f, 0.0f,  //14 l
		 -3.0f, -3.0f,  0.0f,   0.00f,  0.51f,  0.0f,1.0f, 0.0f,  //15 i

		 //sup Dark Lord
		 //x      y       z       S       T       NX    NY    NZ
		  0.0f,  3.0f, -3.0f,   0.41f,  0.41f,  0.0f, -1.0f, 0.0f,  //16 k
		  3.0f,  3.0f,  0.0f,   0.50f,  0.32f,  0.0f, -1.0f, 0.0f,  //17 e
		  0.0f,  3.0f,  3.0f,   0.41f,  0.23f,  0.0f, -1.0f, 0.0f,  //18 a
		 -3.0f,  3.0f,  0.0f,   0.32f,  0.32f,  0.0f, -1.0f, 0.0f,  //19 h

		 //atras Black Sun
		 //x      y       z       S       T       NX    NY    NZ
		  0.0f,  3.0f, -3.0f,   0.905f, 0.995f, 0.0f, 0.0f,1.0f,  //20 k
		  3.0f,  0.0f, -3.0f,   0.815f, 0.905f, 0.0f, 0.0f,1.0f,  //21 f  
		  0.0f, -3.0f, -3.0f,   0.905f, 0.815f, 0.0f, 0.0f,1.0f,  //22 l
		 -3.0f,  0.0f, -3.0f,   0.995f, 0.905f, 0.0f, 0.0f,1.0f,  //23 j  

		 //triang supizq
		 //x      y       z       S       T       NX    NY    NZ
		  0.0f,  3.0f,  3.0f,   0.02f,  0.02f, 0.6f, -0.6f, 1.0f,  //24 a
		 -3.0f,  0.0f,  3.0f,   0.02f,  0.02f, 0.6f, -0.6f, 1.0f,  //25 d
		 -3.0f,  3.0f,  0.0f,   0.02f,  0.02f, 0.6f, -0.6f, 1.0f,  //26 h  

		 //triang sup der
		 //x      y       z       S       T       NX    NY    NZ
		 0.0f,  3.0f,  3.0f,   0.02f,  0.02f,  -0.6f, 0.6f, 1.0f,  //27 a
		 3.0f,  3.0f,  0.0f,   0.02f,  0.02f,  -0.6f, 0.6f, 1.0f,  //28 e
		 3.0f,  0.0f,  3.0f,   0.02f,  0.02f,  -0.6f, 0.6f, 1.0f,  //29 b

		 //triang inf izq
		 //x      y       z       S       T       NX    NY    NZ
		 0.0f, -3.0f,  3.0f,   0.02f,  0.02f, 0.6f,0.6f, 1.0f,  //30 c
		 -3.0f, -3.0f,  0.0f,   0.02f,  0.02f, 0.6f,0.6f, 1.0f,  //31 i
		 -3.0f,  0.0f,  3.0f,   0.02f,  0.02f, 0.6f,0.6f, 1.0f,  //32 d

		 //triang inf der
		 //x      y       z       S       T       NX    NY    NZ
		 0.0f, -3.0f,  3.0f,   0.02f,  0.02f,  -0.6f,0.6f, 1.0f,  //33 c
		 3.0f, -3.0f,  0.0f,   0.02f,  0.02f, -0.6f,0.6f, 1.0f,  //34 g
		 3.0f,  0.0f,  3.0f,   0.02f,  0.02f,  -0.6f,0.6f, 1.0f,  //35 b

		 //triang supizq2
		 //x      y       z       S       T       NX    NY    NZ
		0.0f,  3.0f, -3.0f,   0.02f,  0.02f, 0.6f, -0.6f,-1.0f,  //36 k
		-3.0f,  3.0f,  0.0f,   0.02f,  0.02f,0.6f, -0.6f,-1.0f,  //37 h
		-3.0f,  0.0f, -3.0f,   0.02f,  0.02f, 0.6f, -0.6f,-1.0f,  //38 j

		//triang sup detr2
		//x      y       z       S       T       NX    NY    NZ
	   0.0f,  3.0f, -3.0f,   0.02f,  0.02f,  -0.6f, -0.6f,-1.0f,  //39 k
	   3.0f,  3.0f,  0.0f,   0.02f,  0.02f, -0.6f, -0.6f,-1.0f,  //40 e
	   3.0f,  0.0f, -3.0f,   0.02f,  0.02f, -0.6f, -0.6f,-1.0f,  //41 f

	   //triang inf izq2
	   //x      y       z       S       T       NX    NY    NZ
	 0.0f, -3.0f, -3.0f,   0.02f,  0.02f, 0.6f,0.6f,-1.0f,  //42 l
	 -3.0f, -3.0f,  0.0f,   0.02f,  0.02f, 0.6f,0.6f,-1.0f,  //43 i
	 -3.0f,  0.0f, -3.0f,   0.02f,  0.02f, 0.6f,0.6f,-1.0f,  //44 j

	 //triang inf der2
	 //x      y       z       S       T       NX    NY    NZ
   0.0f, -3.0f, -3.0f,   0.02f,  0.02f,  -0.6f,0.6f,-1.0f,  //45 l
   3.0f, -3.0f,  0.0f,   0.02f,  0.02f,  -0.6f,0.6f,-1.0f,  //46 g
   3.0f,  0.0f, -3.0f,   0.02f,  0.02f,  -0.6f,0.6f,-1.0f,  //47 f
	};

	GLuint holocron_indices[] = {
		// front
		0,  1,  2,
		2,  3,  0,

		// der
		4,  5,  6,
		6,  7,  4,

		// izq
		8,  9, 10,
		10, 11,  8,

		// inf
		12, 13, 14,
		14, 15, 12,

		// sup
		16, 17, 18,
		18, 19, 16,

		// atras
		20, 21, 22,
		22, 23, 20,

		// triangulos
		24, 25, 26,
		27, 28, 29,
		30, 31, 32,
		33, 34, 35,
		36, 37, 38,
		39, 40, 41,
		42, 43, 44,
		45, 46, 47,
	};

	MeshModel* holocron = new MeshModel();

	holocron->CreateMeshModel(holocron_vertices, holocron_indices, 384, 60);
	meshListModel.push_back(holocron);
}



int main()
{
	mainWindow = Window(1366, 768); // 1280, 1024 or 1024, 768
	mainWindow.Initialise();
	CreateObjects();
	CrearDado();
	CrearHolocron();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.3f, 0.5f);

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

	dadoTexture = Texture("Textures/star wars logos edit.png");
	dadoTexture.LoadTextureA();


	holocronTexture = Texture("Textures/star wars logos edit.png");
	holocronTexture.LoadTextureA();


	Holocron_M = Model();
	Holocron_M.LoadModel("Models/holocron_simple.obj");

	Dado_M = Model();
	Dado_M.LoadModel("Models/cubo_starsw.obj");


	Blackhawk_M = Model();
	Blackhawk_M.LoadModel("Models/uh60.obj");

	Rover_M = Model();
	Rover_M.LoadModel("Models/rovermodificado.obj");

	//Añadir modelo del avión
	//Avion_M = Model();
	//Avion_M.LoadModel("Models/");

	// Nave
	Nave_M = Model();
	Nave_M.LoadModel("Models/nave_text2.obj");

	Fuego_M = Model();
	Fuego_M.LoadModel("Models/fuego_azul_text2.obj");

	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");

	skybox = Skybox(skyboxFaces);

	Material_brillante = Material(4.0f, 256);
	Material_opaco = Material(0.3f, 4);




	//lu direccional es nuestro sol
	//luz direccional, sólo 1 y siempre debe de existir
	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,//color blanca
		0.2f, 0.3f,//0.3 es la cantidad de iluminacion, mayor numero mayr iluminacion
		0.0f, 0.0f, -1.0f);///ilumina en z negativo, cambiar x,y,z
	//contador de luces puntuales
	unsigned int pointLightCount = 0;

	//Declaración de primer luz puntual
	pointLights[0] = PointLight(1.0f, 0.0f, 0.0f,//luz roja r=1
		0.0f, 1.0f,// raiacion   color  
		-6.0f, 1.5f, 1.5f,
		0.3f, 0.3f, 0.3f);//jugar con estos valores
	pointLightCount++;


	unsigned int spotLightCount = 0;
	//linterna
	spotLights[0] = SpotLight(1.0f, 1.0f, 1.0f,//blanca
		0.0f, 6.0f,//
		0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		15.0f);//ancho de la luz
	spotLightCount++;

	//luz fija
	spotLights[1] = SpotLight(0.0f, 1.0f, 0.0f,//color verde luz
		1.0f, 2.0f,
		5.0f, 10.0f, 0.0f,
		0.0f, -5.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		15.0f);
	spotLightCount++;

	//se crean mas luces puntuales y spotlight 
	//rover
	//
	spotLights[2] = SpotLight(
		1.0f, 1.0f, 0.8f,  // color  
		0.0f,               // sin ambiente
		6.0f,               // intensidad difusa
		1.0f, -1.0f, 6.0f, // pos
		0.0f, 0.0f, -1.0f, // dir
		1.0f, 0.0f, 0.0f, // atenuacion con lin exp
		20.0f               //
	);
	spotLightCount++;

	//nave
	spotLights[3] = SpotLight(
		0.0f, 0.0f, 1.0f,  // color  azul va en la nave
		0.0f,               // sin ambiente
		8.0f,               // intensidad difusa
		0.0f, 5.0f, 6.0f, // posición inicial 
		3.0f, 2.0f, 6.0f, // dirección inicial 
		1.0f, 0.0f, 0.0f, // atenua 
		20.0f               // 
	);
	spotLightCount++; // 4




	//luz del holocron 

	//Declaración de primer luz puntual
	pointLights[1] = PointLight(0.65f, 0.85f, 0.95f,//luz roja r=1
		1.0f, 0.8f,// raiacion   color  
		-7.0f, 7.0f, 6.0f,
		1.0f, 0.6f, 0.01f);//jugar con estos valores
	pointLightCount++;

	// luz puntual avion
	//Declaración de  luz puntual
	pointLights[2] = PointLight(0.5f, 0.0f, 0.95f,//luz morada
		1.0f, 0.8f,// radiacion color 
		3.0f, 2.0f, 6.0f, //  
		1.0f, 0.6f, 0.01f);//jugar con estos valores
	pointLightCount++;

	//luz puntual fuego izq
	pointLights[3] = PointLight(0.0f, 0.831f, 1.0f,//luz azul
		1.0f, 0.8f,// radiacion color 
		0.0f, 5.0f, 6.0f, //   posiscio
		1.0f, 0.3f, 0.1f);//jugar con estos valores
	pointLightCount++;

	//luz puntual fuego der nave
	pointLights[4] = PointLight(0.0f, 0.821f, 0.95f,//luz azul nave
		1.0f, 0.8f,// radiacion color 
		3.0f, 2.0f, 6.0f, //  posicion 
		1.0f, 0.3f, 0.1f);//jugar con estos valores
	pointLightCount++;



	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0;
	GLuint uniformColor = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);
	////Loop mientras no se cierra la ventana
	//crear  var lowerlight
	glm::vec3 lowerLight;
	glm::mat4 model(1.0);
	glm::mat4 modelaux(1.0);
	glm::vec3 color;


	// 
	glm::vec3 roverPosicionA = glm::vec3(1.0f, -1.0f, 6.0f); // posición inicial del rover
	float roverVel = 0.05f; // velocidad de movimiento
	//nave 
	glm::vec3 navePosicionA = glm::vec3(3.0f, 2.0f, 6.0f); // posición inicial del nave
	float naveVel = 0.05f; // velocidad de movimiento

	glm::vec3 naveFuegoDerOffset = glm::vec3(0.0f, 0.0f, 1.67f); // fuego der respecto a la nave (antes fijo en 2.8, 4.0, 6.0 con la nave en 0, 5, 6)
	float naveFDVel = 0.05f; // velocidad de movimiento

	glm::vec3 naveFuegoIzqOffset = glm::vec3(0.02f, 0.0f, -1.67f); // fuego izq respecto a la nave (antes fijo en 3.2, 4.0, 6.0 con la nave en 0, 5, 6)
	float naveFIVel = 0.05f; // velocidad de movimiento

	// ajuste extra de la luz azul respecto a la llama (en ejes del mundo), por si la luz debe ir un poco atras o abajo
	glm::vec3 luzFuegoAjuste = glm::vec3(0.0f, 0.0f, 0.0f);



	// posiscion y direccion de luces
	glm::vec3 faroRoverOffset = glm::vec3(0.5f, 0.6f, 0.0f);  // frente del rover 
	glm::vec3 faroRoverDir = glm::vec3(1.0f, 0.0f, 0.0f);     // hacia adelante
	//nave
	glm::vec3 faroNaveIzqOffset = glm::vec3(0.4f, 0.0f, 0.0f);   // frente del nave
	glm::vec3 faroNaveIzqDir = glm::vec3(1.0f, 0.0f, 0.0f);      // hacia adelante
	//
	glm::vec3 faroNaveDerOffset = glm::vec3(0.0f, -0.2f, 0.0f); // abajo del nave
	glm::vec3 faroNaveDerDir = glm::vec3(1.0f, 0.0f, 0.0f);    // hacia abajo





	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

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

		//información en el shader de intensidad especular y brillo
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		// luz ligada a la cámara de tipo flash
		//sirve para que en tiempo de ejecución (dentro del while) se cambien propiedades de la luz
		lowerLight = camera.getCameraPosition();
		lowerLight.y -= 0.3f;
		spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());


		// Se actualiza cada frame con la tecla
		roverPosicionA.x = mainWindow.getRoverX();
		//ara nave
		navePosicionA.x = mainWindow.getAvionX();
		navePosicionA.y = mainWindow.getAvionY();
		//para 

		//matriz
		glm::mat4 roverBase = glm::translate(glm::mat4(1.0f), roverPosicionA);
		glm::mat4 naveBase = glm::translate(glm::mat4(1.0f), navePosicionA);

		glm::mat4 naveFuegoDerBase = glm::translate(naveBase, naveFuegoDerOffset);
		glm::mat4 naveFuegoIzqBase = glm::translate(naveBase, naveFuegoIzqOffset);



		// 
		// Posición del faro del rover
		glm::vec3 faroPosRov = glm::vec3(roverBase * glm::vec4(faroRoverOffset, 1.0f));

		// Dirección del faro hacia adelante X
		glm::vec3 faroDirRov = glm::normalize(glm::mat3(roverBase) * faroRoverDir);

		// Actualizar el spotlight del faro cada frame
		spotLights[2].SetFlash(faroPosRov, faroDirRov);

		//

		// 
		// Posición del faro del avion 
		glm::vec3 faroPosA = glm::vec3(naveBase * glm::vec4(faroNaveIzqOffset, 1.0f));

		// Dirección del faro hacia adelante X
		glm::vec3 faroDirA = glm::normalize(glm::mat3(naveBase) * faroNaveIzqDir);

		// Actualizar el spotlight del faro cada frame
		spotLights[3].SetFlash(faroPosA, faroDirA);

		// LUZ PUNTUAL NAVE mover con la nae
		// Posición del faro de nave luz puantual
		glm::vec3 faroPosA2 = glm::vec3(naveBase * glm::vec4(faroNaveDerOffset, 1.0f));

		// Actualizar el poinLight del faro cada frame
				// Actualizar el pointLight de la nave cada frame (una luz puntual solo tiene posicion, no direccion)
		pointLights[2] = PointLight(0.5f, 0.0f, 0.95f,//luz morada
			1.0f, 0.8f,
			faroPosA2.x, faroPosA2.y, faroPosA2.z,
			1.0f, 0.6f, 0.01f);

		// LUCES PUNTUALES AZULES de los fuegos, en la posicion de cada llama (se mueven con la nave)
		glm::vec3 luzFuegoIzqPos = glm::vec3(naveFuegoIzqBase * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)) + luzFuegoAjuste;
		glm::vec3 luzFuegoDerPos = glm::vec3(naveFuegoDerBase * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)) + luzFuegoAjuste;
		pointLights[3] = PointLight(0.0f, 0.831f, 1.0f,//luz azul fuego izq
			1.0f, 0.8f,
			luzFuegoIzqPos.x, luzFuegoIzqPos.y, luzFuegoIzqPos.z,
			1.0f, 0.3f, 0.1f);
		pointLights[4] = PointLight(0.0f, 0.821f, 0.95f,//luz azul fuego der
			1.0f, 0.8f,
			luzFuegoDerPos.x, luzFuegoDerPos.y, luzFuegoDerPos.z,
			1.0f, 0.3f, 0.1f);


		////holocron
		//// Posición del faro del avion 
		//glm::vec3 faroPosA = glm::vec3(avionBase * glm::vec4(faroAvionOffset, 1.0f));
		//glm::vec3 faroDirA = glm::normalize(glm::mat3(avionBase) * faroAvionDir);
		//spotLights[5].SetFlash(-7,5,0, faroDirA);



		//información al shader de fuentes de iluminación
		shaderList[0].SetDirectionalLight(&mainLight);
		// las luces de la nave: el spotlight es el ultimo de los spotlights, apagado se manda un conteo menos
		if (mainWindow.getLucesNave())
		{
			shaderList[0].SetSpotLights(spotLights, spotLightCount);
		}
		else
		{
			shaderList[0].SetSpotLights(spotLights, spotLightCount - 1);
		}

		// puntuales: apagadas se arma un arreglo sin la morada de la nave ([2]), cada motor apaga su luz azul con su propia tecla (1 y 2)
		PointLight pointLightsMostrar[MAX_POINT_LIGHTS];
		unsigned int pointLightCountMostrar = 0;
		for (unsigned int i = 0; i < pointLightCount; i++)
		{
			if (i == 2 && !mainWindow.getLucesNave())
			{
				continue;
			}
			if (i == 3 && !mainWindow.getMotor1()) // luz azul del motor 1 (fuego izq)
			{
				continue;
			}
			if (i == 4 && !mainWindow.getMotor2()) // luz azul del motor 2 (fuego der)
			{
				continue;
			}
			pointLightsMostrar[pointLightCountMostrar] = pointLights[i];
			pointLightCountMostrar++;
		}
		shaderList[0].SetPointLights(pointLightsMostrar, pointLightCountMostrar);



		color = glm::vec3(1.0f, 1.0f, 1.0f);

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));

		pisoTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshListModel[2]->RenderMeshModel();




		//Rover: Sustituir por su propio Rover con jerarquía
		//rover importado
		model = roverBase;
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Rover_M.RenderModel();


		///================================================
		////	
		//ave importado
		//
		model = naveBase;
		model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Nave_M.RenderModel();


		//fuego izq
		model = naveFuegoIzqBase;
		model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Fuego_M.RenderModel();

		//fuego der
		model = naveFuegoDerBase;
		model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Fuego_M.RenderModel();



		//Dado texturizado por codigo
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-5.0f, 5.0f, 6.0));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		dadoTexture.UseTexture();
		meshListModel[4]->RenderMeshModel();

		//Holocron
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-7.0f, 7.0f, 6.0));
		model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		holocronTexture.UseTexture();
		meshListModel[5]->RenderMeshModel();


		//
		//
		//
		// 
		// 
		// 
		// 
		// 
		//Añadir modelo del avión



		//Agave ¿qué sucede si lo renderizan antes del helicóptero?
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 1.0f, -4.0f));
		model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

		//blending: transparencia o traslucidez
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		AgaveTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshListModel[3]->RenderMeshModel();
		glDisable(GL_BLEND);

		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}