#version 330

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 vertexColor;  // color que viene de tus vértices

out vec4 vColor;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;                          // para la cámara

void main()
{
    gl_Position = projection * view * model * vec4(pos, 1.0f);  // ← view en medio
    vColor = vec4(vertexColor, 1.0f);       // ← usa el color del vértice
}