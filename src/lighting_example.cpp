#include <gl_depth_sim/glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <gl_depth_sim/shader_program.h>
#include <gl_depth_sim/mesh_loader.h>
#include <gl_depth_sim/renderable_mesh.h>

#include <iostream>

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int main(int argc, char** argv)
{
  // glfw: initialize and configure
  // ------------------------------
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // glfw window creation
  // --------------------
  GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Window", NULL, NULL);
  if (window == NULL)
  {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }
  glEnable(GL_DEPTH_TEST);

  // Load the mesh
  if (argc != 2)
  {
    std::cerr << "Usage: ./orbit_example <PATH_TO_MESH>\n";
    return 1;
  }

  auto mesh_ptr = gl_depth_sim::loadMesh(argv[1]);

  if (!mesh_ptr)
  {
    std::cerr << "Unable to load mesh from path: " << argv[1] << "\n";
    return 1;
  }
  std::unique_ptr<gl_depth_sim::RenderableMesh> renderable_mesh(new gl_depth_sim::RenderableMesh{ *mesh_ptr });

  // Compile the shaders
  std::string v_shader_path = "/home/mpowelson/workspaces/noether/src/gl_depth_sim/include/gl_depth_sim/shaders/"
                              "deposition_vertex_shader.h";
  std::string f_shader_path = "/home/mpowelson/workspaces/noether/src/gl_depth_sim/include/gl_depth_sim/shaders/"
                              "deposition_fragment_shader.h";
  gl_depth_sim::ShaderProgram shader_program(v_shader_path, f_shader_path, true);
  shader_program.use();

  // render loop
  while (!glfwWindowShouldClose(window))
  {
    double steps = 100;
    for (int i = 0; i < steps; i++)
    {
      // render
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // Set shader properties
      // Color of the light
      shader_program.use();
      shader_program.setUniformVec3("objectColor", Eigen::Vector3f(1, 1, 1));
      // Properties of the light
      shader_program.setUniformVec3("spotLights[0].position",
                                    Eigen::Vector3f(-0.8 + (1.6 / steps) * static_cast<double>(i), -.23, 0.5));
      shader_program.setUniformVec3("spotLights[0].direction", Eigen::Vector3f(0.0, 0.9, -.45));
      shader_program.setFloat("spotLights[0].cutOff", 0.98);
      shader_program.setFloat("spotLights[0].outerCutOff", 0.97);
      shader_program.setFloat("spotLights[0].constant", 1.0f);
      shader_program.setFloat("spotLights[0].linear", 0.14f);
      shader_program.setFloat("spotLights[0].quadratic", .07f);
      shader_program.setUniformVec3("spotLights[0].diffuse", Eigen::Vector3f(1, 1, 1));

      // view/projection transformations
      float projection_array[16] = {
        1.810660, 0.000000, 0.000000,  0.000000,  0.000000, 2.414213, 0.000000,  0.000000,
        0.000000, 0.000000, -1.002002, -1.000000, 0.000000, 0.000000, -0.200200, 0.000000
      };
      glm::mat4 projection =
          glm::make_mat4(projection_array);  // TODO (Colin): Remove all uses of glm and replace with Eigen
      float view_array[16] = { 0.999959,  -0.000372, 0.009044, 0.000000, 0.000000,  0.999157, 0.041049,   0.000000,
                               -0.009052, -0.041047, 0.999116, 0.000000, -0.329678, 0.034711, -12.937737, 1.000000 };
      glm::mat4 view = glm::make_mat4(view_array);

      shader_program.setUniformMat4("projection", projection);
      shader_program.setUniformMat4("view", view);

      // world transformation
      glm::mat4 model = glm::mat4(1.0f);
      shader_program.setUniformMat4("model", model);

      // Render the mesh
      glBindVertexArray(renderable_mesh->vao());

      // compute mvp
      Eigen::Projective3d mvp = Eigen::Projective3d::Identity();
      // depth_program_->setUniformMat4("mvp", mvp.matrix().cast<float>());

      glBeginTransformFeedback(GL_TRIANGLES);
      glDrawElements(GL_TRIANGLES, renderable_mesh->numIndices(), GL_UNSIGNED_INT, 0);
      GLfloat results[renderable_mesh->numIndices()];
      glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(results), results);
      glEndTransformFeedback();

      glBindVertexArray(0);

      // IO stuff
      glfwSwapBuffers(window);
      glfwPollEvents();
    }
  }

  glfwTerminate();
  return 0;
}
