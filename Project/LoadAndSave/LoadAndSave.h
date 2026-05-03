#pragma once

#include "Main/Utils.h"

using namespace std;

namespace LoadAndSave {
	void CreateFolderIfDoesNotExist(string Path) {
		if (!std::filesystem::exists(Path)) {
			std::filesystem::create_directory(Path);
		}
	}

	string GetScriptByModuleName(string ModuleName, int threadIdentity = 0) {
		if (threadIdentity == 0) {
			CreateFolderIfDoesNotExist(".\\Game");

			if (!std::filesystem::exists(".\\Game\\" + ModuleName)) {
				return "";
			}

			return Utils::ReadFile((".\\Game\\" + ModuleName).c_str());
		}
		else if (threadIdentity == 1) {
			CreateFolderIfDoesNotExist(".\\Plugins");

			if (!std::filesystem::exists(".\\Plugins\\" + ModuleName)) {
				return "";
			}

			return Utils::ReadFile((".\\Plugins\\" + ModuleName).c_str());
		}
	}
}