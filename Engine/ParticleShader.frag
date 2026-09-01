#version 460 core


in vec2 TexCoords;
flat in vec4 color;
flat in uint selectedTex;
out vec4 fragment;

uniform sampler2DArray Textures[24];

void main()
{
    uint tex = selectedTex;

    fragment = color;

    if (tex != 0) {
        uint samplerNumber = int(tex / 2048);
        uint samplerTexture = tex % 2048;
        fragment = texture(Textures[samplerNumber], vec3(TexCoords, samplerTexture));
        fragment *= color;
    }
}  