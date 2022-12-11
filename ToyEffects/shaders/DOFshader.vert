#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out float v_depth;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = view * model * vec4(aPos, 1.0);

    TexCoords = aTexCoords;    
    v_depth = gl_Position.z;

    gl_Position = projection * gl_Position;
}