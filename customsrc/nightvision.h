#ifndef NIGHTVISION_H
#define NIGHTVISION_H

#include <glad/glad.h>
#include <glm/glm.hpp>
using glm::mat4;
using glm::vec3;

#include "../helper/glslprogram.h"
#include "../helper/noisetex.h"
#include "camera.h"
#include "light.h"
#include "../helper/objmesh.h"
#include "../helper/teapot.h"

class NightVision
{
private:
    GLuint fsQuad, pass1Index, pass2Index;
    GLuint renderFBO;
    GLuint renderTex;
    GLuint noiseTex;

	void setMatrices(GLSLProgram& prog, mat4 view, mat4 model, mat4 projection);
	void bindTex(GLuint unit, GLuint texture);
public:
    GLSLProgram program;

    void setupFBO(int width, int height);
    void pass1
    (
        mat4& view,
        mat4& model,
        mat4& projection,
        Camera& camera,
        int width,
        int height,
        Light pointLight,
        Light spotLight,
        vec3 ufoPosition,
        GLuint ufoDiffuseTex,
        GLuint ufoNormalTex,
        GLuint rockTex,
        std::unique_ptr<ObjMesh>& ufo,
        std::unique_ptr<ObjMesh>& meteor,
        Teapot& teapot,
        std::vector<vec3> meteorPositions,
        std::vector<float> meteorRotations
    );
    void pass2(mat4& view, mat4& model, mat4& projection);
    void init(int width, int height);
};

#endif