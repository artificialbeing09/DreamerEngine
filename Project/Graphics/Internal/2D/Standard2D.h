#pragma once

#include "../3D/Engine3D.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION 
#include "stb/stb_truetype.h"

namespace Graphics::Engine2D {
    Shader Shape2DShader(NULL, NULL);
    GLuint Shape2DSSBO;
    GLuint ShapeBuffer;

    struct Render2DObject_t {
        glm::vec2 Position = glm::vec2(0.0, 0.0);
        glm::vec2 Size = glm::vec2(1.0, 1.0);
        glm::vec4 Color = glm::vec4(0.0, 1.0, 0.0, 1.0);
        uint32_t Texture0 = 0;
        float CornerSize = 0;
        uint32_t Pad[2];
    };

    struct EngineFont {
        stbtt_fontinfo* StbFont = NULL;
        float scale = 0;
        int ascent = 0;
    };

    map<string, EngineFont> Fonts;

	void Initialize() {
        Shape2DShader = Shader(Utils::ReadFile("Engine/Shape2DShader.vert"), Utils::ReadFile("Engine/Shape2DShader.frag"), false);
        Shape2DSSBO = Gl.GenBuffer();

        vector<float> ShapeInterleaved = {
            0, 0,
            0, 1,
            1, 1,
            0, 0,
            1, 0,
            1, 1,
        };

        ShapeBuffer = Gl.GenBuffer();
        Gl.BindArrayBuffer(ShapeBuffer);
        Gl.BufferStaticArrayData(ShapeInterleaved.data(), ShapeInterleaved.size() * sizeof(float));

        Shape2DShader.use();

        GLint samplers[24] = { 0 };

        for (int i = 0; i < 24; i++) {
            samplers[i] = i;
        }
        
        glUniform1iv(Shape2DShader.uniformLocation("Textures"), 24, samplers);
	}

    void CreateFont(string Name, const char* Data) {
        stbtt_fontinfo* info = new stbtt_fontinfo;
        if (!stbtt_InitFont(info, (const unsigned char*)Data, 0))
        {
            printf("failed\n");
        }

        EngineFont Font;
        Font.StbFont = info;

        /* calculate font scaling */
        float scale = stbtt_ScaleForPixelHeight(info, 128);

        int ascent, descent, lineGap;
        stbtt_GetFontVMetrics(info, &ascent, &descent, &lineGap);

        ascent = (int)round(ascent * scale);
        descent = (int)round(descent * scale);

        Font.scale = scale;
        Font.ascent = ascent;

        Fonts[Name] = Font;

        unsigned char* RenderedGlyph = (unsigned char*)malloc(128 * 128);

        if (RenderedGlyph == 0) {
            return;
        }

        for (unsigned int i = 0; i < 256; i++)
        {
            int c_x1, c_y1, c_x2, c_y2;
            stbtt_GetCodepointBitmapBox(info, i, scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);

            memset(RenderedGlyph, 0, 128 * 128);

            stbtt_MakeCodepointBitmap(info, RenderedGlyph, c_x2 - c_x1, c_y2 - c_y1, 128, scale, scale, i);
            Texture::FlipBitmapVertically(RenderedGlyph, 128, 128);

            auto Test = Texture::TextureImage(128, 128, 1, RenderedGlyph);

            auto Converted = Texture::Convert1ChannelTo4(Test, false);

            string FontName = "Font" + Name + to_string(i);

            Texture::AddTexture(FontName, Converted, true);
        }

        free(RenderedGlyph);
    }

    vector<Render2DObject_t> RenderObjects = { };

    void PreRender() {
        Shape2DShader.use();

        Gl.Disable(Gl.DepthTest);
        Gl.Disable(Gl.CullFace);

        Gl.Enable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void RenderObjectsOfType(vector<Render2DObject_t> Objects) {
        PreRender();

        Shape2DShader.setFloat("ratio", ((float)Gl.h / (float)Gl.w));

        Gl.BindShaderStorageBuffer(Shape2DSSBO);
        Gl.ShaderStorageBufferDataVector(Objects);
        Gl.BindShaderStorageBufferBase(Shape2DSSBO, 5); // Binding = 5 must match GLSL

        Gl.BindArrayBuffer(ShapeBuffer);
        Gl.VertexArray(0, 2, Gl.Float, false, 2 * sizeof(float), NULL);

        Gl.BindVertexArray(0);

        glDrawArraysInstanced(Gl.Triangles, 0, 6, (GLsizei)Objects.size());
    }

    void Render() {
        RenderObjectsOfType(RenderObjects);
    }

    enum TextAlignE {
        Left,
        Center,
        Right
    };

    int CalculateTextWidth(string Text, string FontName) {
        int x = 0;

        auto Font = Graphics::Engine2D::Fonts[FontName];

        char* word = (char*)Text.c_str();

        for (int i = 0; i < strlen(word); ++i)
        {
            /* how wide is this character */
            int ax;
            int lsb;
            stbtt_GetCodepointHMetrics(Font.StbFont, word[i], &ax, &lsb);
            
            /* advance x */
            x += (int)roundf(ax * Font.scale);

            /* add kerning */
            int kern;
            kern = stbtt_GetCodepointKernAdvance(Font.StbFont, word[i], word[i + 1]);
            x += (int)roundf(kern * Font.scale);
        }

        return x;
    }

    void OutputText(vector<Render2DObject_t>& TextObjects, string Text, string FontName, TextAlignE Align = Center, glm::vec2 TxPosition = glm::vec2(0.0, 0.0), float CharacterScale = 0.1, glm::vec3 Color = glm::vec3(1.0, 1.0, 1.0), float Transparency = 1.0) {
        auto Font = Graphics::Engine2D::Fonts[FontName];

        char* word = (char*)Text.c_str();

        int x = 0;

        float Width = (float)((CalculateTextWidth(Text, FontName) / 128.0) * CharacterScale);

        glm::vec2 Position = TxPosition;

        if (Align == Center) {
            Position.x -= (Width / 2.0f) * ((float)Gl.h / (float)Gl.w);
        }
        else if (Align == Right) {
            Position.x -= Width * ((float)Gl.h / (float)Gl.w);
        }

        glm::vec4 TextColor = glm::vec4(Color, Transparency);

        int i;
        for (i = 0; i < strlen(word); ++i)
        {
            /* how wide is this character */
            int ax;
            int lsb;
            stbtt_GetCodepointHMetrics(Font.StbFont, word[i], &ax, &lsb);
            /* (Note that each Codepoint call has an alternative Glyph version which caches the work required to lookup the character word[i].) */

            /* get bounding box for character (may be offset to account for chars that dip above or below the line) */
            int c_x1, c_y1, c_x2, c_y2;
            stbtt_GetCodepointBitmapBox(Font.StbFont, word[i], Font.scale, Font.scale, &c_x1, &c_y1, &c_x2, &c_y2);

            /* compute y (different characters have different heights) */
            int y = Font.ascent + c_y1;

            /* render character (stride and offset is important here) */
            float RenderX = (float)x + roundf(lsb * Font.scale);
            float RenderY = (float)y;

            string Lol = "Font" + FontName + to_string((int)word[i]);

            auto Texture = Texture::Textures[Lol];

            glm::vec2 CharacterPosition = {
                (CharacterScale * (RenderX / 128.0) + (CharacterScale / 2.0)),
                (-(CharacterScale * (RenderY / 128.0)))
            };

            CharacterPosition.x *= ((float)Gl.h / (float)Gl.w);

            TextObjects.push_back({
                (CharacterPosition) + Position,
                {CharacterScale * ((float)Gl.h / (float)Gl.w), CharacterScale},
                TextColor,
                Texture
                });

            /* advance x */
            x += (int)roundf(ax * Font.scale);

            /* add kerning */
            int kern;
            kern = stbtt_GetCodepointKernAdvance(Font.StbFont, word[i], word[i + 1]);
            x += (int)roundf(kern * Font.scale);
        }

        return;
    }
}