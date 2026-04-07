#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float aIntensity;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out float Intensity;
void main()
{
   gl_Position = projection * view * model * vec4(aPos, 1.0);
   Intensity = aIntensity;
}