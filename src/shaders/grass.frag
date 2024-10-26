#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 view;
    mat4 proj;
    vec4 eye;
} camera;

const vec3 lightDir = normalize(vec3(-1.0, -0.2, -1.0));
const vec3 skyCol = vec3(0.1, 0.5, 0.8);

layout(location = 0) in vec3 normal;
layout(location = 1) in vec3 pos;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 baseCol = vec3(0.0, 0.7, 0.3);
    float diffuse = clamp(dot(normal, lightDir), 0.f, 1.f);
    float ambient = 0.3f;

    vec3 rayDir = normalize(camera.eye.xyz - pos);
    vec3 H = normalize(rayDir - lightDir);
    float rim = 1.f - max(0.f, dot(-normal, rayDir));
    rim = 0.5f * rim + 0.5f;
    float sss = max(0.f, dot(rayDir, lightDir));

    float specular = pow(abs(dot(H, normal)), 32);
    vec3 col = baseCol * (diffuse + ambient + specular);
    col += baseCol * sss * rim;
    outColor = vec4(col, 1.f);
    //outColor = vec4(normal * 0.5 + 0.5, 1.f);
}
