#pragma once

#include <glew.h>
#include <glm.hpp>

class Light
{
public:
	Light();
	Light(GLfloat red, GLfloat green, GLfloat blue, 
			GLfloat aIntensity, GLfloat dIntensity);

	//cambia el color de la luz en tiempo de ejecución (práctica 8: faro del avión)
	void SetColor(glm::vec3 nuevoColor);
	//proyecto final: el sol cambia de intensidad con el ciclo de día y noche
	void SetIntensidades(GLfloat aIntensity, GLfloat dIntensity);

	~Light();

protected:
	glm::vec3 color;
	GLfloat ambientIntensity;
	GLfloat diffuseIntensity;
};

