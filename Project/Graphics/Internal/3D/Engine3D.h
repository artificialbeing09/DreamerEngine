#pragma once

#include "Standard3D.h"
#include "Sky.h"

namespace Graphics::Engine3D {
	void Render() {
        PreRender();

        RenderShadowPass();

		static map<string, vector<RenderCubeObject_t>> FilteredRenderObjects = { };
		static map<string, vector<RenderCubeObject_t>> FilteredTransparentRenderObjects = { };

        /*Skybox*/ {
            uint16_t Texture[6] = { 0 };

            for (int I = 0; I < 6; I++) {
                string TextureID = Graphics::Sky::SkyTexture[I];

                Texture[I] = Texture::GetTextureByID(TextureID);
            }

            RenderCubeObject_t Skybox = {
               Camera::Position,
               Texture[0], Texture[1],
               {3.14, 0.0, 0.0},
               Texture[2], Texture[3],
               {-100.0, -100.0, -100.0},
               Texture[4], Texture[5],
               {0.0, 0.0, 0.0},
               1.0f
            };

            RenderObjectsOfMesh(Meshes["Cube"], { Skybox });

            glClear(GL_DEPTH_BUFFER_BIT);
        }

        /*3D*/ {
            auto Planes = Camera::ExtractFrustumPlanes();

            for (const auto& [key, ObjectList] : RenderObjects) {
                vector<pair<float, RenderCubeObject_t>> Visible;
                vector<RenderCubeObject_t> Opaque;

                Visible.reserve(ObjectList.size());
                Opaque.reserve(ObjectList.size());

                for (const auto& Lp : ObjectList) {
                    auto L = Lp.Object;

                    if (Camera::IsSphereVisible(L.Position, glm::length(L.Size) * 0.5f, Planes)) {
                        if (L.Transparency < 0.99f) {
                            glm::vec3 toCamera = L.Position - Camera::Position;
                            float distSq = glm::dot(toCamera, toCamera);
                            Visible.emplace_back(distSq, L);
                        }
                        else {
                            Opaque.emplace_back(L);
                        }
                    }
                }

                std::sort(Visible.begin(), Visible.end(),
                    [](const auto& a, const auto& b) { return a.first > b.first; });

                FilteredRenderObjects[key] = move(Opaque);
                
                {
                    auto& TRenderList = FilteredTransparentRenderObjects[key];
                    TRenderList.clear();
                    TRenderList.reserve(Visible.size());

                    for (auto& [dist, obj] : Visible)
                        TRenderList.push_back(obj);
                }
            }

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