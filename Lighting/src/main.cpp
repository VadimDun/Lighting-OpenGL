#include <GL/glew.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "shader.h"
#include "obj_loader.h"
#include "camera.h"

// Модели
OBJModel tableModel;
OBJModel chairModel;
OBJModel vaseModel;
OBJModel cubeModel;
OBJModel treeModel;
OBJModel hatModel;
OBJModel candyModel;
OBJModel snowmanModel;
OBJModel lampModel;          // модель шара-лампы

// Текстуры
GLuint modelTexture = 0;
GLuint chairTexture = 0;
GLuint vaseTexture = 0;
GLuint cubeTexture = 0;
GLuint treeTexture = 0;
GLuint hatTexture = 0;
GLuint candyTexture = 0;
GLuint snowmanTexture = 0;
GLuint lampTexture = 0;      // текстура для шара-лампы

Shader* shader = nullptr;
Camera* camera = nullptr;

float gPointLightIntensity = 1.0f;
float gDirLightIntensity = 0.4f;
float gSpotLightIntensity = 1.0f;

// Углы прожектора в градусах
float gSpotInnerAngleDeg = 15.0f;    // внутренний угол (яркая область)
float gSpotOuterAngleDeg = 25.0f;    // внешний угол (граница затухания)

bool gPointLightOn = false;
bool gSpotLightOn = false;

glm::vec3 gPointLightPos(5.0f, 5.0f, 5.0f);
glm::vec3 gDirLightDir(-0.3f, -1.0f, -0.2f);

// режим управления:
// 0 = камера, 1 = направленный свет, 2 = точечный источник
int gControlMode = 0;

GLuint loadTexture(const std::string& filename) {
    sf::Image image;
    if (image.loadFromFile(filename)) {
        image.flipVertically();
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        sf::Vector2u size = image.getSize();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
            size.x, size.y,
            0, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        std::cout << "Текстура загружена: " << filename << std::endl;
        return texture;
    }
    else {
        std::cerr << "Не удалось загрузить текстуру: " << filename << std::endl;
        return 0;
    }
}

void initGL() {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    std::cout << " OpenGL инициализирован" << std::endl;
    std::cout << " Версия: " << glGetString(GL_VERSION) << std::endl;
    std::cout << " Vendor: " << glGetString(GL_VENDOR) << std::endl;
}

void initResources() {
    if (!tableModel.load("models/table.obj")) {
        std::cerr << "Ошибка загрузки модели стола" << std::endl;
    }
    else {
        std::cout << "Модель стола загружена: "
            << tableModel.vertices.size() << " вершин, "
            << tableModel.indices.size() << " индексов" << std::endl;
    }
    modelTexture = loadTexture("textures/table.png");

    if (!chairModel.load("models/chair.obj")) {
        std::cerr << "Ошибка загрузки модели стула" << std::endl;
    }
    else {
        std::cout << "Модель стула загружена: "
            << chairModel.vertices.size() << " вершин, "
            << chairModel.indices.size() << " индексов" << std::endl;
    }

    chairTexture = loadTexture("textures/chair.png");

    if (!vaseModel.load("models/vase.obj")) {
        std::cerr << "Ошибка загрузки модели вазы" << std::endl;
    }
    else {
        std::cout << "Модель вазы загружена: "
            << vaseModel.vertices.size() << " вершин, "
            << vaseModel.indices.size() << " индексов" << std::endl;
    }

    vaseTexture = loadTexture("textures/vase.png");

    if (!cubeModel.load("models/cube.obj")) {
        std::cerr << "Ошибка загрузки модели кубика" << std::endl;
    }
    else {
        std::cout << "Модель кубика загружена: "
            << cubeModel.vertices.size() << " вершин, "
            << cubeModel.indices.size() << " индексов" << std::endl;
    }

    cubeTexture = loadTexture("textures/cube.jpg");

    if (!treeModel.load("models/PolyTree.obj")) {
        std::cerr << "Ошибка загрузки модели дерева" << std::endl;
    }
    else {
        std::cout << "Модель дерева загружена: "
            << treeModel.vertices.size() << " вершин, "
            << treeModel.indices.size() << " индексов" << std::endl;
    }

    treeTexture = loadTexture("textures/PolyTree.jpg");

    if (!candyModel.load("models/candy.obj")) {
        std::cerr << "Ошибка загрузки модели конфеты" << std::endl;
    }
    else {
        std::cout << "Модель конфеты загружена: "
            << candyModel.vertices.size() << " вершин, "
            << candyModel.indices.size() << " индексов" << std::endl;
    }

    candyTexture = loadTexture("textures/candy.png");

    if (!snowmanModel.load("models/snowman.obj")) {
        std::cerr << "Ошибка загрузки модели снеговика" << std::endl;
    }
    else {
        std::cout << "Модель снеговика загружена: "
            << snowmanModel.vertices.size() << " вершин, "
            << snowmanModel.indices.size() << " индексов" << std::endl;
    }

    snowmanTexture = loadTexture("textures/snowman.png");

    // Шар, обозначающий положение точечного источника света
    if (!lampModel.load("models/sphere.obj")) {
        std::cerr << "Ошибка загрузки модели шара-лампы" << std::endl;
    }
    else {
        std::cout << "Модель шара-лампы загружена: "
            << lampModel.vertices.size() << " вершин, "
            << lampModel.indices.size() << " индексов" << std::endl;
    }

    // Текстура для лампы
    lampTexture = loadTexture("textures/sphere.jpg");

    shader = new Shader();
    std::cout << "Шейдер инициализирован" << std::endl;
}

void render(float width, float height) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!shader) return;

    shader->use();

    glm::mat4 view = camera->getViewMatrix();
    glm::mat4 projection = camera->getProjectionMatrix(width / height);

    // Точечный источник (лампочка над столом)
    shader->setVec3("pointLight.position", gPointLightPos);
    shader->setFloat("pointLight.intensity", gPointLightIntensity);
    shader->setInt("pointLightEnabled", gPointLightOn ? 1 : 0);

    // Направленный источник (солнечный свет)
    shader->setVec3("dirLight.direction", gDirLightDir);
    shader->setFloat("dirLight.intensity", gDirLightIntensity);

    // Прожектор (фонарик камеры)
    shader->setVec3("spotLight.position", camera->position);
    glm::vec3 spotDir = camera->front;
    shader->setVec3("spotLight.direction", spotDir);
    shader->setFloat("spotLight.intensity", gSpotLightIntensity);
    shader->setInt("spotLightEnabled", gSpotLightOn ? 1 : 0);

    float innerCutOff = glm::cos(glm::radians(gSpotInnerAngleDeg)); // узкий яркий луч
    float outerCutOff = glm::cos(glm::radians(gSpotOuterAngleDeg)); // мягкое затухание
    shader->setFloat("spotLight.cutOff", innerCutOff);
    shader->setFloat("spotLight.outerCutOff", outerCutOff);

    shader->setVec3("viewPos", camera->position);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f, 2.0f, 2.0f));

    shader->setMat4("model", modelMatrix);
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);

    // Стол: Phong
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, modelTexture);
    shader->setInt("textureSampler", 0);
    shader->setInt("shadingModel", 0);
    tableModel.draw();

    // Стул: Toon
    glm::mat4 chairMatrix = glm::mat4(1.0f);
    chairMatrix = glm::translate(chairMatrix, glm::vec3(3.0f, 0.0f, -3.0f));
    chairMatrix = glm::scale(chairMatrix, glm::vec3(0.04f, 0.04f, 0.04f));
    chairMatrix = glm::rotate(chairMatrix, glm::radians(-50.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    shader->setMat4("model", chairMatrix);
    glBindTexture(GL_TEXTURE_2D, chairTexture);
    shader->setInt("textureSampler", 0);
    shader->setInt("shadingModel", 1);
    chairModel.draw();

    // Ваза: Minnaert
    glm::mat4 vaseMatrix = glm::mat4(1.0f);
    vaseMatrix = glm::translate(vaseMatrix, glm::vec3(-1.0f, 2.83f, 0.0f));
    vaseMatrix = glm::scale(vaseMatrix, glm::vec3(0.15f, 0.15f, 0.15f));
    shader->setMat4("model", vaseMatrix);
    glBindTexture(GL_TEXTURE_2D, vaseTexture);
    shader->setInt("textureSampler", 0);
    shader->setInt("shadingModel", 2);
    vaseModel.draw();

    // Куб: Phong
    glm::mat4 cubeMatrix = glm::mat4(1.0f);
    cubeMatrix = glm::translate(cubeMatrix, glm::vec3(1.5f, 3.01f, 0.0f));
    cubeMatrix = glm::scale(cubeMatrix, glm::vec3(0.15f, 0.15f, 0.15f));
    cubeMatrix = glm::rotate(cubeMatrix, glm::radians(60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    shader->setMat4("model", cubeMatrix);
    glBindTexture(GL_TEXTURE_2D, cubeTexture);
    shader->setInt("textureSampler", 0);
    shader->setInt("shadingModel", 0);
    cubeModel.draw();

    // Ёлка: Phong
    glm::mat4 treeMatrix = glm::mat4(1.0f);
    treeMatrix = glm::translate(treeMatrix, glm::vec3(0.0f, 2.85f, -1.0f));
    treeMatrix = glm::scale(treeMatrix, glm::vec3(0.01f, 0.01f, 0.01f));
    treeMatrix = glm::rotate(treeMatrix, glm::radians(70.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    shader->setMat4("model", treeMatrix);
    glBindTexture(GL_TEXTURE_2D, treeTexture);
    shader->setInt("textureSampler", 0);
    shader->setInt("shadingModel", 1);
    treeModel.draw();

    // Конфета: Toon
    glm::mat4 candyMatrix = glm::mat4(1.0f);
    candyMatrix = glm::translate(candyMatrix, glm::vec3(0.6f, 2.9f, 0.4f));
    candyMatrix = glm::scale(candyMatrix, glm::vec3(1.2f, 1.2f, 1.2f));
    candyMatrix = glm::rotate(candyMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    candyMatrix = glm::rotate(candyMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    shader->setMat4("model", candyMatrix);
    glBindTexture(GL_TEXTURE_2D, candyTexture);
    shader->setInt("textureSampler", 0);
    shader->setInt("shadingModel", 1);
    candyModel.draw();

    // Снеговик: Phong
    glm::mat4 snowmanMatrix = glm::mat4(1.0f);
    snowmanMatrix = glm::translate(snowmanMatrix, glm::vec3(-0.5f, 2.85f, 1.7f));
    snowmanMatrix = glm::scale(snowmanMatrix, glm::vec3(0.01f, 0.01f, 0.01f));
    snowmanMatrix = glm::rotate(snowmanMatrix, glm::radians(60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    shader->setMat4("model", snowmanMatrix);
    glBindTexture(GL_TEXTURE_2D, snowmanTexture);
    shader->setInt("textureSampler", 0);
    shader->setInt("shadingModel", 0);
    snowmanModel.draw();

    // Шар-лампа в позиции точечного источника света: Minnaert
    if (gPointLightOn) {
        glm::mat4 lampMatrix = glm::mat4(1.0f);
        lampMatrix = glm::translate(lampMatrix, gPointLightPos);
        lampMatrix = glm::scale(lampMatrix, glm::vec3(0.3f)); // небольшой шар
        shader->setMat4("model", lampMatrix);
        glBindTexture(GL_TEXTURE_2D, lampTexture);
        shader->setInt("textureSampler", 0);
        shader->setInt("shadingModel", 2);
        lampModel.draw();
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

void handleInput(float deltaTime) {
    float moveSpeed = 5.0f * deltaTime;
    float rotateSpeed = 50.0f * deltaTime;
    float intensityChangeSpeed = 0.8f * deltaTime;
    float angleChangeSpeed = 20.0f * deltaTime; // градусов в секунду

    // WASD и стрелки: зависят от режима управления
    if (gControlMode == 0) {
        // Режим управления камерой
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            camera->moveForward(moveSpeed);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            camera->moveBackward(moveSpeed);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            camera->moveLeft(moveSpeed);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            camera->moveRight(moveSpeed);
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            camera->moveUp(moveSpeed);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
            camera->moveDown(moveSpeed);
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            camera->rotatePitch(rotateSpeed);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            camera->rotatePitch(-rotateSpeed);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            camera->rotateYaw(-rotateSpeed);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            camera->rotateYaw(rotateSpeed);
        }
    }
    else if (gControlMode == 1) {
        // Режим управления направленным источником света
        // WASD изменяют направление вектора gDirLightDir
        glm::vec3 dir = gDirLightDir;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            dir.y += moveSpeed; // вверх
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            dir.y -= moveSpeed; // вниз
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            dir.x -= moveSpeed; // влево
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            dir.x += moveSpeed; // вправо
        }

        if (glm::length(dir) > 0.0001f) {
            gDirLightDir = glm::normalize(dir);
        }
        // Стрелки для камеры
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            camera->rotatePitch(rotateSpeed);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            camera->rotatePitch(-rotateSpeed);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            camera->rotateYaw(-rotateSpeed);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            camera->rotateYaw(rotateSpeed);
        }
    }
    else if (gControlMode == 2) {
        // Режим управления точечным источником света
        // WASD двигают его по XZ, стрелки вверх/вниз — по Y
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            gPointLightPos.z -= moveSpeed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            gPointLightPos.z += moveSpeed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            gPointLightPos.x -= moveSpeed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            gPointLightPos.x += moveSpeed;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            gPointLightPos.y += moveSpeed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            gPointLightPos.y -= moveSpeed;
        }
    }

    // Яркость источников света
    // Точечный источник
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
        gPointLightIntensity -= intensityChangeSpeed;
        if (gPointLightIntensity < 0.0f) gPointLightIntensity = 0.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) {
        gPointLightIntensity += intensityChangeSpeed;
        if (gPointLightIntensity > 3.0f) gPointLightIntensity = 3.0f;
    }

    // Направленный
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) {
        gDirLightIntensity -= intensityChangeSpeed;
        if (gDirLightIntensity < 0.0f) gDirLightIntensity = 0.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4)) {
        gDirLightIntensity += intensityChangeSpeed;
        if (gDirLightIntensity > 3.0f) gDirLightIntensity = 3.0f;
    }

    // Прожектор
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num5)) {
        gSpotLightIntensity -= intensityChangeSpeed;
        if (gSpotLightIntensity < 0.0f) gSpotLightIntensity = 0.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num6)) {
        gSpotLightIntensity += intensityChangeSpeed;
        if (gSpotLightIntensity > 3.0f) gSpotLightIntensity = 3.0f;
    }

    // Внутренний угол прожектора
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
        gSpotInnerAngleDeg -= angleChangeSpeed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
        gSpotInnerAngleDeg += angleChangeSpeed;
    }

    // Внешний угол прожектора
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) {
        gSpotOuterAngleDeg -= angleChangeSpeed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::X)) {
        gSpotOuterAngleDeg += angleChangeSpeed;
    }

    // Корректируем диапазоны углов:
    // внутренний < внешний, и оба в разумных пределах
    if (gSpotInnerAngleDeg < 5.0f)  gSpotInnerAngleDeg = 5.0f;
    if (gSpotInnerAngleDeg > 80.0f) gSpotInnerAngleDeg = 80.0f;

    if (gSpotOuterAngleDeg < gSpotInnerAngleDeg + 1.0f)
        gSpotOuterAngleDeg = gSpotInnerAngleDeg + 1.0f;
    if (gSpotOuterAngleDeg > 90.0f) gSpotOuterAngleDeg = 90.0f;

    // Включение света
    static bool pointTogglePressed = false;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)) {
        if (!pointTogglePressed) {
            gPointLightOn = !gPointLightOn;
            std::cout << "Точечный свет: " << (gPointLightOn ? "ВКЛ" : "ВЫКЛ") << std::endl;
            pointTogglePressed = true;
        }
    }
    else {
        pointTogglePressed = false;
    }

    static bool spotTogglePressed = false;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::L)) {
        if (!spotTogglePressed) {
            gSpotLightOn = !gSpotLightOn;
            std::cout << "Прожектор: " << (gSpotLightOn ? "ВКЛ" : "ВЫКЛ") << std::endl;
            spotTogglePressed = true;
        }
    }
    else {
        spotTogglePressed = false;
    }

    // Переключение режима управления: камера / направленный свет / точечный свет
    static bool modeTogglePressed = false;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Tab)) {
        if (!modeTogglePressed) {
            gControlMode = (gControlMode + 1) % 3;

            std::cout << "Режим управления: ";
            if (gControlMode == 0) {
                std::cout << "камера (WASD двигают камеру, стрелки поворачивают)" << std::endl;
            }
            else if (gControlMode == 1) {
                std::cout << "направленный свет (WASD меняют направление света)" << std::endl;
            }
            else if (gControlMode == 2) {
                std::cout << "точечный свет (WASD/стрелки двигают источник света)" << std::endl;
            }

            modeTogglePressed = true;
        }
    }
    else {
        modeTogglePressed = false;
    }

    static bool rKeyPressed = false;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
        if (!rKeyPressed) {
            delete camera;
            camera = new Camera(glm::vec3(0.0f, 5.0f, 20.0f));

            gPointLightIntensity = 1.0f;
            gDirLightIntensity = 0.4f;
            gSpotLightIntensity = 1.0f;

            gSpotInnerAngleDeg = 15.0f;
            gSpotOuterAngleDeg = 25.0f;

            gPointLightOn = false;
            gSpotLightOn = false;

            gPointLightPos = glm::vec3(5.0f, 5.0f, 5.0f);
            gDirLightDir = glm::vec3(-0.3f, -1.0f, -0.2f);

            gControlMode = 0;

            std::cout << "Камера и источники света сброшены к значениям по умолчанию" << std::endl;
            rKeyPressed = true;
        }
    }
    else {
        rKeyPressed = false;
    }
}

int main() {
    setlocale(LC_ALL, "ru.UTF-8");
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 4;
    settings.majorVersion = 3;
    settings.minorVersion = 3;
    settings.attributeFlags = sf::ContextSettings::Core;

    sf::RenderWindow window(sf::VideoMode(1200, 800), "Model Viewer",
        sf::Style::Default, settings);
    window.setVerticalSyncEnabled(true);
    window.setActive(true);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Ошибка инициализации GLEW" << std::endl;
        return -1;
    }

    std::cout << "\n=== MODEL VIEWER ===" << std::endl;
    std::cout << std::endl;

    initGL();
    initResources();

    camera = new Camera(glm::vec3(0.0f, 5.0f, 20.0f),
        glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

    std::cout << std::endl;
    std::cout << "УПРАВЛЕНИЕ:" << std::endl;
    std::cout << " W/A/S/D      - движение" << std::endl;
    std::cout << " SPACE/CTRL   - вверх/вниз" << std::endl;
    std::cout << " Стрелки      - повороты камеры" << std::endl;
    std::cout << " 1/2          - яркость точечного источника" << std::endl;
    std::cout << " 3/4          - яркость направленного источника" << std::endl;
    std::cout << " 5/6          - яркость прожектора" << std::endl;
    std::cout << " Q/E          - внутренний угол прожектора" << std::endl;
    std::cout << " Z/X          - внешний угол прожектора" << std::endl;
    std::cout << " P            - вкл/выкл точечный свет" << std::endl;
    std::cout << " L            - вкл/выкл прожектор" << std::endl;
    std::cout << " TAB          - режим: камера / направление солнца / точечный свет (WASD/стрелки)" << std::endl;
    std::cout << " R            - сбросить камеру и свет" << std::endl;
    std::cout << " ESC          - выход" << std::endl;
    std::cout << std::endl;

    sf::Clock clock;
    bool running = true;

    while (running && window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed ||
                (event.type == sf::Event::KeyPressed &&
                    event.key.code == sf::Keyboard::Escape)) {
                running = false;
            }

            if (event.type == sf::Event::Resized) {
                glViewport(0, 0, event.size.width, event.size.height);
            }
        }

        float deltaTime = clock.restart().asSeconds();

        handleInput(deltaTime);
        render(window.getSize().x, window.getSize().y);

        window.display();
    }

    delete shader;
    delete camera;
    glDeleteTextures(1, &modelTexture);
    glDeleteTextures(1, &chairTexture);
    glDeleteTextures(1, &vaseTexture);
    glDeleteTextures(1, &cubeTexture);
    glDeleteTextures(1, &treeTexture);
    glDeleteTextures(1, &hatTexture);
    glDeleteTextures(1, &candyTexture);
    glDeleteTextures(1, &snowmanTexture);
    glDeleteTextures(1, &lampTexture);

    window.close();
    std::cout << "Программа завершена" << std::endl;

    return 0;
}