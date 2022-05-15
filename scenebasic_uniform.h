#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"

#include "helper/objmesh.h"
#include <glm/glm.hpp>
#include "helper/cube.h"
#include "helper/skybox.h"
#include "customsrc/light.h"
#include "helper/teapot.h"
#include "customsrc/gauss.h"
#include "customsrc/nightvision.h"

class SceneBasic_Uniform : public Scene
{
private:
    GLSLProgram skyboxProgram, normalProgram, spotlightProgram, silhouetteProgram, basicProgram;
    std::unique_ptr<ObjMesh> ufo, meteor;
    Light pointLight, spotLight;
    SkyBox sky;
    GLuint skyboxTex, ufoDiffuseTex, ufoNormalTex, rockTex;
    Teapot teapot;
    Cube cube;

    float counter; 
    float oldT;

    //Shading States
    bool isNormalShading, isSilhouetteShading, isGaussianBlur, isNightVision;

    // Object properties
    glm::vec3 ufoPosition;
    std::vector<vec3> meteorPositions;
    std::vector<float> meteorRotations;

    // For gauss
    Gauss gauss;

    // For night vision
    NightVision nightVision;

    void setMatrices(GLSLProgram& prog);
    void compile();
    void bindTex(GLuint unit, GLuint texture);
public:
    SceneBasic_Uniform();

    void initScene();
    void update(float t, GLFWwindow* window);
    void render();
    void resize(int, int);
    void setNormalShading();
    void setSilhouetteShading();
    void setGaussianShading();
    void setNightVisionShading();
    void setUfoPosition(float newX, float newY, float newZ);
    void setSpotPosition(float newX, float newY, float newZ);
    void setPointPosition(float newX, float newY, float newZ);
};

#endif // SCENEBASIC_UNIFORM_H