#pragma once

#include "Main/Utils.h"

using namespace std;

namespace LoadAndSave {
	void CreateFolderIfDoesNotExist(string Path) {
		if (!std::filesystem::exists(Path)) {
			std::filesystem::create_directory(Path);
		}
	}

	string GetScriptByModuleName(string ModuleName) {
		CreateFolderIfDoesNotExist(".\\Game");

		if (!std::filesystem::exists(".\\Game\\" + ModuleName)) {
			return "";
		}

		return Utils::ReadFile((".\\Game\\" + ModuleName).c_str());
	}
}