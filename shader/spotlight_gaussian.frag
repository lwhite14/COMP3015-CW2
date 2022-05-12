#version 450

in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;

uniform struct SpotLightInfo 
{
	vec4 Position;	// Position in cam coords
	vec3 La;		// Ambient intensity
	vec3 L;			// Diffuse/Specular intensity
	vec3 Direction; // Direction of the spotlight in cam coords.
	float Exponent; // Angular attenuation exponent
	float Cutoff;	// Cutoff angle (between 0 and pi/2)
} Spot;

uniform struct PointLightInfo
{
	vec4 Position;	// Position in cam coords
	vec3 La;		// Ambient intensity
	vec3 L;			// Diffuse/Specular intensity
} Point;

uniform struct MaterialInfo 
{
	vec3 Ka;			// Ambient reflectivity
	vec3 Kd;			// Diffuse reflectivity
	vec3 Ks;			// Specular reflectivity
	float Shininess;	// Specular shininess factor
} Material;

layout(binding=0) uniform sampler2D Texture0;
layout(binding=1) uniform sampler2D ColorTex;

layout( location = 0 ) out vec4 FragColor;

uniform float EdgeThreshold;
uniform int Pass;
uniform float Weight[5];

const vec3 lum = vec3(0.2126, 0.7152, 0.0722);

float luminance( vec3 color )
{
	return dot(lum,color);
}

vec3 blinnPhongSpot( vec3 position, vec3 n ) 
{
	vec3 ambient = Material.Ka * Spot.La; 
	vec3 s = normalize(vec3(Spot.Position - vec4(position, 1.0f)));
	float cosAng = dot(-s, normalize(Spot.Direction));
	float angle = acos( cosAng );
	float spotScale = 0.0;
	vec3 diffuse = vec3(0.0);
	vec3 spec = vec3(0.0);
	if(angle < Spot.Cutoff )
	{
		spotScale = pow( cosAng, Spot.Exponent );
		float sDotN = max( dot(s,n), 0.0 );
		diffuse = Material.Kd * sDotN * (spotScale * Spot.L);
		if( sDotN > 0.0 )
		{
			vec3 v = normalize(-position.xyz);
			vec3 h = normalize( v + s ); 
			spec = Material.Ks * pow( max( dot(h,n), 0.0 ), Material.Shininess );
		}
	}
	return ambient + diffuse + spec;
}

vec3 blinnPhongPoint( vec3 position, vec3 n )
{
	vec3 texColor = texture(ColorTex, TexCoord).rgb;

	vec3 ambient = Material.Ka * Point.La * texColor;

	vec3 s = normalize(vec3(Point.Position - vec4(position, 1.0f)));
	float sDotN = max( dot(s,n), 0.0 );
	vec3 diffuse = Material.Kd * sDotN * Point.L * texColor;

	vec3 spec = vec3(0.0);
	if( sDotN > 0.0 )
	{
		vec3 v = normalize(-position.xyz);
		vec3 h = normalize( v + s ); 
		spec = Material.Ks * pow( max( dot(h,n), 0.0 ), Material.Shininess );
	}
	return ambient + diffuse + spec;
}

vec4 pass1()
{
	vec3 Colour = vec3(0.0); 
	Colour += blinnPhongPoint(Position, normalize(Normal)).xyz;
	Colour += blinnPhongSpot(Position, normalize(Normal)).xyz;
	return vec4(Colour, 1.0f);
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
