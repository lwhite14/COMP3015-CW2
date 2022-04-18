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

layout(binding=0) uniform sampler2D Tex1;

//declare your Material uniform variables here
layout( location = 0 ) out vec4 FragColor;

vec3 blinnPhongSpot( vec3 position, vec3 n ) 
{
	vec3 ambient = Material.Ka * Spot.La; 
	vec3 s = normalize(vec3(Spot.Position - vec4(position, 1.0f)));
	float cosAng = dot(-s, normalize(Spot.Direction)); //cosine of the angle
	float angle = acos( cosAng ); //gives you the actual angle
	float spotScale = 0.0;
	vec3 diffuse = vec3(0.0);
	vec3 spec = vec3(0.0);
	if(angle < Spot.Cutoff ) // If the angle between spot and fragment is below the target angle, apply diffuse/specular lighting
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
	vec3 texColor = texture(Tex1, TexCoord).rgb;

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

void main()
{
	vec3 Colour = vec3(0.0); 
	Colour += blinnPhongPoint(Position, normalize(Normal)).xyz;
	Colour += blinnPhongSpot(Position, normalize(Normal)).xyz;
	FragColor = vec4(Colour, 1.0f);
}