#pragma once

#include "Files.h"

using namespace std;

namespace Studio {
    // "EditorInstances" is defined in Files.h

    void InitIDEAndFiles() {

    }

    void PromptForSaveAll(string CustomMessage = "Do you want to save changes to the opened script(s)?") {
        bool ScriptToSave = false;
        
        for (int I = 0; I < EditorInstances.size(); I++) {
            auto& Editor = EditorInstances[I];

            if (Editor.EditorTextChangedLock) {
                ScriptToSave = true;
                break;
            }
        }

        if (!ScriptToSave) {
            return;
        }

        auto m = pfd::message("Unsaved Scripts", CustomMessage,
            pfd::choice::yes_no, pfd::icon::warning);

        while (!m.ready(200000)) {}

        switch (m.result())
        {
        case pfd::button::yes: break;
        case pfd::button::no: return;
        default: return; // Should not happen
        }

        for (int I = 0; I < EditorInstances.size(); I++) {
            auto& Editor = EditorInstances[I];

            if (Editor.EditorTextChangedLock) {
                Editor.EditorTextChangedLock = false;
                Utils::WriteFile(Editor.ScriptName.c_str(), Editor.editor.GetText().c_str());
            }
        }
    }

    void RenderIDE() {

        for (int I = 0; I < EditorInstances.size(); I++) {
            auto& Editor = EditorInstances[I];

            string IDEWindowTitle = Editor.ScriptName.substr(Editor.ScriptName.find_last_of('/') + 1);

            if (!Editor.IsOpen) {
                if (Editor.EditorTextChangedLock) {
                    auto m = pfd::message("Unsaved Script", "Do you want to save changes to " + IDEWindowTitle + "?",
                        pfd::choice::yes_no, pfd::icon::warning);

                    while (!m.ready(200000)) {}

                    switch (m.result())
                    {
                    case pfd::button::yes: Utils::WriteFile(Editor.ScriptName.c_str(), Editor.editor.GetText().c_str()); break;
                    case pfd::button::no: break;
                    default: break; // Should not happen
                    }
                }

                I--;
                std::swap(Editor, EditorInstances.back());
                EditorInstances.pop_back();

                continue;
            }

            if (Editor.editor.IsTextChanged()) {
                if (!Editor.EditorTextChangedThroughSetText) {
                    Editor.EditorTextChangedLock = true;
                }
                else {
                    Editor.EditorTextChangedThroughSetText = false;
                }
            }

            if (Editor.EditorTextChangedLock) {
                IDEWindowTitle += "*";
            }

            ImGui::Begin((IDEWindowTitle + "###IDE" + to_string(I)).c_str(), &Editor.IsOpen);

            if (ImGui::Button("Save")) {
                Utils::WriteFile(Editor.ScriptName.c_str(), Editor.editor.GetText().c_str());

                Editor.EditorTextChangedLock = false;
            }

            Editor.editor.Render("Script");

            ImGui::End();
        }
    }
}