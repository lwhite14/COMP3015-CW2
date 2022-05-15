#version 450

in vec3 Position; 
in vec3 Normal; 
in vec2 TexCoord;

uniform int Width;
uniform int Height; 
uniform float Radius; 
uniform sampler2D RenderTex; 
uniform sampler2D NoiseTex;

subroutine vec4 RenderPassType(); 
subroutine uniform RenderPassType RenderPass;

struct LightInfo {
	vec4 Position; // Light position in eye coords. 
	vec3 Intensity; // A,D,S intensity
};
uniform LightInfo Light;

struct MaterialInfo { 
	vec3 Ka; // Ambient reflectivity
	vec3 Kd; // Diffuse reflectivity
	vec3 Ks; // Specular reflectivity 
	float Shininess; // Specular shininess factor 
}; 
uniform MaterialInfo Material;

layout( location = 0 ) out vec4 FragColor;

vec3 phongModel( vec3 position, vec3 normal ) 
{
	// Calculate ambient here
	vec3 ambient = Material.Ka * Light.Intensity.x;

	// Calculate diffuse here
	vec3 s = normalize(vec3(Light.Position - vec4(position, 1.0f)));
	float sDotN = max( dot(s,normal), 0.0 );
	vec3 diffuse = Material.Kd * Light.Intensity.y * sDotN;

	// Calculate specular here
	vec3 spec = vec3(0.0);
	if( sDotN > 0.0 )
	{
		vec3 v = normalize(-position.xyz);
		vec3 r = reflect( -s, normal );
		spec = Material.Ks * Light.Intensity.z * pow( max( dot(r,v), 0.0 ), Material.Shininess );
	}
	return ambient + diffuse + spec;
}

float luminance( vec3 color ) 
{
	return dot( color.rgb, vec3(0.2126, 0.7152, 0.0722) );
}

subroutine (RenderPassType) 
vec4 pass1()
{
	return vec4(phongModel( Position, Normal ),1.0);
}

subroutine (RenderPassType)
vec4 pass2()
{
	vec4 noise = texture (NoiseTex, TexCoord); 
	vec4 color = texture (RenderTex, TexCoord); 

	float green = luminance( color.rgb );

	return vec4(0.0, green * clamp ( noise.a, 0., 1.0), 0.0 ,1.0);
}

void main()
{
	FragColor = RenderPass();
}