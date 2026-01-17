// main.cpp

//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

#include "Studio/Studio.h"

using namespace std;

int main()
{
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