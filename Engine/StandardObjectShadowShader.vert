#version 460

uniform mat4 lightSpaceMatrix;

layout(location = 1) in vec3 vPos;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec2 aTexCoord;

struct cubeObjectType {
	vec3 position;
	uint tex12;
	vec3 rotation;
	uint tex34;
	vec3 size;
	uint tex56;
	vec3 color;
	float transparency;
};

layout(std430, binding = 5) buffer InstanceData {
    cubeObjectType instances[];
};

mat4 rotationX(float angle) {
  float s = sin(angle);
  float c = cos(angle);
  return mat4(
    1.0, 0.0, 0.0, 0.0,
    0.0, c,   s,  0.0,
    0.0, -s,   c,   0.0,
    0.0, 0.0, 0.0, 1.0
  );
}

mat4 rotationY(float angle) {
  float s = sin(angle);
  float c = cos(angle);
  return mat4(
    c,    0.0, -s,    0.0,
    0.0,  1.0, 0.0,  0.0,
    s,   0.0, c,    0.0,
    0.0,  0.0, 0.0,  1.0
  );
}

mat4 rotationZ(float angle) {
  float s = sin(angle);
  float c = cos(angle);
  return mat4(
    c,    s,   0.0,  0.0,
    -s,    c,    0.0,  0.0,
    0.0,  0.0,  1.0,  0.0,
    0.0,  0.0,  0.0,  1.0
  );
}

// gl_InstanceID is epic and cool

void main() {
	cubeObjectType instance = instances[gl_InstanceID];

	vec3 realvPos = vPos - vec3(0.5);

	vec3 scaledvPos = realvPos * instance.size;

	mat4 rotation = rotationX(instance.rotation.x) * 
		rotationY(instance.rotation.y) * 
		rotationZ(instance.rotation.z);

	vec4 localPos = rotation * vec4(scaledvPos, 1.0);

	vec4 realPos = vec4(localPos.xyz + instance.position, 1.0);

	gl_Position = lightSpaceMatrix * realPos;
}