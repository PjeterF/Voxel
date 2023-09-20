#include "Mesh.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <GLES3/gl3.h>

Mesh::Mesh(std::vector<float> vertices, std::vector<int> indices, GLuint meshShader)
{
	this->meshShader = meshShader;
	vertexCount = vertices.size();
	indexCount = indices.size();

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData
	(
		GL_ELEMENT_ARRAY_BUFFER,
		indices.size() * sizeof(int),
		&indices[0],
		GL_STATIC_DRAW
	);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 10, (void*)0);
	glEnableVertexAttribArray(0);									
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 10, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);									
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 10, (void*)(7 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}

Mesh::Mesh(std::vector<Chunk::Vertex> vertices, std::vector<Chunk::Index> indices, GLuint meshShader, GLuint shadowShader)
{
	this->meshShader = meshShader;
	this->shadowShader = shadowShader;
	vertexCount = vertices.size();
	indexCount = indices.size()*3;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Chunk::Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData
	(
		GL_ELEMENT_ARRAY_BUFFER,
		indices.size() * sizeof(Chunk::Index),
		&indices[0],
		GL_STATIC_DRAW
	);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Chunk::Vertex), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Chunk::Vertex), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Chunk::Vertex), (void*)(7 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}

void Mesh::draw(PerspectiveCamera* camera, glm::vec3 lightDirection)
{
	
	unsigned int shadowMapFBO;
	glGenFramebuffers(1, &shadowMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);

	unsigned int shadowMapWidth = 1000, shadowMapHeight = 1000;
	unsigned int shadowMap;

	glGenTextures(1, &shadowMap);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 }; //
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Error in framebuffer Error code: " << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	ImGui::Begin("ShadowMap");

	static float lp[3] = { 0, 0, 100 };
	ImGui::InputFloat3("lightPos", lp);
	glm::vec3 lightPos = glm::vec3(lp[0], lp[1], lp[2]);

	static float la[3] = { 500, 500, 0 };
	ImGui::InputFloat3("lookAt", la);
	glm::vec3 lokAt = glm::vec3(la[0], la[1], la[2]);

	static float ortho[4] = { -200, 200, -200, 200 };
	ImGui::InputFloat4("ortho", ortho);


	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glUseProgram(shadowShader);

	// Compute the MVP matrix from the light's point of view
	glm::mat4 lightProjection = glm::ortho(ortho[0], ortho[1], ortho[2], ortho[3], 50.0f, 500.0f);
	glm::mat4 lightView = glm::lookAt(lightPos, lokAt, glm::vec3(0, 0, 1));
	glm::mat4 lightTransform = lightProjection * lightView;

	unsigned int lightProjectionLocationShadowMap = glGetUniformLocation(shadowShader, "lightTransform");
	glUniformMatrix4fv(lightProjectionLocationShadowMap, 1, GL_FALSE, glm::value_ptr(lightTransform));

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);


	ImGui::Image((ImTextureID)shadowMap, ImVec2(200, 200));
	ImGui::End();

	glm::mat4 biasMatrix(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);

	lightDirection = glm::normalize(lightDirection);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(meshShader);

	unsigned int lightProjectionLocation = glGetUniformLocation(meshShader, "lightTransform");
	glUniformMatrix4fv(lightProjectionLocation, 1, GL_FALSE, glm::value_ptr(biasMatrix*lightTransform));

	unsigned int projectionLocation = glGetUniformLocation(meshShader, "projection");
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(camera->getProjection()));
	//glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(lightProjection));

	unsigned int viewLocation = glGetUniformLocation(meshShader, "view");
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(camera->getView()));
	//glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(lightView));

	unsigned int lightDirectionLocation = glGetUniformLocation(meshShader, "lightDirection");
	glUniform3f(lightDirectionLocation, lightDirection.x, lightDirection.y, lightDirection.z);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glUseProgram(0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glDeleteTextures(1, &shadowMap);
}