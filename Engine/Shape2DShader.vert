#version 460
layout (location = 0) in vec2 aPos;

struct renderObject {
	vec2 position;
	vec2 size;
    vec4 color;
    uint tex;
    float cornerSize;
    uint pad[2];
};

layout(std430, binding = 5) buffer InstanceData {
    renderObject instances[];
};

out vec4 color;
out uint tex;
out vec2 texCoord;
out float transpar;
out float cornerSize;
out vec2 size;

void main()
{
    renderObject instance = instances[gl_InstanceID];

    gl_Position = vec4((((aPos - 0.5) * instance.size) + (instance.position - 0.5)) * 2.0, 0.0, 1.0);
    color = instance.color;
    texCoord = aPos;
    tex = instance.tex;
    cornerSize = min(min(instance.cornerSize, instance.size.x * 0.5), instance.size.y * 0.5);
    size = instance.size;
}