#version 330

flat in vec4 Color;
//in vec4 Color;

out vec4 outputColor;

void main()
{
    vec4 outColor = Color;
    outputColor = outColor;
}

