#include <GL/glew.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "shader.h"
#include "obj_loader.h"
#include "camera.h"

// =====================================================
// ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ДЛЯ ОРБИТ
// =====================================================

GLuint orbitShaderProgram = 0;
GLuint orbitVAO = 0, orbitVBO = 0;

// =====================================================
// ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ДЛЯ ИНСТАНЦИРОВАНИЯ
// =====================================================

InstancedShader* instancedShader = nullptr;
Camera* camera = nullptr;

GLuint sunTexture = 0;
GLuint planetTexture = 0;

GLuint instanceVBO = 0;
GLuint instanceVAO = 0;
size_t instanceCount = 0;

// =====================================================
// ИНИЦИАЛИЗАЦИЯ
// =====================================================

void initGL() {
    glClearColor(66.0f / 255.0f, 133.0f / 255.0f, 180.0f / 255.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    std::cout << " OpenGL инициализирован" << std::endl;
    std::cout << "  Версия: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "  Vendor: " << glGetString(GL_VENDOR) << std::endl;
}

void initShaders() {
    instancedShader = new InstancedShader();
    std::cout << "Инстанцированные шейдеры скомпилированы" << std::endl;    
    initOrbitShader();
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


// =====================================================
// ОСНОВНОЙ ЦИКЛ
// =====================================================

void render(float width, float height) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = camera->getViewMatrix();
    glm::mat4 projection = camera->getProjectionMatrix(width / height);


    // Рисуем все инстансы за один вызов
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, planetTexture);
    instancedShader->setInt("textureSampler", 0);

    glBindVertexArray(instanceVAO);
    glDrawElementsInstanced(GL_TRIANGLES,
                           0,
                           GL_UNSIGNED_INT,
                           0,
                           instanceCount);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

// =====================================================
// УПРАВЛЕНИЕ КАМЕРОЙ И ОРБИТАМИ
// =====================================================

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
    } else {
        rKeyPressed = false;
    }
}

int main() {
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 4;
    settings.majorVersion = 3;
    settings.minorVersion = 3;
    settings.attributeFlags = sf::ContextSettings::Core;

    sf::RenderWindow window(sf::VideoMode(1200, 800), "Lightning",
                           sf::Style::Default, settings);
    window.setVerticalSyncEnabled(true);
    window.setActive(true);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Ошибка инициализации GLEW" << std::endl;
        return -1;
    }

    std::cout << "=== СОЛНЕЧНАЯ СИСТЕМА ===" << std::endl;
    std::cout << std::endl;

    initGL();
    initShaders();
    initModelsAndSystem();
    camera = new Camera(glm::vec3(0.0f, 10.0f, 30.0f));

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
    float frameTime = 0.0f;
    int frameCount = 0;

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
        frameTime += deltaTime;
        frameCount++;

        handleInput(deltaTime);

        render(window.getSize().x, window.getSize().y);

        window.display();
    }

    delete instancedShader;
    delete camera;
    glDeleteTextures(1, &sunTexture);
    glDeleteTextures(1, &planetTexture);
    glDeleteBuffers(1, &instanceVBO);
    glDeleteVertexArrays(1, &instanceVAO);
    
    glDeleteProgram(orbitShaderProgram);
    glDeleteBuffers(1, &orbitVBO);
    glDeleteVertexArrays(1, &orbitVAO);

    window.close();
    std::cout << "✅ Программа завершена" << std::endl;

    return 0;
}