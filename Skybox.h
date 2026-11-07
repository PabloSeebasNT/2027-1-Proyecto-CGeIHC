#pragma once

#include "CommonValues.h"
#include <vector>
#include <string>
#include<glew.h>
#include<glm.hpp>
#include<glfw3.h>
#include <gtc/matrix_transform.hpp>
#include<gtc/type_ptr.hpp>
#include "Mesh_tn.h"
#include "Shader_light.h"
class Skybox
{
public:
	Skybox();
	Skybox(std::vector<std::string> faceLocations, bool carasConvencionales = false);
	void DrawSkybox(glm::mat4 viewMatrix, glm::mat4 projectionMatrix, float brillo = 1.0f,
        const Skybox* siguiente = nullptr, float mezcla = 0.0f,
        float giro = 0.0f, float giroSiguiente = 0.0f);
		
	~Skybox();
private:
	MeshModel* skyMesh;
	Shader* skyShader;
	GLuint textureId;
    float sentido = 1.0f;
	GLuint uniformProjection, uniformView, uniformBrillo;

};

