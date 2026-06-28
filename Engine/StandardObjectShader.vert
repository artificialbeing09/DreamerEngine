#version 460

uniform mat4 VP;

layout(location = 1) in vec3 vPos;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec2 aTexCoord;

struct cubeObjectType {
	vec3 position;
	uint tex12;
	vec3 color;
	uint tex34;
	vec3 size;
	uint tex56;
	mat4 rotation;
	float transparency;
	float storage1;
	float storage2;
};

layout(std430, binding = 5) buffer InstanceData {
    cubeObjectType instances[];
};

flat out vec4 color;
out vec4 FragPos;
out vec3 Normal;
out vec3 realNormal;
out vec2 texCoord;
flat out float transpar;
flat out int renderShadows;
out uvec3 tex;

// gl_InstanceID is epic and cool

void main() {
	cubeObjectType instance = instances[gl_InstanceID];

	vec3 realvPos = vPos - vec3(0.5);

	vec3 scaledvPos = realvPos * instance.size;

	mat4 rotation = mat4(instance.rotation);

	vec4 localPos = rotation * vec4(scaledvPos, 1.0);

	vec4 realPos = vec4(localPos.xyz + instance.position, 1.0);

	Normal = normalize(mat3(rotation) * aNormal);
	realNormal = aNormal;
	tex = uvec3(instance.tex12, instance.tex34, instance.tex56);

	renderShadows = int(instance.size.x > 0.0);

	gl_Position = VP * realPos;

	FragPos = realPos;

	texCoord = aTexCoord;

	color = vec4(instance.color, instance.transparency);
}