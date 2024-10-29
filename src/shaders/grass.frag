#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 view;
    mat4 proj;
    vec4 eye;
} camera;

const vec3 lightDir = normalize(vec3(-1.0, -0.1, -1.0));
const vec3 skyCol = vec3(0.1, 0.5, 0.8);

layout(location = 0) in vec3 normal;
layout(location = 1) in vec3 pos;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 baseCol = vec3(0.0, 0.7, 0.3);
    float diffuse = clamp(dot(normal, lightDir), 0.f, 1.f);
    diffuse = 0.f;
    float ambient = 0.7;
    float thickness = 0.2 + 1.5 * uv.y;

    vec3 rayDir = normalize(camera.eye.xyz - pos);
    vec3 H = normalize(rayDir - lightDir);
    float rim = 1.f - max(0.f, dot(-normal, rayDir));
    rim = 0.3f * rim + 0.f;
    float sss = max(0.f, dot(rayDir, lightDir));

    float specular = pow(abs(dot(H, normal)), 24);
    vec3 col = baseCol * thickness * (diffuse + ambient) + specular;
    col += baseCol * rim;
    outColor = vec4(col, 1.f);
    //outColor = vec4(normal * 0.5 + 0.5, 1.f);
}
