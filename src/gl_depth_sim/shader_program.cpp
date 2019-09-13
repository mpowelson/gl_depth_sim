#include "gl_depth_sim/shader_program.h"
#include <gl_depth_sim/glad/glad.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>

void gl_depth_sim::ShaderProgram::init(const std::string& vertex_shader, const std::string& frag_shader)
{
  GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
  const auto* vtext = vertex_shader.c_str();
  glShaderSource(vshader, 1, &vtext, NULL);
  glCompileShader(vshader);

  std::vector<char> log(1024, 0);
  int success;
  glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);

  if (!success)
  {
    glGetShaderInfoLog(vshader, log.size(), NULL, log.data());
    std::cerr << "ERROR (VERTEX SHADER COMPILATION): " << log.data() << "\n";
    throw std::runtime_error("Vshader compile failed");
    // Note: If you get the following error, remove the UTF-8 encoding with sed -i '1s/^\xEF\xBB\xBF//' my_file.txt
  }

  GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
  const auto* ftext = frag_shader.data();
  glShaderSource(fshader, 1, &ftext, NULL);
  glCompileShader(fshader);

  glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(fshader, log.size(), NULL, log.data());
    std::cerr << "ERROR (FRAG SHADER COMPILATION): " << log.data() << "\n";
    throw std::runtime_error("Fshader compile failed");
  }

  // link shaders
  id_ = glCreateProgram();
  glAttachShader(id_, vshader);
  glAttachShader(id_, fshader);

  // Set the name of the outputs for Transform feedback
  const GLchar* feedbackVaryings[] = { "VertexColorOut" };
  // Set it such that tranform feedback is stored in its own buffer.
  glTransformFeedbackVaryings(id_, 1, feedbackVaryings, GL_SEPARATE_ATTRIBS);

  // Feedback must be set up prior to linking
  glLinkProgram(id_);

  // check for linking errors
  glGetProgramiv(id_, GL_LINK_STATUS, &success);
  if (!success)
  {
    glGetProgramInfoLog(id_, log.size(), NULL, log.data());
    std::cerr << "ERROR (PROGRAM LINKING FAILED): " << log.data() << "\n";
    throw std::runtime_error("program link failed");
  }
  glDeleteShader(vshader);
  glDeleteShader(fshader);
}

gl_depth_sim::ShaderProgram::ShaderProgram(const std::string& vertex_shader_path,
                                           const std::string& frag_shader_path,
                                           const bool& extreme_mode)
{
  if (extreme_mode)
    std::cout << "Executing in extreme mode!" << std::endl;

  // These will contain the shader code
  std::string vertex_shader;
  std::string frag_shader;

  // Set up ifstreams
  std::ifstream vertex_shader_ifstream;
  std::ifstream frag_shader_ifsteam;
  vertex_shader_ifstream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  frag_shader_ifsteam.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  try
  {
    // Open the files
    vertex_shader_ifstream.open(vertex_shader_path);
    frag_shader_ifsteam.open(frag_shader_path);

    // Read file into a string stream
    std::stringstream vertex_shader_string_stream;
    std::stringstream fragment_shader_string_stream;
    vertex_shader_string_stream << vertex_shader_ifstream.rdbuf();
    fragment_shader_string_stream << frag_shader_ifsteam.rdbuf();

    // Close the files
    vertex_shader_ifstream.close();
    frag_shader_ifsteam.close();

    // Convert the stream into a string
    vertex_shader = vertex_shader_string_stream.str();
    frag_shader = fragment_shader_string_stream.str();
  }
  catch (std::ifstream::failure e)
  {
    std::cerr << "Shader file has not been correcly read:" << std::endl;
  }

  // Send to init
//  std::cout << vertex_shader << std::endl;
//  std::cout << frag_shader << std::endl;
  init(vertex_shader, frag_shader);
}

gl_depth_sim::ShaderProgram::ShaderProgram(const std::string& vertex_shader, const std::string& frag_shader)
{
  init(vertex_shader, frag_shader);
}

gl_depth_sim::ShaderProgram::~ShaderProgram()
{
  glDeleteProgram(id_);
}

void gl_depth_sim::ShaderProgram::use() const { glUseProgram(id_);}

void gl_depth_sim::ShaderProgram::setInt(const std::string& attr, int val)
{
  GLuint loc = glGetUniformLocation(id_, attr.c_str());
  glUniform1i(loc, val);
}

void gl_depth_sim::ShaderProgram::setFloat(const std::string& attr, float val)
{
  GLuint loc = glGetUniformLocation(id_, attr.c_str());
  glUniform1f(loc, val);
}

void gl_depth_sim::ShaderProgram::setUniformVec3(const std::string &attr, const Eigen::Vector3f &vec)
{
  GLuint loc = glGetUniformLocation(id_, attr.c_str());
  glUniform3fv(loc, 1, vec.data());
}

void gl_depth_sim::ShaderProgram::setUniformMat4(const std::string& attr, const Eigen::Matrix4f& mat)
{
  GLuint loc = glGetUniformLocation(id_, attr.c_str());
  glUniformMatrix4fv(loc, 1, GL_FALSE, mat.data());
}
//tODO Remove this
void gl_depth_sim::ShaderProgram::setUniformMat4(const std::string &name, const glm::mat4 &mat)
{
    glUniformMatrix4fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
