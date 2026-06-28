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

    glGenFramebuffers(1, &Graphics::Engine3D::defaultFBO);

    glGenTextures(1, &Graphics::Engine3D::FBOtextureMap);
    glBindTexture(GL_TEXTURE_2D, Graphics::Engine3D::FBOtextureMap);

    glTexStorage2D(
        GL_TEXTURE_2D,
        1, // mip levels
        (GLenum)GL_RGBA,
        (GLsizei)1024,
        (GLsizei)1024
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenerateMipmap(GL_TEXTURE_2D);

    // Framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, Graphics::Engine3D::defaultFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, Graphics::Engine3D::FBOtextureMap, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

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

        glBindFramebuffer(GL_FRAMEBUFFER, Graphics::Engine3D::defaultFBO);
        
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

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        Studio::GUIRender();
        
        Gl.PostRender();
    }

    Scheduler::Stop();
    glfwTerminate();
}