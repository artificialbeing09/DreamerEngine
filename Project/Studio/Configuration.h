#pragma once

#define GL_SILENCE_DEPRECATION
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "Testing/Test.h"
#include "Graphics/Graphics.h"

#include "TextEditor.h"
#include "LoadAndSave/Serializer.h"

#include "Font/SourceSans.h"

#include "pfd.h"

using namespace std;

namespace Studio {
    bool Enabled = false;
    bool BarInfoEnabled = false;

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

        if (!ConfigFile) {
            cout << "Config file failed to open." << endl;
        }
        else {
            fread(&EngineConfiguration, sizeof(EngineConfiguration), 1, ConfigFile);

            fclose(ConfigFile);
        }



        if (EngineConfiguration.ShadowResolution == 0) {
            EngineConfiguration_t New;

            EngineConfiguration = New;
        }

        if (!glfwInit())
            return;

        float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
        GLFWwindow* window = glfwCreateWindow((int)(600 * main_scale), (int)(600 * main_scale), "Dreamer Engine Pre-configuration", nullptr, nullptr);
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

        constexpr float FontSize = 20.0f;

        ImFontConfig config;
        config.PixelSnapH = true;
        io.Fonts->AddFontFromMemoryCompressedTTF(source_sans_pro_regular_compressed_data, source_sans_pro_regular_compressed_size, FontSize, &config, io.Fonts->GetGlyphRangesDefault());
        config.MergeMode = true;

        io.Fonts->Build();

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