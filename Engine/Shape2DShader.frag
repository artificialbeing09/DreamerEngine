#version 460

in vec4 color;
out vec4 FragColor;
flat in uint tex;
in vec2 texCoord;
flat in float cornerSize;
in vec2 size;
uniform float ratio;
uniform sampler2DArray Textures[24];

void main()
{
    FragColor = color;

    if (cornerSize > 0) {
        vec2 objectStretch = vec2((size.x / ratio), size.y);
        vec2 cornerRadius = vec2(cornerSize, cornerSize) / objectStretch;
        vec2 distFromCenter = 0.5 - texCoord;

        if (abs(distFromCenter.x) >= (0.5 - cornerRadius.x) && abs(distFromCenter.y) >= (0.5 - cornerRadius.y)) {
            vec2 spot = vec2(
                distFromCenter.x < 0 ? (1.0 - cornerRadius.x) : cornerRadius.x, 
                distFromCenter.y < 0 ? (1.0 - cornerRadius.y) : cornerRadius.y);

            if (length((texCoord - spot) * objectStretch) > cornerSize) {
                FragColor.w = 0.0;
            }
        }
    }

    if (tex > 0 && FragColor.w > 0.0)
        FragColor *= texture(Textures[tex / 2048], vec3(texCoord, tex % 2048));
}