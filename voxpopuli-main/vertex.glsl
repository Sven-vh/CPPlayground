#version 330
uniform sampler2D c;
in vec2 u;
out vec4 f;
void main(){
	f=/*sqrt*/(texture(c,u));
}