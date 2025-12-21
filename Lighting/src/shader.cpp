#include "shader.h"
#include <iostream>

const char* vertexShaderSource = R"(
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0);

    TexCoord = texCoord;
    Normal = mat3(transpose(inverse(model))) * normal;
    FragPos = vec3(model * vec4(position, 1.0));
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

struct PointLight {
    vec3 position;
    float intensity;   // общий множитель
};

struct DirLight {
    vec3 direction;
    float intensity;
};

struct SpotLight {
    vec3 position;
    float intensity;
    vec3 direction;
    float cutOff;      // cos(внутреннего угла)
    float outerCutOff; // cos(внешнего угла)
};


uniform sampler2D textureSampler;
uniform vec3 viewPos;

uniform PointLight pointLight;
uniform DirLight dirLight;
uniform SpotLight spotLight;

// 0 или 1 — включён ли источник
uniform int pointLightEnabled;
uniform int spotLightEnabled;

// Модель освещения:
// 0 - Phong, 1 - Toon, 2 - Minnaert
uniform int shadingModel;

//TODO
void main() {
    // Получаем цвет текстуры
    vec4 texColor = texture(textureSampler, TexCoord);
    //if (texColor.a < 0.1) discard;
    
    // Фонговое освещение
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    
    // Амбиентное освещение
    float ambientStrength = 0.4;
    vec3 ambient = ambientStrength * texColor.rgb;
    
    // Диффузное освещение
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * texColor.rgb;
    
    // Спекулярное освещение
    float specularStrength = 0.5;
    vec3 viewDir = normalize(-FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * vec3(1.0);
    
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, texColor.a);
}
)";