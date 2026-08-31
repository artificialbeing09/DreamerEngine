#version 460 core


in vec2 TexCoords;
flat in vec4 color;
flat in uint selectedTex;
out vec4 fragment;

uniform sampler2DArray Textures[24];

void main()
{
    uint tex = selectedTex;

    vec4 image = color;

    if (tex != 0) {
        uint samplerNumber = int(tex / 2048);
        uint samplerTexture = tex % 2048;
        image = texture(Textures[samplerNumber], vec3(TexCoords, samplerTexture));
    }

    float t = image.w + ((1.0 - image.w) * color.w);
    vec3 mixedColor = ((image.xyz * image.w) + (color.xyz * (1.0 - image.w) * color.a));
    
    vec4 result = vec4(mixedColor, t);

    if (color.w > 0.99) {
        result.w = 1.0;
    }

    fragment = result;
}  