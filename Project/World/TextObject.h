#pragma once

#include "UIObject.h"
#include "../Bridge/Scheduler.h"

class TextObject : public UIObject {
public:
	enum SizeConstraintE {
		XY, // No change
		XX, // Change size Y
		YY  // Change size X
	};

	Graphics::Engine2D::Render2DObject_t Object;
	vector<Graphics::Engine2D::Render2DObject_t> TextObjects = {};

	string Text = "";
	string Font = "Default";
	float FontSize = 0.1f;
	Graphics::Engine2D::TextAlignE TextAlign = Graphics::Engine2D::Center;
	glm::vec3 TextColor = glm::vec3(0.5f, 0.5f, 0.5f);
	float TextTransparency = 1.0f;
	SizeConstraintE SizeConstraint = XY;
	glm::vec2 Size = glm::vec2(0.0f, 0.0f);

	string StoredTexture = "";

	bool Entered = false;
	bool RightClicked = false;
	bool LeftClicked = false;
	bool MiddleClicked = false;
	float MousePosX = 0.0;
	float MousePosY = 0.0;

	LuaVector GetBounds() {
		float SizeX = Gl.width * Object.Size.x;
		float SizeY = Gl.height * Object.Size.y;

		float PosX = Gl.width * Object.Position.x;
		float PosY = Gl.height * Object.Position.y;

		return { PosX - (SizeX / 2.0), PosY - (SizeY / 2.0), PosX + (SizeX / 2.0), PosY + (SizeY / 2.0) };
	}

	void UpdateObject() {
		Object.Size = Size;

		if (SizeConstraint == TextObject::YY) {
			Object.Size.x *= ((float)Gl.height / (float)Gl.width);
		}
		else if (SizeConstraint == TextObject::XX) {
			Object.Size.y *= ((float)Gl.width / (float)Gl.height);
		}

		if (AssociatedObject != -1) {
			Graphics::Engine2D::RenderObjects[AssociatedObject] = Object;
		}
	}

	bool UpdateTextFrame = false;

	void UpdateText() {
		UpdateTextFrame = true;

		if (AssociatedObject != -1) {
			UpdateUINextFrame = true;
		}
	}

	void SetPosition(LuaVector Position) { Object.Position = glm::vec2(Position.x, Position.y); UpdateObject(); UpdateText(); }
	LuaVector GetPosition() { return LuaVector(Object.Position.x, Object.Position.y); }

	void SetSize(LuaVector NewSize) { Size = glm::vec2(NewSize.x, NewSize.y); UpdateObject(); }
	LuaVector GetSize() { return LuaVector(Size.x, Size.y); }

	void SetColor(LuaVector Color) { Object.Color = glm::vec4(Color.x, Color.y, Color.z, Object.Color.w); UpdateObject(); }
	LuaVector GetColor() { return LuaVector(Object.Color.x, Object.Color.y, Object.Color.z); }

	void SetTexture(string NewTexture) { StoredTexture = NewTexture; Object.Texture0 = Texture::Textures[NewTexture]; UpdateObject(); }
	string GetTexture() { return StoredTexture; }

	void SetTransparency(double Transparency) { Object.Color.w = (float)Transparency; UpdateObject(); }
	double GetTransparency() { return Object.Color.w; }

	void SetCornerSize(double CornerSize) { Object.CornerSize = (float)CornerSize; UpdateObject(); }
	double GetCornerSize() { return Object.CornerSize; }

	void SetText(string NewText) { Text = NewText; UpdateText(); }
	string GetText() { return Text; }

	void SetFont(string NewText) { if (Graphics::Engine2D::Fonts[NewText].StbFont) Font = NewText; UpdateText(); }
	string GetFont() { return Font; }

	void SetFontSize(double NewFontSize) { FontSize = (float)NewFontSize; UpdateText(); }
	double GetFontSize() { return FontSize; }

	void SetTextTransparency(double NewTextTransparency) { TextTransparency = (float)NewTextTransparency; UpdateText(); }
	double GetTextTransparency() { return TextTransparency; }

	void SetTextAlign(string NewTextAlign) { 
		if (NewTextAlign == "Center")
			TextAlign = Graphics::Engine2D::Center;
		else if (NewTextAlign == "Left")
			TextAlign = Graphics::Engine2D::Left;
		else if (NewTextAlign == "Right")
			TextAlign = Graphics::Engine2D::Right;

		UpdateText();
	}
	string GetTextAlign() { 
		if (TextAlign == Graphics::Engine2D::Center)
			return "Center";
		else if (TextAlign == Graphics::Engine2D::Left)
			return "Left";
		else if (TextAlign == Graphics::Engine2D::Right)
			return "Right";
		return "Unknown";
	}

	void SetSizeConstraint(string NewSizeConstraint) {
		if (NewSizeConstraint == "XY") {
			SizeConstraint = XY;
		}
		else if (NewSizeConstraint == "XX") {
			SizeConstraint = XX;
		}
		else if (NewSizeConstraint == "YY") {
			SizeConstraint = YY;
		}

		UpdateObject();
	}
	string GetSizeConstraint() {
		if (SizeConstraint == XY) {
			return "XY";
		}
		else if (SizeConstraint == XX) {
			return "XX";
		}
		else if (SizeConstraint == YY) {
			return "YY";
		}
		return "Unknown";
	}

	void SetTextColor(LuaVector Color) { TextColor = glm::vec3(Color.x, Color.y, Color.z); UpdateText(); }
	LuaVector GetTextColor() { return LuaVector(TextColor.x, TextColor.y, TextColor.z); }

	int MouseLeftClicked = 0;
	int MouseRightClicked = 0;
	int MouseMiddleClicked = 0;
	int MouseEntered = 0;
	int MouseLeft = 0;
	int MouseLeftUp = 0;
	int MouseRightUp = 0;
	int MouseMiddleUp = 0;
	int MouseMoved = 0;
	int MouseScrollUp = 0;
	int MouseScrollDown = 0;

	TextObject() {
		Type = "TextObject";
		Name = "TextObject";
		Object.Position = glm::vec2(0.0, 0.0);
		Size = glm::vec2(0.1, 0.1);
		Object.Color = glm::vec4(0.5, 0.5, 0.5, 1.0);
		Object.Texture0 = 0;
		Object.CornerSize = 0.0;
	}
};

int idf = 0;

double relativeCursorX = 0.0;
double relativeCursorY = 0.0;

void PreTextObjectFunction() {
	int fbWidth, fbHeight;
	glfwGetFramebufferSize(Gl.window, &fbWidth, &fbHeight);

	int winWidth, winHeight;
	glfwGetWindowSize(Gl.window, &winWidth, &winHeight);

	double scaleX = (double)fbWidth / winWidth;
	double scaleY = (double)fbHeight / winHeight;

	glfwGetCursorPos(Gl.window, &relativeCursorX, &relativeCursorY);

	relativeCursorX *= scaleX;
	relativeCursorY *= scaleY;

	relativeCursorY = Gl.height - relativeCursorY;
}

void TextObjectFrameFunction(Instance* o) {
	TextObject* text = (TextObject*)o;

	LuaVector Bounds = text->GetBounds();

	bool LeftMouseDown = false;
	bool RightMouseDown = false;
	bool MiddleMouseDown = false;

	if (relativeCursorX > Bounds.x && relativeCursorY > Bounds.y &&
		relativeCursorX < Bounds.z && relativeCursorY < Bounds.a) {
		
		if (!text->Entered)
			Scheduler::Event::FireListenerInstance(o, "MouseEntered");

		LeftMouseDown = Gl.KeysDown[GLFW_MOUSE_BUTTON_LEFT];
		RightMouseDown = Gl.KeysDown[GLFW_MOUSE_BUTTON_RIGHT];
		MiddleMouseDown = Gl.KeysDown[GLFW_MOUSE_BUTTON_MIDDLE];

		if (relativeCursorX != text->MousePosX || relativeCursorY != text->MousePosY) {
			Scheduler::Event::FireListenerInstance(o, "MouseMoved");
			text->MousePosX = (float)relativeCursorX;
			text->MousePosY = (float)relativeCursorY;
		}

		text->Entered = true;
	}
	else {
		if (text->Entered)
			Scheduler::Event::FireListenerInstance(o, "MouseLeft");
		text->Entered = false;
	}

	if (!text->LeftClicked && LeftMouseDown)
		Scheduler::Event::FireListenerInstance(o, "MouseLeftClicked");
	if (text->LeftClicked && !LeftMouseDown)
		Scheduler::Event::FireListenerInstance(o, "MouseLeftUp");

	if (!text->RightClicked && RightMouseDown)
		Scheduler::Event::FireListenerInstance(o, "MouseRightClicked");
	if (text->RightClicked && !RightMouseDown)
		Scheduler::Event::FireListenerInstance(o, "MouseRightUp");

	if (!text->MiddleClicked && MiddleMouseDown)
		Scheduler::Event::FireListenerInstance(o, "MouseMiddleClicked");
	if (text->MiddleClicked && !MiddleMouseDown)
		Scheduler::Event::FireListenerInstance(o, "MouseMiddleUp");

	text->LeftClicked = LeftMouseDown;
	text->RightClicked = RightMouseDown;
	text->MiddleClicked = MiddleMouseDown;
}

auto propTextObjectTexture = CreatePropertyDescriptor(TextObject, "TextObject", "Texture", string, L_String, &TextObject::SetTexture, &TextObject::GetTexture);
auto propTextObjectTextAlign = CreatePropertyDescriptor(TextObject, "TextObject", "TextAlign", string, L_String, &TextObject::SetTextAlign, &TextObject::GetTextAlign);
auto propTextObjectSizeConstraint = CreatePropertyDescriptor(TextObject, "TextObject", "SizeConstraint", string, L_String, &TextObject::SetSizeConstraint, &TextObject::GetSizeConstraint);
auto propTextObjectText = CreatePropertyDescriptor(TextObject, "TextObject", "Text", string, L_String, &TextObject::SetText, &TextObject::GetText);
auto propTextObjectFont = CreatePropertyDescriptor(TextObject, "TextObject", "Font", string, L_String, &TextObject::SetFont, &TextObject::GetFont);
auto propTextObjectTransparency = CreatePropertyDescriptor(TextObject, "TextObject", "Transparency", double, L_Number, &TextObject::SetTransparency, &TextObject::GetTransparency);
auto propTextObjectCornerSize = CreatePropertyDescriptor(TextObject, "TextObject", "CornerSize", double, L_Number, &TextObject::SetCornerSize, &TextObject::GetCornerSize);
auto propTextObjectFontSize = CreatePropertyDescriptor(TextObject, "TextObject", "FontSize", double, L_Number, &TextObject::SetFontSize, &TextObject::GetFontSize);
auto propTextObjectTextTransparency = CreatePropertyDescriptor(TextObject, "TextObject", "TextTransparency", double, L_Number, &TextObject::SetTextTransparency, &TextObject::GetTextTransparency);
auto propTextObjectPosition = CreatePropertyDescriptor(TextObject, "TextObject", "Position", LuaVector, L_Vector, &TextObject::SetPosition, &TextObject::GetPosition);
auto propTextObjectSize = CreatePropertyDescriptor(TextObject, "TextObject", "Size", LuaVector, L_Vector, &TextObject::SetSize, &TextObject::GetSize);
auto propTextObjectColor = CreatePropertyDescriptor(TextObject, "TextObject", "Color", LuaVector, L_Vector, &TextObject::SetColor, &TextObject::GetColor);
auto propTextObjectTextColor = CreatePropertyDescriptor(TextObject, "TextObject", "TextColor", LuaVector, L_Vector, &TextObject::SetTextColor, &TextObject::GetTextColor);

auto eventTextObjectMouseLeftClicked = CreateLuaEventDescriptor(TextObject, "TextObject", "MouseLeftClicked", TextObject::MouseLeftClicked);
auto eventTextObjectMouseRightClicked = CreateLuaEventDescriptor(TextObject, "TextObject", "MouseRightClicked", TextObject::MouseRightClicked);
auto eventTextObjectMouseMiddleClicked = CreateLuaEventDescriptor(TextObject, "TextObject", "MouseMiddleClicked", TextObject::MouseMiddleClicked);
auto eventTextObjectMouseLeftDown = CreateLuaEventDescriptor(TextObject, "TextObject", "MouseLeftDown", TextObject::MouseLeftClicked);
auto eventTextObjectMouseRightDown = CreateLuaEventDescriptor(TextObject, "TextObject", "MouseRightDown", TextObject::MouseRightClicked);
auto eventTextObjectMouseMiddleDown = CreateLuaEventDescriptor(TextObject, "TextObject", "MouseMiddleDown", TextObject::MouseMiddleClicked);
auto eventTextObjectMouseEntered = CreateLuaEventDescriptor(TextObject, "TextObject", "MouseEntered", TextObject::MouseEntered);
auto eventTextObjectMouseLeft = CreateLuaEventDescriptor(TextObject, "TextObject", "MouseLeft", TextObject::MouseLeft);
auto eventTextObjectMouseLeftUp = CreateLuaEventDescriptor(TextObject, "TextObject", "MouseLeftUp", TextObject::MouseLeftUp);
auto eventTextObjectMouseRightUp = CreateLuaEventDescriptor(TextObject, "TextObject", "MouseRightUp", TextObject::MouseRightUp);
auto eventTextObjectMouseMiddleUp = CreateLuaEventDescriptor(TextObject, "TextObject", "MouseMiddleUp", TextObject::MouseMiddleUp);
auto eventTextObjectMouseMoved = CreateLuaEventDescriptor(TextObject, "TextObject", "MouseMoved", TextObject::MouseMoved);
//auto eventTextObjectMouseScrollUp = CreateLuaEventDescriptor(TextObject, "TextObject", "MouseScrollUp", TextObject::MouseScrollUp);
//auto eventTextObjectMouseScrollDown = CreateLuaEventDescriptor(TextObject, "TextObject", "MouseScrollDown", TextObject::MouseScrollDown);
// Do these later


CreateClassDescriptor(TextObject, "TextObject", "UIObject");