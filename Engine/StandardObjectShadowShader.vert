#version 460

uniform mat4 lightSpaceMatrix;

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
	float pad;
};

layout(std430, binding = 5) buffer InstanceData {
    cubeObjectType instances[];
};


// gl_InstanceID is epic and cool

void main() {
	cubeObjectType instance = instances[gl_InstanceID];

	vec3 realvPos = vPos - vec3(0.5);

	vec3 scaledvPos = realvPos * instance.size;

	mat4 rotation = mat4(instance.rotation);

	vec4 localPos = rotation * vec4(scaledvPos, 1.0);

	vec4 realPos = vec4(localPos.xyz + instance.position, 1.0);

	gl_Position = lightSpaceMatrix * realPos;
}