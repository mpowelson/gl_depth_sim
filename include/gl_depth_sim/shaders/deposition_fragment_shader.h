#version 330 core
out vec4 FragColor;

in vec3 VertexColorOut;
in vec3 normals;

uniform vec3 objectColor;

void main()
{
   // Add in some ambient light just for visuals
   FragColor = vec4(VertexColorOut * objectColor + vec3(0.2, 0.2, 0.2), 1.0);
   // This is just for debugging. Eventually I will remove normals
//  FragColor = vec4(normals, 1.0);
}
