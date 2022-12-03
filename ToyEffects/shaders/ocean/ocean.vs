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
const float amplitude = 0.05;

float Height()
{
	float component1 = sin(2.0 * PI * uvScroll + (pos.x * 16.0)) * amplitude;
	float component2 = sin(2.0 * PI * uvScroll + (pos.y * pos.x * 8.0)) * amplitude;
	return component1 + component2;
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