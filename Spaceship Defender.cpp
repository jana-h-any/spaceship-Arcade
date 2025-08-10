#include <GL/glut.h>
#include <GL/glu.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <stdio.h>
#include <math.h>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>

using namespace std;

// ===== Global Variables =====
float shipX = 0.0f, shipY = -4.0f, shipZ = -15.0f;  // Stationary at bottom
float shipSpeed = 0.4f;
float enemySpeed = 0.01f;
float tireRotationAngle = 0.0f;

// Game parameters
int score = 0;
int highScore = 0;
int lives = 3;
bool gameOver = false;
bool gamePaused = false;
float gameTime = 0.0f;
float spawnTimer = 0.0f;
float spawnInterval = 3.0f; // Time between enemy spawns

// Camera variables (fixed view)
float camX = 0.0f, camY = 0.0f, camZ = 5.0f;
float camLookX = 0.0f, camLookY = 0.0f, camLookZ = -15.0f;

// Starfield variables
const int NUM_STARS = 1000;
float movementSpeed = 0.1f;
struct Star {
    float x, y, z;
    float brightness;
    float speed;
};
Star stars[NUM_STARS];

// Enemy spaceship variables
struct Enemy {
    float x, y, z;
    float angle;
    bool active;
    bool hit;
    float hitTimer;
    float speed;
};
vector<Enemy> enemies;
const int MAX_ENEMIES = 5;

// Laser variables
struct Laser {
    float x, y, z;
    float speed;
    bool active;
};
vector<Laser> lasers;
float laserSpeed = 1.5f;

// Explosion effects
struct Explosion {
    float x, y, z;
    float size;
    float time;
    float maxTime;
};
vector<Explosion> explosions;

// High score file
const string HIGH_SCORE_FILE = "highscore.txt";

// ===== Function Declarations =====
void initializeStars();
void spawnEnemy();
void setupLighting();
void drawSpaceship();
void drawEnemySpaceship(float x, float y, float z, float angle, bool hit);
void drawStarfield();
void drawText(float x, float y, string text);
void drawHUD();
void resetGame();
void updateEnemies(float deltaTime);
void fireLaser();
void updateLasers(float deltaTime);
void drawLaser(float x, float y, float z);
void addExplosion(float x, float y, float z);
void updateExplosions(float deltaTime);
void drawExplosion(float x, float y, float z, float progress);
void loadHighScore();
void saveHighScore();

void loadHighScore() {
    ifstream file(HIGH_SCORE_FILE);
    if (file.is_open()) {
        file >> highScore;
        file.close();
    }
}

void saveHighScore() {
    ofstream file(HIGH_SCORE_FILE);
    if (file.is_open()) {
        file << highScore;
        file.close();
    }
}

void initializeStars() {
    std::srand(std::time(0));
    for (int i = 0; i < NUM_STARS; ++i) {
        stars[i].x = (std::rand() % 2000 - 1000) / 100.0f;
        stars[i].y = (std::rand() % 2000 - 1000) / 100.0f;
        stars[i].z = (std::rand() % 1000) / -10.0f - 15.0f;
        stars[i].brightness = (std::rand() % 100) / 100.0f * 0.5f + 0.5f;
        stars[i].speed = movementSpeed * (0.5f + stars[i].brightness);
    }
}

void spawnEnemy() {
    if (enemies.size() >= MAX_ENEMIES) return;

    Enemy e;
    e.x = (rand() % 16) - 8.0f;  // Random X position between -8 and 8
    e.y = 10.0f;                  // Start above the screen
    e.z = -15.0f;
    e.angle = 0.0f;
    e.active = true;
    e.hit = false;
    e.hitTimer = 0.0f;
    e.speed = enemySpeed + (rand() % 40) / 500.0f; // Random speed
    enemies.push_back(e);
}

void setupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    // Orange light for spaceship
    GLfloat light0_position[] = { 0.0f, -3.0f, -15.0f, 1.0f };
    GLfloat light0_diffuse[] = { 1.0f, 0.6f, 0.0f, 1.0f };
    GLfloat light0_specular[] = { 1.0f, 0.7f, 0.2f, 1.0f };
    GLfloat light0_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);

    // White ambient light
    GLfloat light1_position[] = { 0.0f, 5.0f, -10.0f, 1.0f };
    GLfloat light1_ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat light1_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat light1_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);
}

void drawSpaceshipBase() {
    glPushMatrix();
    glColor3f(0.6f, 0.6f, 0.6f);
    glScalef(1.5f, 0.3f, 1.5f);
    glutSolidSphere(1.0f, 50, 50);
    glPopMatrix();
}

void drawGlassDome() {
    glPushMatrix();
    glTranslatef(0.0f, 0.3f, 0.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.3f, 0.7f, 1.0f, 0.5f);
    glutSolidSphere(0.6f, 30, 30);
    glDisable(GL_BLEND);
    glPopMatrix();
}

void drawSideLights() {
    float positions[2] = { -0.9f, 0.9f };
    for (int i = 0; i < 2; i++) {
        glPushMatrix();
        glTranslatef(positions[i], -0.1f, 1.1f - fabs(positions[i]));
        glColor3f(1.0f, 0.9f, 0.0f);
        glutSolidSphere(0.15f, 20, 20);
        glPopMatrix();
    }
}

void drawThrusterFlame(float offsetX) {
    glPushMatrix();
    glTranslatef(offsetX, 0.01f, 1.7f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glColor4f(1.0f, 0.3f, 0.0f, 0.2f);
    glutSolidSphere(0.2f, 20, 20);
    glDisable(GL_BLEND);
    glColor3f(1.0f, 0.4f, 0.0f);
    glRotatef(180, 1, 0, 0);
    float flameHeight = 0.4f + 0.05f * sin(glutGet(GLUT_ELAPSED_TIME) * 0.001f);
    glutSolidCone(0.2f, flameHeight, 20, 20);
    glPopMatrix();
}

void drawSpaceship() {
    glPushMatrix();
    glTranslatef(shipX, shipY, shipZ);

    drawSpaceshipBase();
    drawGlassDome();
    drawSideLights();
    drawThrusterFlame(-0.6f);
    drawThrusterFlame(0.6f);

    glPopMatrix();
}

void drawCube(float x, float y, float z, float size) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glutSolidCube(size);
    glPopMatrix();
}

void drawCylinder(float x, float y, float z, float height, float radius) {
    GLUquadric* quad = gluNewQuadric();
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(-90, 1, 0, 0);
    gluCylinder(quad, radius, radius, height, 20, 20);
    glPopMatrix();
    gluDeleteQuadric(quad);
}

void drawSphere(float x, float y, float z, float radius) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glutSolidSphere(radius, 20, 20);
    glPopMatrix();
}

void drawEnemySpaceshipBase(float scale) {
    glPushMatrix();
    glColor3f(0.8f, 0.2f, 0.2f); // Red color for enemy ships
    glScalef(1.5f * scale, 0.3f * scale, 1.5f * scale);
    glutSolidSphere(1.0f, 50, 50);
    glPopMatrix();
}

void drawEnemyGlassDome(float scale) {
    glPushMatrix();
    glTranslatef(0.0f, 0.3f * scale, 0.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 0.3f, 0.3f, 0.5f); // Red tinted glass
    glutSolidSphere(0.6f * scale, 30, 30);
    glDisable(GL_BLEND);
    glPopMatrix();
}

void drawEnemySideLights(float scale) {
    float positions[2] = { -0.9f, 0.9f };
    for (int i = 0; i < 2; i++) {
        glPushMatrix();
        glTranslatef(positions[i] * scale, -0.1f * scale, (1.1f - fabs(positions[i])) * scale);
        glColor3f(1.0f, 0.0f, 0.0f); // Red lights
        glutSolidSphere(0.15f * scale, 20, 20);
        glPopMatrix();
    }
}

void drawEnemyThrusterFlame(float offsetX, float scale) {
    glPushMatrix();
    glTranslatef(offsetX * scale, 0.01f * scale, 1.7f * scale);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glColor4f(1.0f, 0.0f, 0.0f, 0.2f); // Red flame
    glutSolidSphere(0.2f * scale, 20, 20);
    glDisable(GL_BLEND);
    glColor3f(1.0f, 0.0f, 0.0f);
    glRotatef(180, 1, 0, 0);
    float flameHeight = (0.4f + 0.05f * sin(glutGet(GLUT_ELAPSED_TIME) * 0.001f) * scale);
    glutSolidCone(0.2f * scale, flameHeight, 20, 20);
    glPopMatrix();
}

void drawEnemySpaceship(float x, float y, float z, float angle, bool hit) {
    float scale = 0.6f; // Smaller than player's ship

    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(angle, 0.0f, 1.0f, 0.0f);

    if (hit) {
        glColor3f(1.0f, 1.0f, 1.0f); // White when hit
    }
    else {
        glColor3f(0.8f, 0.2f, 0.2f); // Red color for enemy ships
    }

    drawEnemySpaceshipBase(scale);
    drawEnemyGlassDome(scale);
    drawEnemySideLights(scale);
    drawEnemyThrusterFlame(-0.6f, scale);
    drawEnemyThrusterFlame(0.6f, scale);

    glPopMatrix();
}

void drawLaser(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // Laser core (bright white)
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glutSolidSphere(0.1f, 10, 10);

    // Outer glow (blue)
    glColor4f(0.2f, 0.2f, 1.0f, 0.5f);
    glutSolidSphere(0.2f, 10, 10);

    // Draw the laser beam (shotgun spread)
    glColor4f(0.0f, 0.5f, 1.0f, 0.3f);
    glRotatef(90, 1.0f, 0.0f, 0.0f);
    GLUquadric* quad = gluNewQuadric();

    // Main center beam
    gluCylinder(quad, 0.05f, 0.05f, 5.0f, 10, 10);

    // Additional beams for shotgun effect
    for (int i = 0; i < 5; i++) {
        glPushMatrix();
        float offsetX = (rand() % 100 - 50) / 200.0f;
        float offsetY = (rand() % 100 - 50) / 200.0f;
        glTranslatef(offsetX, offsetY, 0);
        gluCylinder(quad, 0.03f, 0.03f, 3.0f + (rand() % 100) / 100.0f, 8, 8);
        glPopMatrix();
    }

    gluDeleteQuadric(quad);

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

void drawExplosion(float x, float y, float z, float progress) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // Explosion core
    float coreSize = 0.5f * progress;
    glColor4f(1.0f, 0.8f, 0.0f, 1.0f);
    glutSolidSphere(coreSize, 20, 20);

    // Outer explosion
    float outerSize = 1.0f * progress;
    glColor4f(1.0f, 0.3f, 0.0f, 1.0f - progress);
    glutSolidSphere(outerSize, 20, 20);

    // Debris particles
    glPointSize(3.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < 20; i++) {
        float dist = progress * 2.0f;
        float px = (rand() % 100 - 50) / 50.0f * dist;
        float py = (rand() % 100 - 50) / 50.0f * dist;
        float pz = (rand() % 100 - 50) / 50.0f * dist;
        float life = 1.0f - progress;
        glColor4f(1.0f, 0.5f + (rand() % 50) / 100.0f, 0.0f, life);
        glVertex3f(px, py, pz);
    }
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

void updateStars() {
    for (int i = 0; i < NUM_STARS; ++i) {
        stars[i].z += stars[i].speed;
        if (stars[i].z > 0.0f) {
            stars[i].x = (std::rand() % 2000 - 1000) / 100.0f;
            stars[i].y = (std::rand() % 2000 - 1000) / 100.0f;
            stars[i].z = -15.0f;
            stars[i].brightness = (std::rand() % 100) / 100.0f * 0.5f + 0.5f;
            stars[i].speed = movementSpeed * (0.5f + stars[i].brightness);
        }
    }
}

void updateEnemies(float deltaTime) {
    for (auto it = enemies.begin(); it != enemies.end(); ) {
        if (!it->active) {
            ++it;
            continue;
        }

        // Move enemy downward
        it->y -= it->speed;

        // Random horizontal movement
        if (rand() % 100 < 3) { // 3% chance to change direction
            it->angle = (rand() % 3 - 1) * 30.0f; // -30, 0, or 30 degrees
        }

        // Apply horizontal movement based on angle
        it->x += sin(it->angle * 3.14159f / 180.0f) * it->speed * 0.5f;

        // Keep within bounds
        it->x = max(-8.0f, min(8.0f, it->x));

        // Check if enemy reached the bottom (hit spaceship)
        if (it->y < shipY + 1.0f && !it->hit) {
            lives--;
            addExplosion(it->x, it->y, it->z);
            it->active = false;
            if (lives <= 0) {
                gameOver = true;
                if (score > highScore) {
                    highScore = score;
                    saveHighScore();
                }
            }
        }

        // Remove if below screen or hit
        if (it->y < -6.0f || it->hit) {
            it = enemies.erase(it);
        }
        else {
            ++it;
        }
    }
}

void fireLaser() {
    if (gameOver || gamePaused) return;

    // Create multiple lasers for shotgun effect
    for (int i = 0; i < 5; i++) {
        Laser laser;
        laser.x = shipX + (rand() % 100 - 50) / 100.0f; // Small random spread
        laser.y = shipY + 1.0f;
        laser.z = shipZ;
        laser.speed = laserSpeed * (0.8f + (rand() % 40) / 100.0f); // Slightly random speed
        laser.active = true;
        lasers.push_back(laser);
    }
}

void updateLasers(float deltaTime) {
    for (auto it = lasers.begin(); it != lasers.end(); ) {
        it->y += it->speed;

        bool hit = false;

        // Check collision with enemies
        for (auto& enemy : enemies) {
            if (enemy.active && !enemy.hit) {
                float dx = it->x - enemy.x;
                float dy = it->y - enemy.y;
                float distance = sqrt(dx * dx + dy * dy);

                if (distance < 1.0f) { // Hit detection
                    enemy.hit = true;
                    score += 10;
                    addExplosion(enemy.x, enemy.y, enemy.z);
                    hit = true;
                    break;
                }
            }
        }

        // Remove laser if it hit something or went off screen
        if (hit || it->y > 10.0f) {
            it = lasers.erase(it);
        }
        else {
            ++it;
        }
    }
}

void addExplosion(float x, float y, float z) {
    Explosion exp;
    exp.x = x;
    exp.y = y;
    exp.z = z;
    exp.size = 1.0f;
    exp.time = 0.0f;
    exp.maxTime = 0.5f;
    explosions.push_back(exp);
}

void updateExplosions(float deltaTime) {
    for (auto it = explosions.begin(); it != explosions.end(); ) {
        it->time += deltaTime;
        if (it->time >= it->maxTime) {
            it = explosions.erase(it);
        }
        else {
            ++it;
        }
    }
}

void drawStarfield() {
    glDisable(GL_LIGHTING);
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < NUM_STARS; ++i) {
        glColor3f(stars[i].brightness, stars[i].brightness, stars[i].brightness);
        glVertex3f(stars[i].x, stars[i].y, stars[i].z);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

void drawHUD() {
    // Switch to 2D projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Draw score
    glColor3f(1.0f, 1.0f, 1.0f);
    stringstream ss;
    ss << "Score: " << score;
    glRasterPos2i(20, h - 30);
    for (char c : ss.str()) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    // Draw high score
    ss.str("");
    ss << "High Score: " << highScore;
    glRasterPos2i(20, h - 60);
    for (char c : ss.str()) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    // Draw lives
    ss.str("");
    ss << "Lives: " << lives;
    glRasterPos2i(20, h - 90);
    for (char c : ss.str()) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    // Draw game over message
    if (gameOver) {
        glColor3f(1.0f, 0.0f, 0.0f);
        string gameOverText = "GAME OVER! Press R to restart";
        glRasterPos2i(w / 2 - gameOverText.length() * 4, h / 2);
        for (char c : gameOverText) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
        }
    }

    // Draw pause message
    if (gamePaused) {
        glColor3f(1.0f, 1.0f, 0.0f);
        string pauseText = "PAUSED - Press P to resume";
        glRasterPos2i(w / 2 - pauseText.length() * 4, h / 2 + 30);
        for (char c : pauseText) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
        }
    }

    // Restore previous projection
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW); 
}

void resetGame() {
    if (score > highScore) {
        highScore = score;
        saveHighScore();
    }
    score = 0;
    lives = 3;
    gameOver = false;
    gamePaused = false;
    gameTime = 0.0f;
    spawnTimer = 0.0f;
    enemies.clear();
    lasers.clear();
    explosions.clear();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(camX, camY, camZ, camLookX, camLookY, camLookZ, 0, 1, 0);

    drawStarfield();
    drawSpaceship();    

    // Draw all active enemies
    for (const auto& enemy : enemies) {
        if (enemy.active && !enemy.hit) {  // Only draw non-hit enemies
            drawEnemySpaceship(enemy.x, enemy.y, enemy.z, enemy.angle, enemy.hit);
        }
    }

    // Draw all active lasers
    for (const auto& laser : lasers) {
        drawLaser(laser.x, laser.y, laser.z);
    }

    // Draw explosions
    for (const auto& exp : explosions) {
        drawExplosion(exp.x, exp.y, exp.z, exp.time / exp.maxTime);
    }

    drawHUD();

    glutSwapBuffers();
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)width / height, 0.1, 200.0);
    glMatrixMode(GL_MODELVIEW);
}

void update(int value) {
    float deltaTime = 0.016f; // Approximate 60 FPS

    if (!gameOver && !gamePaused) {
        gameTime += deltaTime;
        spawnTimer += deltaTime;

        // Spawn new enemies periodically
        if (spawnTimer >= spawnInterval) {
            spawnTimer = 0.0f;
            spawnEnemy();

            // Increase spawn rate over time
            spawnInterval = max(0.5f, 2.0f - gameTime / 30.0f);
        }

        tireRotationAngle += 5.0f;
        if (tireRotationAngle >= 360.0f) tireRotationAngle -= 360.0f;

        updateStars();
        updateEnemies(deltaTime);
        updateLasers(deltaTime);
        updateExplosions(deltaTime);
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void keyboard(unsigned char key, int x, int y) {
    switch (tolower(key)) {
    case ' ': fireLaser(); break;
    case 'r': resetGame(); break;
    case 'p': gamePaused = !gamePaused; break; // Toggle pause
    case 27: exit(0); break;
    }
    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_LEFT: shipX -= shipSpeed; break;
    case GLUT_KEY_RIGHT: shipX += shipSpeed; break;
    }
    // Keep spaceship within bounds
    shipX = max(-8.0f, min(8.0f, shipX));
    glutPostRedisplay();
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    setupLighting();
    initializeStars();
    loadHighScore();

    glClearColor(0.02f, 0.02f, 0.08f, 1.0f);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Space Defender");

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys); // Register special key callback
    glutTimerFunc(0, update, 0);

    printf("=== SPACE DEFENDER ===\n");
    printf("Controls:\n");
    printf("Move: LEFT ARROW (left), RIGHT ARROW (right)\n");
    printf("Shoot: SPACE (shotgun blast)\n");
    printf("Pause: P\n");
    printf("Enemy ships will come at you from above\n");
    printf("Shoot them before they reach you!\n");
    printf("Each hit scores 10 points\n");
    printf("You have 3 lives\n");
    printf("Press 'R' to restart game\n");
    printf("ESC to exit\n");

    glutMainLoop();
    return 0;
}
