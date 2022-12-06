#version 330 core
layout (location = 0) in vec2 position;

uniform vec3 sunPosition;

out vec3 vSunDirection;
out vec3 vSunColor;

const float sun_power = 20.0;



void main() {
	gl_Position = vec4( position, 1.0, 1.0 );

	vSunDirection = normalize( sunPosition );
	vSunColor =vec3(0.7,0.6,0.5)*sun_power;

}
	
