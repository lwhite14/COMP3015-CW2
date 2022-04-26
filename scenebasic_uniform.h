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
    GLSLProgram skyboxProgram, normalProgram, spotlightProgram, silhouetteProgram;
    std::unique_ptr<ObjMesh> ufo, meteor;
    Light pointLight, spotLight;
    SkyBox sky;
    GLuint skyboxTex, ufoDiffuseTex, ufoNormalTex, rockTex;
    Teapot teapot;

    //Shading States
    bool isNormalShading, isSilhouetteShading;

    // Meteor properties
    std::vector<vec3> meteorPositions;
    std::vector<float> meteorRotations;

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
};

#endif // SCENEBASIC_UNIFORM_H