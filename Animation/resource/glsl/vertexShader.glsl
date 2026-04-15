#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in ivec4 boneIds;
layout (location = 4) in vec4 weights;

uniform mat4 model;
uniform mat4 invModel;
uniform mat4 view;
uniform mat4 projection;

// 最大骨骼数量
const int MAX_BONES = 100;
// 每个顶点最后受到 4 个骨骼影响
const int MAX_BONE_INFLUENCE = 4;
// 所有骨骼的变换矩阵
uniform mat4 finalBonesMatrices[MAX_BONES];

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

void main()
{
   vec4 finalPosition = vec4(0.0);
   for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
      if (boneIds[i] == -1) {
         break;
      }
      if (boneIds[i] >= MAX_BONES) {
         finalPosition = vec4(aPos, 1.0);
         break;
      }
      int boneIndex = boneIds[i];
      finalPosition += finalBonesMatrices[boneIndex] * vec4(aPos, 1.0) * weights[i];
   }
   gl_Position = projection * view * model * finalPosition;
   TexCoord = aTexCoord;
   Normal = mat3(transpose(invModel)) * aNormal;
   FragPos = vec3(model * vec4(aPos, 1.0));
}
