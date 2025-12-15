#pragma once

#include "3D/Engine3D.h"
#include "2D/Engine2D.h"

namespace Graphics {
    void Initialize() {
        Gl.Initialize();

        Engine3D::Initialize();

        Engine2D::Initialize();

        Texture::Initialize();
    }
}