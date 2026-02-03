// main.cpp

#include "Studio/Studio.h"

using namespace std;

void ConfigurationWindow() {
    struct EngineConfiguration_t {
        Texture::TextureMemoryUseMode TextureMode = Texture::TextureMemoryUseMode::Low;
        int ShadowResolution = 10;
        bool EnableStudio = false;
        bool EnableConsole = false;
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

    if (EngineConfiguration.EnableConsole) {
        AllocConsole();
    }

    ConfigFile = fopen("concisesettings.ini", "wb");

    fwrite(&EngineConfiguration, sizeof(EngineConfiguration), 1, ConfigFile);

    fclose(ConfigFile);

    return;
}

int main()
{
    FreeConsole();

    ConfigurationWindow();

    auto StartTime = Utils::GetMilliseconds();

    Graphics::Initialize();

    cout << "Graphics initialized in " << Utils::GetMilliseconds() - StartTime << " ms" << endl;

    Scheduler::Start();

    cout << "Scheduler started in " << Utils::GetMilliseconds() - StartTime << " ms" << endl;

    Graphics::Engine3D::CreateMeshVector("Teapot", ObjParser::DefaultParseObj(Utils::ReadFile("Engine/teapot.obj")));

    cout << "Meshes initialized in " << Utils::GetMilliseconds() - StartTime << " ms" << endl;

    Texture::GenerateEngineTextures();

    cout << "Textures initialized in " << Utils::GetMilliseconds() - StartTime << " ms" << endl;

    Services::CreateServices();

    cout << "Services initialized in " << Utils::GetMilliseconds() - StartTime << " ms" << endl;

    Graphics::Engine2D::CreateFont("Default", Utils::ReadFile("Engine/Default.ttf"));
	Graphics::Engine2D::CreateFont("HyperFont", Utils::ReadFile("Engine/ttf_HyperFont.ttf"));

    cout << "Fonts initialized in " << Utils::GetMilliseconds() - StartTime << " ms" << endl;

    Test::Start();

    Studio::InitGUIWindow(Gl.window);

    int i = 0;

	int LastHeight = Gl.height;
	int LastWidth = Gl.width;

    auto Game = GetGameWorld(); // So that it's never dereferenced

	auto SceneUI = Services::GetService<UIScene>("UIScene");
    auto InputService = Services::GetService<Input>("Input");

    while (!Gl.ShouldClose()) {
        i++;
        Utils::FrameRate::Cap();

        Graphics::Engine3D::Camera::CameraStep();

        InputFrameFunction(InputService.get());
        PreTextObjectFunction();

		for (auto o : SceneUI->GetDescendants()) {
			if (o->GetType() == "TextObject") {
				TextObjectFrameFunction(o.get());
			}
		}

        Scheduler::SchedulerStep();
        
        Graphics::Engine3D::Render();

		if (LastHeight != Gl.height || LastWidth != Gl.width) {
			UpdateUINextFrame = true;
			UpdateAllText = true;

            LastHeight = Gl.height;
            LastWidth = Gl.width;
		}

		if (UpdateUINextFrame) {
			UpdateUI();

			UpdateUINextFrame = false;
		}

        Graphics::Engine2D::Render();

        Studio::GUIRender();
        
        Gl.PostRender();
    }

    Scheduler::Stop();
    glfwTerminate();
}