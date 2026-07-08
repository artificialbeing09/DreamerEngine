#pragma once

#include "../Base/Shader.h"


inline glm::mat4 RotationX(float angle) {
    float s = sin(angle);
    float c = cos(angle);
    return glm::mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, c, s, 0.0,
        0.0, -s, c, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

inline glm::mat4 RotationY(float angle) {
    float s = sin(angle);
    float c = cos(angle);
    return glm::mat4(
        c, 0.0, -s, 0.0,
        0.0, 1.0, 0.0, 0.0,
        s, 0.0, c, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

inline glm::mat4 RotationZ(float angle) {
    float s = sin(angle);
    float c = cos(angle);
    return glm::mat4(
        c, s, 0.0, 0.0,
        -s, c, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

namespace Graphics::Engine3D::Camera {
    float CameraSpeed = 1.0;

    glm::vec3 Position;
    glm::vec3 Rotation;

    float Far = 50000.0f;
    float FOVY = glm::radians(45.0f);
    float Close = 0.5f;

    glm::vec3 DirectionFromEuler(float x, float y, float z) {
        return
            glm::rotate(glm::mat4(1.0f), z, glm::vec3(0.0f, 0.0f, 1.0f)) *
            glm::rotate(glm::mat4(1.0f), y, glm::vec3(0.0f, 1.0f, 0.0f)) *
            glm::rotate(glm::mat4(1.0f), x, glm::vec3(1.0f, 0.0f, 0.0f)) *
            glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
    }

    // Retrieved from Camera.Position and Camera.Rotation
    glm::mat4 CalculateView() {
        return glm::lookAt(Position, Position + DirectionFromEuler(Rotation.x, Rotation.y, Rotation.z), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    // Retrieved from Camera.Far, Camera.Close, and Camera.FOVY
    glm::mat4 CalculateProjection() {
        if (Gl.h == 0)
            Gl.h = 1;
        
        return glm::perspective(FOVY, (float)Gl.w / (float)Gl.h, Close, Far);
    }

    struct Plane {
        glm::vec3 normal;
        float d;
    };

    struct Plane6 {
        Plane p[6];
    };

    Plane6 ExtractFrustumPlanes() {
        Plane6 planes;

        glm::mat4 VP = CalculateProjection() * CalculateView();

        // Left
        planes.p[0].normal.x = VP[0][3] + VP[0][0];
        planes.p[0].normal.y = VP[1][3] + VP[1][0];
        planes.p[0].normal.z = VP[2][3] + VP[2][0];
        planes.p[0].d = VP[3][3] + VP[3][0];

        // Right
        planes.p[1].normal.x = VP[0][3] - VP[0][0];
        planes.p[1].normal.y = VP[1][3] - VP[1][0];
        planes.p[1].normal.z = VP[2][3] - VP[2][0];
        planes.p[1].d = VP[3][3] - VP[3][0];

        // Bottom
        planes.p[2].normal.x = VP[0][3] + VP[0][1];
        planes.p[2].normal.y = VP[1][3] + VP[1][1];
        planes.p[2].normal.z = VP[2][3] + VP[2][1];
        planes.p[2].d = VP[3][3] + VP[3][1];

        // Top
        planes.p[3].normal.x = VP[0][3] - VP[0][1];
        planes.p[3].normal.y = VP[1][3] - VP[1][1];
        planes.p[3].normal.z = VP[2][3] - VP[2][1];
        planes.p[3].d = VP[3][3] - VP[3][1];

        // Near
        planes.p[4].normal.x = VP[0][3] + VP[0][2];
        planes.p[4].normal.y = VP[1][3] + VP[1][2];
        planes.p[4].normal.z = VP[2][3] + VP[2][2];
        planes.p[4].d = VP[3][3] + VP[3][2];

        // Far
        planes.p[5].normal.x = VP[0][3] - VP[0][2];
        planes.p[5].normal.y = VP[1][3] - VP[1][2];
        planes.p[5].normal.z = VP[2][3] - VP[2][2];
        planes.p[5].d = VP[3][3] - VP[3][2];

        // Normalize
        for (int I = 0; I < 6; I++) {

            float len = glm::length(planes.p[I].normal);
            planes.p[I].normal /= len;
            planes.p[I].d /= len;
        }

        return planes;
    }

    bool IsSphereVisible(const glm::vec3& center, float radius,
        const Plane6 planes) {
        for (int I = 0; I < 6; I++) {
            float distance = glm::dot(planes.p[I].normal, center) + planes.p[I].d;
            if (distance < -radius)
                return false; // sphere is completely outside
        }
        return true; // at least partially inside
    }

    void CameraStep() {
        if (Gl.KeysDown[GLFW_KEY_W])
            Position += DirectionFromEuler(Rotation.x, Rotation.y, Rotation.z) * CameraSpeed;

        if (Gl.KeysDown[GLFW_KEY_S])
            Position -= DirectionFromEuler(Rotation.x, Rotation.y, Rotation.z) * CameraSpeed;

        if (Gl.KeysDown[GLFW_KEY_A])
            Position += DirectionFromEuler(0.0f, Rotation.y + glm::radians(90.0f), 0.0f) * CameraSpeed;

        if (Gl.KeysDown[GLFW_KEY_D])
            Position += DirectionFromEuler(0.0f, Rotation.y - glm::radians(90.0f), 0.0f) * CameraSpeed;

        if (Gl.KeysDown[GLFW_KEY_UP])
            CameraSpeed *= 1.1f;

        if (Gl.KeysDown[GLFW_KEY_DOWN])
            CameraSpeed /= 1.1f;

        if (Gl.KeysDown[GLFW_KEY_I])
            FOVY += glm::radians(1.0f);

        if (Gl.KeysDown[GLFW_KEY_O])
            FOVY -= glm::radians(1.0f);

        CameraSpeed = glm::clamp(CameraSpeed, 0.0f, 20.0f);
        FOVY = glm::clamp(FOVY, glm::radians(1.0f), glm::radians(89.0f));

        static double lastxpos, lastypos;
        double xpos, ypos;
        glfwGetCursorPos(Gl.window, &xpos, &ypos);

        if (Gl.KeysDown[GLFW_MOUSE_BUTTON_2]) {
            Rotation += glm::vec3(glm::radians(1.0f) * (lastypos - ypos), glm::radians(1.0f) * (lastxpos - xpos), 0.0);
        }

        Rotation = glm::vec3(
            glm::clamp(Rotation.x, glm::radians(-89.9f), glm::radians(89.9f)),
            Rotation.y,
            glm::clamp(Rotation.z, glm::radians(-89.9f), glm::radians(89.9f))
        );

        lastxpos = xpos;
        lastypos = ypos;
    }
}