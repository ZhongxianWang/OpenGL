#version 330 core
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse[16];
uniform sampler2D texture_specular[16];

uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float shininess;

uniform vec3 lightPos;
uniform vec3 viewPos;
out vec4 FragColor;

void main()
{
    FragColor = texture(texture_diffuse[0], TexCoord);
}

