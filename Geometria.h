/*
Geometria.h - Proyecto final CGeIHC 2027-1
Figuras creadas por código que usa todo el proyecto (cubo, cilindro, cono, esfera, plano y rocas) y la función
Dibujar, que manda la matriz, el color y la emisión al shader y dibuja una malla.

Todas las mallas tienen el formato del curso: x y z, u v, nx ny nz. Como en el dado de la práctica 6, la normal se
guarda hacia el INTERIOR del objeto, porque así la usa shaders/shader_light.frag (ver AgregarVertice).
*/
#pragma once
#include <vector>
#include <glew.h>
#include <glm.hpp>
#include "Mesh_tn.h"
#include "Model.h"
#include "Material.h"
#include "Texture.h"

//Ubicaciones de los uniforms del shader de iluminación. main las asigna en cada cuadro después de UseShader
struct UniformesEscena
{
	GLuint model, color, especular, brillo, emision, desplazamientoUV, transparencia;
};
extern UniformesEscena U;

//Figuras básicas compartidas (CrearPrimitivas las crea una sola vez al inicio):
//cubo de lado 1 centrado; cilindro de radio 1 y alto 1 centrado; cono de radio 1 con la base en y = 0 y la punta en
//y = 1; esfera de radio 1; plano de 1 x 1 sobre XZ con la cara hacia +Y
extern MeshModel* mCubo;
extern MeshModel* mCilindro;
extern MeshModel* mCono;
extern MeshModel* mEsfera;
extern MeshModel* mEsferaBaja;	//esfera con menos triángulos para partículas, burbujas y piezas lejanas
extern MeshModel* mPlano;
void CrearPrimitivas();

//Materiales compartidos (brillo especular): brillante para pieles y cascos, opaco para roca y piso, metal para
//estructuras. UsarMaterial los manda al shader con las ubicaciones de U
extern Material matBrillante;
extern Material matOpaco;
extern Material matMetal;
void UsarMaterial(Material& m);

//Crea y carga una textura. Se guarda como apuntador y no se borra: así su destructor (glDeleteTextures) no corre
//al cerrar el programa, cuando ya no hay contexto de OpenGL
Texture* CargarTextura(const char* ruta);

//Para armar mallas propias en los demás archivos
void AgregarVertice(std::vector<GLfloat>& v, glm::vec3 p, float u, float t, glm::vec3 normalExterior);
MeshModel* GuardarMalla(std::vector<GLfloat>& v, std::vector<unsigned int>& idx);
MeshModel* CrearTroncoCono(float radioSuperior, int lados);	//base de radio 1 en y = 0, tapa de radio r en y = 1
MeshModel* CrearEsfera(int pisos, int lados);
MeshModel* CrearRoca(int semilla);	//esfera deformada con ruido, radio aproximado 1

//Ruido de valor en 3D (de -1 a 1) y su suma por octavas; mismo resultado en todas las computadoras
float Ruido3(glm::vec3 p);
float Fbm3(glm::vec3 p, int octavas);

//Dibuja una malla con su matriz, el color que multiplica a la textura y su emisión (0: solo la iluminan las luces).
//La textura y el material se eligen antes con UseTexture y UseMaterial
void Dibujar(MeshModel* malla, const glm::mat4& model, glm::vec3 color, float emision = 0.0f);
//Igual para los modelos cargados con Assimp; cada material del OBJ pone su propio color
void DibujarModelo(Model& modelo, const glm::mat4& model, float emision = 0.0f);
