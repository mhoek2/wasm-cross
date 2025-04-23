#version 300 es
precision mediump float;  // or mediump depending on your needs

//#version 330 core

in vec4 var_Color;

out vec4 out_color;

void main() 
{
	out_color = var_Color;
}