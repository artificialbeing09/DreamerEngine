// main.cpp

#include "Studio/Studio.h"

using namespace std;

int main()
{
    Studio::ConfigurationWindow();

    Graphics::Initialize();

    Scheduler::Start();

    // TODOs starting from top to bottom of immediate importance.

    // Bug/performance

    // TODO: Fix performance issues with ray casting
    // TODO: Add a system that stores objects permanently on the GPU and only change when object is changed (save memory transfer slowness)
    //       -  render re-ordering be completely on the GPU because of the way objects are stored currently (compute shader)
    //       -  a system to detect changes in the object primitive (add a call back maybe)
    //       -  have the object data buffer be seperated through the objects with no changes in a set amount of time vs with
    //       -  have it be specifically and non-memory efficiently garbage collected so that cpu/gpu time is the least with it
    
    // Features

    // TODO: Test CFrames
    // TODO: Add selectionboxes and particles
    // TODO: Add move, rotate, scale, part insert plugins
    // TODO: Voxel terrain
    // TODO: Working (optimized) physics
    // TODO: GUI on a surface or mid air facing camera
    // TODO: http/websocket library
    
    // After-works

    // TODO: Make a simple game to demonstrate the capabilities.

    // Future

    // TODO: Add rigs/character bodies
    // TODO: Re-structure the codebase so that it's easier to add features/scalable and more readable
    //       (mainly for graphics and non-instance stuff, instances are good)

    Graphics::ImportEngineObjects();

    Services::CreateServices();

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

        Gl.PreRender();

        if (Studio::Enabled) {
            Studio::RefreshFrame();
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