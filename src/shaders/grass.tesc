#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(vertices = 4) out;

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 view;
    mat4 proj;
    vec4 eye;
} camera;

in gl_PerVertex
{
    vec4 gl_Position;
} gl_in[gl_MaxPatchVertices];

layout(location = 0) in vec4 tc_v0[];
layout(location = 1) in vec4 tc_v1[];
layout(location = 2) in vec4 tc_v2[];
layout(location = 3) in vec4 tc_up[];

layout(location = 0) out vec4 te_v0[];
layout(location = 1) out vec4 te_v1[];
layout(location = 2) out vec4 te_v2[];
layout(location = 3) out vec4 te_up[];

void main() {
	// Don't move the origin location of the patch
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

	te_v0[gl_InvocationID] = tc_v0[gl_InvocationID];
    te_v1[gl_InvocationID] = tc_v1[gl_InvocationID];
    te_v2[gl_InvocationID] = tc_v2[gl_InvocationID];
    te_up[gl_InvocationID] = tc_up[gl_InvocationID];

    vec3 pos = tc_v0[gl_InvocationID].xyz;
    float dist = length(pos - camera.eye.xyz);
    float lod = clamp(floor(12.0 - dist), 2.0, 12.0);

    gl_TessLevelInner[0] = 1;
    gl_TessLevelInner[1] = lod;
    gl_TessLevelOuter[0] = lod;
    gl_TessLevelOuter[1] = 1;
    gl_TessLevelOuter[2] = lod;
    gl_TessLevelOuter[3] = 1;
}
