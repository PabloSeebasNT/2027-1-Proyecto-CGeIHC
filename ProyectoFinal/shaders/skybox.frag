#version 330
in vec3 TexCoords;
out vec4 color;
uniform samplerCube skybox;
uniform float brillo;	//ciclo de día y noche: 1 de día, menos de noche
void main()
{
color=vec4(texture(skybox,TexCoords).rgb*brillo,1.0);
}