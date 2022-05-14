#include "nightvision.h"

#include <sstream>
#include <iostream>
using std::cerr;
using std::endl;

#include <glm/gtc/matrix_transform.hpp>
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

void NightVision::init(int width, int height)
{
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

    glEnable(GL_DEPTH_TEST);

    setupFBO(width, height);

    GLfloat verts[] = {
        -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f
    };
    GLfloat tc[] = {
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
    };

    unsigned int handle[2];
    glGenBuffers(2, handle);

    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), verts, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), tc, GL_STATIC_DRAW);

    glGenVertexArrays(1, &fsQuad);
    glBindVertexArray(fsQuad);

    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte*)NULL + (0)));
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, ((GLubyte*)NULL + (0)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    program.use();
    GLuint programHandle = program.getHandle();
    pass1Index = glGetSubroutineIndex(programHandle, GL_FRAGMENT_SHADER, "pass1");
    pass2Index = glGetSubroutineIndex(programHandle, GL_FRAGMENT_SHADER, "pass2");

    program.setUniform("Width", width);
    program.setUniform("Height", height);
    program.setUniform("Radius", width / 3.5f);
    program.setUniform("Light.Intensity", vec3(1.0f, 1.0f, 1.0f));

    noiseTex = NoiseTex::generatePeriodic2DTex(200.0f, 0.5f, 512, 512);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, noiseTex);

    program.setUniform("RenderTex", 0);
    program.setUniform("NoiseTex", 1);
}


void NightVision::setupFBO(int width, int height)
{
    glGenFramebuffers(1, &renderFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);

    glGenTextures(1, &renderTex);
    glBindTexture(GL_TEXTURE_2D, renderTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTex, 0);

    GLuint depthBuf;
    glGenRenderbuffers(1, &depthBuf);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);

    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void NightVision::pass1
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
    std::unique_ptr<ObjMesh>& ufo,
    std::unique_ptr<ObjMesh>& meteor,
    Teapot& teapot,
    std::vector<vec3> meteorPositions,
    std::vector<float> meteorRotations
)
{
    program.use();

    glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &pass1Index);

    view = camera.ViewLookAt(view);
    projection = glm::perspective(glm::radians(80.0f), (float)width / height, 0.3f, 1000.0f);

    // UFO
    program.setUniform("Light.Position", pointLight.position);
    program.setUniform("Material.Kd", 0.9f, 0.9f, 0.9f);
    program.setUniform("Material.Ks", 0.95f, 0.95f, 0.95f);
    program.setUniform("Material.Ka", 0.1f, 0.1f, 0.1f);
    program.setUniform("Material.Shininess", 100.0f);
    model = mat4(1.0f);
    model = glm::rotate(model, glm::radians(-90.0f), vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, ufoPosition);
    setMatrices(program, view, model, projection);
    ufo->render();

    // Meteors
    program.setUniform("Light.Position", pointLight.position);
    program.setUniform("Material.Kd", 0.9f, 0.9f, 0.9f);
    program.setUniform("Material.Ks", 0.95f, 0.95f, 0.95f);
    program.setUniform("Material.Ka", 0.1f, 0.1f, 0.1f);
    program.setUniform("Material.Shininess", 100.0f);
    for (unsigned int i = 0; i < meteorPositions.size(); i++)
    {
        model = mat4(1.0f);
        model = glm::rotate(model, glm::radians(meteorRotations[i]), vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, meteorPositions[i]);
        setMatrices(program, view, model, projection);
        meteor->render();
    }

    // Teapot
    program.setUniform("Light.Position", pointLight.position);
    program.setUniform("Material.Kd", 0.9f, 0.9f, 0.9f);
    program.setUniform("Material.Ks", 0.95f, 0.95f, 0.95f);
    program.setUniform("Material.Ka", 0.1f, 0.1f, 0.1f);
    program.setUniform("Material.Shininess", 100.0f);
    model = mat4(1.0f);
    model = glm::translate(model, vec3(0.0f, -11.25f, 0.0f));
    model = glm::rotate(model, glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
    setMatrices(program, view, model, projection);
    teapot.render();
}

void NightVision::pass2(mat4& view, mat4& model, mat4& projection)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, noiseTex);
    glDisable(GL_DEPTH_TEST);

    glClear(GL_COLOR_BUFFER_BIT);

    program.use();
    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &pass2Index);

    model = mat4(1.0f);
    view = mat4(1.0f);
    projection = mat4(1.0f);
    setMatrices(program, view, model, projection);

    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void NightVision::setMatrices(GLSLProgram& prog, mat4 view, mat4 model, mat4 projection)
{
    mat4 mv = view * model;
    prog.setUniform("MVP", projection * mv);
    prog.setUniform("ModelViewMatrix", mv);
    prog.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
}