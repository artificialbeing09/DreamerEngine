#pragma once

#include "IDE.h"

using namespace std;

namespace Studio {
    bool Running = false;

    string SavedSerializedWorld = "";

    void Run() {
        PromptForSaveAll();

        SavedSerializedWorld = Serializer::Serialize(GetGameWorld());

        Scheduler::Event::FireListenerInstance(Services::GetService<Scene>("Scene").get(), "StudioGameModified");

        Scheduler::Lua::RunScript(LoadAndSave::GetScriptByModuleName("main.lua", 0), "main.lua", 0);
        Running = true;
    }

    void Stop() {
        PromptForSaveAll("Scripts have been edited during gameplay. This may have been unintended. Do you still want to save those changes?");

        Scheduler::ExitAllThreadsWithIdentity(0);
        Running = false;

        Serializer::DeserializeIntoObject(GetGameWorld(), SavedSerializedWorld, true);
    }

	void RenderControls() {
        if (!Running) {
            if (ImGui::Button("Run")) {
                Run();
            }
        }
        else {
            if (ImGui::Button("Stop")) {
                Stop();
            }
        }

        ImGui::Separator();

        if (ImGui::Button("Save")) {
            PromptForSaveAll();

            if (CurrentMapSelected.size() == 0) {
                auto f = pfd::save_file("Choose file to save",
                    filesystem::current_path().string() + "\\Game\\main.map",
                    { "Dreamer Engine Game Map File (.map)", "*.map" },
                    pfd::opt::force_overwrite);

                CurrentMapSelected = f.result();
            }

            Scheduler::Event::FireListenerInstance(Services::GetService<Scene>("Scene").get(), "StudioGameModified");

            string Serialized = Serializer::Serialize(GetGameWorld());

            Utils::WriteFile(CurrentMapSelected.c_str(), Serialized.c_str());
        }

        if (ImGui::Button("Save As...")) {
            PromptForSaveAll();

            auto f = pfd::save_file("Choose file to save",
                filesystem::current_path().string() + "\\Game\\main.map",
                { "Dreamer Engine Game Map File (.map)", "*.map" },
                pfd::opt::force_overwrite);

            CurrentMapSelected = f.result();

            Scheduler::Event::FireListenerInstance(Services::GetService<Scene>("Scene").get(), "StudioGameModified");

            string Serialized = Serializer::Serialize(GetGameWorld());

            Utils::WriteFile(CurrentMapSelected.c_str(), Serialized.c_str());
        }

        if (ImGui::Button("Open")) {
            PromptForSaveAll();

            auto f = pfd::open_file("Choose file to open",
                filesystem::current_path().string() + "\\Game",
                { "Dreamer Engine Game Map File (.map)", "*.map" },
                pfd::opt::none);

            if (f.result().size() > 0) {
                string Path = f.result()[0];
                string FoundFile = Utils::ReadFile(Path.c_str());

                Serializer::DeserializeIntoObject(GetGameWorld(), FoundFile, true);
            }
            else {
                cout << "No File" << endl;
            }
        }

        return;
	}
}