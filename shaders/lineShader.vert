#version 450 core

layout(location = 0) in vec3 iPos;

void main(){
	gl_Position = vec4(iPos.x, iPos.y, iPos.z, 1.0f);
}
