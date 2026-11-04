#include "Window.h"

Window::Window()
{
	width = 800;
	height = 600;
	mainWindow = NULL;
	mouseFirstMoved = true;
	lastX = lastY = xChange = yChange = 0.0f;
	for (size_t i = 0; i < 1024; i++)
	{
		keys[i] = 0;
		pulsadas[i] = 0;
	}
	reiniciarControles();
}
Window::Window(GLint windowWidth, GLint windowHeight)
{
	width = windowWidth;
	height = windowHeight;
	//el objeto temporal de "mainWindow = Window(...)" se destruye antes de Initialise; con NULL no destruye nada
	mainWindow = NULL;
	//sin estos valores iniciales el primer movimiento del ratón usaba basura y giraba la cámara
	mouseFirstMoved = true;
	lastX = 0.0f;
	lastY = 0.0f;
	xChange = 0.0f;
	yChange = 0.0f;
	for (size_t i = 0; i < 1024; i++)
	{
		keys[i] = 0;
		pulsadas[i] = 0;
	}
	reiniciarControles();
}

//Estado inicial de la cámara y de las luces de teclado (también se usa al terminar cada captura)
void Window::reiniciarControles()
{
	camara = CAM_AVATAR;
	linterna = false;
	reflectores = true;
}

int Window::Initialise()
{
	//Inicialización de GLFW
	if (!glfwInit())
	{
		printf("Falló inicializar GLFW");
		glfwTerminate();
		return 1;
	}
	//Asignando variables de GLFW y propiedades de ventana
	//macOS solo ofrece el perfil Core hasta OpenGL 4.1; los shaders usan #version 330
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//para solo usar el core profile de OpenGL y no tener retrocompatibilidad
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	//CREAR VENTANA
	mainWindow = glfwCreateWindow(width, height, "Proyecto Final CGeIHC 2027-1: Viajes espaciales y evolucion de especies", NULL, NULL);

	if (!mainWindow)
	{
		printf("Fallo en crearse la ventana con GLFW");
		glfwTerminate();
		return 1;
	}
	//Obtener tamaño de Buffer
	glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

	//asignar el contexto
	glfwMakeContextCurrent(mainWindow);

	//MANEJAR TECLADO y MOUSE
	createCallbacks();


	//permitir nuevas extensiones
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		printf("Falló inicialización de GLEW");
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return 1;
	}

	glEnable(GL_DEPTH_TEST); //HABILITAR BUFFER DE PROFUNDIDAD

	//Asignar Viewport
	glViewport(0, 0, bufferWidth, bufferHeight);
	//Callback para detectar que se está usando la ventana
	glfwSetWindowUserPointer(mainWindow, this);
	return 0;
}

void Window::createCallbacks()
{
	glfwSetKeyCallback(mainWindow, ManejaTeclado);
	glfwSetCursorPosCallback(mainWindow, ManejaMouse);
}
GLfloat Window::getXChange()
{
	GLfloat theChange = xChange;
	xChange = 0.0f;
	return theChange;
}

GLfloat Window::getYChange()
{
	GLfloat theChange = yChange;
	yChange = 0.0f;
	return theChange;
}

bool Window::getPulsada(int key)
{
	if (key < 0 || key >= 1024 || !pulsadas[key]) return false;
	pulsadas[key] = false;
	return true;
}


void Window::ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	//Cámaras y luces: cambian solo cuando se presiona la tecla (GLFW_PRESS), no mientras se mantiene
	if (action == GLFW_PRESS)
	{
		if (key >= GLFW_KEY_1 && key <= GLFW_KEY_5)
		{
			theWindow->camara = key - GLFW_KEY_0;
			//al cambiar de cámara se descarta el movimiento del ratón acumulado para que no dé un salto
			theWindow->xChange = theWindow->yChange = 0.0f;
		}
		if (key == GLFW_KEY_L) theWindow->linterna = !theWindow->linterna;
		if (key == GLFW_KEY_R) theWindow->reflectores = !theWindow->reflectores;
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			theWindow->keys[key] = true;
			theWindow->pulsadas[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			theWindow->keys[key] = false;
		}
	}
}

void Window::ManejaMouse(GLFWwindow* window, double xPos, double yPos)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (theWindow->mouseFirstMoved)
	{
		theWindow->lastX = xPos;
		theWindow->lastY = yPos;
		theWindow->mouseFirstMoved = false;
	}

	theWindow->xChange = xPos - theWindow->lastX;
	theWindow->yChange = theWindow->lastY - yPos;

	theWindow->lastX = xPos;
	theWindow->lastY = yPos;
}


Window::~Window()
{
	glfwDestroyWindow(mainWindow);
	glfwTerminate();

}
