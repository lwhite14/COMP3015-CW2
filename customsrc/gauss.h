#ifndef GAUSS_H
#define GAUSS_H

#include <glad/glad.h>
#include <glm/glm.hpp>
using glm::mat4;
using glm::vec3;

#include "../helper/glslprogram.h"
#include "camera.h"
#include "light.h"
#include "../helper/skybox.h"
#include "../helper/objmesh.h"
#include "../helper/teapot.h"

class Gauss 
{
private:
    GLuint fsQuad;
    GLuint renderFBO, intermediateFBO;
    GLuint renderTex, intermediateTex;

    void setMatrices(GLSLProgram& prog, mat4 view, mat4 model, mat4 projection);
    void bindTex(GLuint unit, GLuint texture);

public:
	GLSLProgram spotlightProgram, normalProgram;

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
        GLSLProgram& skyboxProgram,
        SkyBox& sky,
        std::unique_ptr<ObjMesh>& ufo,
        std::unique_ptr<ObjMesh>& meteor,
        Teapot& teapot,
        vec3 ufoPosition,
        GLuint ufoDiffuseTex,
        GLuint ufoNormalTex,
        GLuint rockTex,
        std::vector<vec3> meteorPositions,
        std::vector<float> meteorRotations
    );
    void pass2(mat4& view, mat4& model, mat4& projection);
    void pass3(mat4& view, mat4& model, mat4& projection);
    void init(int width, int height);
    float gauss(float, float);
};

#endif