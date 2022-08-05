#include "../headers/bone.h"

Mesh::Mesh(const std::vector<Vertex>&  vertices,
           const std::vector<uint>&    indices,
           const std::vector<Texture>& textures){

	this->vertices = vertices;
	this->indices  = indices;
	this->textures = textures;

	setupMesh();
}

void Mesh::Draw(Shader &shader){

	unsigned char diffuseNr  = 0;
	unsigned char specularNr = 0;
	unsigned char normalNr   = 0;
	unsigned char heightNr   = 0;

	for(unsigned int i = 0; i < textures.size(); i++){

		glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding

		// retrieve texture number (the N in diffuse_textureN)
		std::string number;
		std::string name = textures[i].type;

		if(name == "texture_diffuse"){
			number = std::to_string(diffuseNr++);
		}else if(name == "texture_specular"){
			number = std::to_string(specularNr++); // transfer unsigned int to string
		}else if(name == "texture_normal"){
			number = std::to_string(normalNr++); // transfer unsigned int to string
		}else if(name == "texture_height"){
			number = std::to_string(heightNr++); // transfer unsigned int to string
		}
		// now set the sampler to the correct texture unit
		glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
		//and then bind the texture
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
	}

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

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);


	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

	// vertex tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

	// vertex bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

	// ids
	glEnableVertexAttribArray(5);
	glVertexAttribIPointer(5, 4, GL_INT           , sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

	// weights
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));

	glBindVertexArray(0);
}
