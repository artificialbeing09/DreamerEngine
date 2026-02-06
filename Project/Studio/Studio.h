#pragma once

#include "Testing/Test.h"
#include "Graphics/Graphics.h"

#define GL_SILENCE_DEPRECATION
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "TextEditor.h"

using namespace std;

namespace Studio {
    int LoadedItem = 0;
    int TotalItems = 5;

    bool Enabled = false;
    bool BarInfoEnabled = false;

    GLFWwindow* window = NULL;
    float main_scale = 1.0f;
    const char* glsl_version = "#version 130";

    map<void*, bool> ExpandedChildrenLists = {}; 
    shared_ptr<Instance> SelectedObject = NULL;
    // Note to self: use deque whenever possible.

    void RenderDescendants(shared_ptr<Instance> Start, int LeftOffset);

    void RenderDescendants(shared_ptr<Instance> Start, int LeftOffset) {
        auto Children = Start->GetChildren();

        for (auto Inst : Children) {
            bool Set = ExpandedChildrenLists[Inst.get()];
            
            ImGui::SetCursorPosX(LeftOffset * 8);

            if (ImGui::ArrowButton(to_string((long long)Inst.get()).c_str(), Set ? ImGuiDir_Left : ImGuiDir_Right))
                ExpandedChildrenLists[Inst.get()] = !Set;

            ImGui::SameLine();

            bool Pushed = false;

            if (Inst.get() == SelectedObject.get()) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.5f, 0.0f, 1.0f));
                Pushed = true;
            }

            if (ImGui::Button((Inst->GetName()).c_str()))
                if (Inst.get() == SelectedObject.get())
                    SelectedObject = NULL;
                else
                    SelectedObject = Inst;

            if (Pushed) {
                ImGui::PopStyleColor(1);
            }

            if (ExpandedChildrenLists[Inst.get()] == true)
                RenderDescendants(Inst, LeftOffset + 1);
        }
    }

    void TaskbarInfoRender() {
        string Info = "[";

        Info += to_string(0) + " ms]";

        glfwSetWindowTitle(Gl.window, Info.c_str());
    }

    TextEditor editor;

    void InitGUIElements() {
        auto lang = TextEditor::LanguageDefinition::Lua();
        editor.SetLanguageDefinition(lang);
    }

    void GUIRender() {
        if (BarInfoEnabled) {
            TaskbarInfoRender();
        }

        if (!Enabled)
            return;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        {
            ImGui::Begin("Explorer"); 

            RenderDescendants(GetGameWorld(), 0);

            ImGui::End();

            ImGui::Begin("Properties"); 

            if (SelectedObject != NULL) {
                for (auto i : ClassPropertyDescriptorList[SelectedObject->GetType()]) {
                    ImGui::Text(i.first.c_str());

                    ImGui::SameLine();

                    auto PropertyInfo = i.second;

                    if (PropertyInfo->Type == L_String) {
                        string* CurrentText = (string*)i.second->GetFunction(SelectedObject);

                        ImGui::Text(CurrentText->c_str());
                    }
                    else if (PropertyInfo->Type == L_Object) {
                        shared_ptr<Instance>* CurrentText = (shared_ptr<Instance>*)i.second->GetFunction(SelectedObject);

                        ImGui::Text((*CurrentText)->GetName().c_str());
                    }
                    else if (PropertyInfo->Type == L_Vector) {
                        LuaVector* CurrentText = (LuaVector*)i.second->GetFunction(SelectedObject);

                        string VectorString = "";

                        VectorString += to_string(CurrentText->x) + ", ";
                        VectorString += to_string(CurrentText->y) + ", ";
                        VectorString += to_string(CurrentText->z) + ", ";
                        VectorString += to_string(CurrentText->a) + "";

                        ImGui::Text(VectorString.c_str());
                    }
                    else if (PropertyInfo->Type == L_Int) {
                        int64_t* CurrentText = (int64_t*)i.second->GetFunction(SelectedObject);

                        ImGui::Text(to_string(*CurrentText).c_str());
                    }
                    else if (PropertyInfo->Type == L_Number) {
                        double* CurrentText = (double*)i.second->GetFunction(SelectedObject);

                        ImGui::Text(to_string(*CurrentText).c_str());
                    }
                }
            }
            
            ImGui::End();

            ImGui::Begin("IDE");

            editor.Render("Script");

            ImGui::Button("Save");

            ImGui::End();

            ImGui::Begin("Controls");

            if (ImGui::Button("Run")) {

            }

            if (ImGui::Button("Stop")) {

            }

            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    int InitGUIWindow(GLFWwindow* customWindow = NULL) {
        if (!Enabled)
            return 0;

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

        ImGui::StyleColorsDark();
        
        ImGuiStyle& style = ImGui::GetStyle();
        style.ScaleAllSizes(main_scale);
        style.FontScaleDpi = main_scale;

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);

        InitGUIElements();
        
        return 0;
    }

    void ConfigurationWindow() {
        struct EngineConfiguration_t {
            Texture::TextureMemoryUseMode TextureMode = Texture::TextureMemoryUseMode::Low;
            int ShadowResolution = 10;
            bool EnableStudio = false;
            bool EnableConsole = false;
            bool YieldOnRequires = false;
            bool TaskbarInfo = false;
            char Zero = 0;
        };

        EngineConfiguration_t EngineConfiguration;

        FILE* ConfigFile = fopen("concisesettings.ini", "rb+");

        fread(&EngineConfiguration, sizeof(EngineConfiguration), 1, ConfigFile);

        fclose(ConfigFile);

        if (EngineConfiguration.ShadowResolution == 0) {
            EngineConfiguration_t New;

            EngineConfiguration = New;
        }

        if (!glfwInit())
            return;

        float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
        GLFWwindow* window = glfwCreateWindow((int)(600 * main_scale), (int)(600 * main_scale), "Concise Pre-configuration", nullptr, nullptr);
        if (window == nullptr)
            return;
        glfwMakeContextCurrent(window);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        style.ScaleAllSizes(main_scale);
        style.FontScaleDpi = main_scale;

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 130");

        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        vector<const char*> PerformanceModes = {
            "Highest",
            "UltraHigh",
            "High",
            "Medium",
            "Low",
            "UltraLow",
            "UltraUltraLow",
            "Windows95",
            "Headless"
        };

        while (!glfwWindowShouldClose(window))
        {
            this_thread::sleep_for(chrono::milliseconds(1000 / 30));

            glfwPollEvents();
            if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
            {
                ImGui_ImplGlfw_Sleep(10);
                continue;
            }

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
            ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

            {
                static float f = 0.0f;
                static int counter = 0;

                ImGui::Begin("Hello, world!", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar);

                ImGui::Checkbox("Studio", &EngineConfiguration.EnableStudio);
                ImGui::Checkbox("Console", &EngineConfiguration.EnableConsole);
                ImGui::Checkbox("Yield on requires", &EngineConfiguration.YieldOnRequires);
                ImGui::Checkbox("Debug info in title", &EngineConfiguration.TaskbarInfo);

                ImGui::SliderInt("Shadow Resolution", &EngineConfiguration.ShadowResolution, 1, 20, "%d00", ImGuiSliderFlags_AlwaysClamp);

                ImGui::BeginListBox("Performance Mode");

                for (int i = 0; i < PerformanceModes.size(); i++) {
                    const bool isSelected = ((Texture::TextureMemoryUseMode)i == EngineConfiguration.TextureMode);

                    if (ImGui::Selectable(PerformanceModes[i], isSelected)) { EngineConfiguration.TextureMode = (Texture::TextureMemoryUseMode)i; }

                    if (isSelected) { ImGui::SetItemDefaultFocus(); }
                }

                ImGui::EndListBox();

                if (ImGui::Button("Play"))
                    glfwSetWindowShouldClose(window, true);

                ImGui::SameLine();

                if (ImGui::Button("Close"))
                    exit(0);

                ImGui::End();
            }

            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
        }

        glfwDestroyWindow(window);
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();

        Texture::PerformanceMode = EngineConfiguration.TextureMode;
        Graphics::Engine3D::SHADOW_SIZE = EngineConfiguration.ShadowResolution * 100;
        Studio::Enabled = EngineConfiguration.EnableStudio;
        Studio::BarInfoEnabled = EngineConfiguration.TaskbarInfo;
        Scheduler::YieldOnRequires = EngineConfiguration.YieldOnRequires;

        if (!EngineConfiguration.EnableConsole) {
#ifdef WIN32
            FreeConsole();
#endif
        }

        ConfigFile = fopen("concisesettings.ini", "wb");

        fwrite(&EngineConfiguration, sizeof(EngineConfiguration), 1, ConfigFile);

        fclose(ConfigFile);

        return;
    }
}