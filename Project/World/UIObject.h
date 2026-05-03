#pragma once

#include "Instance.h"

bool UpdateUINextFrame = true;


class UIObject : public Instance {
public:
	int64_t ZIndex = 0;
	int AssociatedObject = -1;

	void OnParentChanged(shared_ptr<Instance>) override {
		UpdateUINextFrame = true;
	}

	void SetZIndex(int64_t Z) {
		ZIndex = Z;

		UpdateUINextFrame = true;
	}

	int64_t GetZIndex() {
		return ZIndex;
	}

	UIObject() {
		Type = "UIObject";
		Name = "UIObject";
	}
};

auto propUIObjectZIndex = CreatePropertyDescriptor(UIObject, "UIObject", "ZIndex", int64_t, L_Int, &UIObject::SetZIndex, &UIObject::GetZIndex);
CreateClassDescriptor(UIObject, "UIObject", "Instance");