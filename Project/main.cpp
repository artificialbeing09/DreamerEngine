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

    // None
    
    // Minor bugs
    // 
    // TODO: Weird studio ui locations every time it's opened
    // TODO: Stop key inputs from being registered by lua scripts when the ImGUI interface is being interacted with
    // TODO: Studio GUI objects not having the correct mouse cursor position
    // TODO: Skybox orientation may be incorrect
    
    // Features

    // TODO: Fix mouse position in studio
    // TODO: Fix performance issues with ray casting
    //       - make BVH table
    // TODO: Add move, rotate, scale, part insert plugins
    // TODO: Actually improve lights
    //       -  spot light is a circle, add point lights and diff shaped lights
    // TODO: Add actual selection box
    // TODO: Working (optimized) physics (fuck nah)
    // TODO: GUI on a surface
    //       -  options for facing certain directions (at camera, etc) (like billboard gui)
    // TODO: Voxel terrain
    //       -  add smooth terrain and minecraft-like terrain with blocks
    //       -  ofc physics interacts with it
    // TODO: http/websocket library
    //       -  add http requests first
    //       -  then sockets
    //       -  then https (god no)
    // TODO: Add rigs/character bodies and animations
    // TODO: Softbody physics (hell nah)
    // TODO: Re-structure the codebase so that it's easier to add features/scalable and more readable
    //       (mainly for graphics and non-instance stuff, instances are good)
    // TODO: Add sandboxed Lua instancing (already kind of exists on its own but, just make it standardized ig)
    
    // After-works

    // TODO: Make a simple game to demonstrate the capabilities.
    // TODO: Add documentation

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
            Physics::SimulateCubes();
        }


        InputFrameFunction(InputService.get());
        PreTextObjectFunction();

		for (auto o : SceneUI->GetDescendants()) {
			if (o->GetType() == "TextObject") {
				TextObjectFrameFunction(o.get());
			}
		}

        UpdateParticles();

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