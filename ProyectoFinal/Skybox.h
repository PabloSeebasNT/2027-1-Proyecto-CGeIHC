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
	Skybox(std::vector<std::string> faceLocations);
	void DrawSkybox(glm::mat4 viewMatrix, glm::mat4 projectionMatrix, float brillo = 1.0f);
		
	~Skybox();
private:
	MeshModel* skyMesh;
	Shader* skyShader;
	GLuint textureId;
	GLuint uniformProjection, uniformView, uniformBrillo;

};

