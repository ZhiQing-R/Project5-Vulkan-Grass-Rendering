#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 view;
    mat4 proj;
    mat4 viewInv;
    mat4 projInv;
    vec4 eye;
} camera;

layout(set = 1, binding = 0) uniform sampler2D colorSampler;

layout(set = 2, binding = 0) uniform Time {
    float deltaTime;
    float totalTime;
} time;

layout(set = 3, binding = 0) uniform sampler2D noiseSampler;


layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;


const vec3 lightDir = normalize(vec3(-1.0, -0.8f, 0.2));



vec3 getRayDir(vec2 fragCoord)
{
	//fragCoord.y = 1.f - fragCoord.y;
	vec4 ndc = vec4(2.0 * fragCoord - 1.f, -1.f, 1.f);
    vec4 dirEye = camera.projInv * ndc;
	dirEye /= dirEye.w;
    dirEye.w = 0;
    vec3 dirWorld = (camera.viewInv * dirEye).xyz;
    return normalize(dirWorld);
}

uint tea(in uint val0, in uint val1)
{
  uint v0 = val0;
  uint v1 = val1;
  uint s0 = 0;

  for(uint n = 0; n < 16; n++)
  {
    s0 += 0x9e3779b9;
    v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
    v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
  }

  return v0;
}

uint initRandom(in uvec2 resolution, in uvec2 screenCoord, in uint frame)
{
  return tea(screenCoord.y * resolution.x + screenCoord.x, frame);
}

uint pcg(inout uint state)
{
  uint prev = state * 747796405u + 2891336453u;
  uint word = ((prev >> ((prev >> 28u) + 4u)) ^ prev) * 277803737u;
  state     = prev;
  return (word >> 22u) ^ word;
}

float rand(inout uint seed)
{
  uint r = pcg(seed);
  return uintBitsToFloat(0x3f800000 | (r >> 9)) - 1.0f;
}

vec2 rand2(inout uint prev)
{
  return vec2(rand(prev), rand(prev));
}


float hash(vec3 p)
{
    p  = fract( p*0.3183099312 +.1 );
	p *= 17.0;
    return fract( p.x*p.y*p.z*(p.x+p.y+p.z) );
}

float noise1( in vec3 x )
{
    vec3 i = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);
	
    return mix(mix(mix( hash(i+vec3(0,0,0)), 
                        hash(i+vec3(1,0,0)),f.x),
                   mix( hash(i+vec3(0,1,0)), 
                        hash(i+vec3(1,1,0)),f.x),f.y),
               mix(mix( hash(i+vec3(0,0,1)), 
                        hash(i+vec3(1,0,1)),f.x),
                   mix( hash(i+vec3(0,1,1)), 
                        hash(i+vec3(1,1,1)),f.x),f.y),f.z);
}

float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);

    float a = texture( noiseSampler, x.xy / 64.0 + (p.z+0.0)*141.71623).x;
    float b = texture( noiseSampler, x.xy / 64.0 + (p.z+1.0)*141.71623).x;
	a = clamp(8.0*(a-0.5) + 0.5, 0.0, 1.0);
	b = clamp(8.0*(b-0.5) + 0.5, 0.0, 1.0);
	return mix( a, b, f.z );
}


const mat3 m = mat3( 0.00,  0.80,  0.60,
                    -0.80,  0.36, -0.48,
                    -0.60, -0.48,  0.64 );

float fbm( vec3 p )
{
    float f;
    f  = 0.5000*noise( p ); p = m*p*2.02;
    f += 0.2500*noise( p ); p = m*p*2.03;
    f += 0.1250*noise( p ); p = m*p*2.01;
    f += 0.0625*noise( p );
    return f;
}

const float cloudHeight = 65.f;
const vec3 cloudDir = vec3(1.0, 0.3, 1.0);
const vec3 sunCol = vec3(0.8,0.55,0.5);
const vec3 skyCol = vec3(0.4,0.55,0.5);

vec4 mapClouds( in vec3 p )
{
	//float d = 2.5-0.1*abs(cloudHeight - p.y);
    p.xz *= 1.5;
    float d = 1.0 - (abs(p.y-cloudHeight)+0.5)/4.0;
	d += 15.1 * (fbm( (p + cloudDir * time.totalTime * 9.f) * 0.02 ) - 0.5f);
	d = clamp( d, 0.0, 1.0 );
	
	vec4 res = vec4( d );

	res.xyz = mix( 0.8*vec3(1.0,0.95,0.8), vec3(0.1), res.x );
	//res.xyz *= 0.65;
	
	return res;
}

vec4 raymarchClouds( in vec3 ro, in vec3 rd, in vec3 bcol, float tmax)
{
	vec4 sum = vec4(0, 0, 0, 0);
	uint randSeed = tea(floatBitsToUint(fragTexCoord.x + time.totalTime), floatBitsToUint(fragTexCoord.y + time.deltaTime));

	float sun = clamp( dot(rd,-lightDir), 0.0, 1.0 );
	float t = 0.2f * rand(randSeed);
	for(int i = 0; i < 16; i++)
	{
		if( sum.w > 0.99 || t > tmax ) break;
		vec3 pos = ro + t*rd;
		vec4 col = mapClouds( pos );

		float distToCloud = (cloudHeight - 10.f - pos.y) / rd.y;
		float dt = max(0.01, max(0.5 * rand(randSeed) * distToCloud, 0.1 * t));
        float sha = 1.f - mapClouds(pos - 1.f * lightDir).w;
	
		col.xyz *= vec3(0.4,0.52,0.6);
		
        col.xyz += vec3(1.0,0.7,0.4)*0.4*pow( sun, 6.0 )*(1.0-col.w);

        col.xyz += sha * sunCol * 5.f;
		
		col.xyz = mix( col.xyz, bcol, 0.95-exp(-0.02*t*t) );
		
		//col.a *= 0.5;
		col.rgb *= col.a;

		sum = sum + col * (1.0 - sum.a);	
		
		t += dt;
	}

	return clamp( sum, 0.0, 1.0 );
}

vec3 volumeLight(in vec3 ro, in vec3 rd, in vec3 bcol) {
    
    vec4 sum = vec4(0,0,0,0);
	vec3 ld = -lightDir;
	uint randSeed = tea(floatBitsToUint(fragTexCoord.x + time.deltaTime) + 16u, floatBitsToUint(fragTexCoord.y + time.totalTime) + 13u);
    
    float s = 10.f;
    float t = 0.;
	vec3 total = vec3(0.0);
    t += s*rand(randSeed);
    
    for (int i=0; i < 8; i++) { // raymarching loop
        vec3 p = ro + rd*t; // current point
        float h = 0.1; // density of the fog

        float distToCloud = (cloudHeight - 5.f - p.y) / ld.y;
		vec3 cloudP = p + distToCloud * ld;
        float sha = 1.f - mapClouds(cloudP).w;
		sha = 3.0 * (sha - 0.3);
                  
        // coloring
        vec3 col = sunCol*sha;
		total += col;
            
        sum.rgb += h*s*exp(sum.a)*col; // add the color to the final result
        sum.a += -h*s; // beer's law
        t += s;
    }
    total = max(total, 0.0);
	return 0.6 * total / 90.f + 0.1 * skyCol;
    return sum.xyz;
}

vec3 ACES(vec3 color)
{
	color = color * (color * 2.51f + 0.03f) / (color * (color * 2.43f + 0.59f) + 0.14f);
    color = 1.3 * (color - 0.5) + 0.5;
    return color;
}


void main()
{
    vec4 color = texture(colorSampler, fragTexCoord);
	vec3 rd = getRayDir(fragTexCoord);
	vec3 col = vec3(0.84,0.95,1.0)*0.77 - rd.y*0.6;
	col *= 0.55;
	float sun = clamp( dot(rd,-lightDir), 0.0, 1.0 );
    col += vec3(1.0,0.7,0.3)*0.3*pow( sun, 6.0 );
	vec3 bcol = col;

	if (color.a == 1.f)
	{
	    col = volumeLight(camera.eye.xyz, rd, bcol);
        col += color.rgb;
        col = ACES(col);
		outColor = vec4(col, 1.f);
		return;
	}

	col += sunCol*0.35*pow( sun, 3.0 );
	vec4 res = raymarchClouds( camera.eye.xyz, rd, bcol, 1000.f);
	col = col*(1.0-res.w) + res.xyz;
	col += volumeLight(camera.eye.xyz, rd, bcol);
    
    col = ACES(col);

//	col = clamp( col, 0.0, 1.0 );
//	col = pow( col, vec3(0.45) );
//	col = col*0.1 + 0.9*col*col*(3.0-2.0*col);
//	col = mix( col, vec3(col.x+col.y+col.z)*0.33, 0.2 );
//	col *= vec3(1.06,1.05,1.0);
    
    outColor = vec4(col, 1.f);
	//outColor = color;
}