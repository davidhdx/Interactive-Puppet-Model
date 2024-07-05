#pragma once

#include <vector>
#include<GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

class Sphere {
protected:
	std::vector<GLfloat> vertexDataVector;
	std::vector<GLuint> indices;
	glm::vec3* normals;
public:
	Sphere(int resolution, float radius);
	~Sphere();
};
