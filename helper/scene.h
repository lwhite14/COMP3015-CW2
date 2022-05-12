#pragma once

#include <glm/glm.hpp>
#include "../customsrc/camera.h"

class Scene
{
protected:
	glm::mat4 model, view, projection;

public:
    int width;
    int height;
    Camera camera;

	Scene() : width(800), height(600), camera(width, height) { }
	virtual ~Scene() {}

	void setDimensions( int w, int h ) {
	    width = w;
	    height = h;
	}
	
    // Load textures, initialize shaders, etc.
    virtual void initScene() = 0;

    // This is called prior to every frame. Use this to update your animation.
    virtual void update(float t, GLFWwindow* window) = 0;

    // Draw your scene.
    virtual void render() = 0;

    // Called when screen is resized
    virtual void resize(int, int) = 0;

    // Called when wanting to switch to normal shading
    virtual void setNormalShading() = 0;

    // Called when wanting to switch to silhouette shading
    virtual void setSilhouetteShading() = 0;

    // Called when wanting to switch to a gaussian blur filter
    virtual void setGaussianShading() = 0;

    // Called when wanting to change the UFOs position.
    virtual void setUfoPosition(float newX, float newY, float newZ) = 0;

    virtual void setSpotPosition(float newX, float newY, float newZ) = 0; 

    virtual void setPointPosition(float newX, float newY, float newZ) = 0; 
};
