#version 450 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 tex;
layout (location = 3) in uint textureID;
layout (location = 4) in uint bones;
layout (location = 5) in vec4 weights;

ivec4 boneIds = ivec4((uint( bones         & 255u)),
                      (uint((bones >>  8u) & 255u)),
                      (uint((bones >> 16u) & 255u)),
                      (uint((bones >> 24u) & 255u)));

uniform mat4 model;
uniform float modelsize;
uniform bool animated;
uniform mat4 b_Mats[250];
mat4 BoneTransform;

layout (std140) uniform VP{
	mat4 STDprojection;
	mat4 STDview;
};

out VS_OUT{
	vec2 TexCoords;
	flat uint TextureID;
} vs_out;

void main(){
	if (!animated){
		gl_Position = STDprojection * STDview * model * vec4(pos, modelsize);
	} else {
		BoneTransform  = b_Mats[boneIds[0]] * weights[0];
		BoneTransform += b_Mats[boneIds[1]] * weights[1];
		BoneTransform += b_Mats[boneIds[2]] * weights[2];
		BoneTransform += b_Mats[boneIds[3]] * weights[3];

		gl_Position = STDprojection * STDview * model * BoneTransform * vec4(pos, modelsize);
	}
	vs_out.TexCoords = tex;
	vs_out.TextureID = textureID;
}
