#ifndef GL_DEPTH_SIM_SHADER_PROGRAM_H
#define GL_DEPTH_SIM_SHADER_PROGRAM_H

#include <string>
#include <Eigen/Dense>

namespace gl_depth_sim
{
class ShaderProgram
{
public:
  ShaderProgram(const std::string& vertex_shader_path, const std::string& frag_shader_path, const bool& extreme_mode);
  ShaderProgram(const std::string& vertex_shader, const std::string& frag_shader);
  ~ShaderProgram();

  void init(const std::string& vertex_shader, const std::string& frag_shader);

  unsigned int id() const { return id_; }

  // Interaction with attributes
  void setInt(const std::string& attr, int val);
  void setUniformMat4(const std::string& attr, const Eigen::Matrix4f& mat);

private:
  unsigned int id_;
};

}  // namespace gl_depth_sim

#endif
