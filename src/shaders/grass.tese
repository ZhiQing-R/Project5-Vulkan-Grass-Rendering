#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(quads, equal_spacing, ccw) in;

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 view;
    mat4 proj;
} camera;

// TODO: Declare tessellation evaluation shader inputs and outputs
layout(location = 0) in vec4 te_v0[];
layout(location = 1) in vec4 te_v1[];
layout(location = 2) in vec4 te_v2[];
layout(location = 3) in vec4 te_up[];

layout(location = 0) out vec3 normal;
layout(location = 1) out vec3 pos;

const float curvature = 0.3f;

vec3 slerp(in vec3 a, in vec3 b, in float t)
{
	float dot = dot(a, b);
	dot = clamp(dot, -1.f, 1.f);
	float theta = acos(dot) * t;
	vec3 relative = normalize(b - a * dot);
	return a * cos(theta) + relative * sin(theta);
}

void main() {
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

	float angle = te_v0[0].w;
    float height = te_v1[0].w;
    float width = te_v2[0].w;

    vec3 v0 = te_v0[0].xyz;
    vec3 v1 = te_v1[0].xyz;
    vec3 v2 = te_v2[0].xyz;

    vec3 a = v0 + v * (v1 - v0);
    vec3 b = v1 + v * (v2 - v1);
    vec3 c = a + v * (b - a);

     vec3 dir = vec3(cos(angle), 0, sin(angle));
     vec3 tangent = normalize(b - a);
     normal = normalize(cross(tangent, dir));
     vec3 camFwd = normalize(vec3(camera.view[0].z, camera.view[1].z, camera.view[2].z));
     vec3 right;
     if (dot(camFwd, normal) > 0.f)
     {
		normal = -normal;
        tangent = -tangent;
	 }

     right = cross(tangent, normal);
     a = slerp(normal, -right, curvature);
     b = reflect(-a, normal);
     normal = normalize(slerp(a, b, u));
     //normal = normalize(slerp(normal, vec3(0,1,0), v));
     

     vec3 c0 = c - dir * width;
     vec3 c1 = c + dir * width;

     //float t = u;
     float t = u + 0.5f * v - u * v;

     pos = mix(c0, c1, t);
     gl_Position = camera.proj * camera.view * vec4(pos, 1.f);
}
