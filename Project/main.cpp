// main.cpp

#include "Studio/Studio.h"

using namespace std;

int main()
{
    Studio::ConfigurationWindow();

    Graphics::Initialize();

    Scheduler::Start();

    // TODO: Make it so that it's easy to add new meshes/fonts
    // 
    // TODO: Fix performance issues with ray casting
    //  - also add a system that brings objects to gpu without a need
    // TODO: Fix the skybox being upside down (and probably all textures on cubes being messed up)

    Graphics::Engine3D::CreateMeshVector("Teapot", ObjParser::DefaultParseObj(Utils::ReadFile("Engine/teapot.obj")));
    Graphics::Engine3D::CreateMeshVector("Arrow", ObjParser::DefaultParseObj(Utils::ReadFile("Engine/arrow.obj")));

    Texture::GenerateEngineTextures();

    Services::CreateServices();

    Graphics::Engine2D::CreateFont("Default", Utils::ReadFile("Engine/Default.ttf"));
	Graphics::Engine2D::CreateFont("HyperFont", Utils::ReadFile("Engine/ttf_HyperFont.ttf"));

    if (GetConsoleWindow()) {
        Test::Start();
    }

    if (!Studio::Enabled) {
        Studio::Run();
    }
    else {
        Plugin::LoadPlugins();
    }

    Studio::InitGUIWindow(Gl.window);

    int i = 0;

	int LastHeight = Gl.height;
	int LastWidth = Gl.width;

    auto Game = GetGameWorld(); // So that it's never dereferenced

	auto SceneUI = Services::GetService<UIScene>("UIScene");
    auto InputService = Services::GetService<Input>("Input");

    if (Studio::Enabled) {
        glGenFramebuffers(1, &Graphics::Engine3D::defaultFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, Graphics::Engine3D::defaultFBO);

        glGenTextures(1, &Graphics::Engine3D::FBOtextureMap);
        glBindTexture(GL_TEXTURE_2D, Graphics::Engine3D::FBOtextureMap);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Graphics::Engine3D::FBOtextureMap, 0);
    }

    while (!Gl.ShouldClose()) {
        i++;
        Utils::FrameRate::Cap();

        Gl.PreRender();

        if (Studio::Enabled) {
            Gl.w = Studio::SceneWindowSizeX;
            Gl.h = Studio::SceneWindowSizeY;

            glBindTexture(GL_TEXTURE_2D, Graphics::Engine3D::FBOtextureMap);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Gl.w, Gl.h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glViewport(0, 0, Gl.w, Gl.h);

            Gl.MouseX -= Studio::SceneCursorOffsetX;
            Gl.MouseY -= Studio::SceneCursorOffsetY;
        }
        else {
            Gl.w = Gl.width;
            Gl.h = Gl.height;
        }

        Gl.MouseY = Gl.h - Gl.MouseY;

        Graphics::Engine3D::Camera::CameraStep();

        if (Studio::Running) {
            //Physics::SimulateCubes();
        }

        //InputFrameFunction(InputService.get());
        PreTextObjectFunction();

		for (auto o : SceneUI->GetDescendants()) {
			if (o->GetType() == "TextObject") {
				TextObjectFrameFunction(o.get());
			}
		}

        Scheduler::SchedulerStep();
        
        Graphics::Engine3D::Render();

		if (LastHeight != Gl.h || LastWidth != Gl.w) {
			UpdateUINextFrame = true;
			UpdateAllText = true;

            LastHeight = Gl.h;
            LastWidth = Gl.w;
		}

		if (UpdateUINextFrame) {
			UpdateUI();

			UpdateUINextFrame = false;
		}

        Graphics::Engine2D::Render();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        Studio::GUIRender();
        
        Gl.PostRender();
    }

    Scheduler::Stop();
    glfwTerminate();
}