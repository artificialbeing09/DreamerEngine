#pragma once

#include "Configuration.h"

using namespace std;

namespace Studio {
    string CurrentMapSelected = "";

    struct IDEEditorInstance {
        string ScriptName = "";
        string IDEText = "";
        TextEditor editor;
        bool EditorTextChangedLock = false;
        bool EditorTextChangedThroughSetText = false;
        bool IsOpen = true;
    };

    deque<IDEEditorInstance> EditorInstances = {};

    enum FileEntryType {
        SerializedObject,
        Script,
        Folder
    };

    struct FileEntry;
    struct FileEntry {
        FileEntryType T = FileEntryType::Script;
        string Name = "";
        string Path = "";
        deque<FileEntry*> List = {};
    };

    FileEntry FilesList;

    void UpdateFileEntryList(FileEntry* Parent, string BasePath);

    void UpdateFileEntryList(FileEntry* Parent = &FilesList, string BasePath = ".\\Game") {
        if (Parent == &FilesList) {
            FilesList.List = {};
        }

        for (const auto& dirEntry : filesystem::directory_iterator(BasePath)) {
            bool IsDirectory = filesystem::is_directory(dirEntry);

            string Path = dirEntry.path().generic_string();
            string EntryName = Path.substr(Path.find_last_of("/") + 1);

            FileEntry* New = new FileEntry;
            New->Name = EntryName;
            New->Path = Path;
            New->List = {};

            if (IsDirectory) {
                New->T = FileEntryType::Folder;

                UpdateFileEntryList(New, Path);
            }
            else {
                if (EntryName.substr(EntryName.find_last_of('.')) == ".map") {
                    New->T = FileEntryType::SerializedObject;
                }
                else {
                    New->T = FileEntryType::Script;
                }
            }

            Parent->List.push_back(New);
        }
    }

    void RenderFilesList(FileEntry* Start, int LeftOffset);

    void RenderFilesList(FileEntry* Start, int LeftOffset) {
        for (auto Inst : Start->List) {
            string Name = Inst->Name;

            ImGui::PushID((int)Inst);

            ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_OpenOnArrow;

            if (Inst->T != FileEntryType::Folder)
                flag |= ImGuiTreeNodeFlags_Leaf;

            bool Open = ImGui::TreeNodeEx(Name.c_str(), flag);
            bool Clicked = ImGui::IsItemClicked();

            if (Clicked) {
                if (Inst->T == FileEntryType::SerializedObject) {
                    string SerializedText = Utils::ReadFile(Inst->Path.c_str());

                    CurrentMapSelected = Inst->Path;

                    Serializer::DeserializeIntoObject(GetGameWorld(), SerializedText, true);
                }
                else if (Inst->T == FileEntryType::Script) {
                    IDEEditorInstance* NewInst = new IDEEditorInstance;
                    NewInst->ScriptName = Inst->Path;
                    NewInst->IDEText = Utils::ReadFile(Inst->Path.c_str());

                    auto lang = TextEditor::LanguageDefinition::Lua();
                    NewInst->editor.SetLanguageDefinition(lang);
                    NewInst->editor.SetText(NewInst->IDEText);
                    NewInst->EditorTextChangedLock = false;
                    NewInst->EditorTextChangedThroughSetText = true;

                    EditorInstances.emplace_back(*NewInst);
                }
            }

            if (Open) {
                RenderFilesList(Inst, LeftOffset + 1);

                ImGui::TreePop();
            }

            ImGui::PopID();
        }
    }

    int FilesCounter = 0;

    void RenderFiles() {
        if (FilesCounter++ % 60 == 0) {
            UpdateFileEntryList();
        }

        RenderFilesList(&FilesList, 0);
    }
}