
#include "Testing/Test.h"
#include "Graphics/Graphics.h"

#define GL_SILENCE_DEPRECATION
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

using namespace std;

namespace Studio {
    bool Enabled = false;

    GLFWwindow* window = NULL;
    float main_scale = 1.0f;
    const char* glsl_version = "#version 130";

    map<void*, bool> ExpandedChildrenLists = {}; 
    void* SelectedObject = NULL;
    // Note to self: use deque whenever possible.

    void RenderDescendants(shared_ptr<Instance> Start, int LeftOffset);

    void RenderDescendants(shared_ptr<Instance> Start, int LeftOffset) {
        auto Children = Start->GetChildren();

        for (auto Inst : Children) {
            bool Set = ExpandedChildrenLists[Inst.get()];
            string ButtonText = ">";

            if (Set)
                ButtonText = "<";

            ImGui::SetCursorPosX(LeftOffset * 8);

            ButtonText += "##" + to_string((long long)Inst.get()); 
            if (ImGui::Button(ButtonText.c_str()))
                ExpandedChildrenLists[Inst.get()] = !Set;

            ImGui::SameLine();

            bool Pushed = false;

            if (Inst.get() == SelectedObject) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.5f, 0.0f, 1.0f));
                Pushed = true;
            }

            if (ImGui::Button((Inst->GetName()).c_str()))
                if (Inst.get() == SelectedObject)
                    SelectedObject = NULL;
                else
                    SelectedObject = Inst.get();

            if (Pushed) {
                ImGui::PopStyleColor(1);
            }

            if (ExpandedChildrenLists[Inst.get()] == true)
                RenderDescendants(Inst, LeftOffset + 1);
        }
    }

    void GUIRender() {
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
        
        return 0;
    }
}