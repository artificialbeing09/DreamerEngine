#pragma once

#include "../World/World.h"

using namespace std;

void SortUIObjects(UIObject* obj, std::vector<UIObject*>& out) {
	if (!obj) return;

	// Output this node
	out.push_back(obj);

	vector<UIObject*> kids = {};

	for (auto Obj : obj->GetChildren()) {
		if (Obj->GetType() == "TextObject" || Obj->GetType() == "UIObject") {
			kids.push_back((UIObject*)Obj.get());
		}
	}

	// Sort children by zindex DESCENDING

	std::sort(kids.begin(), kids.end(),
		[](UIObject* a, UIObject* b) {
			return a->ZIndex < b->ZIndex;
		}
	);

	// Recurse in sorted order
	for (auto* child : kids)
		SortUIObjects(child, out);
}

bool UpdateAllText = false;

void UpdateUI() {
	vector<UIObject*> SortedObjects = {};

	auto SceneUI = Services::GetService<UIScene>("UIScene");

	SortUIObjects(SceneUI.get(), SortedObjects);

	Graphics::Engine2D::RenderObjects = {};

	static vector<UIObject*> AssociatedObjects = {};

	for (auto Obj : AssociatedObjects) {
		Obj->AssociatedObject = -1;
	}

	for (int i = 0; i < SortedObjects.size(); i++) {
		UIObject* Object = SortedObjects[i];

		if (Object->GetType() == "TextObject") {
			TextObject* TextObj = (TextObject*)Object;
			auto RenderObj = TextObj->Object;

			RenderObj.Size = TextObj->Size;

			if (TextObj->SizeConstraint == TextObject::YY) {
				RenderObj.Size.x *= ((float)Gl.h / (float)Gl.w);
			}
			else if (TextObj->SizeConstraint == TextObject::XX) {
				RenderObj.Size.y *= ((float)Gl.w / (float)Gl.h);
			}

			Object->AssociatedObject = (int)Graphics::Engine2D::RenderObjects.size();
			Graphics::Engine2D::RenderObjects.emplace_back(RenderObj);

			if (TextObj->UpdateTextFrame || UpdateAllText) {
				TextObj->TextObjects = {};

				Graphics::Engine2D::OutputText(
					TextObj->TextObjects,
					TextObj->Text,
					TextObj->Font,
					TextObj->TextAlign,
					TextObj->Object.Position,
					TextObj->FontSize,
					TextObj->TextColor,
					TextObj->TextTransparency
				);

				TextObj->UpdateTextFrame = false;
			}

			Graphics::Engine2D::RenderObjects.insert(
				Graphics::Engine2D::RenderObjects.end(),
				TextObj->TextObjects.begin(),
				TextObj->TextObjects.end()
			);
		}
	}

	UpdateAllText = false;
}