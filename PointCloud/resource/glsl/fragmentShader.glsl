#version 330 core
in float Intensity;
out vec4 FragColor;
uniform float minIntensity;
uniform float maxIntensity;
void main()
{
   float normalizedIntensity = (Intensity - minIntensity) / (maxIntensity - minIntensity);
   vec3 color;
   if (normalizedIntensity < 0.25) {
       // 蓝到青：蓝色逐渐减少，绿色逐渐增加
       float t = normalizedIntensity * 4.0;
       color = vec3(0.0, t, 1.0);
   } else if (normalizedIntensity < 0.5) {
       // 青到绿：蓝色逐渐减少，绿色保持
       float t = (normalizedIntensity - 0.25) * 4.0;
       color = vec3(0.0, 1.0, 1.0 - t);
   } else if (normalizedIntensity < 0.75) {
       // 绿到黄：红色逐渐增加
       float t = (normalizedIntensity - 0.5) * 4.0;
       color = vec3(t, 1.0, 0.0);
    } else {
        // 黄到红：绿色逐渐减少
        float t = (normalizedIntensity - 0.75) * 4.0;
        color = vec3(1.0, 1.0 - t, 0.0);
    }
    FragColor = vec4(color, 1.0);
}