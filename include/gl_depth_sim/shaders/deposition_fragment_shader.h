#version 330 core
out vec4 FragColor;

in vec3 VertexColor;

uniform vec3 objectColor;

void main()
{
   // Add in some ambient light just for visuals
   FragColor = vec4(VertexColor * objectColor + vec3(0.2, 0.2, 0.2), 1.0);
}
