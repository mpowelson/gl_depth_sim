#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;


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
   vec3 result = vec3(0, 0, 0);

   // Add light from all spotlights
   for(int i = 0; i < NR_LIGHTS; i++)
     result += CalcSpotLight(spotLights[i], norm, aPos);

   // Set the result
   VertexColor = result;

}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 vertPos)
{
    vec3 lightDir = normalize(light.position - vertPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // attenuation
    float distance = length(light.position - vertPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 diffuse = light.diffuse * diff;
//    diffuse *= attenuation * intensity;
    return diffuse;
}
