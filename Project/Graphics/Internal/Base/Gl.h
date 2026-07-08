#pragma once

#include "../../../Main/Utils.h"

void placeholder_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {}

class Gl_t {
private:
    static inline GLFWkeyfun keyCallback;

    static inline void internal_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (action == GLFW_PRESS)
            KeysDown[key] = true;

        if (action == GLFW_RELEASE)
            KeysDown[key] = false;

        if (keyCallback)
            keyCallback(window, key, scancode, action, mods);
    }

    static inline GLFWmousebuttonfun mouseCallback;

    static inline void internal_mouse_callback(GLFWwindow* window, int button, int action, int mods)
    {
        if (action == GLFW_PRESS)
            KeysDown[button] = true;

        if (action == GLFW_RELEASE)
            KeysDown[button] = false;

        if (mouseCallback)
            mouseCallback(window, button, action, mods);
    }
public:
    GLFWwindow* window;

    int width, height; // true width/height
    int w, h; // stored width/height

    static inline bool KeysDown[400]; // Also used for mouse input

    void Initialize() {
        if (!glfwInit()) {
            cout << "FUCK" << endl;

            exit(-1);

            return;
        }

        window = glfwCreateWindow(1000, 1000, "3d game engine", 0, 0);
        glfwMakeContextCurrent(window);

        auto glewValue = glewInit();

        if (glewValue != GLEW_OK) {
            cout << "Error: " << glewGetErrorString(glewValue) << endl;

            exit(-1);

            return;
        }

        keyCallback = placeholder_key_callback;

        glfwSetKeyCallback(window, (GLFWkeyfun)internal_key_callback);
        glfwSetMouseButtonCallback(window, (GLFWmousebuttonfun)internal_mouse_callback);
    }

    void SetKeyCallback(GLFWkeyfun newFunc) {
        keyCallback = newFunc;
    }

    double MouseX, MouseY;

    void PreRender() {
        glfwMakeContextCurrent(window);
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glfwGetCursorPos(window, &MouseX, &MouseY);

        glClearColor(0.0, 0.5, 1.0, 1.0);

        ClearDepthBuffer();
        ClearColorBuffer();
    }

    bool ShouldClose() {
        return glfwWindowShouldClose(window);
    }

    void SetMousePos(double NewMouseX, double NewMouseY) {
        MouseX = NewMouseX;
        MouseY = NewMouseY;
        glfwSetCursorPos(window, NewMouseX, NewMouseY);
    }

    // Type types

    static const int Byte = GL_BYTE;
    static const int UnsignedByte = GL_UNSIGNED_BYTE;
    static const int Char = GL_UNSIGNED_BYTE;
    static const int Short = GL_SHORT;
    static const int UnsignedShort = GL_UNSIGNED_SHORT;
    static const int Int = GL_INT;
    static const int UnsignedInt = GL_UNSIGNED_INT;

    static const int HalfFloat = GL_HALF_FLOAT;
    static const int Float = GL_FLOAT;
    static const int Double = GL_DOUBLE;
    static const int Fixed = GL_FIXED;

    // Draw array types

    static const int Triangles = GL_TRIANGLES;
    static const int Points = GL_POINTS;
    static const int TriangleStrip = GL_TRIANGLE_STRIP;
    static const int Patches = GL_PATCHES;
    static const int Lines = GL_LINES;

    // Enable features

    static const int DepthTest = GL_DEPTH_TEST;
    static const int CullFace = GL_CULL_FACE;


    GLuint GenBuffer() {
        GLuint b;
        glGenBuffers(1, &b);
        return b;
    }

    void BindArrayBuffer(GLuint b) {
        glBindBuffer(GL_ARRAY_BUFFER, b);
    }

    void BindShaderStorageBuffer(GLuint b) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, b);
    }

    void BindShaderStorageBufferBase(GLuint b, GLuint binding) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, b);
    }

    template <typename T>
    void ShaderStorageBufferDataVector(vector<T> List) {
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(T) * List.size(), List.data(), GL_STATIC_DRAW);
    }

    void ShaderStorageBufferData(void* ptr, size_t size) {
        glBufferData(GL_SHADER_STORAGE_BUFFER, size, ptr, GL_STATIC_DRAW);
    }

    void BindBuffer(GLuint m, GLuint b) { // General func
        glBindBuffer(m, b);
    }



    template <typename T>
    void BufferStaticArrayData(T v, size_t size = sizeof(T)) {
        glBufferData(GL_ARRAY_BUFFER, size, v, GL_STATIC_DRAW);
    }

    /**
    *
    * \param size vec3 => 3 components (3 steps for next element), int => 1 component
    * \param type int buffer[] => Gl.Int
    *
    **/
    void VertexArray(GLuint location, GLuint size, GLenum type, GLboolean normalized = GL_FALSE, GLsizei stride = 0, const void* pointer = NULL) {
        glVertexAttribPointer(location, size, type, normalized, stride, pointer);
        glEnableVertexAttribArray(location);
    }

    void DisableVertexArray(GLuint location) {
        glDisableVertexAttribArray(location);
    }

    void EnableVertexArray(GLuint location) {
        glEnableVertexAttribArray(location);
    }

    void BindVertexArray(GLuint location) {
        glBindVertexArray(location);
    }

    void DrawArrays(GLenum mode, GLint count, GLint first = 0) {
        glDrawArrays(mode, first, count);
    }

    void ClearDepthBuffer() {
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    void ClearColorBuffer() {
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Enable(GLenum a) {
        glEnable(a);
    }

    void Disable(GLenum a) {
        glDisable(a);
    }

    glm::mat4 DirectionFromEuler(float x, float y, float z) {
        return
            glm::rotate(glm::mat4(1.0f), z, glm::vec3(0.0f, 0.0f, 1.0f)) *
            glm::rotate(glm::mat4(1.0f), y, glm::vec3(0.0f, 1.0f, 0.0f)) *
            glm::rotate(glm::mat4(1.0f), x, glm::vec3(1.0f, 0.0f, 0.0f));
    }

    void PostRender() {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
};

Gl_t Gl;
