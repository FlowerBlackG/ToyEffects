#version 450 core
out vec4 FragColor;

in vec2 TexCoords;
in float v_depth;

uniform float near_distance;  // 近平面的模糊衰减范围 10.0
uniform float far_distance;  // 远平面的模糊衰减范围 10.0
uniform float near_plane;    // 近平面 -12.0
uniform float far_plane;     // 远平面 -20.0

uniform sampler2D textureDiffuse1;

void main()
{    
    FragColor = texture2D(textureDiffuse1, TexCoords);
 
    float blur = 0;
 
    // 根据深度计算模糊因子
    if(v_depth <= near_plane && v_depth >= far_plane)
    {
        blur = 0;
    }
    else if(v_depth > near_plane)
    {
        blur = clamp(v_depth, near_plane, near_plane + near_distance);
        blur = (blur - near_plane) / near_distance;
    }
    else if(v_depth < far_plane)
    {
        blur = clamp(v_depth, far_plane - far_distance, far_plane);
        blur = (far_plane - blur) / far_distance;
    }
 
    // 将模糊因子写入alpha通道
    FragColor.a = blur;
}