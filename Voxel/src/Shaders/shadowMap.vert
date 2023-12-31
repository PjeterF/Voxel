#version 300 es
precision highp float;

layout(location = 0) in vec3 modelPos;

uniform mat4 lightTransform;

void main()
{
	gl_Position = lightTransform * vec4(modelPos, 1);
}