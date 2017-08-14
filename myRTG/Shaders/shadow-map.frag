#version 420 core
in vec4 pos;

uniform vec3 lightPos;
uniform float zdist;

void main()
{
    // get fragment depth
    gl_FragDepth = length(pos.xyz - lightPos) / zdist;
	//gl_FragDepth = 0;
}  