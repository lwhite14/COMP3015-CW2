#ifndef Light_H
#define Light_H

#include <glm/glm.hpp>
using glm::vec3;
using glm::vec4;

class Light 
{
public:
	vec4 position;
	vec3 ambient;
	vec3 diffSpec;
	
	vec3 direction;
	float exponent;
	float cutoff;

	Light() {}

	Light(vec4 newPos, vec3 newAm, vec3 newDiffSpec) 
	{
		position = newPos;
		ambient = newAm;
		diffSpec = newDiffSpec;

		direction = vec3(0, 0, 0);
		exponent = 0.0f;
		cutoff = 0.0f;
	}

	Light(vec4 newPos, vec3 newAm, vec3 newDiffSpec, vec3 newDir, float newExp, float newCut)
	{
		position = newPos;
		ambient = newAm;
		diffSpec = newDiffSpec;
		direction = newDir;
		exponent = newExp;
		cutoff = newCut;
	}
};


#endif // Light_H
