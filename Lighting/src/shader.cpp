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

vec3 calcDirLight(DirLight light, vec3 norm, vec3 viewDir, vec3 baseColor)
{
    vec3 lightDir = normalize(-light.direction);

    float diff = max(dot(norm, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    vec3 ambient  = 0.15 * baseColor;
    vec3 diffuse  = diff * baseColor;
    vec3 specular = 0.3 * spec * vec3(1.0);

    return (ambient + diffuse + specular) * light.intensity;
}

vec3 calcPointLight(PointLight light, vec3 norm, vec3 fragPos, vec3 viewDir, vec3 baseColor)
{
    vec3 lightDir = normalize(light.position - fragPos);

    float diff = max(dot(norm, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    vec3 ambient  = 0.2 * baseColor;
    vec3 diffuse  = diff * baseColor;
    vec3 specular = 0.5 * spec * vec3(1.0);

    return (ambient + diffuse + specular) * light.intensity;
}

vec3 calcSpotLight(SpotLight light, vec3 norm, vec3 fragPos, vec3 viewDir, vec3 baseColor)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float theta   = dot(lightDir, normalize(-light.direction));

    float epsilon   = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    float diff = max(dot(norm, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    vec3 ambient  = 0.1 * baseColor;
    vec3 diffuse  = diff * baseColor;
    vec3 specular = 0.6 * spec * vec3(1.0);

    vec3 color = ambient + diffuse + specular;
    return color * intensity * light.intensity;
}

void main() {
    vec4 texColor = texture(textureSampler, TexCoord);

    vec3 baseColor = texColor.rgb;

    vec3 norm    = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Суммируем вклад трёх источников (база для Phong)
    vec3 result  = vec3(0.0);

    result += calcDirLight(dirLight, norm, viewDir, baseColor);

    if (pointLightEnabled == 1) {
        result += calcPointLight(pointLight, norm, FragPos, viewDir, baseColor);
    }

    if (spotLightEnabled == 1) {
        result += calcSpotLight(spotLight, norm, FragPos, viewDir, baseColor);
    }

    if (shadingModel == 0) { // Phong (уже посчитан в result)
        FragColor = vec4(result, texColor.a);
    } else if (shadingModel == 1) { // Toon Shading
        float brightness = max(result.r, max(result.g, result.b));
        brightness = clamp(brightness, 0.0, 1.0);

        float level = 0.0;
        if (brightness < 0.2)
            level = 0.0;
        else if (brightness < 0.4)
            level = 0.3;
        else if (brightness < 0.7)
            level = 1.0;
        else
            level = 1.3;

        vec3 toonColor = baseColor * level;
        FragColor = vec4(toonColor, texColor.a);
    } else if (shadingModel == 2) { // Minnaert 
        const float k = 0.8;
        vec3 v2 = viewDir;
        float nv = max(dot(norm, v2), 0.0);
        float d2 = pow(1.0 - nv, 1.0 - k);
        
        vec3 minnaertResult = vec3(0.0);
        
        vec3 dirLightDir = normalize(-dirLight.direction);
        float nl_dir = max(dot(norm, dirLightDir), 0.0);
        float d1_dir = pow(nl_dir, 1.0 + k);
        vec3 dirContrib = baseColor * d1_dir * d2 * dirLight.intensity;
        minnaertResult += dirContrib;
        
        if (pointLightEnabled == 1) {
            vec3 pointLightDir = normalize(pointLight.position - FragPos);
            float nl_point = max(dot(norm, pointLightDir), 0.0);
            float d1_point = pow(nl_point, 1.0 + k);
            vec3 pointContrib = baseColor * d1_point * d2 * pointLight.intensity;
            minnaertResult += pointContrib;
        }
        
        if (spotLightEnabled == 1) {
            vec3 spotLightDir = normalize(spotLight.position - FragPos);
            float theta = dot(spotLightDir, normalize(-spotLight.direction));
            float epsilon = spotLight.cutOff - spotLight.outerCutOff;
            float spotIntensity = clamp((theta - spotLight.outerCutOff) / epsilon, 0.0, 1.0);
            
            float nl_spot = max(dot(norm, spotLightDir), 0.0);
            float d1_spot = pow(nl_spot, 1.0 + k);
            vec3 spotContrib = baseColor * d1_spot * d2 * spotIntensity * spotLight.intensity;
            minnaertResult += spotContrib;
        }
        
        FragColor = vec4(minnaertResult, texColor.a);
    } else {
        // На всякий случай — Phong по умолчанию
        FragColor = vec4(result, texColor.a);
    }
}
)";


Shader::Shader() {
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexShaderSource, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    programID = glCreateProgram();
    glAttachShader(programID, vertex);
    glAttachShader(programID, fragment);
    glLinkProgram(programID);
    checkCompileErrors(programID, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::~Shader() {
    glDeleteProgram(programID);
}

void Shader::use() const {
    glUseProgram(programID);
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(programID, name.c_str()),
        1, GL_FALSE, &mat[0][0]);
}

void Shader::setVec3(const std::string& name, const glm::vec3& vec) const {
    glUniform3fv(glGetUniformLocation(programID, name.c_str()), 1, &vec[0]);
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
}

void Shader::checkCompileErrors(GLuint shader, const std::string& type) {
    int success;
    char infoLog[1024];

    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "Ошибка компиляции " << type << " шейдера:" << std::endl
                << infoLog << std::endl;
        }
    }
    else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "Ошибка линкования программы:" << std::endl
                << infoLog << std::endl;
        }
    }
}