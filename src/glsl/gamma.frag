#version 300 es
precision highp float;  // or mediump depending on your needs

//#version 330 core

out vec4 out_color;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{ 
    out_color = texture(screenTexture, TexCoords);
}