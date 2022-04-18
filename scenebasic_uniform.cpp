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


SceneBasic_Uniform::SceneBasic_Uniform() :  pointLight(Light(vec4(45.0f, -25.0f, 125.0f, 1.0),
                                                vec3(0.0f, 0.0f, 0.15f),
                                                vec3(1.0f))),
                                            spotLight(Light(vec4(0.0f, 30.0f, 0.0f, 1.0f),
                                                vec3(0.0f),
                                                vec3(0.2f, 0.95f, 0.2f),
                                                vec3(),
                                                40.0f,
                                                glm::radians(20.0f))),
                                            sky(500.0f),
                                            isBlur(false),
                                            firstBack(false),
                                            teapot(14, mat4(1.0f))
{
    ufo = ObjMesh::load("media/ufo.obj");
    meteor = ObjMesh::load("media/meteor.obj");
}


void SceneBasic_Uniform::initScene()
{
    meteorPositions = std::vector<vec3>
    {
       vec3(6.0f, -40.0f, 0.0f),
       vec3(-80.0f, -60.0f, -50.0f),
       vec3(-80.0f, -60.0f, -50.0f)
    };
    meteorRotations = std::vector<float>
    {
        0.0f,
        -0.5f,
        1.0f
    };

    compile();
    glEnable(GL_DEPTH_TEST);
    initGauss();

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

        normalGaussianProgram.compileShader("shader/normal_map.vert");
        normalGaussianProgram.compileShader("shader/normal_map_gaussian.frag");
        normalGaussianProgram.link();

        spotlightGaussianProgram.compileShader("shader/spotlight.vert");
        spotlightGaussianProgram.compileShader("shader/spotlight_gaussian.frag");
        spotlightGaussianProgram.link();
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

    int blurState = glfwGetKey(window, GLFW_KEY_SPACE);
    if (blurState == GLFW_PRESS)
    {
        isBlur = true;
        firstBack = true;
    }
    if (blurState == GLFW_RELEASE) 
    {
        isBlur = false;
        if (firstBack) 
        {
            firstBack = false;
            projection = glm::perspective(glm::radians(80.0f), (float)width / height, 0.3f, 1000.0f);
        }
    }
}

void SceneBasic_Uniform::render()
{
    if (!isBlur) 
    {
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        view = camera.ViewLookAt(view);

        // Skybox
        skyboxProgram.use();
        model = mat4(1.0f);
        setMatrices(skyboxProgram);
        sky.render();

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
        model = glm::rotate(model, 5.0f, vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, vec3(5.0f, 20.0f, 0.0f));
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
            model = glm::rotate(model, meteorRotations[i], vec3(0.0f, 1.0f, 0.0f));
            model = glm::translate(model, meteorPositions[i]);
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
    }
    else if (isBlur)
    {
        pass1();
        pass2();
        pass3();
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

void SceneBasic_Uniform::initGauss() 
{
    compile();
    setupFBO();
    // Array for full-screen quad
    GLfloat verts[] =
    {
        -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f
    };
    GLfloat tc[] =
    {
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
    };
    // Set up the buffers
    unsigned int handle[2];
    glGenBuffers(2, handle);
    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), tc, GL_STATIC_DRAW);
    // Set up the vertex array object
    glGenVertexArrays(1, &fsQuad);
    glBindVertexArray(fsQuad);
    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0); // Vertex position
    glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2); // Texture coordinates
    glBindVertexArray(0);

    float weights[5], sum, sigma2 = 8.0f;

    // Compute and sum the weights
    weights[0] = gauss(0, sigma2);
    sum = weights[0];
    for (int i = 1; i < 5; i++)
    {
        weights[i] = gauss(float(i), sigma2);
        sum += 2 * weights[i];
    }

    // Normalize the weights and set the uniform
    for (int i = 0; i < 5; i++)
    {
        std::stringstream uniName;
        uniName << "Weight[" << i << "]";
        float val = weights[i] / sum;
        normalGaussianProgram.use();
        normalGaussianProgram.setUniform(uniName.str().c_str(), val);
        spotlightGaussianProgram.use();
        spotlightGaussianProgram.setUniform(uniName.str().c_str(), val);
    }
}

void SceneBasic_Uniform::setupFBO()
{
    // Generate and bind the framebuffer
    glGenFramebuffers(1, &renderFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
    // Create the texture object
    glGenTextures(1, &renderTex);
    glBindTexture(GL_TEXTURE_2D, renderTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    // Bind the texture to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTex, 0);
    // Create the depth buffer
    GLuint depthBuf;
    glGenRenderbuffers(1, &depthBuf);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    // Bind the depth buffer to the FBO
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);
    // Set the targets for the fragment output variables
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);
    // Unbind the framebuffer, and revert to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // Generate and bind the framebuffer
    glGenFramebuffers(1, &intermediateFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
    // Create the texture object
    glGenTextures(1, &intermediateTex);
    glActiveTexture(GL_TEXTURE0); // Use texture unit 0
    glBindTexture(GL_TEXTURE_2D, intermediateTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    // Bind the texture to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, intermediateTex, 0);
    // No depth buffer needed for this FBO
    // Set the targets for the fragment output variables
    glDrawBuffers(1, drawBuffers);
    // Unbind the framebuffer, and revert to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneBasic_Uniform::pass1()
{
    normalGaussianProgram.use();
    normalGaussianProgram.setUniform("Pass", 1);
    spotlightGaussianProgram.use();
    spotlightGaussianProgram.setUniform("Pass", 1);
    glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    view = camera.ViewLookAt(view);
    projection = glm::perspective(glm::radians(80.0f), (float)width / height, 0.3f, 1000.0f);

    // Skybox
    skyboxProgram.use();
    model = mat4(1.0f);
    setMatrices(skyboxProgram);
    sky.render();

    // Render Objects
    // UFO
    normalGaussianProgram.use();
    normalGaussianProgram.setUniform("Light.Position", pointLight.position);
    normalGaussianProgram.setUniform("Light.La", pointLight.ambient);
    normalGaussianProgram.setUniform("Light.L", pointLight.diffSpec);
    normalGaussianProgram.setUniform("Material.Kd", vec3(0.5f));
    normalGaussianProgram.setUniform("Material.Ks", vec3(0.5f));
    normalGaussianProgram.setUniform("Material.Ka", vec3(0.25f, 0.25f, 1.0f));
    normalGaussianProgram.setUniform("Material.Shininess", 128.0f);
    model = mat4(1.0f);
    model = glm::rotate(model, 5.0f, vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, vec3(5.0f, 20.0f, 0.0f));
    setMatrices(normalGaussianProgram);
    bindTex(GL_TEXTURE1, ufoDiffuseTex);
    bindTex(GL_TEXTURE2, ufoNormalTex);
    ufo->render();

    // Meteors
    spotlightGaussianProgram.use();
    spotlightGaussianProgram.setUniform("Spot.L", spotLight.diffSpec);
    spotlightGaussianProgram.setUniform("Spot.La", spotLight.ambient);
    spotlightGaussianProgram.setUniform("Spot.Exponent", spotLight.exponent);
    spotlightGaussianProgram.setUniform("Spot.Cutoff", spotLight.cutoff);
    mat3 normalMatrix = mat3(vec3(view[0]), vec3(view[1]), vec3(view[2]));
    spotLight.direction = normalMatrix * vec3(-spotLight.position);
    spotlightGaussianProgram.setUniform("Spot.Position", view * spotLight.position);
    spotlightGaussianProgram.setUniform("Spot.Direction", spotLight.direction);
    spotlightGaussianProgram.setUniform("Point.Position", pointLight.position);
    spotlightGaussianProgram.setUniform("Point.La", pointLight.ambient);
    spotlightGaussianProgram.setUniform("Point.L", pointLight.diffSpec);
    spotlightGaussianProgram.setUniform("Material.Kd", vec3(0.5f));
    spotlightGaussianProgram.setUniform("Material.Ks", vec3(0.5f));
    spotlightGaussianProgram.setUniform("Material.Ka", vec3(0.5f, 0.5f, 0.65f));
    spotlightGaussianProgram.setUniform("Material.Shininess", 1024.0f);
    bindTex(GL_TEXTURE1, rockTex);
    for (unsigned int i = 0; i < meteorPositions.size(); i++)
    {
        model = mat4(1.0f);
        model = glm::rotate(model, meteorRotations[i], vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, meteorPositions[i]);
        setMatrices(spotlightGaussianProgram);
        meteor->render();
    }

    // Teapot
    spotlightGaussianProgram.setUniform("Light.L", pointLight.diffSpec);
    spotlightGaussianProgram.setUniform("Light.La", pointLight.ambient);
    spotlightGaussianProgram.setUniform("Light.Position", pointLight.position);
    spotlightGaussianProgram.setUniform("Material.Kd", 1.0f, 1.0f, 1.0f);
    spotlightGaussianProgram.setUniform("Material.Ks", 1.0f, 1.0f, 1.0f);
    spotlightGaussianProgram.setUniform("Material.Ka", 1.0f, 1.0f, 1.0f);
    spotlightGaussianProgram.setUniform("Material.Shininess", 100.0f);
    bindTex(GL_TEXTURE1, rockTex);
    model = mat4(1.0f);
    model = glm::translate(model, vec3(0.0f, -11.25f, 0.0f));
    model = glm::rotate(model, glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
    setMatrices(spotlightGaussianProgram);
    teapot.render();
}

void SceneBasic_Uniform::pass2()
{
    normalGaussianProgram.use();
    normalGaussianProgram.setUniform("Pass", 2);    
    spotlightGaussianProgram.use();
    spotlightGaussianProgram.setUniform("Pass", 2);
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderTex);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    model = mat4(1.0f);
    view = mat4(1.0f);
    projection = mat4(1.0f);
    normalGaussianProgram.use();
    setMatrices(normalGaussianProgram);
    spotlightGaussianProgram.use();
    setMatrices(spotlightGaussianProgram);
    // Render the full-screen quad
    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void SceneBasic_Uniform::pass3()
{
    normalGaussianProgram.use();
    normalGaussianProgram.setUniform("Pass", 3);
    spotlightGaussianProgram.use();
    spotlightGaussianProgram.setUniform("Pass", 3);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, intermediateTex);
    glClear(GL_COLOR_BUFFER_BIT);
    model = mat4(1.0f);
    view = mat4(1.0f);
    projection = mat4(1.0f);
    normalGaussianProgram.use();
    setMatrices(normalGaussianProgram);
    spotlightGaussianProgram.use();
    setMatrices(spotlightGaussianProgram);
    // Render the full-screen quad
    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

float SceneBasic_Uniform::gauss(float x, float sigma2)
{
    double coeff = 1.0 / (glm::two_pi<double>() * sigma2);
    double expon = -(x * x) / (2.0 * sigma2);
    return (float)(coeff * exp(expon));
}