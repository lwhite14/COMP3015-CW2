#include "scenebasic_uniform.h"

#include <sstream>
#include <iostream>
using std::cerr;
using std::endl;

#include <glm/gtc/matrix_transform.hpp>
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

#include "helper/texture.h"
#include "helper/noisetex.h"


SceneBasic_Uniform::SceneBasic_Uniform() :  ufoPosition(vec3(0.0f, 20.0f, 0.0f)),
                                            pointLight(Light(vec4(45.0f, -25.0f, 125.0f, 1.0),
                                                vec3(0.0f, 0.0f, 0.15f),
                                                vec3(1.0f))),
                                            spotLight(Light(vec4(vec3(0.0f, 20.0f, 0.0f), 1.0f),
                                                vec3(0.0f),
                                                vec3(0.2f, 0.95f, 0.2f),
                                                vec3(),
                                                40.0f,
                                                glm::radians(20.0f))),
                                            sky(500.0f),
                                            teapot(14, mat4(1.0f)),
                                            counter(0.0f),
                                            oldT(0.0f),
                                            isNormalShading(true), 
                                            isSilhouetteShading(false),
                                            isGaussianBlur(false),
                                            isNightVision(false)
{
    ufo = ObjMesh::loadWithAdjacency("media/ufo.obj");
    meteor = ObjMesh::loadWithAdjacency("media/meteor.obj");
}


void SceneBasic_Uniform::initScene()
{
    meteorPositions = std::vector<vec3>
    {
       vec3(6.0f, -40.0f, 0.0f),
       vec3(-40.0f, -60.0f, -50.0f),
       vec3(-100.0f, -60.0f, 42.0f)
    };
    meteorRotations = std::vector<float>
    {
        0.0f,
        -30.0f,
        60.0f
    };

    compile();
    glEnable(GL_DEPTH_TEST);
    nightVision.init(width, height);
    gauss.init(width, height);

    projection = mat4(1.0f);
    view = mat4(1.0f);

    skyboxTex = Texture::loadCubeMap("media/texture/nova/nova");
    ufoDiffuseTex = Texture::loadTexture("media/texture/ufo_diffuse.png");
    ufoNormalTex = Texture::loadTexture("media/texture/ufo_normal.png");
    rockTex = Texture::loadTexture("media/texture/rock.jpg");
}

void SceneBasic_Uniform::compile()
{
    try
    {
        skyboxProgram.compileShader("shader/skybox.vert");
        skyboxProgram.compileShader("shader/skybox.frag");
        skyboxProgram.link();

        normalProgram.compileShader("shader/normal_map.vert");
        normalProgram.compileShader("shader/normal_map.frag");
        normalProgram.link();

        spotlightProgram.compileShader("shader/spotlight.vert");
        spotlightProgram.compileShader("shader/spotlight.frag");
        spotlightProgram.link();

        silhouetteProgram.compileShader("shader/silhouette_lines.vert");
        silhouetteProgram.compileShader("shader/silhouette_lines.frag");
        silhouetteProgram.compileShader("shader/silhouette_lines.geom");
        silhouetteProgram.link();

        basicProgram.compileShader("shader/basic.vert");
        basicProgram.compileShader("shader/basic.frag");
        basicProgram.link(); 
        
        gauss.spotlightProgram.compileShader("shader/spotlight.vert");
        gauss.spotlightProgram.compileShader("shader/spotlight_gaussian.frag");
        gauss.spotlightProgram.link();
        
        gauss.normalProgram.compileShader("shader/normal_map.vert");
        gauss.normalProgram.compileShader("shader/normal_map_gaussian.frag");
        gauss.normalProgram.link();

        nightVision.program.compileShader("shader/night_vision.vert");
        nightVision.program.compileShader("shader/night_vision.frag");
        nightVision.program.link();
    }
    catch (GLSLProgramException& e)
    {
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

void SceneBasic_Uniform::update(float t, GLFWwindow* window)
{
    camera.UpdateDeltaTime();
    camera.Movement();
    camera.KeyCallback(window);
    camera.MouseCallback(window);

    float dt = t - oldT;
    meteorRotations[1] = meteorRotations[1] + (dt*50.0f);
    meteorRotations[2] = meteorRotations[2] + (dt*50.0f);
    if (meteorRotations[1] > 360.0f) 
    {
        meteorRotations[1] = meteorRotations[1] - 360.0f;
    }
    if (meteorRotations[2] > 360.0f)
    {
        meteorRotations[2] = meteorRotations[2] - 360.0f;
    }
    oldT = t;
}

void SceneBasic_Uniform::render()
{
    if (isNormalShading || isSilhouetteShading) // Executed if not Gaussian Blur
    {
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        view = camera.ViewLookAt(view);

        // Skybox
        skyboxProgram.use();
        model = mat4(1.0f);
        setMatrices(skyboxProgram);
        sky.render();
    }
    if (isNormalShading) 
    {
        // UFO
        normalProgram.use();
        normalProgram.setUniform("Light.Position", pointLight.position);
        normalProgram.setUniform("Light.La", pointLight.ambient);
        normalProgram.setUniform("Light.L", pointLight.diffSpec);
        normalProgram.setUniform("Material.Kd", vec3(0.5f));
        normalProgram.setUniform("Material.Ks", vec3(0.5f));
        normalProgram.setUniform("Material.Ka", vec3(0.25f, 0.25f, 1.0f));
        normalProgram.setUniform("Material.Shininess", 128.0f);
        model = mat4(1.0f);
        model = glm::translate(model, ufoPosition);
        model = glm::rotate(model, glm::radians(-90.0f), vec3(0.0f, 1.0f, 0.0f));
        setMatrices(normalProgram);
        bindTex(GL_TEXTURE0, ufoDiffuseTex);
        bindTex(GL_TEXTURE1, ufoNormalTex);
        ufo->render();

        // Meteors
        spotlightProgram.use();
        spotlightProgram.setUniform("Spot.L", spotLight.diffSpec);
        spotlightProgram.setUniform("Spot.La", spotLight.ambient);
        spotlightProgram.setUniform("Spot.Exponent", spotLight.exponent);
        spotlightProgram.setUniform("Spot.Cutoff", spotLight.cutoff);
        mat3 normalMatrix = mat3(vec3(view[0]), vec3(view[1]), vec3(view[2]));
        spotLight.direction = normalMatrix * vec3(-spotLight.position);
        spotlightProgram.setUniform("Spot.Position", view * spotLight.position);
        spotlightProgram.setUniform("Spot.Direction", spotLight.direction);
        spotlightProgram.setUniform("Point.Position", pointLight.position);
        spotlightProgram.setUniform("Point.La", pointLight.ambient);
        spotlightProgram.setUniform("Point.L", pointLight.diffSpec);
        spotlightProgram.setUniform("Material.Kd", vec3(0.5f));
        spotlightProgram.setUniform("Material.Ks", vec3(0.5f));
        spotlightProgram.setUniform("Material.Ka", vec3(0.5f, 0.5f, 0.65f));
        spotlightProgram.setUniform("Material.Shininess", 1024.0f);
        bindTex(GL_TEXTURE0, rockTex);
        for (unsigned int i = 0; i < meteorPositions.size(); i++)
        {
            model = mat4(1.0f);
            model = glm::translate(model, meteorPositions[i]);
            model = glm::rotate(model, glm::radians(meteorRotations[i]), vec3(0.0f, 1.0f, 0.0f));
            setMatrices(spotlightProgram);
            meteor->render();
        }

        // Teapot
        spotlightProgram.setUniform("Light.L", pointLight.diffSpec);
        spotlightProgram.setUniform("Light.La", pointLight.ambient);
        spotlightProgram.setUniform("Light.Position", pointLight.position);
        spotlightProgram.setUniform("Material.Kd", 1.0f, 1.0f, 1.0f);
        spotlightProgram.setUniform("Material.Ks", 1.0f, 1.0f, 1.0f);
        spotlightProgram.setUniform("Material.Ka", 1.0f, 1.0f, 1.0f);
        spotlightProgram.setUniform("Material.Shininess", 100.0f);
        model = mat4(1.0f);
        model = glm::translate(model, vec3(0.0f, -11.25f, 0.0f));
        model = glm::rotate(model, glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
        setMatrices(spotlightProgram);
        teapot.render();

        // Spotlight Indicator
        basicProgram.use();
        basicProgram.setUniform("Colour", vec3(1.0f, 0.0f, 0.0f));
        model = mat4(1.0f);
        model = glm::translate(model, vec3(spotLight.position.x, spotLight.position.y, spotLight.position.z));
        setMatrices(basicProgram);
        cube.render();

        // Pointlight Indicator
        model = mat4(1.0f);
        model = glm::translate(model, vec3(pointLight.position.x, pointLight.position.y, pointLight.position.z));
        setMatrices(basicProgram);
        cube.render();

    }
    if (isSilhouetteShading) 
    {
        // UFO
        silhouetteProgram.use();
        silhouetteProgram.setUniform("EdgeWidth", 0.008f);
        silhouetteProgram.setUniform("PctExtend", 0.25f);
        silhouetteProgram.setUniform("LineColor", vec4(0.0f, 0.0f, 0.0f, 1.0f));
        silhouetteProgram.setUniform("Material.Kd", 0.1f, 0.25f, 0.7f);
        silhouetteProgram.setUniform("Material.Ka", 0.2f, 0.2f, 0.2f);
        silhouetteProgram.setUniform("Light.Position", pointLight.position);
        silhouetteProgram.setUniform("Light.Intensity", pointLight.diffSpec);
        model = mat4(1.0f);       
        model = glm::translate(model, ufoPosition);
        model = glm::rotate(model, glm::radians(-90.0f), vec3(0.0f, 1.0f, 0.0f));
        setMatrices(silhouetteProgram);
        ufo->render();

        // Meteors
        silhouetteProgram.setUniform("EdgeWidth", 0.001f);
        silhouetteProgram.setUniform("PctExtend", 0.25f);
        silhouetteProgram.setUniform("LineColor", vec4(0.0f, 0.0f, 0.0f, 1.0f));
        silhouetteProgram.setUniform("Material.Kd", 0.1f, 0.25f, 0.7f);
        silhouetteProgram.setUniform("Material.Ka", 0.2f, 0.2f, 0.2f);
        silhouetteProgram.setUniform("Light.Position", pointLight.position);
        silhouetteProgram.setUniform("Light.Intensity", pointLight.diffSpec);
        for (unsigned int i = 0; i < meteorPositions.size(); i++)
        {
            model = mat4(1.0f);
            model = glm::translate(model, meteorPositions[i]);
            model = glm::rotate(model, glm::radians(meteorRotations[i]), vec3(0.0f, 1.0f, 0.0f));
            setMatrices(silhouetteProgram);
            meteor->render();
        }
    }

    if (isGaussianBlur) 
    {
        gauss.pass1(view, model, projection, camera, width, height, pointLight, spotLight, skyboxProgram, sky, ufo, meteor, teapot, ufoPosition, ufoDiffuseTex, ufoNormalTex, rockTex, meteorPositions, meteorRotations);
        gauss.pass2(view, model, projection);
        gauss.pass3(view, model, projection);
    }
    if (isNightVision) 
    {
        nightVision.pass1(view, model, projection, camera, width, height, pointLight, spotLight, ufoPosition, ufo, meteor, teapot, meteorPositions, meteorRotations);
        glFlush();
        nightVision.pass2(view, model, projection);
    }
}

void SceneBasic_Uniform::setMatrices(GLSLProgram& prog)
{
    mat4 mv = view * model;
    prog.setUniform("MVP", projection * mv);
    prog.setUniform("ModelViewMatrix", mv);
    prog.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
}

void SceneBasic_Uniform::resize(int w, int h)
{
    glViewport(0, 0, w, h);
    width = w;
    height = h;
    projection = glm::perspective(glm::radians(80.0f), (float)w / h, 0.3f, 1000.0f);
}

void SceneBasic_Uniform::bindTex(GLuint unit, GLuint texture)
{
    glActiveTexture(unit);
    glBindTexture(GL_TEXTURE_2D, texture);
}

void SceneBasic_Uniform::setNormalShading()
{
    if (isGaussianBlur || isNightVision)
    {
        projection = glm::perspective(glm::radians(80.0f), (float)width / height, 0.3f, 1000.0f);
    }

    isNormalShading = true;
    isSilhouetteShading = false;
    isGaussianBlur = false;
    isNightVision = false;
}

void SceneBasic_Uniform::setSilhouetteShading()
{
    if (isGaussianBlur || isNightVision)
    {
        projection = glm::perspective(glm::radians(80.0f), (float)width / height, 0.3f, 1000.0f);
    }

    isNormalShading = false;
    isSilhouetteShading = true;
    isGaussianBlur = false;
    isNightVision = false;
}

void SceneBasic_Uniform::setGaussianShading()
{
    if (isGaussianBlur || isNightVision)
    {
        projection = glm::perspective(glm::radians(80.0f), (float)width / height, 0.3f, 1000.0f);
    }

    isNormalShading = false;
    isSilhouetteShading = false;
    isGaussianBlur = true;
    isNightVision = false;
}

void SceneBasic_Uniform::setNightVisionShading()
{
    if (isGaussianBlur || isNightVision)
    {
        projection = glm::perspective(glm::radians(80.0f), (float)width / height, 0.3f, 1000.0f);
    }

    isNormalShading = false;
    isSilhouetteShading = false;
    isGaussianBlur = false;
    isNightVision = true;
}

void SceneBasic_Uniform::setUfoPosition(float newX, float newY, float newZ)
{
    ufoPosition = vec3(newX, newY, newZ);
}

void SceneBasic_Uniform::setSpotPosition(float newX, float newY, float newZ)
{
    spotLight.position = vec4(vec3(newX, newY, newZ), 1.0f);
}

void SceneBasic_Uniform::setPointPosition(float newX, float newY, float newZ) 
{
    pointLight.position = vec4(vec3(newX, newY, newZ), 1.0f);
}