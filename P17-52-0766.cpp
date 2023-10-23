#include <iostream>
#include <ctime>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <string>
#include <glut.h>

const int screenWidth = 1000;
const int screenHeight = 800;
const int playerSize = 20;
const int obstacleSize = 30;
const int collectableSize = 15;
const int powerUpSize = 30;
const int maxLives = 5;
const int maxCollectables = 5;
const int maxPowerUps = 8;
int powerUpType1Duration = 300;
int powerUpType2Duration = 10;
int gameTime = 600;
const float endGameSquareX = 50; 
const float endGameSquareY = 50; 
const float endGameSquareSize = 100; 
bool hasReachedEndGame = false;
bool hasReachedTarget = false;
bool hasTimeRunOut = false;
float rotationAngle = 0.0f;
float playerX = screenWidth / 2;
float playerY = screenHeight / 2;
float playerSpeed = 5;
float playerRotation = 0;
int score = 0;

std::vector<std::pair<float, float>> obstacles;
std::vector<std::pair<float, float>> collectables;

struct PowerUp {
    float x;
    float y;
    int type;  
};

std::vector<PowerUp> powerUps;
int powerUpType1Timer = 0;
int powerUpType2Timer = 0;
int lives = maxLives;
bool gameRunning = true;


void drawCircle(float x, float y, float radius) {
    int numSegments = 100;
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < numSegments; i++) {
        float theta = 2.0f * 3.1415926f * float(i) / float(numSegments);
        float dx = radius * cosf(theta);
        float dy = radius * sinf(theta);
        glVertex2f(x + dx, y + dy);
    }
    glEnd();
}

void drawSquare(float x, float y, float size) {
    glBegin(GL_QUADS);
    glVertex2f(x - size / 2, y - size / 2);
    glVertex2f(x + size / 2, y - size / 2);
    glVertex2f(x + size / 2, y + size / 2);
    glVertex2f(x - size / 2, y + size / 2);
    glEnd();
}

void drawRectangle(float x, float y, float width, float height) {
    glBegin(GL_QUADS);
    glVertex2f(x - width / 2, y - height / 2);
    glVertex2f(x + width / 2, y - height / 2);
    glVertex2f(x + width / 2, y + height / 2);
    glVertex2f(x - width / 2, y + height / 2);
    glEnd();
}

void drawTriangle(float x, float y, float size) {
    glBegin(GL_TRIANGLES);
    glVertex2f(x, y + size / 2);
    glVertex2f(x - size / 2, y - size / 2);
    glVertex2f(x + size / 2, y - size / 2);
    glEnd();
}

void drawPlayer() {
    glPushMatrix();
    glTranslatef(playerX, playerY, 0);
    float angle = 0.0f;
    if (playerRotation == 0) {
        angle = -90.0f;  
    }
    else if (playerRotation == 90) {
        angle = 0.0f;  
    }
    else if (playerRotation == 180) {
        angle = 90.0f;  
    }
    else if (playerRotation == 270) {
        angle = 180.0f; 
    }
    glRotatef(angle, 0, 0, 1);
    glColor3f(0, 0, 1); 
    drawCircle(0, 0, playerSize);
    drawRectangle(0, playerSize, playerSize, playerSize * 2);
    drawSquare(playerSize / 2, -playerSize * 2, playerSize / 2);
    drawTriangle(-playerSize / 2, -playerSize * 2, playerSize);
    glPopMatrix();
}

void renderBitmapString(float x, float y, void* font, const std::string& string) {
    glRasterPos2f(x, y);
    for (char c : string) {
        glutBitmapCharacter(font, c);
    }
}

void drawGameTime() {
    glColor3f(1.0f, 1.0f, 1.0f); 
    int textWidth = 80; 
    int textHeight = 20; 
    int x = screenWidth - textWidth - 10; 
    int y = screenHeight - textHeight - 10; 
    glRasterPos2f(x, y);
    std::string timeText = "Time: " + std::to_string(gameTime);
    renderBitmapString(x, y, GLUT_BITMAP_HELVETICA_18, timeText);
}

void drawGameScore() {
    glColor3f(1.0f, 1.0f, 1.0f); 
    int textWidth = 80; 
    int textHeight = 20; 
    int x = (screenWidth - textWidth) / 2; 
    int y = screenHeight - textHeight - 10; 
    glRasterPos2f(x, y);
    std::string scoreText = "Score: " + std::to_string(score);
    renderBitmapString(x, y, GLUT_BITMAP_HELVETICA_18, scoreText);
    glutPostRedisplay(); 
}

void drawHealthBar() {
    glColor3f(0.0f, 1.0f, 0.0f); 
    glRectf(10, screenHeight - 30, 10 + (lives * 20), screenHeight - 10);
}

void drawObstacles() {
    for (const auto& obstacle : obstacles) {
        glColor3f(1, 0, 0);
        drawCircle(obstacle.first, obstacle.second, obstacleSize);
        float innerCircleSize = obstacleSize * 0.6; 
        glColor3f(1, 1, 1); 
        drawCircle(obstacle.first, obstacle.second, innerCircleSize);
    }
}

void drawCollectables() {
    for (const auto& collectable : collectables) {
        float x = collectable.first;
        float y = collectable.second;

        glColor3f(0, 1, 0); 
        glBegin(GL_TRIANGLES);
        glVertex2f(x, y + collectableSize); 
        glVertex2f(x - collectableSize, y - collectableSize); 
        glVertex2f(x + collectableSize, y - collectableSize); 
        glEnd();

        float squareSize = collectableSize / 2;
        float topSquareY = y - (collectableSize / 2);
        float bottomSquareY = y - collectableSize;
        float leftBound = x - collectableSize;
        float rightBound = x + collectableSize;

        glColor3f(1, 0, 0); 
        float redSquareOffset = 0.5 * squareSize * sin(0.002 * glutGet(GLUT_ELAPSED_TIME)); 
        float clampedRedOffset = fmaxf(fminf(redSquareOffset, topSquareY - bottomSquareY), 0);
        drawSquare(x, topSquareY + clampedRedOffset, squareSize);
        glColor3f(0, 0, 1);
        float blueSquareOffset = 0.5 * squareSize * sin(0.002 * glutGet(GLUT_ELAPSED_TIME)); 
        float clampedBlueOffset = fmaxf(fminf(blueSquareOffset, topSquareY - bottomSquareY), 0);
        drawSquare(x, bottomSquareY + clampedBlueOffset, squareSize);
    }
}

void drawPowerUps() {
    for (const auto& powerUp : powerUps) {
        float x = powerUp.x;
        float y = powerUp.y;

        if (powerUp.type == 1) {
            glPushMatrix();
            glTranslatef(x, y, 0);
            glRotatef(0.5 * glutGet(GLUT_ELAPSED_TIME), 0, 0, 1);

            glColor3f(1.0f, 1.0f, 0.0f); 
            drawSquare(0, 0, powerUpSize);

            float rectWidth = powerUpSize / 4; 
            float rectHeight = powerUpSize * 0.8;

            glColor3f(0.0f, 0.0f, 1.0f); 
            drawRectangle(-rectWidth, 0, rectWidth, rectHeight);
            glColor3f(1.0f, 0.0f, 0.0f); 
            drawRectangle(0, 0, rectWidth, rectHeight);
            glColor3f(0.0f, 1.0f, 0.0f); 
            drawRectangle(rectWidth, 0, rectWidth, rectHeight);
            glPopMatrix();
        }
        else if (powerUp.type == 2) {
            glPushMatrix();
            glTranslatef(x, y, 0);
            glRotatef(0.5 * glutGet(GLUT_ELAPSED_TIME), 0, 0, 1);

            glColor3f(0.0f, 0.0f, 1.0f);
            drawSquare(0, 0, powerUpSize);

            float rectWidth = powerUpSize / 4;
            float rectHeight = powerUpSize * 0.8;

            glColor3f(1.0f, 1.0f, 0.0f);
            drawRectangle(-rectWidth, 0, rectWidth, rectHeight);
            glColor3f(0.5f, 0.0f, 0.5f); 
            drawRectangle(0, 0, rectWidth, rectHeight);
            glColor3f(0.0f, 1.0f, 1.0f);
            drawRectangle(rectWidth, 0, rectWidth, rectHeight);
            glPopMatrix();
        }
    }
}

void drawEndGameSquare() {
    float endGameSquareRotationSpeed = -0.001f;; 
    float innerSquareSize = endGameSquareSize * 0.3;

    glColor3f(1, 1, 0);
    drawSquare(endGameSquareX, endGameSquareY, endGameSquareSize);

    float endGameCenterX = endGameSquareX;
    float endGameCenterY = endGameSquareY;

    for (int i = 0; i < 4; i++) {
        glPushMatrix();
        float innerSquareX = endGameCenterX + innerSquareSize * cos(rotationAngle + i * 90 * 3.1415926 / 180);
        float innerSquareY = endGameCenterY + innerSquareSize * sin(rotationAngle + i * 90 * 3.1415926 / 180);
        glTranslatef(innerSquareX, innerSquareY, 0);
        glRotatef(rotationAngle + 90 * i, 0, 0, 1);
        if (i == 0) {
            glColor3f(1, 0, 0); 
        }
        else if (i == 1) {
            glColor3f(0, 1, 0); 
        }
        else if (i == 2) {
            glColor3f(0, 0, 1); 
        }
        else if (i == 3) {
            glColor3f(1, 1, 1); 
        }
        drawSquare(0, 0, innerSquareSize);
        glPopMatrix();
    }
    rotationAngle += endGameSquareRotationSpeed;
    if (rotationAngle >= 360.0f) {
        rotationAngle -= 360.0f;
    }
}

bool isCollision(float x1, float y1, float r1, float x2, float y2, float r2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float distance = std::sqrt(dx * dx + dy * dy);
    return distance < r1 + r2;
}

void update(int value) {
    if (gameRunning) {
        float prevX = playerX;
        float prevY = playerY;

        playerX += playerSpeed * std::cos(playerRotation * 3.1415926 / 180);
        playerY += playerSpeed * std::sin(playerRotation * 3.1415926 / 180);

        // borders
        if (playerX < 0 || playerX > screenWidth || playerY < 0 || playerY > screenHeight) {
            lives--;
            if (lives <= 0) {
                gameRunning = false;
            }
            else {
                playerX = screenWidth / 2;
                playerY = screenHeight / 2;
            }
        }
        if (gameTime > 0) {
            gameTime--;
        }
        else {
            gameRunning = false;
        }
        for (const auto& obstacle : obstacles) {
            if (isCollision(playerX, playerY, playerSize, obstacle.first, obstacle.second, obstacleSize)) {
                lives--;
                if (lives <= 0) {
                    gameRunning = false;
                }
                playerX = screenWidth / 2;
                playerY = screenHeight / 2;
            }
        }
        for (auto it = collectables.begin(); it != collectables.end();) {
            if (isCollision(playerX, playerY, playerSize, it->first, it->second, collectableSize)) {
                score++;
                it = collectables.erase(it);
                drawGameScore(); 
            }
            else {
                ++it;
            }
        }

        for (auto it = powerUps.begin(); it != powerUps.end();) {
            if (isCollision(playerX, playerY, playerSize, it->x, it->y, powerUpSize)) {
                if (it->type == 1) {
                    playerSpeed = 10; 
                    powerUpType1Timer = powerUpType1Duration; 
                }
                if (it->type == 2) {
                    powerUpType2Timer = powerUpType2Duration; 
                }
                it = powerUps.erase(it);
            }
            else {
                ++it;
            }
        }

        if (powerUpType1Timer > 0) {
            powerUpType1Timer--;
            if (powerUpType1Timer == 0) {
                playerSpeed = 5; 
            }
        }
        if (powerUpType2Timer > 0) {
            score += 1;
            powerUpType2Timer--;

        }
        if (!hasReachedEndGame && isCollision(playerX, playerY, playerSize, endGameSquareX, endGameSquareY, endGameSquareSize)) {
            hasReachedEndGame = true;
            gameRunning = false;
            hasReachedTarget = true;
        }
        if (gameTime == 0) {
            gameRunning = false;
            hasTimeRunOut = true;
        }

        glutTimerFunc(60, update, 0);
    }
    glutPostRedisplay();
}
void keyboard(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_RIGHT:
        playerRotation = 0;
        break;
    case GLUT_KEY_UP:
        playerRotation = 90;
        break;
    case GLUT_KEY_LEFT:
        playerRotation = 180;
        break;
    case GLUT_KEY_DOWN:
        playerRotation = 270;
        break;
    }
    glutPostRedisplay(); 
}

void drawRotatingSquares() {
    float squareSize = 10.0f;
    float centerX = screenWidth / 2;
    float centerY = screenHeight / 2;
    float distanceFromCenter = 100.0f;

    glColor3f(1, 1, 1);
    for (int i = 0; i < 4; i++) {
        glPushMatrix();
        glTranslatef(centerX + distanceFromCenter * cos(rotationAngle + i * 90 * 3.1415926 / 180),
            centerY + distanceFromCenter * sin(rotationAngle + i * 90 * 3.1415926 / 180), 0);
        glRotatef(rotationAngle + 90 * i, 0, 0, 1);
        drawSquare(0, squareSize * 2, squareSize);
        glPopMatrix();
    }
}

void draw() {
    glClear(GL_COLOR_BUFFER_BIT);

    if (gameRunning) {
        glColor3f(1.0f, 1.0f, 0.0f); 
        glLineWidth(10.0); 

        //two line boundries
        glBegin(GL_LINES);
        glVertex2f(0, screenHeight);
        glVertex2f(screenWidth, screenHeight);
        glVertex2f(0, 0);
        glVertex2f(screenWidth, 0);
        glVertex2f(0, screenHeight);
        glVertex2f(0, 0);
        glVertex2f(screenWidth, screenHeight);
        glVertex2f(screenWidth, 0);
        glEnd();

        glColor3f(1.0f, 0.0f, 0.0f); 
        glLineWidth(5.0);
        glBegin(GL_LINES);
        glVertex2f(0, screenHeight);
        glVertex2f(screenWidth, screenHeight);
        glVertex2f(0, 0);
        glVertex2f(screenWidth, 0);
        glVertex2f(0, screenHeight);
        glVertex2f(0, 0);
        glVertex2f(screenWidth, screenHeight);
        glVertex2f(screenWidth, 0);
        glEnd();

        glLineWidth(1.0);

        drawPlayer();
        drawObstacles();
        drawCollectables();
        drawPowerUps();
        drawHealthBar();
        drawGameTime();
        drawGameScore();
        drawEndGameSquare(); 
        drawRotatingSquares();
    }
    else {
        if (hasReachedEndGame && !hasTimeRunOut) {
            glColor3f(1.0f, 1.0f, 1.0f); 
            std::string message = "You won!";
            int messageWidth = 100; 
            int messageHeight = 20; 
            int messageX = (screenWidth - messageWidth) / 2;
            int messageY = screenHeight / 2;
            glRasterPos2f(messageX, messageY);
            renderBitmapString(messageX, messageY, GLUT_BITMAP_HELVETICA_18, message);
            std::string scoreMessage = "Your score: " + std::to_string(score);
            int scoreX = (screenWidth - messageWidth) / 2;
            int scoreY = messageY - messageHeight - 10; 
            glRasterPos2f(scoreX, scoreY);
            renderBitmapString(scoreX, scoreY, GLUT_BITMAP_HELVETICA_18, scoreMessage);
        }
        else if (hasTimeRunOut || lives <= 0) {
            glColor3f(1.0f, 1.0f, 1.0f); 
            std::string message = "You lost!";
            int messageWidth = 100; 
            int messageHeight = 20;
            int messageX = (screenWidth - messageWidth) / 2;
            int messageY = screenHeight / 2;
            glRasterPos2f(messageX, messageY);
            renderBitmapString(messageX, messageY, GLUT_BITMAP_HELVETICA_18, message);
            std::string scoreMessage = "Your score: " + std::to_string(score);
            int scoreX = (screenWidth - messageWidth) / 2;
            int scoreY = messageY - messageHeight - 10; 
            glRasterPos2f(scoreX, scoreY);
            renderBitmapString(scoreX, scoreY, GLUT_BITMAP_HELVETICA_18, scoreMessage);
        }
    }

    glutSwapBuffers();
}

void updateRotation(int value) {
    rotationAngle += 1.0f; 
    glutPostRedisplay();
    glutTimerFunc(100, updateRotation, 0); 
}

int getRandomNumber() {
    return rand() % 5;
}

std::pair<float, float> generateRandomPosition(float size, const std::vector<std::pair<float, float>>& existingEntities, float padding) {
    while (true) {
        int entityX = rand() % static_cast<int>(screenWidth - size - 2 * padding) + static_cast<int>(size + padding);
        int entityY = rand() % static_cast<int>(screenHeight - size - 2 * padding) + static_cast<int>(size + padding);

        float entityXFloat = static_cast<float>(entityX);
        float entityYFloat = static_cast<float>(entityY);

        bool collision = false;
        for (const auto& existingEntity : existingEntities) {
            if (isCollision(entityXFloat, entityYFloat, size, existingEntity.first, existingEntity.second, size)) {
                collision = true;
                break;
            }
        }
        if (!collision) {
            return std::make_pair(entityXFloat, entityYFloat);
        }
    }
}

int main(int argc, char** argv) {
    srand(static_cast<unsigned>(time(0)));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(screenWidth, screenHeight);
    glutCreateWindow("Game");

    glutDisplayFunc(draw);

    glutSpecialFunc(keyboard);

    int numObstacles = 10;
    for (int i = 0; i < numObstacles; i++) {
        obstacles.push_back(generateRandomPosition(obstacleSize, obstacles, 0));
    }

    for (int i = 0; i < maxCollectables; i++) {
        collectables.push_back(generateRandomPosition(collectableSize, collectables, 10));
    }

    for (int i = 0; i < maxPowerUps; i++) {
        std::pair<float, float> powerUpPos1 = generateRandomPosition(powerUpSize, obstacles, 10);
        std::pair<float, float> powerUpPos2 = generateRandomPosition(powerUpSize, obstacles, 10);

        PowerUp newPowerUpType1;
        newPowerUpType1.x = powerUpPos1.first;
        newPowerUpType1.y = powerUpPos1.second;
        newPowerUpType1.type = 1;
        powerUps.push_back(newPowerUpType1);

        PowerUp newPowerUpType2;
        newPowerUpType2.x = powerUpPos2.first;
        newPowerUpType2.y = powerUpPos2.second;
        newPowerUpType2.type = 2;
        powerUps.push_back(newPowerUpType2);
    }

    glutTimerFunc(0, updateRotation, 0);
    glutTimerFunc(0, update, 0);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenWidth, 0, screenHeight, -1, 1);

    glutMainLoop();
    return 0;
}