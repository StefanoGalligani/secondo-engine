#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <wtypes.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include <glm/glm/gtx/vector_angle.hpp>
#include <sgObject3D.h>
#include <sgCamera3D.h>
#include <sgSpotLight3D.h>
#include <sgUtils.h>
#include <sgRenderer.h>

#include <iostream>
#include <chrono>
#include <thread>

sg::Renderer renderer = sg::Renderer();
sg::Object3D planeObj = sg::Object3D();
sg::Object3D lightObj = sg::Object3D();
sg::Camera3D mainCamera = sg::Camera3D();
sg::SpotLight3D spotLight = sg::SpotLight3D();

bool showTriangulation = false;
bool pressedCTRL = false;
int pressedL = 0;
int pressedR = 0;
double prevx = 0;
double prevy = 0;

float resx = 1080;
float resy = 720;
float shadowResx = 2048;
float shadowResy = 2048;

void windowResizeListener(GLFWwindow* window, int x, int y) {
    resx = x;
    resy = y;
    glViewport(0, 0, resx, resy);
}

void keyListener(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == 341) {
        pressedCTRL = (action == 1);
    }
    if (key == 32 && action == 1) {
        showTriangulation = !showTriangulation;
        renderer.SetShowTriangulation(showTriangulation);
    }
    if (key == 262 && action > 0) {
        renderer.IncreaseTessellationLevel();
    }
    if (key == 263 && action > 0) {
        renderer.DecreaseTessellationLevel();
    }
    if (key == 256) {
        glfwDestroyWindow(window);
    }
    std::cout << "Key: " << key << std::endl;
}

void mouseButtonListener(GLFWwindow* window, int button, int action, int mods) {
    if (button == 0) {
        pressedL = action;
    }
    if (button == 1) {
        pressedR = action;
    }
}

void mouseDragListener(GLFWwindow* window, double xpos, double ypos) {
    if (pressedCTRL) {
        if (pressedL) {
            double movex = (xpos - prevx) / resx;
            double movey = -(ypos - prevy) / resy;
            //lightObj.RotateAround(glm::vec3(0, 1, 0), glm::vec3(0, 0, 0), movex);
            lightObj.RotateAround(-lightObj.Right(), glm::vec3(0, 0, 0), movey);
            spotLight.SetLocalPosition(lightObj.GetLocalPosition());
            spotLight.LookAt(glm::vec3(0, 0, 0));
        }
        if (pressedR) {
            float movey = (ypos - prevy) / resy;
            lightObj.Translate(lightObj.GetLocalPosition() * movey);
            spotLight.SetLocalPosition(lightObj.GetLocalPosition());
            spotLight.LookAt(glm::vec3(0, 0, 0));
        }
    }
    else {
        if (pressedL) {
            double movex = -(xpos - prevx) / resx;
            double movey = (ypos - prevy) / resy;
            mainCamera.RotateAround(glm::vec3(0, 1, 0), glm::vec3(0, 0, 0), movex);
            mainCamera.RotateAround(-mainCamera.Right(), glm::vec3(0, 0, 0), movey);
            mainCamera.LookAt(glm::vec3(0, 0, 0));
        }
        if (pressedR) {
            float movey = (ypos - prevy) / resy;
            mainCamera.Translate(mainCamera.GetLocalPosition() * movey);
            mainCamera.LookAt(glm::vec3(0, 0, 0));
        }
    }
    prevx = xpos;
    prevy = ypos;
}

void InitObjects() {
    printf("Initializing buffers\n");

    sg::Vertex planeVertices[4] = {
        sg::Vertex{glm::vec3(50, 0, -50), glm::vec2(1, 0), glm::vec3(0,1,0)},
        sg::Vertex{glm::vec3(-50, 0, -50), glm::vec2(0, 0), glm::vec3(0,1,0)},
        sg::Vertex{glm::vec3(-50, 0, 50), glm::vec2(0, 1), glm::vec3(0,1,0)},
        sg::Vertex{glm::vec3(50, 0, 50), glm::vec2(1, 1), glm::vec3(0,1,0)}
    };
    sg::Triangle planeTriangles[2] = {
        sg::Triangle{{0,3,1}},
        sg::Triangle{{1,3,2}}
    };
    sg::Material planeMaterials[1] = {
        sg::Material(new char[2] {'m','\0'}, new float[3] {0.4, 0.4, 0.4}, new float[3] {0.4, 0.4, 0.4}, new float[3] {0.6, 0.6, 0.6}, 20, 1)
    };
    sg::Mesh planeMeshes[1] = {
        sg::Mesh(new char[2] {'n','\0'}, planeMaterials[0].name, planeTriangles, 2)
    };
    planeObj.LoadModelFromData(planeVertices, 4, planeMaterials, 1, planeMeshes, 1);
    planeObj.CastsShadows = true;
    planeObj.ReceivesShadows = true;
    planeObj.Lit = true;
    planeObj.SetPatches(4);

    lightObj.LoadModelFromObj("res/light/light.obj");
    lightObj.SetLocalPosition(0, 30, 60);

    mainCamera.SetPerspective(9.0f / 4, (float)resx / resy, 0.5f, 50000.0f);
    mainCamera.SetLocalPosition(glm::vec3(0, 30, 55));
    mainCamera.LookAt(glm::vec3(0, 0, 0));

    spotLight.SetPerspective(1.0f, (float)shadowResx / shadowResy, 1.0f, 1000.0f);
    spotLight.SetLocalPosition(lightObj.GetLocalPosition());
    spotLight.LookAt(glm::vec3(0, 0, 0));

    renderer.AddObject(&planeObj);
    renderer.AddObject(&lightObj);
    renderer.SetMainCamera(&mainCamera);
    renderer.SetSpotLight(&spotLight);

    printf("Buffers ready\n");
}

int main(int argc, char* argv[]) {
    if (renderer.InitWindow("Quarto", resx, resy) < 0) return -1;
    renderer.InitPrograms();

    glfwSetKeyCallback(renderer.GetWindow(), keyListener);
    glfwSetMouseButtonCallback(renderer.GetWindow(), mouseButtonListener);
    glfwSetCursorPosCallback(renderer.GetWindow(), mouseDragListener);
    glfwSetWindowSizeCallback(renderer.GetWindow(), windowResizeListener);

    InitObjects();
    renderer.SetAmbientLight(0.2);
    renderer.SetNormalTexture((argc > 1) ? argv[1] : "res/teapot/teapot_normal.png");
    renderer.SetDisplacementTexture((argc > 1) ? argv[1] : "res/teapot/teapot_disp.png");
    renderer.SetupShadows(&spotLight, shadowResx, shadowResy);

    printf("Starting rendering\n");
    while (!glfwWindowShouldClose(renderer.GetWindow()))
    {
        renderer.RenderFrame();
        glfwPollEvents();
    }

    printf("Terminating");
    glfwTerminate();
    return 0;
}