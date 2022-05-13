#include "gui.h"

void GUIWindow::drawElements(Scene& scene)
{
    //ImGui::ShowDemoWindow();

    ImGuiWindowFlags windowFlags = 0;
    windowFlags |= ImGuiWindowFlags_NoTitleBar;
    windowFlags |= ImGuiWindowFlags_NoBackground;
    windowFlags |= ImGuiWindowFlags_NoResize;

    ImGui::Begin("Debug", NULL, windowFlags);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    if (ImGui::Button("Normal Shading")) 
    {
        scene.setNormalShading();
    }
    if (ImGui::Button("Silhouette Shading")) 
    {
        scene.setSilhouetteShading();
    }
    if (ImGui::Button("Gaussian Blur"))
    {
        scene.setGaussianShading();
    }
    if (ImGui::Button("Night Vision"))
    {
        scene.setNightVisionShading();
    }
    ImGui::DragFloat3(" - UFO Position", ufoPos, 0.25f, -200.0f, 200.0f);
    ImGui::DragFloat3(" - Spot Light Position", spotPos, 0.25f, -200.0f, 200.0f);
    ImGui::DragFloat3(" - Point Light Position", pointPos, 0.25f, -250.0f, 250.0f);
    ImGui::Text("Press Left Shift to toggle to debug menu");
    ImGui::SetWindowPos(ImVec2(0, 0));
    ImGui::SetWindowSize(ImVec2(500, 260));
    ImGui::End();
}

GUIWindow::GUIWindow() 
{
    ufoPos[0] = 0.0f;
    ufoPos[1] = 20.0f;
    ufoPos[2] = 0.0f;
    ufoPos[3] = 0.0f;

    spotPos[0] = 0.0f;
    spotPos[1] = 20.0f;
    spotPos[2] = 0.0f;
    spotPos[3] = 0.0f;

    pointPos[0] = 45.0f;
    pointPos[1] = -25.0f;
    pointPos[2] = 125.0f;
    pointPos[3] = 0.0f;
}

void GUIWindow::init(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");
}

void GUIWindow::perFrame(Scene& scene)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    drawElements(scene);
    scene.setUfoPosition(ufoPos[0], ufoPos[1], ufoPos[2]);
    scene.setSpotPosition(spotPos[0], spotPos[1], spotPos[2]);
    scene.setPointPosition(pointPos[0], pointPos[1], pointPos[2]);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void GUIWindow::cleanUp()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}