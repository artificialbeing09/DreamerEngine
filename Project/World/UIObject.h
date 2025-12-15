#pragma once

#include "Instance.h"

bool UpdateUINextFrame = true;

class UIObject : public Instance {
public:
	int ZIndex = 0;
	int AssociatedObject = -1;

	void OnParentChanged(shared_ptr<Instance>) override {
		UpdateUINextFrame = true;
	}

	void SetZIndex(int Z) {
		ZIndex = Z;

		UpdateUINextFrame = true;
	}

	int GetZIndex() {
		return ZIndex;
	}

	UIObject() {
		Type = "UIObject";
		Name = "UIObject";
	}
};

auto propUIObjectZIndex = CreatePropertyDescriptor(UIObject, "UIObject", "ZIndex", int, L_Int, &UIObject::SetZIndex, &UIObject::GetZIndex);
CreateClassDescriptor(UIObject, "UIObject", "Instance");