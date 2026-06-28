// main.cpp

#include "Studio/Studio.h"

using namespace std;

int main()
{
    Studio::ConfigurationWindow();

    Graphics::Initialize();

    Scheduler::Start();


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

    while (!Gl.ShouldClose()) {
        i++;
        Utils::FrameRate::Cap();

        Graphics::Engine3D::Camera::CameraStep();

        if (Studio::Running) {
            Physics::SimulateCubes();
        }

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