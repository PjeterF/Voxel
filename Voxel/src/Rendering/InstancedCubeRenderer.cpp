#include "InstancedCubeRenderer.hpp"

InstancedCubeRenderer::InstancedCubeRenderer(PerspectiveCamera* camera, GLuint shaderProgramID)
{
	this->camera = camera;
	this->shaderProgramID = shaderProgramID;

	std::vector<float> vertices =
	{
		-0.5 ,-0.5, 0.5,
		0.5, -0.5, 0.5,
		0.5, 0.5, 0.5,
		-0.5, 0.5, 0.5,

		-0.5 ,-0.5, -0.5,
		0.5, -0.5, -0.5,
		0.5, 0.5, -0.5,
		-0.5, 0.5, -0.5
	};

	std::vector<int> indices =
	{
	0, 1, 2,
	2, 3, 0,

	1, 5, 6,
	6, 2, 1,

	7, 6, 5,
	5, 4, 7,

	4, 0, 3,
	3, 7, 4,

	3, 2, 6,
	6, 7, 3,

	4, 5, 1,
	1, 0, 4
	};

	VAO = new VertexArray;
	VAO->bind();

	VBO = new VertexBuffer(vertices);
	VBO->bind();

	EBO = new ElementBuffer(indices);
	EBO->bind();

	VAO->setAttributePointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

	VAO->unbind();
}

void InstancedCubeRenderer::commisionInstance(float x, float y, float z, float size, float r, float g, float b, float a)
{
	positions.push_back(glm::vec3(x, y, z));
	sizes.push_back(size);
	colors.push_back(glm::vec4(r, g, b, a));
}

void InstancedCubeRenderer::drawInstances()
{
	if(positions.empty())
		return;

	unsigned int instancePos;
	glGenBuffers(1, &instancePos);
	glBindBuffer(GL_ARRAY_BUFFER, instancePos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * positions.size(), &positions[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	unsigned int instanceSize;
	glGenBuffers(1, &instanceSize);
	glBindBuffer(GL_ARRAY_BUFFER, instanceSize);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * sizes.size(), &sizes[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	unsigned int instanceColor;
	glGenBuffers(1, &instanceColor);
	glBindBuffer(GL_ARRAY_BUFFER, instanceColor);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * colors.size(), &colors[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	VAO->bind();
	glBindBuffer(GL_ARRAY_BUFFER, instancePos);
	VAO->setAttributePointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glVertexAttribDivisor(1, 1);

	VAO->bind();
	glBindBuffer(GL_ARRAY_BUFFER, instanceSize);
	VAO->setAttributePointer(2, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glVertexAttribDivisor(2, 1);

	VAO->bind();
	glBindBuffer(GL_ARRAY_BUFFER, instanceColor);
	VAO->setAttributePointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glVertexAttribDivisor(3, 1);

	glUseProgram(shaderProgramID);

	unsigned int projectionLocation = glGetUniformLocation(shaderProgramID, "projection");
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(camera->getProjection()));

	unsigned int viewLocation = glGetUniformLocation(shaderProgramID, "view");
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(camera->getView()));

	VAO->bind();
	glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, positions.size());
	VAO->unbind();

	positions.clear();
	sizes.clear();
	colors.clear();

	glDeleteBuffers(1, &instancePos);
	glDeleteBuffers(1, &instanceSize);
	glDeleteBuffers(1, &instanceColor);
}
