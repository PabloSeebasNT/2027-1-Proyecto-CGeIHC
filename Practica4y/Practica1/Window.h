#pragma once
#include<stdio.h>
#include<glew.h>
#include<glfw3.h>

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
    GLfloat getmuevex() { return muevex; }
    GLfloat getRoverX() { return roverX; } //
    GLfloat getAvionX() { return avionX; } //
    GLfloat getAvionY() { return avionY; } //

    bool getLucesNave() { return lucesNave; } // luces de l
    bool getMotor1() { return motor1; } // motor 1 (fuego izq y su luz azul)
    bool getMotor2() { return motor2; } // motor 2 (fuego der y su luz azul)

    bool  getShouldClose() { return glfwWindowShouldClose(mainWindow); }
    bool* getsKeys() { return keys; }
    void  swapBuffers() { return glfwSwapBuffers(mainWindow); }

    ~Window();

private:
    GLFWwindow* mainWindow;
    GLint  width, height;
    bool   keys[1024];
    GLint  bufferWidth, bufferHeight;
    GLfloat lastX;
    GLfloat lastY;
    GLfloat xChange;
    GLfloat yChange;
    GLfloat muevex;
    //movi rover y avion
    GLfloat roverX;  
    GLfloat avionX;
    GLfloat avionY;
    //nave
    bool    lucesNave;
    bool    motor1;
    bool    motor2;

    bool    mouseFirstMoved;



    void createCallbacks();
    static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
    static void ManejaMouse(GLFWwindow* window, double xPos, double yPos);
};