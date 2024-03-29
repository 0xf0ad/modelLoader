#version 450 core

out vec4 FragColor;

in VS_OUT{
	vec2 TexCoords;
	flat uint TextureID;
} fs_in;

uniform sampler2D texture_diffuse[32];

void main(){
	FragColor = texture(texture_diffuse[fs_in.TextureID], fs_in.TexCoords);
}
