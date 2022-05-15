#include "gauss.h"

#include <sstream>
#include <iostream>
using std::cerr;
using std::endl;

#include <glm/gtc/matrix_transform.hpp>
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

#include "../helper/noisetex.h"
#include "camera.h"
#include "light.h"
#include "../helper/skybox.h"
#include "../helper/objmesh.h"
#include "../helper/teapot.h"

void Gauss::init(int width, int height)
{
    setupFBO(width, height);
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
        normalProgram.use();
        normalProgram.setUniform(uniName.str().c_str(), val);
        spotlightProgram.use();
        spotlightProgram.setUniform(uniName.str().c_str(), val);
    }
}

void Gauss::setupFBO(int width, int height)
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

void Gauss::pass1
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
)
{
    normalProgram.use();
    normalProgram.setUniform("Pass", 1);
    spotlightProgram.use();
    spotlightProgram.setUniform("Pass", 1);
    glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    view = camera.ViewLookAt(view);
    projection = glm::perspective(glm::radians(80.0f), (float)width / height, 0.3f, 1000.0f);

    // Skybox
    skyboxProgram.use();
    model = mat4(1.0f);
    setMatrices(skyboxProgram, view, model, projection);
    sky.render();

    // Render Objects
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
    setMatrices(normalProgram, view, model, projection);
    bindTex(GL_TEXTURE1, ufoDiffuseTex);
    bindTex(GL_TEXTURE2, ufoNormalTex);
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
    bindTex(GL_TEXTURE1, rockTex);
    for (unsigned int i = 0; i < meteorPositions.size(); i++)
    {
        model = mat4(1.0f);
        model = glm::translate(model, meteorPositions[i]);
        model = glm::rotate(model, glm::radians(meteorRotations[i]), vec3(0.0f, 1.0f, 0.0f));
        setMatrices(spotlightProgram, view, model, projection);
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
    setMatrices(spotlightProgram, view, model, projection);
    teapot.render();
}

void Gauss::pass2(mat4& view, mat4& model, mat4& projection)
{
    normalProgram.use();
    normalProgram.setUniform("Pass", 2);
    spotlightProgram.use();
    spotlightProgram.setUniform("Pass", 2);
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderTex);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    model = mat4(1.0f);
    view = mat4(1.0f);
    projection = mat4(1.0f);
    normalProgram.use();
    setMatrices(normalProgram, view, model, projection);
    spotlightProgram.use();
    setMatrices(spotlightProgram, view, model, projection);
    // Render the full-screen quad
    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Gauss::pass3(mat4& view, mat4& model, mat4& projection)
{
    normalProgram.use();
    normalProgram.setUniform("Pass", 3);
    spotlightProgram.use();
    spotlightProgram.setUniform("Pass", 3);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, intermediateTex);
    glClear(GL_COLOR_BUFFER_BIT);
    model = mat4(1.0f);
    view = mat4(1.0f);
    projection = mat4(1.0f);
    normalProgram.use();
    setMatrices(normalProgram, view, model, projection);
    spotlightProgram.use();
    setMatrices(spotlightProgram, view, model, projection);
    // Render the full-screen quad
    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

float Gauss::gauss(float x, float sigma2)
{
    double coeff = 1.0 / (glm::two_pi<double>() * sigma2);
    double expon = -(x * x) / (2.0 * sigma2);
    return (float)(coeff * exp(expon));
}

void Gauss::setMatrices(GLSLProgram& prog, mat4 view, mat4 model, mat4 projection)
{
    mat4 mv = view * model;
    prog.setUniform("MVP", projection * mv);
    prog.setUniform("ModelViewMatrix", mv);
    prog.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
}

void Gauss::bindTex(GLuint unit, GLuint texture)
{
    glActiveTexture(unit);
    glBindTexture(GL_TEXTURE_2D, texture);
}
