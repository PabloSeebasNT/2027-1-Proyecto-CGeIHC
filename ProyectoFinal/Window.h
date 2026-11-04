#pragma once
#include<stdio.h>
#include<glew.h>
#include<glfw3.h>

//Cámaras del proyecto final (teclas 1 a 5)
enum ModoCamara { CAM_AVATAR = 1, CAM_CARRERA = 2, CAM_NAVE = 3, CAM_RECORRIDO = 4, CAM_LIBRE = 5 };

class Window
{
public:
	Window();
	Window(GLint windowWidth, GLint windowHeight);
	int Initialise();
	GLfloat getBufferWidth() { return bufferWidth; }
	GLfloat getBufferHeight() { return bufferHeight; }
	GLfloat getXChange();
	GLfloat getYChange();
	bool getShouldClose() {
		return  glfwWindowShouldClose(mainWindow);}
	bool* getsKeys() { return keys; }
	void swapBuffers() { return glfwSwapBuffers(mainWindow); }

	//Estado que cambia con el teclado (ver ManejaTeclado); main solo lo lee
	int getCamara() { return camara; }
	bool getLinterna() { return linterna; }
	bool getReflectores() { return reflectores; }
	//Teclas de un solo disparo (animaciones que se inician con teclado): regresa true una sola vez por cada vez que
	//se presiona la tecla. Cualquier integrante puede usarla para sus animaciones, por ejemplo getPulsada(GLFW_KEY_E)
	bool getPulsada(int key);
	//Para las capturas: fijan el estado sin usar el teclado
	void setCamara(int valor) { camara = valor; }
	void setLinterna(bool valor) { linterna = valor; }
	void setReflectores(bool valor) { reflectores = valor; }
	void reiniciarControles();

	~Window();
private:
	GLFWwindow *mainWindow;
	GLint width, height;
	bool keys[1024];
	bool pulsadas[1024];	//se ponen en true al presionar y getPulsada las regresa a false
	GLint bufferWidth, bufferHeight;
	void createCallbacks();
	GLfloat lastX;
	GLfloat lastY;
	GLfloat xChange;
	GLfloat yChange;
	bool mouseFirstMoved;
	int camara;			//1 a 5, ver ModoCamara
	bool linterna;		//L: spotlight del avatar
	bool reflectores;	//R: spotlights de la plataforma de aterrizaje de Pyros
	static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
	static void ManejaMouse(GLFWwindow* window, double xPos, double yPos);

};
