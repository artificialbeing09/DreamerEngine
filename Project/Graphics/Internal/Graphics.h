#pragma once

#include "3D/Engine3D.h"
#include "2D/Engine2D.h"

namespace Graphics {
    void ImportEngineObjects() {
        Texture::GenerateEngineTextures();

        string Directory = "Engine/";

        for (auto S : Utils::Files::get_descendants(Directory)) {
            string Str = S.string();
            string TextureName = (Str.c_str() + Directory.size());

            for (int I = 0; I < TextureName.size(); I++)
                if (TextureName[I] == '\\')
                    TextureName[I] = '/';

            // File extension doesn't matter
            for (int I = 0; I < 4; I++)
                TextureName.pop_back();

            if (Str.ends_with(".obj"))
                Graphics::Engine3D::CreateMeshVector(TextureName, ObjParser::DefaultParseObj(Utils::ReadFile(Str.c_str())));

            if (Str.ends_with(".ttf"))
                Graphics::Engine2D::CreateFont(TextureName, Utils::ReadFile(Str.c_str()));
        }
    }

    void Initialize() {
        Gl.Initialize();

        Engine3D::Initialize();

        Engine2D::Initialize();

        Texture::Initialize();
    }
}