#pragma once

#include "Controls.h"

using namespace std;

namespace Studio {

    int SceneWindowSizeX = 0;
    int SceneWindowSizeY = 0;

    int SceneCursorOffsetX = 0;
    int SceneCursorOffsetY = 0;

    GLuint FBODepth;

    void InitializeSeperateFrameBuffer() {
        glGenFramebuffers(1, &Graphics::Engine3D::defaultFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, Graphics::Engine3D::defaultFBO);

        glGenTextures(1, &Graphics::Engine3D::FBOtextureMap);
        glBindTexture(GL_TEXTURE_2D, Graphics::Engine3D::FBOtextureMap);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Graphics::Engine3D::FBOtextureMap, 0);

        glGenRenderbuffers(1, &FBODepth);
        glBindRenderbuffer(GL_RENDERBUFFER, FBODepth);

        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 1024, 1024);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, FBODepth);
    }

    void RefreshFrame() {
        Gl.w = Studio::SceneWindowSizeX;
        Gl.h = Studio::SceneWindowSizeY;

        glBindRenderbuffer(GL_RENDERBUFFER, FBODepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, Gl.w, Gl.h);

        glBindTexture(GL_TEXTURE_2D, Graphics::Engine3D::FBOtextureMap);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Gl.w, Gl.h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glViewport(0, 0, Gl.w, Gl.h);

        Gl.MouseX -= Studio::SceneCursorOffsetX;
        Gl.MouseY -= Studio::SceneCursorOffsetY;
    }

	void RenderScene() {
        ImVec2 windowsize = ImGui::GetWindowSize();

        ImVec2 pos = ImGui::GetCursorScreenPos();

        ImVec2 available_space = ImVec2(ImGui::GetWindowSize().x + ImGui::GetWindowPos().x, ImGui::GetWindowSize().y);

        ImGui::GetWindowDrawList()->AddImage(
            (void*)Graphics::Engine3D::FBOtextureMap, pos,
            available_space, ImVec2(0, 1), ImVec2(1, 0));

        SceneWindowSizeX = available_space.x - pos.x;
        SceneWindowSizeY = available_space.y - pos.y;

        SceneCursorOffsetX = pos.x;
        SceneCursorOffsetY = pos.y;
	}
}