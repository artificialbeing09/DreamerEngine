#pragma once

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_DXT_IMPLEMENTATION

#include "Shader.h"
#include "stb/stb_image.h"
#include "stb/stb_image_resize2.h"
#include "stb/stb_dxt.h"

namespace Texture {
    class TextureImage {
    public:
        size_t Width = 1;
        size_t Height = 1;
        size_t Channels = 3;
        unsigned char* RawData;
        bool Compressed = false;
        size_t CompressedSize = 0;

        void Delete() {
            stbi_image_free(RawData);
        }

        TextureImage(size_t W, size_t H, size_t nChannels, unsigned char* Data, bool Compressd = false, size_t CompressdSize = 0) {
            Width = W;
            Height = H;
            Channels = nChannels;
            RawData = Data;
            Compressed = Compressd;
            CompressedSize = CompressdSize;
        }
    };

    TextureImage StitchImagesHorizontal(
        TextureImage ImgA,
        TextureImage ImgB
    ) {
        if (ImgA.Channels != ImgB.Channels) {
            cout << "wuh oh mommy didnt like that" << endl;

            return TextureImage(0, 0, 0, 0);
        }

        size_t OutWidth = ImgA.Width + ImgB.Width;

        size_t OutHeight = ImgA.Height >= ImgB.Height ? ImgA.Height : ImgB.Height;

        size_t OutChannels = ImgA.Channels;

        size_t ImageSize = (OutWidth * OutHeight * OutChannels);
        unsigned char* Stitched = new unsigned char[ImageSize];

        // Fill with black (in case of padding)
        memset(Stitched, 0, ImageSize);

        // Copy first image
        for (int y = 0; y < ImgA.Height; y++) {
            memcpy(
                Stitched + (y * OutWidth * OutChannels),
                ImgA.RawData + (y * ImgA.Width * OutChannels),
                ImgA.Width * OutChannels
            );
        }

        // Copy second image
        for (int y = 0; y < ImgB.Height; y++) {
            memcpy(
                Stitched + (y * OutWidth * OutChannels) + (ImgA.Width * OutChannels),
                ImgB.RawData + (y * ImgB.Width * OutChannels),
                ImgB.Width * OutChannels
            );
        }

        return TextureImage(OutWidth, OutHeight, OutChannels, Stitched);
    }

    void FlipBitmapVertically(unsigned char* data, int width, int height)
    {
        int stride = width; // 1 byte per pixel if you used stbtt_MakeCodepointBitmap
        for (int y = 0; y < height / 2; ++y)
        {
            unsigned char* row_top = data + y * stride;
            unsigned char* row_bottom = data + (height - 1 - y) * stride;
            for (int x = 0; x < width; ++x)
            {
                unsigned char tmp = row_top[x];
                row_top[x] = row_bottom[x];
                row_bottom[x] = tmp;
            }
        }
    }

    TextureImage Convert1ChannelTo4(TextureImage Image, bool DeleteImage = false) {
        unsigned char* rgba = (unsigned char*)malloc(Image.Height * Image.Width * 4);
        for (int i = 0; i < Image.Width * Image.Height; ++i) {
            unsigned char g = Image.RawData[i];
            rgba[i * 4 + 0] = 255;  // R
            rgba[i * 4 + 1] = 255;  // G
            rgba[i * 4 + 2] = 255;  // B
            rgba[i * 4 + 3] = g; // A (opaque)
        }

        if (DeleteImage)
            Image.Delete();

        return TextureImage(Image.Width, Image.Height, 4, rgba);
    }

    TextureImage ResizeTexture(TextureImage Image, size_t Width, size_t Height, bool DeleteImage = false) {
        unsigned char* ResizedData = new unsigned char[Width * Height * Image.Channels];

        stbir_resize_uint8_srgb(Image.RawData, (int)Image.Width, (int)Image.Height, 0,
            ResizedData, (int)Width, (int)Height, 0,
            (stbir_pixel_layout)Image.Channels);

        if (DeleteImage) {
            Image.Delete();
        }
        
        return TextureImage(Width, Height, Image.Channels, ResizedData);
    }

    TextureImage CompressDXT5(TextureImage Image, bool DeleteImage = false) {
        size_t blocksX = Image.Width / 4;
        size_t blocksY = Image.Height / 4;
        size_t totalBlocks = (size_t)blocksX * blocksY;
        size_t totalSize = totalBlocks * 16; // 16 bytes per DXT5 block
        const uint8_t* src = Image.RawData;

        uint8_t* compressed = (uint8_t*)malloc(totalSize);
        if (!compressed) return TextureImage(0,0,0,0);

        uint8_t block[4 * 4 * 4]; // temporary buffer for a 4x4 RGBA block
        uint8_t* dst = compressed;

        int h = (int)Image.Height;
        int w = (int)Image.Width;

        const int stride = w * 4;

        for (int by = 0; by < h; by += 4) {
            for (int bx = 0; bx < w; bx += 4) {
                uint8_t* bp = block;

                for (int y = 0; y < 4; y++) {
                    int sy = by + y;
                    sy = (sy < h) ? sy : (h - 1);

                    const uint8_t* row = src + sy * stride;

                    for (int x = 0; x < 4; x++) {
                        int sx = bx + x;
                        sx = (sx < w) ? sx : (w - 1);

                        const uint8_t* p = row + sx * 4;
                        bp[0] = p[0];
                        bp[1] = p[1];
                        bp[2] = p[2];
                        bp[3] = p[3];
                        bp += 4;
                    }
                }

                // Compress 4x4 block to DXT5 (bc3)
                stb_compress_dxt_block(dst, block, 1, STB_DXT_NORMAL);
                dst += 16;
            }
        }

        if (DeleteImage) {
            Image.Delete();
        }

        return TextureImage(Image.Width, Image.Height, 4, compressed, true, totalSize);
    }

    struct TextureSpace {
        unsigned int TextureObject = 0;
        size_t Width = 0;
        size_t Height = 0;
        size_t Depth = 0;
    };

    map<int, TextureSpace> AllocatedTextures;

    unsigned int AllocateTextureSpace(int TextureSlot = 0, size_t Width = 512, size_t Height = 512, size_t Depth = 1, size_t Format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT) {
        auto OldTexture = AllocatedTextures[TextureSlot];
        
        glActiveTexture(GL_TEXTURE0 + TextureSlot);

        unsigned int texture1;

        glGenTextures(1, &texture1);
        glBindTexture(GL_TEXTURE_2D_ARRAY, texture1);

        glTexStorage3D(
            GL_TEXTURE_2D_ARRAY,
            1, // mip levels
            (GLenum)Format,
            (GLsizei)Width,
            (GLsizei)Height,
            (GLsizei)Depth
        );

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

        if (OldTexture.Height > 0) {
            glCopyImageSubData(OldTexture.TextureObject, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0,
                texture1, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0,
                (GLsizei)Width, (GLsizei)Height, (GLsizei)OldTexture.Depth);

            glDeleteTextures(1, &OldTexture.TextureObject);
        }

        return texture1;
    }

    map<string, uint16_t> Textures;
    char OccupiedTextures[49152];
    uint16_t TopTexture = 0;

    enum TextureMemoryUseMode {
        Highest,
        UltraHigh,
        High,
        Medium,
        Low,
        UltraLow,
        UltraUltraLow,
        Windows95,
        Headless
    };

    TextureMemoryUseMode PerformanceMode = UltraHigh;
    bool ImageCompression = true;

    uint16_t GetTopTexture(int Offset = 0) {
        for (int i = Offset; i < 49152; i++)
            if (OccupiedTextures[i] == 0)
                return i;

        if (Offset > 0) {
            for (int i = 0; i < Offset; i++)
                if (OccupiedTextures[i] == 0)
                    return i;
        }

        static bool LimitMessageShown = false;

        if (!LimitMessageShown) {
            LimitMessageShown = true;
            cout << "URGENT WARNING: Texture Limit of 49152 has been reached! Please optimize your texture usage. All new textures are being overwritten at texture ID 65535." << endl;

        }
        return 49152;
    }

    pair<size_t, size_t> GetTextureSizeFromTextureSlot(uint16_t Slot) {
        if (PerformanceMode == Highest) {
            if (Slot == 0) {
                return { 2048, 2048 };
            }

            if (Slot == 1) {
                return { 1024, 1024 };
            }

            if (Slot < 4) {
                return { 512, 512 };
            }

            if (Slot < 12) {
                return { 256, 256 };
            }

            return { 128, 128 };
        }
        if (PerformanceMode == UltraHigh) {
            if (Slot == 0) {
                return { 1024, 1024 };
            }

            if (Slot < 3) {
                return { 512, 512 };
            }

            if (Slot < 11) {
                return { 256, 256 };
            }

            return { 128, 128 };
        }

        if (PerformanceMode == High) {
            if (Slot < 2)
                return { 512, 512 };

            if (Slot < 10)
                return { 256, 256 };

            return { 128, 128 };
        }

        if (PerformanceMode == Medium) {
            if (Slot < 1)
                return { 512, 512 };

            if (Slot < 3)
                return { 256, 256 };

            if (Slot < 10)
                return { 128, 128 };

            return { 64, 64 };
        }

        if (PerformanceMode == Low) {
            if (Slot < 2)
                return { 256, 256 };

            if (Slot < 4)
                return { 128, 128 };

            return { 64, 64 };
        }

        if (PerformanceMode == UltraLow)
            return { 64, 64 };

        if (PerformanceMode == UltraUltraLow)
            return { 32, 32 };

        if (PerformanceMode == Windows95)
            return { 16, 16 };

        return { 8, 8 };
    }

    uint16_t GetBestTextureSlotForTextureSize(size_t Width, size_t Height) {
        size_t Square = max(Width, Height);

        uint16_t BestSlot = 0;
        size_t BestSquare = 0;

        for (int i = 0; i < 24; i++) {
            auto Res = GetTextureSizeFromTextureSlot(i);

            if (BestSquare == Res.first) {
                continue;
            }

            if (Res.first == Square) {
                return i;
            }

            if (Res.first > Square) {
                BestSlot = i;
                BestSquare = Res.first;
            }
        }

        return BestSlot;
    }

    size_t TextureSpaceDepthStep = 128;

    void CheckTextureSpace(uint16_t TextureID) {
        uint16_t TextureSlot = (TextureID / 2048);
        uint16_t TextureNum = (TextureID % 2048);

        if (AllocatedTextures[TextureSlot].Depth <= TextureNum) {
            auto TextureSize = GetTextureSizeFromTextureSlot(TextureSlot);

            size_t Depth = ((TextureNum / TextureSpaceDepthStep) + 1) * TextureSpaceDepthStep;

            unsigned int Texture = AllocateTextureSpace(TextureSlot, TextureSize.first, TextureSize.second, Depth, ImageCompression ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_RGBA8);

            AllocatedTextures[TextureSlot] = {
                Texture,
                TextureSize.first,
                TextureSize.second,
                Depth
            };
        }

        glActiveTexture(GL_TEXTURE0 + TextureSlot);
        glBindTexture(GL_TEXTURE_2D_ARRAY, AllocatedTextures[TextureSlot].TextureObject);

    }

    uint16_t AddTexture(string TextureID, TextureImage Image, bool DeleteTex = false, int SlotOffset = 0, bool Overwrite = false) {
        uint16_t SelectedSpot = 0;
        
        if (Overwrite) {
            SelectedSpot = Textures[TextureID];

            if (SelectedSpot == 0)
                Overwrite = false;
        }

        if (!Overwrite) {
            int NewSlotOffset = SlotOffset;

            if (NewSlotOffset == 0)
                NewSlotOffset = GetBestTextureSlotForTextureSize(Image.Width, Image.Height);

            SelectedSpot = GetTopTexture(NewSlotOffset * 2048);
        }

        CheckTextureSpace(SelectedSpot);

        auto TextureSize = GetTextureSizeFromTextureSlot(SelectedSpot / 2048);

        size_t TextureWidth = TextureSize.first;
        size_t TextureHeight = TextureSize.second;

        bool DeleteImage = false;

        if (Image.Width != TextureWidth || Image.Height != TextureHeight) {
            Image = ResizeTexture(Image, TextureWidth, TextureHeight, DeleteTex);
            if (ImageCompression)
                Image = CompressDXT5(Image, true);
        }
        else
            if (ImageCompression)
                Image = CompressDXT5(Image, DeleteTex);
            else if (DeleteTex)
                DeleteImage = true;

        // Counterpart: glGetCompressedTextureSubImage

        if (ImageCompression) {
            glCompressedTexSubImage3D(
                GL_TEXTURE_2D_ARRAY,
                0,
                0, 0, SelectedSpot % 2048,  // x, y, layer index
                (GLsizei)TextureWidth,
                (GLsizei)TextureHeight,
                1, // depth = 1 layer
                GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
                (GLsizei)Image.CompressedSize,
                Image.RawData
            );
        }
        else {
            glTexSubImage3D(
                GL_TEXTURE_2D_ARRAY,
                0,
                0,
                0,
                SelectedSpot % 2048,
                (GLsizei)TextureWidth,
                (GLsizei)TextureHeight,
                1, // depth = 1 layer
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                Image.RawData
            );
        }

        if (DeleteImage) {
            Image.Delete();
        }

        Textures[TextureID] = SelectedSpot;
        OccupiedTextures[SelectedSpot] = 1;

        return SelectedSpot;
    }

    void RemoveTexture(string TextureID) {
        uint16_t Texture = Textures[TextureID];

        OccupiedTextures[Texture] = '\0'; // Clang wanted me to make it \0
    }

    void GenerateEngineTextures() {
        string Directory = "Engine/";

        for (auto S : Utils::Files::get_descendants(Directory)) {
            string Str = S.string();

            if (Str.ends_with(".png")) {
                int width, height, nrChannels;
                stbi_set_flip_vertically_on_load(true);
                unsigned char* data = stbi_load(Str.c_str(), &width, &height, &nrChannels, 0);
                TextureImage Image(width, height, nrChannels, data);

                if (data == NULL) {
                    cout << "Image failed to load" << endl;
                }

                string TextureName = (Str.c_str() + Directory.size());

                for (int I = 0; I < TextureName.size(); I++)
                    if (TextureName[I] == '\\')
                        TextureName[I] = '/';
                
                AddTexture(TextureName, Image, true);
            }
        }
    }

    struct UrlParts {
        std::string scheme;
        std::string host;
        int port = 0;
        std::string path;
    };

    UrlParts parse_url(const std::string& url) {
        UrlParts parts;

        auto scheme_end = url.find("://");
        if (scheme_end == std::string::npos) throw std::runtime_error("Invalid URL");

        parts.scheme = url.substr(0, scheme_end);
        auto host_start = scheme_end + 3;

        auto path_start = url.find('/', host_start);
        if (path_start == std::string::npos) {
            parts.host = url.substr(host_start);
            parts.path = "/";
        }
        else {
            parts.host = url.substr(host_start, path_start - host_start);
            parts.path = url.substr(path_start);
        }

        // Optional: parse port if present in host
        auto colon_pos = parts.host.find(':');
        if (colon_pos != std::string::npos) {
            parts.port = std::stoi(parts.host.substr(colon_pos + 1));
            parts.host = parts.host.substr(0, colon_pos);
        }
        else {
            parts.port = (parts.scheme == "https") ? 443 : 80;
        }

        return parts;
    }

    string GetIDByTexture(uint16_t TextureID) {
        for (auto Index : Textures) {
            if (Index.second == TextureID) {
                return Index.first;
            }
        }

        return "";
    }

    uint16_t GetTextureByID(string TextureID) {
        uint16_t Texture = Textures[TextureID];

        if (!Texture) {
            std::regex url_regex(
                R"(http?:\/\/(www\.)?[-a-zA-Z0-9@:%._+~#=]{1,256}\.[a-zA-Z]{2,}([-a-zA-Z0-9@:%_+.~#?&//=]*))"
            );

            //cout << "Bad Texture: " << TextureID << endl;

            if (regex_match(TextureID, url_regex) == true) {
                size_t TopSize = GetTextureSizeFromTextureSlot(0).first;

                unsigned char* data = (unsigned char*)malloc(TopSize * TopSize * 4);
                TextureImage Image(TopSize, TopSize, 4, data);

                AddTexture(TextureID, Image, true, 0, false);

                //cout << "Yes URL" << endl;

                thread t([] (string TextureID) {
                    
                    string body = "";

                    int width, height, nrChannels;
                    stbi_set_flip_vertically_on_load(true);
                    unsigned char* data = stbi_load_from_memory(
                        reinterpret_cast<const unsigned char*>(body.data()),
                        body.size(),
                        &width, &height, &nrChannels, 0
                    );
                    TextureImage Image(width, height, nrChannels, data);

                    if (data == NULL) {
                        //cout << "Image failed to load" << stbi_failure_reason() << endl;
                        return;
                    }

                    AddTexture(TextureID, Image, true);
                }, TextureID);

                t.detach();
            }

            return 0;
        }

        return Texture;
    }

    void Initialize() {
        size_t TopSize = GetTextureSizeFromTextureSlot(0).first;

        unsigned char* data = (unsigned char*)malloc(TopSize * TopSize * 4);
        TextureImage Image(TopSize, TopSize, 4, data);

        AddTexture("none", Image, false);
        AddTexture("none2", Image, false);
        AddTexture("none3", Image, true);
    }
}