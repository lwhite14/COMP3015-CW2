#version 450 

in vec3 Vec;

layout( location = 0 ) out vec4 FragColor;

layout(binding=0) uniform samplerCube SkyBoxTex;

void main()
{
	vec3 texColor = texture(SkyBoxTex, normalize(Vec)).rgb;
	FragColor = vec4 ( texColor, 1.0 );
}	// Takes input texture and applies it to box (skybox).