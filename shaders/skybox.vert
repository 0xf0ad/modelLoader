#version 450 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

layout (std140) uniform VP{
	mat4 STDprojection;
	mat4 STDview;
};

void main(){
	TexCoords = aPos;
	vec4 pos = STDprojection * mat4(mat3(STDview)) * vec4(aPos, 1.0);
	gl_Position = pos.xyww;
}
