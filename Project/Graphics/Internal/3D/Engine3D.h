#pragma once

#include "Standard3D.h"
#include "Sky.h"

namespace Graphics::Engine3D {
    map<string, vector<RenderCubeObject_t>> FilteredRenderObjects = { };
    map<string, vector<RenderCubeObject_t>> FilteredTransparentRenderObjects = { };

    void MidCalculations() {
        auto Planes = Camera::ExtractFrustumPlanes();

        for (const auto& [key, ObjectList] : RenderObjects) {
            glm::vec3 CameraPos = Camera::Position;

            vector<RenderCubeObject_t>& Visible = FilteredRenderObjects[key];
            vector<RenderCubeObject_t>& Opaque = FilteredTransparentRenderObjects[key];

            Visible.clear();
            Opaque.clear();

            if (Visible.max_size() < ObjectList.size())
                Visible.reserve(ObjectList.size());

            if (Opaque.max_size() < ObjectList.size())
                Opaque.reserve(ObjectList.size());
            
            for (auto& Lp : ObjectList) {
                auto L = Lp.Object;

                if (!Camera::IsSphereVisible(L.Position, L.SizeLength, Planes)) {
                    L.CameraDist = 0.0;
                    continue;
                }

                if (L.Transparency < 0.99f) {
                    glm::vec3 toCamera = L.Position - CameraPos;
                    float distSq = glm::dot(toCamera, toCamera);
                    L.CameraDist = distSq;

                    Visible.push_back(L);
                }
                else {
                    Opaque.push_back(L);
                }
            }

            std::sort(Visible.begin(), Visible.end(),
                [](const auto& a, const auto& b) { return a.CameraDist > b.CameraDist; });
        }
    }

	void Render() {
        thread TC(MidCalculations);

        PreRender();

        RenderShadowPass();

        /*Skybox*/ {
            uint16_t Texture[6] = { 0 };

            for (int I = 0; I < 6; I++) {
                string TextureID = Graphics::Sky::SkyTexture[I];

                Texture[I] = Texture::GetTextureByID(TextureID);
            }

            RenderCubeObject_t Skybox;
            
            Skybox.Position = Camera::Position;
            Skybox.Texture0 = Texture[0];
            Skybox.Texture1 = Texture[1];
            Skybox.Size = glm::vec3(-100.0, -100.0, -100.0);
            Skybox.Texture2 = Texture[2];
            Skybox.Texture3 = Texture[3];
            Skybox.Texture4 = Texture[4];
            Skybox.Texture5 = Texture[5];
            RenderObjectsOfMesh(Meshes["Cube"], { Skybox });

            glClear(GL_DEPTH_BUFFER_BIT);
        }

        /*3D*/ {
            TC.join();

            for (const auto& [key, ObjectList] : FilteredRenderObjects) {
                RenderObjectsOfMesh(Meshes[key], ObjectList);
            }

            glDepthMask(GL_FALSE);
            for (const auto& [key, ObjectList] : FilteredTransparentRenderObjects) {
                RenderObjectsOfMesh(Meshes[key], ObjectList);
            }
            glDepthMask(GL_TRUE);
        }

        PostRender();
	}
}