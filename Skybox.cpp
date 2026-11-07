#include "Skybox.h"



Skybox::Skybox()
{
}

Skybox::Skybox(std::vector<std::string> faceLocations, bool carasConvencionales)
{
    // El vertex shader heredado usa -pos; los cubemaps convencionales necesitan pos.
    sentido = carasConvencionales ? -1.0f : 1.0f;
	skyShader = new Shader();
	skyShader->CreateFromFiles("shaders/skybox.vert", "shaders/skybox.frag");
	uniformProjection = skyShader->GetProjectionLocation();
	uniformView = skyShader->GetViewLocation();
	uniformBrillo = glGetUniformLocation(skyShader->GetProgramID(), "brillo");

	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
	int width, height, bitDepth;
	//Texture::LoadTextureA deja activado el volteo vertical; las caras del cubemap se usan como vienen, si se
	//voltean las orillas de caras vecinas ya no coinciden
	stbi_set_flip_vertically_on_load(false);
	for (size_t i = 0; i < 6; i++)
	{

		// Todas las caras se suben como RGB, incluso si el PNG contiene canal alpha.
		unsigned char *texData = stbi_load(faceLocations[i].c_str(), &width, &height, &bitDepth, STBI_rgb);
		if (!texData)
		{
			printf("No se encontró : %s", faceLocations[i].c_str());
			return;
		}
		//para cambiar el origen a la esquina inferior izquierda como necesitamos
		//stbi_set_flip_vertically_on_load(true);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X +i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texData); //SIN CANAL ALPHA A ENOS QUE QUERAMOS AGREGAR EFECTO DE PARALLAX
		stbi_image_free(texData); //para liberar la información de la imagen
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//Creando el Mesh del skybox
	unsigned int skyboxIndices[] = {
		// front
		0, 1, 2,
		2, 1, 3,
		// right
		2, 3, 5,
		5, 3, 7,
		// back
		5, 7, 4,
		4, 7, 6,
		// left
		4, 6, 0,
		0, 6, 1,
		// top
		4, 0, 5,
		5, 0, 2,
		// bottom
		1, 6, 3,
		3, 6, 7
	};

	float skyboxVertices[] = {
		-1.0f, 1.0f, -1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,//v0
		-1.0f, -1.0f, -1.0f,	0.0f, 0.0f,		0.0f, 0.0f, 0.0f,//V1
		1.0f, 1.0f, -1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,//V2
		1.0f, -1.0f, -1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,//V3
		-1.0f, 1.0f, 1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,//V4
		1.0f, 1.0f, 1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,//V5
		-1.0f, -1.0f, 1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,//V6
		1.0f, -1.0f, 1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f//V7
	};
	skyMesh = new MeshModel();
	skyMesh->CreateMeshModel(skyboxVertices, skyboxIndices, 64, 36);

}

void Skybox::DrawSkybox(glm::mat4 viewMatrix, glm::mat4 projectionMatrix, float brillo,
    const Skybox* siguiente, float mezcla, float giro, float giroSiguiente)
{
	viewMatrix = glm::mat4(glm::mat3(viewMatrix));

	glDepthMask(false);
	skyShader->UseShader();
	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniform1f(uniformBrillo, brillo);
    GLuint programa = skyShader->GetProgramID();
    glUniform1f(glGetUniformLocation(programa, "sentido"), sentido);
    glUniform1i(glGetUniformLocation(programa, "skybox"), 0);
    glUniform1i(glGetUniformLocation(programa, "skyboxSiguiente"), 1);
    glUniform1f(glGetUniformLocation(programa, "mezcla"), siguiente ? mezcla : 0.0f);
    glUniform1f(glGetUniformLocation(programa, "giro"), giro);
    glUniform1f(glGetUniformLocation(programa, "giroSiguiente"), giroSiguiente);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, siguiente ? siguiente->textureId : textureId);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
	//skyShader->Validate();
	skyMesh->RenderMeshModel();
	glDepthMask(true);

}


Skybox::~Skybox()
{
}
