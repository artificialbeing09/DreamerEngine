#pragma once

#include "Configuration.h"

using namespace std;

namespace Plugin {
    void LoadPlugins() {
        LoadAndSave::CreateFolderIfDoesNotExist("./Plugins");

        for (const auto& dirEntry : filesystem::directory_iterator("./Plugins")) {
            bool IsDirectory = filesystem::is_directory(dirEntry);

            string Path = dirEntry.path().generic_string();
            string EntryName = Path.substr(Path.find_first_of("/Plugins") + 9);

            if (IsDirectory) {
                Scheduler::Lua::RunScript(LoadAndSave::GetScriptByModuleName(EntryName + "/main.lua", 1), EntryName, 1);
            }
            else {
                Scheduler::Lua::RunScript(LoadAndSave::GetScriptByModuleName(EntryName, 1), EntryName, 1);
            }
        }
    }
}