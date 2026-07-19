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
    vec3 diffTexColor = texture(texture_diffuse[0], TexCoord).rgb;
    vec3 specTexColor = texture(texture_specular[0], TexCoord).rgb;

    // 环境光：使用材质 ambient 系数
    vec3 ambientColor = ambient * diffTexColor;

    // 漫反射光计算
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuseColor = diffuse * diff * diffTexColor;

    // 镜面反射光计算（Blinn-Phong）：使用材质 specular 系数和 shininess
    vec3 viewDir = normalize(viewPos - FragPos);
    //vec3 halfwayDir = normalize(lightDir + viewDir);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specularColor = specular * spec * specTexColor;

    // 最终颜色计算
    vec3 result = ambientColor + diffuseColor + specularColor;
    FragColor = vec4(result, 1.0);
}

