#include "gl_depth_sim/glad/glad.h"
#include "gl_depth_sim/sim_depth_camera.h"
// OpenGL context
#include "GLFW/glfw3.h"

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <glm/gtx/io.hpp>

const static std::string vertex_shader_source =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 mvp;\n"

    "void main()\n"
    "{\n"
      "gl_Position = mvp * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\n";

const static std::string frag_shader_source =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
      "FragColor = vec4(gl_FragCoord.x / 800.0f, gl_FragCoord.y / 600.0f, 1.0f, 1.0f);\n"
    "}\n";


gl_depth_sim::SimDepthCamera::SimDepthCamera(const gl_depth_sim::CameraProperties& camera)
  : camera_{camera}
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window_ = glfwCreateWindow(camera.width, camera.height, "gl_depth_sim", NULL, NULL);
  if (window_ == NULL)
  {
    glfwTerminate();
    throw std::runtime_error("Failed to create GLFW window");
  }

  glfwMakeContextCurrent(window_);

  // glad: load all OpenGL function pointers
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    glfwTerminate();
    throw std::runtime_error("Failed to initialize GLAD");
  }

  // Load Shaders
  depth_program_.reset(new ShaderProgram(vertex_shader_source, frag_shader_source));

//  projection_ = glm::perspective(glm::radians(60.0f), float(camera.width) / float(camera.height), camera.z_near,
//                                 camera.z_far);
  projection_ = glm::mat4(1.0f);

  projection_[0][0] = 2.0 * camera.fx / camera.width;
  projection_[1][1] = 2.0 * camera.fy/ camera.height;
  projection_[2][0] = 1.0 - 2.0 * camera.cx / camera.width;
  projection_[2][1] = 2.0 * camera.cy / camera.height - 1.0;
  projection_[2][2] = (camera.z_far + camera.z_near) / (camera.z_near - camera.z_far);
  projection_[2][3] = -1.0f;

  projection_[3][2] = 2 * camera.z_far * camera.z_near / (camera.z_near- camera.z_far);
  projection_[3][3] = 0.0f;

  // CREATE A FRAME BUFFER OBJECT FOR COLOR & DEPTH
  // Create frame buffer and make it active
  glGenFramebuffers(1, &fbo_);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

  GLuint texColor;
  glGenTextures(1, &texColor);

  GLuint texDepth;
  glGenTextures(1, &texDepth);

  // Create color texture
  glBindTexture(GL_TEXTURE_2D, texColor);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, camera.width, camera.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  // Attach the color to the active fbo
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColor, 0);

  // Create a depth texture
  glBindTexture(GL_TEXTURE_2D, texDepth);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, camera.width, camera.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);


  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texDepth, 0);

  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    throw std::runtime_error("Framebuffer configuration failed");

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glfwSwapInterval(0);
}

gl_depth_sim::SimDepthCamera::~SimDepthCamera()
{
  glfwDestroyWindow(window_);
  glDeleteFramebuffers(1, &fbo_);
  glfwTerminate();
}

static double t = 0.0;

static glm::mat4 toGLM(const Eigen::Affine3d& pose)
{
  glm::mat4 m;
  for (int j = 0; j < 4; ++j)
  {
    for (int i = 0; i < 4; ++i)
    {
      // mat4 is (col, row)
      m[i][j] = pose(j, i);
    }
  }
  return m;
}

gl_depth_sim::DepthImage gl_depth_sim::SimDepthCamera::render(const Eigen::Affine3d& pose)
{
  t += 0.1;

  float x = 3.0 * cos(t);
  float y = 3.0 * sin(t);
  float z = 1.0;//2.0 * cos(t);


//  float x = 1.0, y=1.0, z= 1.0;
  glm::mat4 view = glm::lookAt(
    glm::vec3(x,y,z), // Camera is at (4,3,3), in World Space
    glm::vec3(0,0,0), // and looks at the origin
    glm::vec3(0,0,1)  // Head is up (set to 0,-1,0 to look upside-down)
  );
  std::cout << "View matrix\n" << view << "\n";

  Eigen::Affine3d p;
  p.setIdentity();

  // Define a pose looking at 0,0,0 from x,y,z

  {
    Eigen::Vector3d z_axis = (Eigen::Vector3d(x,y,z) - Eigen::Vector3d(0,0,0)).normalized();
    Eigen::Vector3d y_hint = Eigen::Vector3d(0, 0, 1);
    Eigen::Vector3d x_axis = (y_hint.cross(z_axis).normalized());
    Eigen::Vector3d y_axis = (z_axis.cross(x_axis).normalized());

    p.matrix().col(0).head<3>() = x_axis;
    p.matrix().col(1).head<3>() = y_axis;
    p.matrix().col(2).head<3>() = z_axis;
    p.translation() = Eigen::Vector3d(x,y,z);

    std::cout << "P\n" << p.matrix() << "\n";

    std::cout << "PINV\n" << p.inverse().matrix() << "\n";

  }

  std::cout << "p2\n" << p.matrix() << "\n";

  view = toGLM(p.inverse());

  std::cout << "AFTER CONVERSION\n" << view << "\n\n";

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(depth_program_->id());

  // Render each object
  for (const auto& obj : objects_)
  {
    glBindVertexArray(obj.mesh->vao());

    // compute mvp
    glm::mat4 model (1.0f);

    //     TODO:
    //    model = obj.pose
    glm::mat4 mvp = projection_ * view * model;
    depth_program_->setUniformMat4("mvp", mvp);

    glDrawElements(GL_TRIANGLES, obj.mesh->numIndices(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0); // no need to unbind it every time
  }

  // Pull depth data
  DepthImage img;
  img.cols = camera_.width;
  img.rows = camera_.height;
  img.data.resize(img.cols * img.rows);

//  glReadPixels(0, 0, camera_.height, camera_.width, GL_DEPTH_COMPONENT, GL_FLOAT, img.data.data());

  glfwSwapBuffers(window_);

  return img;
}

bool gl_depth_sim::SimDepthCamera::add(const Mesh& mesh, const Eigen::Affine3d& pose)
{
  std::unique_ptr<RenderableMesh> renderable_mesh (new RenderableMesh(mesh));

  RenderableObjectState state;
  state.mesh = std::move(renderable_mesh);
  state.pose = pose;

  objects_.push_back(std::move(state));

  return true;
}