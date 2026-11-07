#version 330
in vec3 TexCoords;
out vec4 color;
uniform samplerCube skybox;
uniform samplerCube skyboxSiguiente;
uniform float mezcla;
uniform float giro, giroSiguiente;
uniform float sentido;
uniform float brillo;	//ciclo de día y noche: 1 de día, menos de noche
vec3 Girar(vec3 p, float a)
{
    return vec3(cos(a)*p.x + sin(a)*p.z, p.y, -sin(a)*p.x + cos(a)*p.z);
}
void main()
{
    vec3 direccion = TexCoords * sentido;
    // Las caras front/back del paquete usan Z opuesto al cubemap de OpenGL.
    // Corregirlo evita reflejar horizontalmente las nubes y el texto del cielo.
    if (sentido < 0.0) direccion.z = -direccion.z;
    vec3 actual = texture(skybox, Girar(direccion, giro)).rgb;
    vec3 siguiente = texture(skyboxSiguiente, Girar(direccion, giroSiguiente)).rgb;
    color = vec4(mix(actual, siguiente, mezcla)*brillo, 1.0);
}
