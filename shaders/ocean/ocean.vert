//#version 330
//
//layout (location = 0) in vec3 pos;
//layout (location = 1) in vec2 tex;
//layout (location = 2) in vec3 norm;
//
//out vec4 vCol;
//out vec2 TexCoord;
//out vec3 Normal;
//out vec3 FragPos;
//
//uniform mat4 model;
//uniform mat4 projection;
//uniform mat4 view;
//
//uniform float uvScroll;
//
//
//const float PI = 3.14159;
//const float amplitude = 0.1 ;
//float k=0;
//float f=0;
//vec3 midpos;
////const float amplitude = 0.03;
//
////计算纵向
//float Height(vec2 d,float wave_length)
//{
//	//float speeded=uvScroll*1.5;
//	//float component1 = sin( 2.0 * PI *speeded  -2*0.707*pos.x-0.707*pos.y) * amplitude;
//	//float component2 = sin(2.0 * PI *speeded -2*pos.x) * amplitude;
//	//float component3 = sin(2.0 * PI *speeded -2*pos.y) * amplitude;
//	//return component1 + component2+component3;
//
//
//	k=(2.0 * PI)/wave_length;
//	float c = sqrt(9.8 / k);
//	f=k*(dot(d,pos.xz)-c*uvScroll);
//	//f=k*(pos.x-c*uvScroll);
//	float h1=amplitude*sin(f);
//	return h1;
//
//}
//
////计算横向
//float Width(vec2 d,float wave_length)
//{
//	float c = sqrt(9.8 / wave_length);
//	float f=k*(dot(d,pos.xz)-4*c*uvScroll);
//	float w1=amplitude*2.0*cos(f);
//	return w1;
//}
//
//vec3 GerstnerWave (vec4 wave, vec3 p, inout vec3 tangent, inout vec3 binormal) {
//    float steepness = wave.z;
//    float wavelength = wave.w;
//    float k = 2 * PI / wavelength;
//    float c = sqrt(9.8 / k);                // _WaveSpeed
//    vec2 d = normalize(wave.xy);
//    float f = k * (dot(d, p.xz) - c * uvScroll);
//    float a = steepness / k;                // _Amplitude
//
//    tangent += vec3(
//    -d.x * d.x * (steepness * sin(f)),
//    d.x * (steepness * cos(f)),
//    -d.x * d.y * (steepness * sin(f))
//    );
//    binormal += vec3(
//    -d.x * d.y * (steepness * sin(f)),
//    d.y * (steepness * cos(f)),
//    -d.y * d.y * (steepness * sin(f))
//    );
//    return vec3(      // 输出顶点偏移量
//    d.x * (a * cos(f)),
//    a * sin(f),
//    d.y * (a * cos(f))
//    );
//}
//
//void addWave()
//{
//	vec4 _WaveA = vec4(0.7,0.3,3.0,10);
//	vec4 _WaveB = vec4(0.6,0.4,3.0,10);
//	vec3 tangent = vec3(1, 0, 0);
//	vec3 binormal = vec3(0, 0, 1);
//// 注意两个wave都是针对于原顶点gridPoint的偏移
//	midpos += GerstnerWave(_WaveA, pos, tangent, binormal);    
//	midpos += GerstnerWave(_WaveB, pos, tangent, binormal);
//
//////波1
////	vec2 d=vec2(0.7,0.3);//权值
////	float height = Height(d,3.0f);
////	float width=Width(d,3.0f);
////	vec4 wave1=vec4(pos.x+width, pos.y+width, height, 1.0);
//////波2
////	d=vec2(0.4,0.6);
////	height = Height(d,2.0f);
////	width=Width(d,2.0f);
////	wave1=vec4(wave1.x+width, wave1.y+width, wave1.z+height, 1.0);
//
//	Normal = normalize(cross(binormal, tangent));
//	gl_Position = projection * view * model * vec4(midpos,1.0);
//}
//
//void main()
//{
//	midpos=pos;
//	addWave();
//
//	vCol = vec4(clamp(midpos, 0.05f, 1.0f), 1.0f);
//	//vCol = vec4(0.7f, 0.7f, 0.7f, 1.0f);
//	
//	vec2 time = vec2(uvScroll, 0.0f);
//	
//	TexCoord = tex + time/2.0f;
//	
//	//修正法线
////	vec3 tangent = normalize(vec3(1 - k * amplitude * sin(f), k * amplitude * cos(f), 0));
////	vec3 norm_changed = vec3(-tangent.y, tangent.x, 0);
////	Normal = mat3(transpose(inverse(model))) * norm;
//	
//	FragPos = (model * vec4(midpos, 1.0)).xyz;
//}

#version 330

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in vec3 norm;

out vec4 vCol;
out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;

uniform float uvScroll;


const float PI = 3.14159;
const float amplitude = 0.05 ;
//const float amplitude = 0.03;

float Height()
{
	//float component1 = sin(2.0 * PI * uvScroll + (pos.x * 16.0)) * amplitude;
	//float component2 = sin(2.0 * PI * uvScroll + (pos.y * pos.x * 8.0)) * amplitude;
	//return component1 + component2;
	float speeded=uvScroll*1.5;
	float component1 = sin( 2.0 * PI *speeded  -2*0.707*pos.x-0.707*pos.y) * amplitude;
	float component2 = sin(2.0 * PI *speeded -2*pos.x) * amplitude;
	float component3 = sin(2.0 * PI *speeded -2*pos.y) * amplitude;
	return component1 + component2+component3;
}

void main()
{

	float height = Height();
	gl_Position = projection * view * model * vec4(pos.x, pos.y, pos.z + height, 1.0);

	vCol = vec4(clamp(pos, 0.05f, 1.0f), 1.0f);
	//vCol = vec4(0.7f, 0.7f, 0.7f, 1.0f);
	
	vec2 time = vec2(uvScroll, 0.0f);
	
	TexCoord = tex + time;
	
	Normal = mat3(transpose(inverse(model))) * norm;
	
	FragPos = (model * vec4(pos, 1.0)).xyz;
}