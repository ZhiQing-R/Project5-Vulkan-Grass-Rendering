
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform ModelBufferObject {
    mat4 model;
};

layout(location = 0) in vec4 v0;
layout(location = 1) in vec4 v1;
layout(location = 2) in vec4 v2;
layout(location = 3) in vec4 up;

layout(location = 0) out vec4 tc_v0;
layout(location = 1) out vec4 tc_v1;
layout(location = 2) out vec4 tc_v2;
layout(location = 3) out vec4 tc_up;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = model * vec4(v0.xyz, 1.f);
	tc_v0 = model * v0;
	tc_v1 = model * v1;
	tc_v2 = model * v2;
	tc_up = model * up;
}
