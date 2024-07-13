#version 330
in vec4 p;
in vec2 t;
out vec2 u;
void main(){
	u=t;
	gl_Position=p;
}