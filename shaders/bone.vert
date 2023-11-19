#version 450 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 tex;


uniform mat4 model;

uniform float modelsize;

uniform bool animated;

uniform mat4 b_Mats[125];
uniform mat4 offsets[125];
mat4 BoneTransform;

layout (std140) uniform VP{
	mat4 STDprojection;
	mat4 STDview;
};


// THIS SHIYT AIYNT WORKING

void main(){
	if (!animated){
		gl_Position = STDprojection * STDview * model * vec4(pos, modelsize);
	}else{
		gl_Position = STDprojection * STDview * model * b_Mats[gl_InstanceID] /* (offsets[gl_InstanceID] */ * vec4(pos, 1.0f);
	}
}
