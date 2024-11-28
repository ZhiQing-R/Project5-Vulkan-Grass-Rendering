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

layout(set = 1, binding = 0) buffer CulledBladesBuffer {
	Blade culledBlades[];
} culledBladesBuffer;

layout(set = 2, binding = 0) uniform Time {
    float deltaTime;
    float totalTime;
} time;

layout(set = 3, binding = 0) uniform sampler2D noiseSampler;


layout(location = 0) in vec4 in_pos;
layout(location = 1) in vec4 in_normal;

layout(location = 0) out vec3 out_normal;
layout(location = 1) out vec3 out_pos;
layout(location = 3) out vec2 out_uv;

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

uint lowbias32(uint x)
{
    x ^= x >> 17;
    x *= 0xed5ad4bbU;
    x ^= x >> 11;
    x *= 0xac4c1b51U;
    x ^= x >> 15;
    x *= 0x31848babU;
    x ^= x >> 14;
    return x;
}

float uintHash(uint seed)
{
    seed = lowbias32(seed);
    return uintBitsToFloat(0x3f800000 | (seed >> 9)) - 1.0f;
}

vec3 perlin2DTex(vec2 p)
{
    p = fract(p);
    vec3 val = texture(noiseSampler, p).rgb;
    return 2.0 * (val - 0.5);
}

mat3 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat3(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c);
}

const mat3 yRot90 = mat3(
	vec3(0, 0, 1),
	vec3(0, 1, 0),
	vec3(-1, 0, 0)
);

vec3 calCurveNormal(in vec3 v0, in vec3 v1, in vec3 v2, in vec3 sNor, float t)
{
    vec3 eT = normalize(v2 - v1);
    vec3 eB = yRot90 * eT;
    vec3 eN = cross(eB, eT);
    if (eN.y < 0.0) eN = -eN;
    return normalize(mix(sNor, eN, t));
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
    vec3 xzOffset = vec3(v2.x - v0.x, 0, v2.z - v0.z) * 0.2;
    //v1 += xzOffset;
    v1.y += height * 0.6;
    v1 += xzOffset;
    v2.y -= height * 0.1;
    v2 += xzOffset;

    float uvy = in_pos.y / 1.6;

    vec3 a = v0 + uvy * (v1 - v0);
    vec3 b = v1 + uvy * (v2 - v1);
    vec3 c = a + uvy * (b - a);

    //angle = clamp(angle, -0.2 * 3.14159265, 0.2 * 3.14159265);

    mat3 rotation = mat3(
		vec3(cos(angle), 0, sin(angle)),
		vec3(0, 1, 0),
		vec3(-sin(angle), 0, cos(angle))
	);

    vec3 oNor = vec3(-sin(angle), 0, cos(angle));
    vec3 cNor = calCurveNormal(v0, v1, v2, oNor, uvy);
    vec3 tangent = normalize(b - a);
    vec3 bitangent = normalize(cross(cNor, tangent));

    vec3 v = cross(oNor, cNor);
    float va = acos(dot(oNor, cNor));
    mat3 rot = rotationMatrix(v, va);

//    vec3 bitangent = vec3(1,0,0);
//    vec3 cnormal = normalize(cross(tangent, bitangent));
//    if (cnormal.y < 0)
//	{
//		cnormal = -cnormal;
//	}
//    bitangent = normalize(cross(cnormal, tangent));
//    mat3 TBN = mat3(tangent, cnormal, dir);
//
//    vec3 normal = normalize(TBN * in_normal.xyz);
//    //normal = cnormal;
//    vec3 camFwd = normalize(vec3(camera.view[0].z, camera.view[1].z, camera.view[2].z));
//
//    if (dot(camFwd, normal) < 0.f)
//    {
//	    normal = -normal;
//	}

    vec3 pos = in_pos.xyz * rotation;
    float xzDist = length(in_pos.xz);
    pos = pos * vec3(height, 1.0, height) + c;
    pos += height * (in_pos.x * bitangent + in_pos.z * cNor);
    if (in_pos.y > 1.0)
    {
        //pos.y -= pow(xzDist * height * 0.9, 2.0) * (in_pos.z) * 8.0;
    }

    // leaf waving
    uint leafID = gl_VertexIndex / 36;
    if (leafID > 0)
    {
        float leafHash = uintHash(leafID);
        float uvy = max(0.0, in_pos.y - 1.0);
        vec3 offset = perlin2DTex(leafHash + in_pos.xz * 0.005 + 0.01 * time.totalTime) * uvy * 15.0;
        pos += offset * vec3(0.1, 1.0, 0.1);
    }
    
    out_normal = rot * vec3(in_normal);
    out_pos = pos.xyz;
    out_uv = in_pos.xy;

    gl_Position = camera.proj * camera.view * vec4(pos, 1.f);
}