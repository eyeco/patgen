#version 330 core

in vec3 inPosition;
in vec4 inColor;
in vec2 inUV;

out vec4 vColor;
out vec2 vUV;

void main()
{
	gl_Position = vec4( inPosition, 1 );
	vColor = inColor;
	vUV = inUV;
}
