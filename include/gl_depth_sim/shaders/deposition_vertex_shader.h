#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 VertexColorIn;

// Struct used to define one gun
struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    // These are used for attenuation
    float constant;
    float linear;
    float quadratic;
  
    // Defines the diffuse lighting. This is a color
    vec3 diffuse;     
};

#define NR_LIGHTS 1

// Resulting vertex color
out vec3 VertexColor;
out vec3 normals;
out vec3 VertexColorOut;

// All of the transforms for the mesh
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Define all of the spotlights
uniform SpotLight spotLights[NR_LIGHTS];

// function prototypes
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos);

void main()
{    

   gl_Position = projection * view * model * vec4(aPos, 1.0);

   // I'm not sure if this needs to be done
   vec3 Position = vec3(model * vec4(aPos, 1.0));
   vec3 Normal = mat3(transpose(inverse(model))) * aNormal;
   vec3 norm = normalize(aNormal);
//   vec3 norm = aNormal;

   // Reset the color for the vertex. Eventually this will need to be stored in the vertex buffer
//   vec3 result = vec3(0, 0, 0);
   vec3 result = VertexColorIn;

   // Add light from all spotlights
   for(int i = 0; i < NR_LIGHTS; i++)
     result += CalcSpotLight(spotLights[i], norm, aPos)*0.01;

   // Set the result
   VertexColor = result;
   normals = norm;
   VertexColorOut = result;

}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 vertPos)
{

  vec3 lightDir = normalize(light.position - vertPos);

  // check if lighting is inside the spotlight cone
  float theta = dot(lightDir, normalize(-light.direction));

  if(theta > light.cutOff) // remember that we're working with angles as cosines instead of degrees so a '>' is used.
  {

    // diffuse
    vec3 norm = normalize(normal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff;

    // attenuation
    float distance    = length(light.position - vertPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 result = diffuse * attenuation;
    return result;
  }
  else
  {
    return vec3(0,0,0);
  }


}
