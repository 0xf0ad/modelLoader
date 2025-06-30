#version 450 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormals;
layout (location = 4) in uint boneIDs;
layout (location = 5) in vec4 weights;

ivec4 boneIds = ivec4((int( boneIDs         & 255u)),
                      (int((boneIDs >>  8u) & 255u)),
                      (int((boneIDs >> 16u) & 255u)),
                      (int((boneIDs >> 24u) & 255u)));


uniform mat4 model;
uniform float scale;

layout (std140) uniform VP{
	mat4 STDprojection;
	mat4 STDview;
};

uniform bool animated;
uniform mat4 b_Mats[100];
mat4 BoneTransform;

void main(){
	if(!animated){
		gl_Position = STDprojection * STDview * model * vec4((aPosition + (aNormals * scale)), 1.0f);
	}else{
		BoneTransform  = b_Mats[boneIds[0]] * weights[0];
		BoneTransform += b_Mats[boneIds[1]] * weights[1];
		BoneTransform += b_Mats[boneIds[2]] * weights[2];
		BoneTransform += b_Mats[boneIds[3]] * weights[3];

		//calculate the normals
		vec3 Normal = mat3(transpose(inverse(STDview * model))) * aNormals;

		vec4 PosL = BoneTransform * vec4((aPosition + (Normal * scale)), 1.0f);
		gl_Position = STDprojection * STDview * model * PosL;
	}
}
