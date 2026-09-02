#pragma once

#include "Configuration.h"
#include "Controls.h"
#include "Explorer.h"
#include "Files.h"
#include "IDE.h"
#include "Plugin.h"
#include "Properties.h"
#include "Scene.h"

using namespace std;

namespace Studio {
    GLFWwindow* window = NULL;
    float main_scale = 1.0f;
    const char* glsl_version = "#version 130";


    int Counter = 0;

    void GUIRender() {
        if (!Enabled)
            return;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        Counter++;


        ImGuiID dockspace_id = ImGui::GetID("My Dockspace");
        ImGuiViewport* viewport = ImGui::GetMainViewport();

        // Create settings
        if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr)
        {
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);
            ImGuiID dock_id_left = 0;
            ImGuiID dock_id_right = 0;
            ImGuiID dock_id_right_top = 0;
            ImGuiID dock_id_right_bottom = 0;
            ImGuiID dock_id_main = dockspace_id;
            ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Left, 0.20f, &dock_id_left, &dock_id_main);
            ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Right, 0.20f, &dock_id_right, &dock_id_main);
            ImGui::DockBuilderSplitNode(dock_id_right, ImGuiDir_Up, 0.50f, &dock_id_right_top, &dock_id_right_bottom);
            ImGuiID dock_id_main_2 = 0;
            ImGuiID dock_id_left_top = 0;
            ImGuiID dock_id_left_bottom = 0;
            ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Up, 0.50f, &dock_id_left_top, &dock_id_left_bottom);

            for (int I = 0; I < 20; I++) {
                ImGui::DockBuilderDockWindow(("IDE" + to_string(I)).c_str(), dock_id_main);
            }
            ImGui::DockBuilderDockWindow("Scene", dock_id_main);
            ImGui::DockBuilderDockWindow("Properties", dock_id_left_top);
            ImGui::DockBuilderDockWindow("Explorer", dock_id_left_bottom);
            ImGui::DockBuilderDockWindow("Controls", dock_id_right_top);
            ImGui::DockBuilderDockWindow("Files", dock_id_right_bottom);

            ImGui::DockBuilderFinish(dockspace_id);
        }

        ImGui::DockSpaceOverViewport(dockspace_id, viewport, ImGuiDockNodeFlags_PassthruCentralNode);
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

            ImGui::Begin("Scene"); RenderScene(); ImGui::End();

            ImGui::PopStyleVar();

            ImGui::Begin("Explorer"); RenderExplorer(); ImGui::End();

            ImGui::Begin("Properties"); RenderProperties(); ImGui::End();

            ImGui::Begin("Files"); RenderFiles(); ImGui::End();

            ImGui::Begin("Controls"); RenderControls(); ImGui::End();

            RenderIDE();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    int InitGUIWindow(GLFWwindow* customWindow = NULL) {
        if (!Enabled)
            return 0;

        InitializeSeperateFrameBuffer();

        main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
        window = customWindow;

        if (window == nullptr)
            return 1;

        glfwMakeContextCurrent(window);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        style.ScaleAllSizes(main_scale);
        style.FontScaleDpi = main_scale;

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);
        
        Studio::InitIDEAndFiles();
        Studio::InitProperties();

        return 0;
    }
}