#version 450

in vec3 LightDir;
in vec2 TexCoord;
in vec3 ViewDir;

layout( location = 0 ) out vec4 FragColor;

uniform float EdgeThreshold;
uniform int Pass;
uniform float Weight[5];

layout(binding=0) uniform sampler2D Texture0;		// Render texture
layout(binding=1) uniform sampler2D ColorTex;		// Diffuse texture
layout(binding=2) uniform sampler2D NormalMapTex;	// Normal map texture

//light information struct
uniform struct LightInfo 
{
	vec4 Position;	// Light position in eye coords.
	vec3 La;		// Ambient light intensity
	vec3 L;			// Specular and diffuse intensity
} Light;

//material information struct
uniform struct MaterialInfo 
{
	vec3 Ka;			// Ambient reflectivity
	vec3 Kd;			// Diffuse reflectivity
	vec3 Ks;			// Specular reflectivity
	float Shininess;	// Specular shininess factor
} Material;

const vec3 lum = vec3(0.2126, 0.7152, 0.0722);

float luminance( vec3 color )
{
	return dot(lum,color);
}

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
}

vec4 pass1()
{
	vec3 normal = texture(NormalMapTex, TexCoord).rgb;
    normal = normalize(normal * 2.0 - 1.0); 
	return vec4(blinnPhong(ViewDir, normal), 1.0);
} // Texture/shade the object like normal. 

vec4 pass2()
{
	ivec2 pix = ivec2( gl_FragCoord.xy );
	vec4 sum = texelFetch(Texture0, pix, 0) * Weight[0];
	sum += texelFetchOffset( Texture0, pix, 0, ivec2(0,1) ) * Weight[1];
	sum += texelFetchOffset( Texture0, pix, 0, ivec2(0,-1) ) * Weight[1];
	sum += texelFetchOffset( Texture0, pix, 0, ivec2(0,2) ) * Weight[2];
	sum += texelFetchOffset( Texture0, pix, 0, ivec2(0,-2) ) * Weight[2];
	sum += texelFetchOffset( Texture0, pix, 0, ivec2(0,3) ) * Weight[3];
	sum += texelFetchOffset( Texture0, pix, 0, ivec2(0,-3) ) * Weight[3];
	sum += texelFetchOffset( Texture0, pix, 0, ivec2(0,4) ) * Weight[4];
	sum += texelFetchOffset( Texture0, pix, 0, ivec2(0,-4) ) * Weight[4];
	return sum;
} // Pixels in the y

vec4 pass3()
{
	ivec2 pix = ivec2( gl_FragCoord.xy );
	vec4 sum = texelFetch(Texture0, pix, 0) * Weight[0];
	sum += texelFetchOffset( Texture0, pix, 0, ivec2(1,0) ) * Weight[1];
	sum += texelFetchOffset( Texture0, pix, 0, ivec2(-1,0) ) * Weight[1];
	sum += texelFetchOffset( Texture0, pix, 0, ivec2(2,0) ) * Weight[2];
	sum += texelFetchOffset( Texture0, pix, 0, ivec2(-2,0) ) * Weight[2];
	sum += texelFetchOffset( Texture0, pix, 0, ivec2(3,0) ) * Weight[3];
	sum += texelFetchOffset( Texture0, pix, 0, ivec2(-3,0) ) * Weight[3];
	sum += texelFetchOffset( Texture0, pix, 0, ivec2(4,0) ) * Weight[4];
	sum += texelFetchOffset( Texture0, pix, 0, ivec2(-4,0) ) * Weight[4];
	return sum;
} // Pixels in the x

void main()
{
	if( Pass == 1 )
		FragColor = pass1();
	else if( Pass == 2 )
		FragColor = pass2();
	else if( Pass == 3 )
		FragColor = pass3();
}

