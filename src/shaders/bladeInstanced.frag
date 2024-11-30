#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 view;
    mat4 proj;
    mat4 viewInv;
    mat4 projInv;
    vec4 eye;
} camera;



layout(location = 0) in vec3 normal;
layout(location = 1) in vec3 pos;
layout(location = 2) in vec3 center;
layout(location = 3) in vec3 uv;

layout(location = 0) out vec4 outColor;

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

vec3 normalFromTerrain(float x, float y)
{
    // sample the height map
    float eps  = 0.01f;
    float fx0 = terrainHeight(vec2(x-eps,y)), fx1 = terrainHeight(vec2(x+eps,y));
    float fy0 = terrainHeight(vec2(x,y-eps)), fy1 = terrainHeight(vec2(x,y+eps));

    vec3 n = normalize(vec3((fx0 - fx1)/(1.0*eps), 1, (fy0 - fy1)/(1.0*eps)));
    return n;
}

const vec3 lightDir = normalize(vec3(-1.0, -0.8f, 0.2));
const vec3 sunCol = vec3(0.8,0.55,0.6);
const vec3 skyCol = 1.2 * vec3(0.81,0.665,0.45);
const float curvature = 0.3f;

void main() {
    float se = curvature * 2.f * abs(uv.x - 0.5f);
    se = sqrt(1.f - se * se);
    vec3 bPos = pos + normal * se * uv.z;
    vec3 nor = normalize(bPos - center);
    vec3 terrainNor = normalFromTerrain(pos.x, pos.z);

    vec3 baseCol = vec3(0.24, 0.45, 0.33) * 0.6;
    float terrainDiffuse = clamp(dot(terrainNor, -lightDir), 0.f, 1.f);
    float diffuse = clamp(dot(nor, -lightDir), 0.f, 1.f) * terrainDiffuse;
    vec3 ambient = 0.95 * mix(skyCol, baseCol, 0.5);
    float thickness = pow(0.2 + 0.8 * uv.y, 2.0);

    vec3 rayDir = normalize(camera.eye.xyz - pos);
    vec3 H = normalize(rayDir - lightDir);
    float rim = 1.f - max(0.f, dot(-nor, rayDir));
    rim = 0.1f * rim + 0.f;
    float sss = max(0.f, dot(rayDir, lightDir));

    float specular = pow(max(0.f, abs(dot(H, nor))), 32) * terrainDiffuse * 0.6;
    vec3 col = baseCol * thickness * (diffuse + ambient) + specular;
    col += baseCol * rim;
    col += (sss * thickness * 0.2) * mix(sunCol, baseCol, 0.5);
    outColor = vec4(col, 1.f);
    //outColor = vec4(nor * 0.5 + 0.5, 1.f);
    //outColor = vec4(uv.x, uv.y, 0.f, 1.f);
    //outColor = vec4(vec3(terrainDiffuse), 1.f);
}