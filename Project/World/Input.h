#pragma once

#include "Instance.h"

using namespace Scheduler::Event;

class Input : public Instance {
protected:
public:
	int InputDown = 0;
	int InputUp = 0;
	int WindowFocused = 0;
	int WindowUnfocused = 0;

	Input() {
		Type = "Input";
		Name = "Input";
	}
};

char ShiftCharToUppercase(char c) {
    // Letters uppercase
    if (c >= 'a' && c <= 'z')
        return c - ('a' - 'A');

    // Number row + punctuation shifted
    switch (c) {
        case '1': return '!';
        case '2': return '@';
        case '3': return '#';
        case '4': return '$';
        case '5': return '%';
        case '6': return '^';
        case '7': return '&';
        case '8': return '*';
        case '9': return '(';
        case '0': return ')';

        case '-': return '_';
        case '=': return '+';
        case '[': return '{';
        case ']': return '}';
        case '\\': return '|';
        case ';': return ':';
        case '\'': return '"';
        case ',': return '<';
        case '.': return '>';
        case '/': return '?';
        case '`': return '~';
    }

    // Any other char unchanged
    return c;
}

bool ShiftDown() {
    return Gl.KeysDown[GLFW_KEY_LEFT_SHIFT] || Gl.KeysDown[GLFW_KEY_RIGHT_SHIFT];
}

std::string KeyToText(int key, int scancode) { // ChatGPT provided this code.
    const char* name = glfwGetKeyName(key, scancode);
    if (name) return std::string(name);

    static const std::unordered_map<int, std::string> specialKeys = {
        {GLFW_KEY_SPACE, "Space"},
        {GLFW_KEY_ESCAPE, "Escape"},
        {GLFW_KEY_ENTER, "Enter"},
        {GLFW_KEY_TAB, "Tab"},
        {GLFW_KEY_BACKSPACE, "Backspace"},
        {GLFW_KEY_INSERT, "Insert"},
        {GLFW_KEY_DELETE, "Delete"},
        {GLFW_KEY_RIGHT, "Right"},
        {GLFW_KEY_LEFT, "Left"},
        {GLFW_KEY_DOWN, "Down"},
        {GLFW_KEY_UP, "Up"},
        {GLFW_KEY_PAGE_UP, "PageUp"},
        {GLFW_KEY_PAGE_DOWN, "PageDown"},
        {GLFW_KEY_HOME, "Home"},
        {GLFW_KEY_END, "End"},
        {GLFW_KEY_CAPS_LOCK, "CapsLock"},
        {GLFW_KEY_SCROLL_LOCK, "ScrollLock"},
        {GLFW_KEY_NUM_LOCK, "NumLock"},
        {GLFW_KEY_PRINT_SCREEN, "PrintScreen"},
        {GLFW_KEY_PAUSE, "Pause"},
        {GLFW_KEY_F1, "F1"}, {GLFW_KEY_F2, "F2"}, {GLFW_KEY_F3, "F3"},
        {GLFW_KEY_F4, "F4"}, {GLFW_KEY_F5, "F5"}, {GLFW_KEY_F6, "F6"},
        {GLFW_KEY_F7, "F7"}, {GLFW_KEY_F8, "F8"}, {GLFW_KEY_F9, "F9"},
        {GLFW_KEY_F10, "F10"}, {GLFW_KEY_F11, "F11"}, {GLFW_KEY_F12, "F12"},
        {GLFW_KEY_LEFT_SHIFT, "LeftShift"},
        {GLFW_KEY_RIGHT_SHIFT, "RightShift"},
        {GLFW_KEY_LEFT_CONTROL, "LeftCtrl"},
        {GLFW_KEY_RIGHT_CONTROL, "RightCtrl"},
        {GLFW_KEY_LEFT_ALT, "LeftAlt"},
        {GLFW_KEY_RIGHT_ALT, "RightAlt"},
        {GLFW_KEY_LEFT_SUPER, "LeftSuper"},
        {GLFW_KEY_RIGHT_SUPER, "RightSuper"},
        {GLFW_KEY_MENU, "Menu"},
        {GLFW_MOUSE_BUTTON_1, "LeftMouse"},
        {GLFW_MOUSE_BUTTON_2, "RightMouse"},
        {GLFW_MOUSE_BUTTON_3, "MiddleMouse"},
    };

    auto it = specialKeys.find(key);
    if (it != specialKeys.end())
        return it->second;

    return "Unknown";
}

void InputFrameFunction(Input* InputService) {
	static bool KeysDown[400];

	for (int i = 0; i < 400; i++) {
		if (KeysDown[i] != Gl.KeysDown[i]) {
			bool NewValue = Gl.KeysDown[i];

			string L = KeyToText(i, 0);

            if (L.size() == 1 && ShiftDown())
                L = ShiftCharToUppercase(L[0]);

			if (NewValue)
				Scheduler::Event::FireListenerInstance(InputService, "InputDown", { GetEventParamFromT(L), GetEventParamFromT(ShiftDown()) });
			else
				Scheduler::Event::FireListenerInstance(InputService, "InputUp", { GetEventParamFromT(L), GetEventParamFromT(ShiftDown()) });

			KeysDown[i] = NewValue;
		}
	}

	static bool WindowLastFocused = false;

	bool WindowFocused = glfwGetWindowAttrib(Gl.window, GLFW_FOCUSED);

	if (WindowLastFocused != WindowFocused) {
		if (WindowFocused)
			Scheduler::Event::FireListenerInstance(InputService, "WindowFocused");
		else
			Scheduler::Event::FireListenerInstance(InputService, "WindowUnfocused");

		WindowLastFocused = WindowFocused;
	}
}

auto eventInputInputPressed = CreateLuaEventDescriptor(Input, "Input", "InputPressed", Input::InputDown);
auto eventInputInputDown = CreateLuaEventDescriptor(Input, "Input", "InputDown", Input::InputDown);
auto eventInputInputUp = CreateLuaEventDescriptor(Input, "Input", "InputUp", Input::InputUp);
auto eventInputWindowFocused = CreateLuaEventDescriptor(Input, "Input", "WindowFocused", Input::WindowFocused);
auto eventInputWindowUnfocused = CreateLuaEventDescriptor(Input, "Input", "WindowUnfocused", Input::WindowUnfocused);

CreateClassDescriptor(Input, "Input", "Instance");