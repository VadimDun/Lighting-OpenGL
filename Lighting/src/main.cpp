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


void initGL() {
    glClearColor(66.0f / 255.0f, 133.0f / 255.0f, 180.0f / 255.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    std::cout << " OpenGL инициализирован" << std::endl;
    std::cout << "  Версия: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "  Vendor: " << glGetString(GL_VENDOR) << std::endl;
}

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


// TODO
void render(float width, float height) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glm::mat4 view = camera->getViewMatrix();
    //glm::mat4 projection = camera->getProjectionMatrix(width / height);


    //// Рисуем все инстансы за один вызов
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, planetTexture);
    //instancedShader->setInt("textureSampler", 0);

    //glBindVertexArray(instanceVAO);
    //glDrawElementsInstanced(GL_TRIANGLES,
    //                       0,
    //                       GL_UNSIGNED_INT,
    //                       0,
    //                       instanceCount);
    //glBindVertexArray(0);

    //glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

// TODO
void handleInput(float deltaTime) {
    float moveSpeed = 5.0f * deltaTime;
    float rotateSpeed = 50.0f * deltaTime;

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

    static bool rKeyPressed = false;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
        if (!rKeyPressed) {
            delete camera;
            camera = new Camera(glm::vec3(0.0f, 10.0f, 30.0f));
            std::cout << "Камера сброшена в начальную позицию" << std::endl;
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
    std::cout << "  УПРАВЛЕНИЕ:" << std::endl;
    std::cout << "  W/A/S/D - движение вперёд/назад/влево/вправо" << std::endl;
    std::cout << "  SPACE/CTRL - движение вверх/вниз" << std::endl;
    std::cout << "  СТРЕЛКИ - повороты камеры" << std::endl;
    std::cout << "  R - сбросить камеру в начальную позицию" << std::endl;
    std::cout << "  ESC - выход" << std::endl;
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
    std::cout << "✓ Программа завершена" << std::endl;

    return 0;
}