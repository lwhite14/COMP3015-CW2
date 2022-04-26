#version 450

uniform struct LightInfo 
{
	vec4 Position;
	vec3 Intensity;
} Light;

//material information struct
uniform struct MaterialInfo 
{
	vec3 Ka;
	vec3 Kd;
	float Shininess;
} Material;

uniform vec4 LineColor;

in vec3 GPosition;
in vec3 GNormal;

flat in int GIsEdge;

layout (location = 0) out vec4 FragColor;

const int levels = 3;
const float scaleFactor = 1.0;

vec3 toonShade()
{
	vec3 s = normalize(Light.Position.xyz - GPosition.xyz);
	vec3 ambient = Material.Ka;
	float cosine = dot(s, GNormal);
	vec3 diffuse = Material.Kd * ceil(cosine * levels) * scaleFactor;

	return Light.Intensity * (ambient + diffuse);
}

void main()
{
	if (GIsEdge == 1)
	{
		FragColor = LineColor;
	}
	else
	{
		FragColor = vec4(toonShade(), 1.0);
	}
}