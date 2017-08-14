#version 420 core

out vec4 color;
void main()
{
	if(dot(gl_PointCoord-0.5,gl_PointCoord-0.5)>0.25) 
		discard;
	 else
		color = vec4(0,0.9,0.2,0.9);
}