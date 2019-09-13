#ifndef GL_DEPTH_SIM_SHADER_PROGRAM_H
#define GL_DEPTH_SIM_SHADER_PROGRAM_H

#include <string>
#include <Eigen/Dense>
#include <glm/glm.hpp>

namespace gl_depth_sim
{
class ShaderProgram
{
public:
  ShaderProgram(const std::string& vertex_shader_path, const std::string& frag_shader_path, const bool& extreme_mode);
  ShaderProgram(const std::string& vertex_shader, const std::string& frag_shader);
  ~ShaderProgram();

  void init(const std::string& vertex_shader, const std::string& frag_shader);

  /** @brief Must be called before shader will be used. */
  void use() const;

  unsigned int id() const { return id_; }

  // Interaction with attributes
  void setInt(const std::string& attr, int val);
  void setFloat(const std::string& attr, float val);
  void setUniformVec3(const std::string& attr, const Eigen::Vector3f& vec);
  void setUniformMat4(const std::string& attr, const Eigen::Matrix4f& mat);
  void setUniformMat4(const std::string& attr, const glm::mat4& mat);

private:
  unsigned int id_;
};

}  // namespace gl_depth_sim

#endif
