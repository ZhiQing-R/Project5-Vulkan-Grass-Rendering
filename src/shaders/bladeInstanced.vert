#version 450
#extension GL_ARB_separate_shader_objects : enable

struct Blade {
    vec4 v0;
    vec4 v1;
    vec4 v2;
    vec4 up;
};

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 view;
    mat4 proj;
    mat4 viewInv;
    mat4 projInv;
    vec4 eye;
} camera;

layout(set = 1, binding = 0) uniform ModelBufferObject {
    mat4 model;
};

layout(set = 2, binding = 0) buffer CulledBladesBuffer {
	Blade culledBlades[];
} culledBladesBuffer;


layout(location = 0) in vec2 uv;

layout(location = 0) out vec3 out_normal;
layout(location = 1) out vec3 out_pos;
layout(location = 2) out vec3 out_center;
layout(location = 3) out vec3 out_uv;

out gl_PerVertex {
    vec4 gl_Position;
};


// ref: https://github.com/ashima/webgl-noise/blob/master/src/noise2D.glsl
vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec2 mod289(vec2 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 permute(vec3 x) {
  return mod289(((x*34.0)+10.0)*x);
}

float snoise(vec2 v)
  {
  const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                      0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                     -0.577350269189626,  // -1.0 + 2.0 * C.x
                      0.024390243902439); // 1.0 / 41.0
// First corner
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);

// Other corners
  vec2 i1;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;

// Permutations
  i = mod289(i); // Avoid truncation effects in permutation
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
		+ i.x + vec3(0.0, i1.x, 1.0 ));

  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;

// Gradients: 41 points uniformly over a line, mapped onto a diamond.
// The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;

// Normalise gradients implicitly by scaling m
// Approximation of: m *= inversesqrt( a0*a0 + h*h );
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

// Compute final noise value at P
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}

float terrainHeight(vec2 v)
{
	return snoise(v * 0.01f) * 10.f;
}


const float curvature = 0.8f;

vec3 slerp(in vec3 a, in vec3 b, in float t)
{
	float dot = dot(a, b);
	dot = clamp(dot, -1.f, 1.f);
	float theta = acos(dot) * t;
	vec3 relative = normalize(b - a * dot);
	return a * cos(theta) + relative * sin(theta);
}

void main()
{

    Blade blade = culledBladesBuffer.culledBlades[gl_InstanceIndex];
    float angle = blade.v0.w;
    float height = blade.v1.w;
    float width = blade.v2.w;

    vec3 v0 = blade.v0.xyz;
    vec3 v1 = blade.v1.xyz;
    vec3 v2 = blade.v2.xyz;

    vec3 a = v0 + uv.y * (v1 - v0);
    vec3 b = v1 + uv.y * (v2 - v1);
    vec3 c = a + uv.y * (b - a);

    vec3 dir = vec3(cos(angle), 0, sin(angle));
    vec3 tangent = normalize(b - a);
    vec3 normal = normalize(cross(dir, tangent));
    vec3 camFwd = normalize(vec3(camera.view[0].z, camera.view[1].z, camera.view[2].z));

    if (dot(camFwd, normal) < 0.f)
    {
	    normal = -normal;
        tangent = -tangent;
	}
//
//    vec3 bentDir = normalize(vec3(v2.x - v0.x, 0.f, v2.z - v0.z));
//    vec3 rotDir = normalize(vec3(-bentDir.y, 0, bentDir.x));
//    float edge = -(uv.x * 2.f - 1.f);
//    rotDir = normalize(rotDir + curvature * edge * bentDir);
//    normal = normalize(cross(rotDir, tangent));

    

    

//    a = mix(normal, -dir, curvature);
//    b = reflect(-a, normal);
//    normal = normalize(mix(a, b, uv.x));
    //normal = normalize(slerp(normal, vec3(0,1,0), v));
    
    vec3 c0 = c - dir * width;
    vec3 c1 = c + dir * width;

    //float t = uv.x;
    float t = uv.x + 0.5f * uv.y - uv.x * uv.y;

    vec4 pos = vec4(mix(c0, c1, t), 1.f);
    pos = model * pos;
    //float terrain = terrainHeight(v0.xz);
    //pos.y += terrain;
    //c.y += terrain;

    out_normal = normal;
    out_pos = pos.xyz;
    out_center = c;
    out_uv = vec3(uv, width);

    gl_Position = camera.proj * camera.view * pos;
}