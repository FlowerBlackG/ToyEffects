#version 450 core
out vec4 FragColor;

in vec2 TexCoords;
in float v_depth;

uniform float near_distance;  // ��ƽ���ģ��˥����Χ 10.0
uniform float far_distance;  // Զƽ���ģ��˥����Χ 10.0
uniform float near_plane;    // ��ƽ�� -12.0
uniform float far_plane;     // Զƽ�� -20.0

uniform sampler2D textureDiffuse1;

void main()
{    
    FragColor = texture2D(textureDiffuse1, TexCoords);
 
    float blur = 0;
 
    // ������ȼ���ģ������
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
 
    // ��ģ������д��alphaͨ��
    FragColor.a = blur;
}