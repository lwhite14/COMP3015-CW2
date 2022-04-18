#version 450

in vec3 LightDir;
in vec2 TexCoord;
in vec3 ViewDir;

layout( location = 0 ) out vec4 FragColor;

 //light information struct
uniform struct LightInfo 
{
	vec4 Position;	// Light position in eye coords.
	vec3 La;		// Ambient light intensity
	vec3 L;			// Specular/Diffuse light intensity
} Light;

//material information struct
uniform struct MaterialInfo 
{
	vec3 Ka;			// Ambient reflectivity
	vec3 Kd;			// Diffuse reflectivity
	vec3 Ks;			// Specular reflectivity
	float Shininess;	// Specular shininess factor
} Material;

layout(binding=0) uniform sampler2D ColorTex;		// Diffuse texture
layout(binding=1) uniform sampler2D NormalMapTex;	// Normal map texture

vec3 blinnPhong( vec3 position, vec3 normal ) 
{
	vec3 texColor = texture(ColorTex, TexCoord).rgb;

	vec3 ambient = Material.Ka * Light.La * texColor;

	vec3 s = normalize(vec3(Light.Position - vec4(position, 1.0)));
	float sDotN = max( dot(s,normal), 0.0 );
	vec3 diffuse = Material.Kd * sDotN * Light.L * texColor;

	vec3 spec = vec3(0.0);
	if( sDotN > 0.0 )
	{
		vec3 v = normalize(-position.xyz);
		vec3 h = normalize( v + s ); 
		spec = Material.Ks * pow( max( dot(h,normal), 0.0 ), Material.Shininess );
	}
	return ambient + diffuse + spec;
} // Normal blinnPhong used to calculate lighting for fragment.

void main()
{
    vec3 normal = texture(NormalMapTex, TexCoord).rgb;
    normal = normalize(normal * 2.0 - 1.0); 
	FragColor = vec4(blinnPhong(ViewDir, normal), 1.0);
} // Apply texture with normal map used as normal for blinnPhong.