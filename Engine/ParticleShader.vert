#version 460
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

struct particleType {
	vec3 position;
	uint tex;
    vec4 color;
    vec3 velocity;
    float timeStart;
    vec3 acceleration;
    float timeEnd;
    float lifeTime;
    float rateOffset;
    float storage0;
    float storage1;
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

    float scale = 1.0f;
    TexCoords = vertex.zw;
    color = instance.color;
    selectedTex = instance.tex;
    float relativeTime = (currentTime - instance.timeStart) + instance.rateOffset;

    float t = mod(relativeTime, instance.lifeTime);
    vec3 newPos = instance.position + (instance.velocity * t) + (instance.acceleration * 0.5 * t*t);

    vec4 realPos = vec4((vertex.xy - vec2(0.5, 0.5)) * scale, 0.0, 1.0);

    realPos = rotation * realPos;

    realPos += vec4(newPos, 0.0);

    gl_Position = projection * realPos;

    if (relativeTime < 0.0) {
        gl_Position = vec4(0., 0., 0., -1.);
    }
}