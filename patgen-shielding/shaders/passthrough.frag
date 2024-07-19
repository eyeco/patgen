#version 330 core

in vec4 vColor;
in vec2 vUV;

out vec4 col;

uniform sampler2D tex;

void main()
{
	vec3 c = texture( tex, vec2( vUV.x, vUV.y ) ).rgb;

	col = vec4( c, 1 );
}