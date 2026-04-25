#version 460

flat in vec4 color;
flat in float transparency;
in vec4 FragPos;
in vec3 Normal;
out vec4 fragment;


in vec3 realNormal;
in vec2 texCoord;
in flat uvec3 tex;
in flat int renderShadows;

uniform vec3 cameraPos;

uniform sampler2DArray Materials;
uniform sampler2DArray Textures[24];

uniform sampler2DArrayShadow shadowMap;

uniform vec3 lightPos[50];
uniform vec3 lightDir[50];
uniform vec3 lightColor[50];
uniform mat4 lightSpaceMatrix[50];
uniform int numberOfLights;

vec2 poissonDisk[16] = vec2[](
vec2(-0.94201624, -0.39906216),
vec2(0.94558609, -0.76890725),
vec2(-0.094184101, -0.92938870),
vec2(0.34495938, 0.29387760),
vec2(-0.91588581, 0.45771432),
vec2(-0.81544232, -0.87912464),
vec2(-0.38277543, 0.27676845),
vec2(0.97484398, 0.75648379),
vec2(0.44323325, -0.97511554),
vec2(0.53742981, -0.47373420),
vec2(-0.26496911, -0.41893023),
vec2(0.79197514, 0.19090188),
vec2(-0.24188840, 0.99706507),
vec2(-0.81409955, 0.91437590),
vec2(0.19984126, 0.78641367),
vec2(0.14383161, -0.14100790)
);

float getVisibility(mat4 lightSpaceMatrix, vec3 lightDir, int shadowIndex) { 
    //https://github.com/iamkroot/shadow-mapping/blob/master/shaders/standard_frag.glsl
    
    vec4 fragPosLightSpace = lightSpaceMatrix * FragPos;
    
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    bool outside =
        projCoords.x < -1.0 || projCoords.x > 1.0 ||
        projCoords.y < -1.0 || projCoords.y > 1.0 ||
        projCoords.z <  0.0 || projCoords.z > 1.0;

    if (outside) {
        return 0.0f;
    }
    
    // normalize to [0,1] range
    // full formula: (((far-near) * coord) + near + far) / 2.0
    // we have far = 1, near = 0
    projCoords = projCoords * 0.5 + 0.5;

    // declare a bias to deal with shadow acne
    float cosTheta = clamp(dot(Normal, lightDir), 0.0, 1.0);
    float bias = clamp(0.0005 * tan(acos(cosTheta)), 0.0, 0.01);
    projCoords.z -= bias;
    float visibility = 1.0;
    float spreadParam = 500.0;
    for (int i = 0; i < 16; i++) {
        if (texture(shadowMap, vec4(projCoords.xy + poissonDisk[i] / spreadParam, shadowIndex, projCoords.z)).r < projCoords.z){
            visibility -= 0.05;
        }
    }
    return visibility;
}

vec3 calculateGlobalLight(vec3 lightDir, vec3 lightColor) {
    float ambientStrength = 0.5;
    vec3 ambient = ambientStrength * lightColor;

    float diff = max(dot(Normal, lightDir), 0.0) * 0.5;
    vec3 diffuse = diff * lightColor;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(cameraPos - vec3(FragPos));
    vec3 reflectDir = reflect(-lightDir, Normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 overalllighting = ambient + (diffuse + specular);

    return overalllighting;
}

vec3 calculateLightingWithShadowMultiplier(mat4 lightSpaceMatrix, vec3 lightDir, int shadowIndex, vec3 lightColor) {
    float ambientStrength = 0.5;
    vec3 ambient = ambientStrength * lightColor;

    float shadow = getVisibility(lightSpaceMatrix, lightDir, shadowIndex);

    float diff = max(dot(Normal, lightDir), 0.0) * 0.5;
    vec3 diffuse = diff * lightColor;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(cameraPos - vec3(FragPos));
    vec3 reflectDir = reflect(-lightDir, Normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 overalllighting = ((diffuse + specular) * shadow);

    return overalllighting;
}

void main() {
    uint tex[6] = { 
        tex.x & 0xFFFFu,
        (tex.x >> 16) & 0xFFFFu,
        tex.y & 0xFFFFu,
        (tex.y >> 16) & 0xFFFFu,
        tex.z & 0xFFFFu,
        (tex.z >> 16) & 0xFFFFu
    };

    vec3 n = normalize(realNormal);

    vec3 absN = abs(n);

    int dir = 0; // 0 = +X, 1 = -X, 2 = +Y, 3 = -Y, 4 = +Z, 5 = -Z

    if (absN.x >= absN.y && absN.x >= absN.z) {
        dir = (n.x >= 0.0) ? 0 : 1;
    }
    else if (absN.y >= absN.x && absN.y >= absN.z) {
        dir = (n.y >= 0.0) ? 2 : 3;
    }
    else {
        dir = (n.z >= 0.0) ? 4 : 5;
    }

    uint selectedTexture = tex[dir];
    vec4 image = color;

    if (selectedTexture != 0) {
        uint samplerNumber = int(selectedTexture / 2048);
        uint samplerTexture = selectedTexture % 2048;
        image = texture(Textures[samplerNumber], vec3(texCoord, samplerTexture));
    }

    if (renderShadows == 0) {
        fragment = image;
        return;
    }

    vec3 globalLightColor = vec3(1.0, 1.0, 1.0);
    
    vec3 overalllighting = calculateGlobalLight(normalize(-vec3(-0.5f, -1.0f, -1.0f)), globalLightColor);

    for (int i = 0; i < numberOfLights; i++) {
        overalllighting += calculateLightingWithShadowMultiplier(lightSpaceMatrix[i], normalize(-lightDir[i]), i, lightColor[i]) * 0.5;
    }
    
    float t = image.w + ((1.0 - image.w) * color.w);
    vec3 mixedColor = ((image.xyz * image.w) + (color.xyz * (1.0 - image.w) * color.a));
    
    vec4 result = vec4(overalllighting * mixedColor, t);

    if (color.w > 0.99) {
        result.w = 1.0;
    }

    fragment = result;
}