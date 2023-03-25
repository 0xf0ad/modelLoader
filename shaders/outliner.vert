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
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[100];
mat4 BoneTransform;

void main(){
    if(!animated){
        gl_Position = STDprojection * STDview * model * vec4((aPosition + (aNormals * scale)), 1.0f);
    }else{
        BoneTransform  = finalBonesMatrices[boneIds[0]] * weights[0];
        BoneTransform += finalBonesMatrices[boneIds[1]] * weights[1];
		BoneTransform += finalBonesMatrices[boneIds[2]] * weights[2];
		BoneTransform += finalBonesMatrices[boneIds[3]] * weights[3];

        //vec4 NormalL = BoneTransform * vec4(aNormals, 0.0);
        
        //calculate the normals
        mat3 normalMatrix = transpose(inverse(mat3(BoneTransform)));
        //vec3 T = normalize(normalMatrix * aTangent);
        //vec3 B = normalize(normalMatrix * aBitangent);
        vec3 N = normalize(normalMatrix * aNormals);
		
        vec4 PosL = BoneTransform * vec4((aPosition + (N.xyz * scale)), 1.0f);
		gl_Position = STDprojection * STDview * model * PosL;
        //vec4 NormalL = BoneTransform * vec4(Normal, 0.0);
    }
}
