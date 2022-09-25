#include "../headers/bone.h"
#include <glm/ext/vector_float3.hpp>

Mesh::Mesh(const std::vector<Vertex>&  vertices,
           const std::vector<uint>&    indices,
           const std::vector<Texture>& textures){

	this->vertices = vertices;
	this->indices  = indices;
	this->textures = textures;

	//setupMesh();
}

Mesh::Mesh(const std::vector<uint>&    indices,
           const std::vector<Texture>& textures){

	this->indices  = indices;
	this->textures = textures;

	//setupMesh();
}

Mesh::Mesh(const std::vector<Texture>& textures){

	this->textures = textures;

	//setupMesh();
}


void Mesh::Draw(Shader &shader){

	//batch the textures and upload them the GPU
	GLint location = glGetUniformLocation(shader.ID, "texture_diffuse");
	glUniform1iv(location, defuseTexturesIDs.size() , defuseTexturesIDs.data());
	
	//upload the inddeces to the GPU
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);
}

void Mesh::setupMesh(){
	glGenVertexArrays(true, &VAO);
	glGenBuffers(true, &VBO);
	glGenBuffers(true, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), &indices[0], GL_STATIC_DRAW);


	unsigned char sizeofVertex = sizeof(Vertex);
	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeofVertex, (void*)0);

	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeofVertex, (void*)offsetof(Vertex, Normal));

	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeofVertex, (void*)offsetof(Vertex, TexCoords));

	// vertex tangent
	//glEnableVertexAttribArray(3);
	//glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeofVertex, (void*)offsetof(Vertex, Tangent));

	// vertex bitangent
	//glEnableVertexAttribArray(4);
	//glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeofVertex, (void*)offsetof(Vertex, Bitangent));

	// vertex bitangent
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 1, GL_UNSIGNED_BYTE , sizeofVertex, (void*)offsetof(Vertex, textureIndex));

	// ids
	glEnableVertexAttribArray(4);
	glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT  , sizeofVertex, (void*)offsetof(Vertex, boneIDs));

	// weights
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeofVertex, (void*)offsetof(Vertex, weights));

	glBindVertexArray(0);

	for(unsigned int i = 0; i != textures.size(); i++){

		glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
		
		std::string name = textures[i].type;
		unsigned char textureID = textures[i].id;

		if     (name == "texture_diffuse"){
			defuseTexturesIDs.push_back(textureID);
			glBindTextureUnit(textureID , textureID);
		}
		else if(name == "texture_specular")
			specularTexturesIDs.push_back(textureID);
		else if(name == "texture_normal")
			normalTexturesIDs.push_back(textureID);
		else if(name == "texture_height")
			heightTexturesIDs.push_back(textureID);
	}
}
