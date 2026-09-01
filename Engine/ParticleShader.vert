#version 460
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

struct particleType {
	vec3 position;
	uint tex;
    vec4 color;
    float storage0;
    float storage1;
    float scale;
    float pad;
};

layout(std430, binding = 5) buffer InstanceData {
    particleType instances[];
};

out vec2 TexCoords;
flat out vec4 color;
flat out uint selectedTex;

uniform mat4 projection;
uniform float currentTime;
uniform mat4 rotation;

void main()
{
    particleType instance = instances[gl_InstanceID];

    TexCoords = vertex.zw;
    selectedTex = instance.tex;

    vec4 realPos = vec4((vertex.xy - vec2(0.5, 0.5)) * instance.scale, 0.0, 1.0);

    realPos = rotation * realPos;

    realPos += vec4(instance.position, 0.0);

    color = instance.color;

    gl_Position = projection * realPos;

    if (instance.scale == 0.0) {
        gl_Position = vec4(0., 0., 0., -1.);
    }
}