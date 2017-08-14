#version 420 core
#define MAX_LIGHTS 1

layout (std140, binding = 0) uniform ViewingTransforms{
	mat4 Projection;
	mat4 View;
	mat4 Model;
};
layout (std140, binding = 1) uniform Light{
	vec4 col;
	vec4 pos;
	vec4 dir;
	float spotAngle;
	float attenuation;
	int isOn;
	int type;
} lights[MAX_LIGHTS];
layout (std140, binding = 2) uniform Surface{
	vec4 Kd; // diffuse Color
	vec4 Ks; // specular Color
	int isVertexColored;
	float smoothness;
};

uniform uint numLights;
uniform uint pass; 
uniform samplerCube depthMap;
uniform sampler2D prevPass;
uniform float zdist;

in vec3 fpos;
in vec3 fnorm;
in vec3 pos;
in vec3 norm;
in vec3 eye;
in vec4 vcol;

out vec4 color;

float ShadowCalculation()
{
    // Get vector between fragment position and light position
    vec3 dir = fpos - lights[0].pos.xyz;
	dir += fnorm * dot(dir, -fnorm) * 0.05;
    // Use the light to fragment vector to sample from the depth map
    float lightDepth = texture(depthMap, dir).r * zdist;
    // It is currently in linear range between [0,1]. Re-transform back to original value
    // Now get current linear depth as the length between the fragment and light position
    float camDepth = length(dir);
    // Now test for shadows
    float bias = 0.005; 
	float shadow = camDepth -  bias > lightDepth ? 1.0 : 0.0;
    return shadow;
}  

void main()
{
	const float PI = 3.14159265359;
	vec3 col = vec3(0.0);
	{
		vec3 l = vec3(lights[0].pos) - fpos;
		float r = length(l);
		//r = (0.5 * r < 2.0) ? 2.0 : r;
		l = normalize(l);
		vec3 n = fnorm;
		vec3 Ecos = lights[0].col.rgb * max(dot(n, l), 0);
		vec3 v = normalize(eye-fpos);
		vec3 h = normalize(l + v);
		float cosH = max(dot(n, h), 0);
		float shad = ShadowCalculation(); 
		col =
			(Ks.rgb * pow(cosH, 0) + Kd.rgb) * Ecos;
		col *= 1-shad;
		col += 0.05 * Kd.rgb;
		col /= numLights;
	}    
	vec2 res = vec2(800.0, 600.0);
	vec2 coord = (gl_FragCoord.xy - 0.5) /(res * 1);
	if(pass == numLights)
		coord = (gl_FragCoord.xy - 0.5) / res;

	vec3 old = vec3(texture(prevPass, coord));  
	color = vec4(old + col, 1);
}
                                                                               