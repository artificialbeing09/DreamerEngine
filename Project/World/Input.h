#pragma once

#include "Part.h"

using namespace Scheduler::Event;



// compute the near and far intersections of the cube (stored in the x and y components) using the slab method
// no intersection means vec.x > vec.y (really tNear > tFar)

// Note for self: Translating from world space to local space (relative to the cube) = inverse model matrix * world space location/direction
// 
// Source: https://gist.github.com/DomNomNom/46bb1ce47f68d255fd5d
// Source 2: https://en.wikipedia.org/wiki/Slab_method

glm::vec2 intersectAABB(glm::vec3 rayOrigin, glm::vec3 rayDir, glm::vec3 boxMin = glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3 boxMax = glm::vec3(0.5f, 0.5f, 0.5f)) {
    glm::vec3 tMin = (boxMin - rayOrigin) / rayDir;
    glm::vec3 tMax = (boxMax - rayOrigin) / rayDir;
    glm::vec3 t1 = min(tMin, tMax);
    glm::vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return glm::vec2(tNear, tFar);
};

glm::mat4 GetModelMatrix(glm::vec3 Position, glm::mat4 Rotation, glm::vec3 Size) {
    glm::mat4 T = glm::translate(glm::mat4(1.0f), Position);
    glm::mat4 R = glm::mat4(Rotation);
    glm::mat4 S = glm::scale(glm::mat4(1.0f), Size);

    return T * R * S;
}

bool RayIntersectCube(glm::vec3 rayOrigin, glm::vec3 rayDirection, RenderCubeObject_t Primitive, float& distance) {
    glm::mat4 InverseModel = glm::inverse(GetModelMatrix(Primitive.Position, Primitive.Rotation, Primitive.Size));

    glm::vec3 LocalRayOrigin = InverseModel * glm::vec4(rayOrigin, 1.0f);
    glm::vec3 LocalRayDirection = InverseModel * glm::vec4(rayDirection, 0.0f);

    glm::vec2 tnearfar = intersectAABB(LocalRayOrigin, LocalRayDirection);

    if (tnearfar.x > tnearfar.y)
        return false;

    distance = (tnearfar.x >= 0.0f) ? tnearfar.x : tnearfar.y;

    return tnearfar.x <= tnearfar.y && tnearfar.y >= 0.0f;
}

double LastMouseX = 0.0;
double LastMouseY = 0.0;


class Input : public Instance {
protected:
public:
	int InputDown = 0;
	int InputUp = 0;
    int MouseMoved = 0;
	int WindowFocused = 0;
	int WindowUnfocused = 0;

    shared_ptr<Instance> MouseTarget = NULLInstance;

    shared_ptr<Instance> GetMouseTarget() {
        return MouseTarget;
    }

    void SetMouseTarget(shared_ptr<Instance> target) { }

    double GetMouseX() {
        return LastMouseX;
    }

    double GetMouseY() {
        return LastMouseY;
    }

    void SetMouseX(double X) { 
        double xpos, ypos;
        glfwGetCursorPos(Gl.window, &xpos, &ypos);

        LastMouseX = X;

        glfwSetCursorPos(Gl.window, X, ypos);
    }

    void SetMouseY(double Y) {
        double xpos, ypos;
        glfwGetCursorPos(Gl.window, &xpos, &ypos);

        LastMouseY = Y;

        glfwSetCursorPos(Gl.window, xpos, Y);
    }

    int CastRayAllParts(lua_State* L) {
        LuaVector RayOrigin = luaL_checkvector(L, 1);
        LuaVector RayDirection = luaL_checkvector(L, 2);
        bool ReturnClosest = lua_toboolean(L, 3);

        vector<RenderObjectStore_t> PartList = {};

        // filter function in param 4

        if (lua_isfunction(L, 4)) {
            
            for (auto& List : Graphics::Engine3D::RenderObjects) {
                for (auto& PrimitiveInfo : List.second) {
                    Instance* Fake = (Instance*)PrimitiveInfo.Storage;

                    lua_pushvalue(L, 4);
                    lua_pushinstance(L, Fake->GetShared());
                    lua_call(L, 1, 1);
                    bool res = lua_toboolean(L, -1);
                    lua_pop(L, 1);

                    if (res) {
                        PartList.push_back(PrimitiveInfo);
                    }
                }
            }
        }
        else {
            for (auto& List : Graphics::Engine3D::RenderObjects) {
                for (auto& PrimitiveInfo : List.second) {
                    PartList.push_back(PrimitiveInfo);
                }
            }
        }

        if (ReturnClosest) {
            float distance = 398472983478923894.0f;
            void* Closest = NULL;

            for (auto& PrimitiveInfo : PartList) {
                float storedist = 0.0f;

                if (RayIntersectCube(glm::vec3(RayOrigin.x, RayOrigin.y, RayOrigin.z), glm::vec3(RayDirection.x, RayDirection.y, RayDirection.z), PrimitiveInfo.Object, storedist)) {
                    if (storedist < distance) {
                        distance = storedist;
                        Closest = PrimitiveInfo.Storage;
                    }
                }
            }

            if (Closest) {
                lua_pushinstance(L, ((Instance*)Closest)->GetShared());

                return 1;
            }
            else {
                return 0;
            }
        }
        else {
            lua_newtable(L);

            int i = 0;

            for (auto& PrimitiveInfo : PartList) {
                float storedist = 0.0f;

                if (RayIntersectCube(glm::vec3(RayOrigin.x, RayOrigin.y, RayOrigin.z), glm::vec3(RayDirection.x, RayDirection.y, RayDirection.z), PrimitiveInfo.Object, storedist)) {
                    lua_pushinstance(L, ((Instance*)(PrimitiveInfo.Storage))->GetShared());
                    lua_rawseti(L, -2, ++i);
                }
            }

            return 1;

        }

        return 0;
    }

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

    double xpos, ypos;
    glfwGetCursorPos(Gl.window, &xpos, &ypos);

    double x = ((2.0 * xpos) / (double)Gl.width) - 1.0;
    double y = 1.0 - ((2.0 * ypos) / (double)Gl.height);
    double z = 1.0;
    glm::vec3 ray_nds = glm::vec3(x, y, z);
    glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);
    glm::vec4 ray_eye = glm::inverse(Graphics::Engine3D::Camera::CalculateProjection()) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
    glm::vec3 ray_wor = glm::vec3(glm::inverse(Graphics::Engine3D::Camera::CalculateView()) * ray_eye);
    ray_wor = glm::normalize(ray_wor);

    // Ray_wor is the direction
    // Camera position is the position

    float distance = 398472983478923894.0f;
    void* Closest = NULL;

    for (auto& List : Graphics::Engine3D::RenderObjects) {
        for (auto& PrimitiveInfo : List.second) {
            float storedist = 0.0f;

            if (RayIntersectCube(Graphics::Engine3D::Camera::Position, ray_wor, PrimitiveInfo.Object, storedist)) {
                if (storedist < distance) {
                    distance = storedist;
                    Closest = PrimitiveInfo.Storage;
                }
            }
        }
    }

    if (Closest) {
        InputService->MouseTarget = ((Instance*)Closest)->GetShared();
    }
    else {
        InputService->MouseTarget = NULLInstance;
    }

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

    if (LastMouseX != xpos && LastMouseY != ypos) {
        Scheduler::Event::FireListenerInstance(InputService, "MouseMoved", { GetEventParamFromT(xpos - LastMouseX), GetEventParamFromT(ypos - LastMouseY) });
    }

    LastMouseX = xpos;
    LastMouseY = ypos;
}

auto eventInputInputPressed = CreateLuaEventDescriptor(Input, "Input", "InputPressed", Input::InputDown);
auto eventInputInputDown = CreateLuaEventDescriptor(Input, "Input", "InputDown", Input::InputDown);
auto eventInputInputUp = CreateLuaEventDescriptor(Input, "Input", "InputUp", Input::InputUp);
auto eventInputMouseMoved = CreateLuaEventDescriptor(Input, "Input", "MouseMoved", Input::MouseMoved);
auto eventInputWindowFocused = CreateLuaEventDescriptor(Input, "Input", "WindowFocused", Input::WindowFocused);
auto eventInputWindowUnfocused = CreateLuaEventDescriptor(Input, "Input", "WindowUnfocused", Input::WindowUnfocused);
auto callInputCastRayAllParts = CreateLuaNamecallDescriptor(Input, "Input", "CastRayAllParts", &Input::CastRayAllParts);
auto propInputMouseTarget = CreatePropertyDescriptor(Input, "Input", "MouseTarget", shared_ptr<Instance>, L_Object, &Input::SetMouseTarget, &Input::GetMouseTarget);
auto propInputMouseX = CreatePropertyDescriptor(Input, "Input", "MouseX", double, L_Number, &Input::SetMouseX, &Input::GetMouseX);
auto propInputMouseY = CreatePropertyDescriptor(Input, "Input", "MouseY", double, L_Number, &Input::SetMouseY, &Input::GetMouseY);

CreateClassDescriptor(Input, "Input", "Instance");