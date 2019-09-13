#include "gl_depth_sim/renderable_mesh.h"
#include "gl_depth_sim/glad/glad.h"
#include <iostream>

gl_depth_sim::RenderableMesh::RenderableMesh(const gl_depth_sim::Mesh& mesh)
  : num_indices_{mesh.numIndices()}
{
  // TODO: Do I need to keep around the mesh in this object?
  std::cout << "Constructor" << std::endl;
  setupGL(mesh);

;}

gl_depth_sim::RenderableMesh::~RenderableMesh()
{
  glDeleteVertexArrays(1, &vao_);
  glDeleteBuffers(1, &vbo_);
  glDeleteBuffers(1, &tbo_);
  glDeleteBuffers(1, &ebo_);
}

void gl_depth_sim::RenderableMesh::setupGL(const Mesh& mesh)
{
  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_);
  glGenBuffers(1, &tbo_);
  glGenBuffers(1, &ebo_);

  glBindVertexArray(vao_);
  EigenAlignedVec<Eigen::Vector3f> combined;
  std::cout << "# Vert: " << mesh.vertices().size() << "  # Normals: " << mesh.normals().size() << std::endl;
  for (int i = 0; i < mesh.vertices().size(); i++)
  {
    combined.push_back(mesh.vertices()[i]);
    combined.push_back(mesh.normals()[i]);
//    std::cout << "\nVertices: \n" << mesh.vertices()[i] << "\nNormals: \n" << mesh.normals()[i];
  }

  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Eigen::Vector3f) * mesh.vertices().size() * 2, combined.data(),
               GL_STATIC_DRAW);

  Eigen::Vector3f init_data[mesh.indices().size()];
  for (int i = 0; i < mesh.vertices().size(); i++)
    init_data[i] = Eigen::Vector3f(0, 0, 0);

  // Set the tranfer feedback buffer to be dynamic since it changes frequently
  glBindBuffer(GL_ARRAY_BUFFER, tbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Eigen::Vector3f) * mesh.vertices().size(), init_data, GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo_);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) *  mesh.indices().size(), mesh.indices().data(),
               GL_STATIC_DRAW);

  // These are in the vertex buffer
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  // vertex positions
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2*sizeof(Eigen::Vector3f), (void*)0);
  glEnableVertexAttribArray(0);
  // vertex normals
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 2*sizeof(Eigen::Vector3f), (void*)(sizeof(Eigen::Vector3f)));
  glEnableVertexAttribArray(1);

  // These are in the tranform feedback buffer
  glBindBuffer(GL_ARRAY_BUFFER, tbo_);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, sizeof(Eigen::Vector3f), (void*)0);
  glEnableVertexAttribArray(2);


  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

}
