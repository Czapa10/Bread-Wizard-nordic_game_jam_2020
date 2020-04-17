#version 400 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;

out vec2 UV;

void main()
{
	UV = aUV;
	gl_Position = vec4(aPos, 0, 1);
}

===
#version 400 core

in vec2 UV;
out vec4 OutColor;
uniform sampler2D Sampler;

void main()
{
	OutColor = texture(Sampler, UV); 
	//OutColor = vec4(1, 1, 0, 1);
}

