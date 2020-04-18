#version 400 core

layout (location = 0) in vec2 aVertexOffset;
layout (location = 1) in vec2 aUV;

out vec2 UV;
uniform vec2 Pos;
uniform mat4 Projection;

void main()
{
	gl_Position = Projection * vec4(aVertexOffset + Pos, 0, 1);
	UV = aUV;
}

===
#version 400 core

in vec2 UV;
out vec4 OutColor;
uniform sampler2D Sampler;

void main()
{
	OutColor = texture(Sampler, UV); 
}

