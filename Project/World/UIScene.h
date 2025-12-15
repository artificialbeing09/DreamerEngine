#pragma once

#include "UIObject.h"

class UIScene : public UIObject {
public:
	UIScene() {
		Type = "UIScene";
		Name = "UIScene";
	}
};

CreateClassDescriptor(UIScene, "UIScene", "UIObject");