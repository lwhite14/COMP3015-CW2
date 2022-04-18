#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"

#include "helper/objmesh.h"
#include <glm/glm.hpp>
#include "helper/cube.h"
#include "helper/skybox.h"
#include "helper/light.h"
#include "helper/teapot.h"

class SceneBasic_Uniform : public Scene
{
private:
    GLSLProgram skyboxProgram, normalProgram, spotlightProgram, normalGaussianProgram, spotlightGaussianProgram;
    std::unique_ptr<ObjMesh> ufo, meteor;
    Light pointLight, spotLight;
    SkyBox sky;
    GLuint skyboxTex, ufoDiffuseTex, ufoNormalTex, rockTex;
    Teapot teapot;

    // Meteor properties
    std::vector<vec3> meteorPositions;
    std::vector<float> meteorRotations;

    // For gaussian blur
    GLuint fsQuad;
    GLuint renderFBO, intermediateFBO;
    GLuint renderTex, intermediateTex;
    bool isBlur, firstBack; 

    void setMatrices(GLSLProgram& prog);
    void compile();
    void bindTex(GLuint unit, GLuint texture);
    void setupFBO();
    void pass1();
    void pass2();
    void pass3();
    void initGauss();
    float gauss(float, float);
public:
    SceneBasic_Uniform();

    void initScene();
    void update(float t, GLFWwindow* window);
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H