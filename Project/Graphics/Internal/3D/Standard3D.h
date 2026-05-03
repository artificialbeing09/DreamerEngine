#pragma once

#include "Camera.h"
#include "../Parser/Obj.h"

using namespace std;

struct RenderCubeObject_t {
    // 0 = +X, 1 = -X, 2 = +Y, 3 = -Y, 4 = +Z, 5 = -Z

    glm::vec3 Position = glm::vec3(0.0, 0.0, 0.0);
    uint16_t Texture0 = 0;
    uint16_t Texture1 = 0;
    glm::vec3 Rotation = glm::vec3(0.0, 0.0, 0.0);
    uint16_t Texture2 = 0;
    uint16_t Texture3 = 0;
    glm::vec3 Size = glm::vec3(1.0, 1.0, 1.0);
    uint16_t Texture4 = 0;
    uint16_t Texture5 = 0;
    glm::vec3 Color = glm::vec3(0.0, 0.0, 0.0);
    float Transparency = 0.0;
    float SizeLength = 2;
    float Storage2 = 2;
    float Storage3 = 0;
    float Storage4 = 0;
};

struct PhysicsExtraInformation_t {
    glm::vec3 Velocity = glm::vec3(0.0, 0.0, 0.0);
    glm::vec3 RotationVelocity = glm::vec3(0.0, 0.0, 0.0);

    glm::mat3 InertiaTensorLocal = glm::mat3(1.0);
    glm::mat3 InvInertiaTensorLocal = glm::mat3(1.0);

    glm::mat3 InertiaTensorWorld = glm::mat3(1.0);
    glm::mat3 InvInertiaTensorWorld = glm::mat3(1.0);
    double Mass = 1.0;
    bool Anchored = false;
};

struct LightObject_t {
    glm::vec3 Position = glm::vec3(10.0f, 40.0f, 10.0f);
    glm::vec3 Direction = glm::vec3(0.0f, -1.0f, 0.0f);
    glm::vec3 Color = glm::vec3(1.0f, 1.0f, 1.0f);
    float nearPlane = 1.0f;
    float farPlane = 100.0f;
    float FOV = glm::radians(80.0f); // In radians
};

struct RenderLightObject_t {
    glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);
    glm::vec3 LightDir = glm::vec3(0.0f, -1.0f, 0.0f);
    glm::vec3 LightPos = glm::vec3(0.0f, 0.0f, 0.0f);
    unsigned int shadowMapIndex = 0;
    glm::vec3 Color = glm::vec3(1.0f, 1.0f, 1.0f);
};

struct RenderObjectStore_t {
    void* Storage;
    PhysicsExtraInformation_t Physics;
    RenderCubeObject_t Object;
};

vector<float> CubeInterleaved = {
    0, 0, 0,   -1, 0, 0,   0, 0,
    0, 0, 1,   -1, 0, 0,   1, 0,
    0, 1, 1,   -1, 0, 0,   1, 1,

    0, 0, 0,   -1, 0, 0,   0, 0,
    0, 1, 1,   -1, 0, 0,   1, 1,
    0, 1, 0,   -1, 0, 0,   0, 1,



    1, 1, 0,    0, 0, -1,   0, 1,
    0, 0, 0,    0, 0, -1,   1, 0,
    0, 1, 0,    0, 0, -1,   1, 1,

    1, 1, 0,    0, 0, -1,   0, 1,
    1, 0, 0,    0, 0, -1,   0, 0,
    0, 0, 0,    0, 0, -1,   1, 0,



    1, 0, 1,    0,-1, 0,   1, 1,
    0, 0, 0,    0,-1, 0,   0, 0,
    1, 0, 0,    0,-1, 0,   1, 0,

    1, 0, 1,    0,-1, 0,   1, 1,
    0, 0, 1,    0,-1, 0,   0, 1,
    0, 0, 0,    0,-1, 0,   0, 0,



    1, 0, 0,    1, 0, 0,   1, 0,
    1, 1, 1,    1, 0, 0,   0, 1,
    1, 0, 1,    1, 0, 0,   0, 0,

    1, 1, 1,    1, 0, 0,   0, 1,
    1, 0, 0,    1, 0, 0,   1, 0,
    1, 1, 0,    1, 0, 0,   1, 1,



    1, 1, 1,    0, 1, 0,   1, 0,
    0, 1, 0,    0, 1, 0,   0, 1,
    0, 1, 1,    0, 1, 0,   0, 0,

    1, 1, 1,    0, 1, 0,   1, 0,
    1, 1, 0,    0, 1, 0,   1, 1,
    0, 1, 0,    0, 1, 0,   0, 1,



    1, 1, 1,    0, 0, 1,   1, 1,
    0, 1, 1,    0, 0, 1,   0, 1,
    1, 0, 1,    0, 0, 1,   1, 0,

    0, 1, 1,    0, 0, 1,   0, 1,
    0, 0, 1,    0, 0, 1,   0, 0,
    1, 0, 1,    0, 0, 1,   1, 0,
};

namespace Graphics::Engine3D {
    Shader StandardObjectShader(NULL, NULL);
    Shader ShadowShader(NULL, NULL);
    GLuint StandardObjectSSBO;

    struct StandardObjectMesh {
        GLuint GLBuffer;
        size_t VertexCount;
    };

    map<string, StandardObjectMesh> Meshes; // Modular, can store different new meshes (and can be unloaded if needed)

    StandardObjectMesh GenerateStandardObjectMesh(void* ArrayPtr, size_t ArraySize) {
        GLuint GLBuffer = Gl.GenBuffer();
        Gl.BindArrayBuffer(GLBuffer);
        Gl.BufferStaticArrayData(ArrayPtr, ArraySize * sizeof(float));

        return {
            GLBuffer,
            ArraySize / 6
        };
    }

    void CreateMesh(string Name, void* ArrayPtr, size_t ArraySize) {
        GLuint GLBuffer = Gl.GenBuffer();
        Gl.BindArrayBuffer(GLBuffer);
        Gl.BufferStaticArrayData(ArrayPtr, ArraySize * sizeof(float));

        Meshes[Name] = {
            GLBuffer,
            ArraySize / 6
        };
    }

    template <typename T>
    void CreateMeshVector(string Name, vector<T> vec) {
        /*
            Usage:

            Graphics::CreateMeshVector("Teapot", ObjParser::DefaultParseObj(Utils::ReadFile("Engine/sphere.obj")));
        */

        CreateMesh(Name, vec.data(), vec.size());
    }

    void PreRender() {
        StandardObjectShader.use();

        glClearColor(0.0, 0.5, 1.0, 1.0);

        Gl.PreRender();
        Gl.ClearDepthBuffer();
        Gl.ClearColorBuffer();

        Gl.Enable(Gl.DepthTest);
        Gl.Enable(Gl.CullFace);

        Gl.Enable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void PostRender() { }

    glm::mat4 lightProjection, lightView, lightSpaceMatrix;

    

    struct LightObjectStore_t {
        void* Storage;
        LightObject_t Light;
    };

    map<string, deque<RenderObjectStore_t>> RenderObjects;
    deque<LightObjectStore_t> Lights = {  }; //{glm::vec3(12.0f, 40.0f, 12.0f)}, {}
    RenderLightObject_t RenderLights[20];

    unsigned int depthFBO;
    unsigned int depthMap;
    unsigned int SHADOW_SIZE = 512;

    void RenderShadowPassObjectType(StandardObjectMesh Mesh, glm::mat4 lightSpaceMat, vector<RenderCubeObject_t> Objects) {
        if (Objects.size() <= 0) {
            return;
        }

        ShadowShader.use();
        ShadowShader.setMat4("lightSpaceMatrix", lightSpaceMat);
        ShadowShader.setVec3("cameraPos", Camera::Position);

        Gl.BindShaderStorageBuffer(StandardObjectSSBO);
        Gl.ShaderStorageBufferDataVector(Objects);
        Gl.BindShaderStorageBufferBase(StandardObjectSSBO, 5); // Binding = 5 must match GLSL

        Gl.BindArrayBuffer(Mesh.GLBuffer);
        Gl.VertexArray(1, 3, Gl.Float, false, 8 * sizeof(float), NULL);
        Gl.VertexArray(2, 3, Gl.Float, false, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        Gl.VertexArray(3, 2, Gl.Float, false, 8 * sizeof(float), (void*)(6 * sizeof(float)));

        Gl.BindVertexArray(1);

        glDrawArraysInstanced(Gl.Triangles, 0, (GLsizei)Mesh.VertexCount, (GLsizei)Objects.size());
    }

    void CalculateLights() {
        for (int I = 0; I < Lights.size(); I++) {
            auto& Light = Lights[I].Light;

            glm::vec3 LightDir = glm::normalize(Light.Direction);

            if (LightDir.x == 0.0 && (abs(LightDir.x - LightDir.y) < 0.01 || abs(LightDir.x - LightDir.z) < 0.01))
                Light.Direction += glm::vec3(0.01f, 0.f, 0.f);
            if (LightDir.y == 0.0 && (abs(LightDir.x - LightDir.y) < 0.01 || abs(LightDir.y - LightDir.z) < 0.01))
                Light.Direction += glm::vec3(0.f, 0.01f, 0.f);
            if (LightDir.z == 0.0 && (abs(LightDir.x - LightDir.z) < 0.01 || abs(LightDir.y - LightDir.z) < 0.01))
                Light.Direction += glm::vec3(0.f, 0.f, 0.01f);

            lightProjection = glm::perspective(Light.FOV, 1.0f, Light.nearPlane, Light.farPlane);

            lightView = glm::lookAt(Light.Position,
                Light.Direction + Light.Position,
                glm::vec3(0.0f, 1.0f, 0.0f));

            lightSpaceMatrix = lightProjection * lightView;

            RenderLights[I] = { lightSpaceMatrix, LightDir, Light.Position, (unsigned int)I, Light.Color };
        }
    }

    void RenderShadowPass() {
        ShadowShader.use();

        CalculateLights();

        glViewport(0, 0, SHADOW_SIZE, SHADOW_SIZE);
        glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);

        for (int I = 0; I < Lights.size(); I++) {
            auto& Light = RenderLights[I];

            glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0, I);
            glClear(GL_DEPTH_BUFFER_BIT);

            for (const auto& [key, ObjectList] : RenderObjects) {
                vector<RenderCubeObject_t> ObjectListVector;
                ObjectListVector.reserve(ObjectList.size());

                for (const auto& Obj : ObjectList)
                    ObjectListVector.emplace_back(Obj.Object);

                RenderShadowPassObjectType(Meshes[key], Light.lightSpaceMatrix, ObjectListVector);
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        Gl.PreRender();
    }

    void RenderObjectsOfMesh(StandardObjectMesh Mesh, vector<RenderCubeObject_t> Objects) {
        if (Objects.size() <= 0) {
            return;
        }

        glm::mat4 VP = Camera::CalculateProjection() * Camera::CalculateView();

        StandardObjectShader.use();
        StandardObjectShader.setMat4("VP", VP);
        StandardObjectShader.setVec3("cameraPos", Camera::Position);
        StandardObjectShader.setInt("numberOfLights", Lights.size());

        for (int I = 0; I < Lights.size(); I++) {
            auto& Light = RenderLights[I];

            StandardObjectShader.setMat4("lightSpaceMatrix[" + to_string(I) + "]", Light.lightSpaceMatrix);
            StandardObjectShader.setVec3("lightPos[" + to_string(I) + "]", Light.LightPos);
            StandardObjectShader.setVec3("lightDir[" + to_string(I) + "]", Light.LightDir);
            StandardObjectShader.setVec3("lightColor[" + to_string(I) + "]", Light.Color);
        }

        glActiveTexture(GL_TEXTURE26);
        glBindTexture(GL_TEXTURE_2D_ARRAY, depthMap);
        StandardObjectShader.setInt("shadowMap", 26);

        Gl.BindShaderStorageBuffer(StandardObjectSSBO);
        Gl.ShaderStorageBufferDataVector(Objects);
        Gl.BindShaderStorageBufferBase(StandardObjectSSBO, 5); // Binding = 5 must match GLSL

        Gl.BindArrayBuffer(Mesh.GLBuffer);
        Gl.VertexArray(1, 3, Gl.Float, false, 8 * sizeof(float), NULL);
        Gl.VertexArray(2, 3, Gl.Float, false, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        Gl.VertexArray(3, 2, Gl.Float, false, 8 * sizeof(float), (void*)(6 * sizeof(float)));

        Gl.BindVertexArray(1);

        glDrawArraysInstanced(Gl.Triangles, 0, (GLsizei)Mesh.VertexCount, (GLsizei)Objects.size());
    }

    void initShadowMap() { // GPT code, which will soon be completely transformed into normal working code knowing me.
        glGenFramebuffers(1, &depthFBO);

        // Create depth texture array
        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D_ARRAY, depthMap);
        glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT24, SHADOW_SIZE, SHADOW_SIZE, 50);

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);


        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);

        // Optional: depth comparison for shadow sampling
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

        // Framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);

        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "Depth framebuffer not complete!" << std::endl;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Initialize() {
        ShadowShader = Shader(Utils::ReadFile("Engine/StandardObjectShadowShader.vert"), Utils::ReadFile("Engine/StandardObjectShadowShader.frag"), false);
        ShadowShader.use();

        StandardObjectShader = Shader(Utils::ReadFile("Engine/StandardObjectShader.vert"), Utils::ReadFile("Engine/StandardObjectShader.frag"), false);
        StandardObjectShader.use();

        StandardObjectSSBO = Gl.GenBuffer();

        initShadowMap();

        Meshes["Cube"] = GenerateStandardObjectMesh(CubeInterleaved.data(), CubeInterleaved.size());

        GLint samplers[24] = { 0 };

        for (int i = 0; i < 24; i++) {
            samplers[i] = i;
        }

        glUniform1iv(StandardObjectShader.uniformLocation("Textures"), 24, samplers);
    }
}