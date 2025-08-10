#include <GL/glut.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <fstream>

const int NUM_STARS = 1000;
float movementSpeed = 0.1f;

struct Star {
    float x, y, z;
    float brightness;
    float speed;
};

Star stars[NUM_STARS];

// Spaceship variables 
float shipX = -5.0f, shipY = 0.0f, shipZ = -10.0f;
float shipVelocity = 0.0f;
float gravity = -0.005f;
bool gameOver = false;
bool gamePaused = false;

struct Pipe {
    float x;
    float gapY;
    float gapSize = 6.0f;
};

std::vector<Pipe> pipes;
int score = 0;
int highScore = 0;

const char* HIGH_SCORE_FILE = "highscore.dat";

// Helper: render text to screen
void renderBitmapString(float x, float y, void* font, const char* string) {
    glRasterPos2f(x, y);
    while (*string) {
        glutBitmapCharacter(font, *string);
        ++string;
    }
}

void loadHighScore() {
    std::ifstream file(HIGH_SCORE_FILE, std::ios::binary);
    if (file.is_open()) {
        file.read(reinterpret_cast<char*>(&highScore), sizeof(highScore));
        file.close();
    }
}

void saveHighScore() {
    std::ofstream file(HIGH_SCORE_FILE, std::ios::binary);
    if (file.is_open()) {
        file.write(reinterpret_cast<const char*>(&highScore), sizeof(highScore));
        file.close();
    }
}

void drawTextOverlay() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);

    std::string scoreText = "Score: " + std::to_string(score);
    renderBitmapString(10, 570, GLUT_BITMAP_HELVETICA_18, scoreText.c_str());

    std::string highScoreText = "High Score: " + std::to_string(highScore);
    renderBitmapString(10, 540, GLUT_BITMAP_HELVETICA_18, highScoreText.c_str());

    if (gamePaused) {
        renderBitmapString(350, 300, GLUT_BITMAP_HELVETICA_18, "PAUSED");
        renderBitmapString(300, 270, GLUT_BITMAP_HELVETICA_18, "Press P to resume");
    }

    if (gameOver) {
        renderBitmapString(300, 300, GLUT_BITMAP_HELVETICA_18, "Game Over!");
        renderBitmapString(250, 270, GLUT_BITMAP_HELVETICA_18, "Press any key to restart...");
    }

    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void initializeStars() {
    std::srand(std::time(0));
    for (int i = 0; i < NUM_STARS; ++i) {
        stars[i].x = (std::rand() % 2000 - 1000) / 100.0f;
        stars[i].y = (std::rand() % 2000 - 1000) / 100.0f;
        stars[i].z = (std::rand() % 1000) / -10.0f - 1.0f;
        stars[i].brightness = (std::rand() % 100) / 100.0f * 0.5f + 0.5f;
        stars[i].speed = movementSpeed * (0.5f + stars[i].brightness);
    }
    pipes.push_back({ 20.0f, 0.0f });
    loadHighScore();
}

void updateStars() {
    for (int i = 0; i < NUM_STARS; ++i) {
        stars[i].z += stars[i].speed;
        if (stars[i].z > 0.0f) {
            stars[i].x = (std::rand() % 2000 - 1000) / 100.0f;
            stars[i].y = (std::rand() % 2000 - 1000) / 100.0f;
            stars[i].z = -100.0f;
            stars[i].brightness = (std::rand() % 100) / 100.0f * 0.5f + 0.5f;
            stars[i].speed = movementSpeed * (0.5f + stars[i].brightness);
        }
    }
}

void drawPlanet() {
    GLfloat ambient[] = { 0.3f, 0.25f, 0.25f, 1.0f };
    GLfloat diffuse[] = { 0.7f, 0.6f, 0.5f, 1.0f };
    GLfloat specular[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    GLfloat shininess = 30.0f;

    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);

    glPushMatrix();
    glTranslatef(0.0f, -14.0f, -30.0f);
    glScalef(6.0f, 1.0f, 1.0f);
    glRotatef(25, 1, 0, 0);
    glutSolidSphere(8.0, 100, 100);
    glPopMatrix();
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

void drawPipes() {
    glColor3f(0.2f, 1.0f, 0.2f);
    for (Pipe& pipe : pipes) {
        glPushMatrix();
        glTranslatef(pipe.x, 0.0f, -10.0f);

        float topY = 10.0f;
        float bottomY = -10.0f;

        // Top Pipe
        glPushMatrix();
        glTranslatef(0, pipe.gapY + pipe.gapSize / 2.0f + (topY - (pipe.gapY + pipe.gapSize / 2.0f)) / 2.0f, 0);
        glScalef(1.0f, (topY - (pipe.gapY + pipe.gapSize / 2.0f)), 1.0f);
        glutSolidCube(1.0);
        glPopMatrix();

        // Bottom Pipe
        glPushMatrix();
        glTranslatef(0, bottomY + ((pipe.gapY - pipe.gapSize / 2.0f) - bottomY) / 2.0f, 0);
        glScalef(1.0f, ((pipe.gapY - pipe.gapSize / 2.0f) - bottomY), 1.0f);
        glutSolidCube(1.0);
        glPopMatrix();

        glPopMatrix();
    }
}

void updateGame() {
    if (gameOver || gamePaused) return;

    shipVelocity += gravity;
    shipY += shipVelocity;

    // Add new pipe
    if (pipes.empty() || pipes.back().x < 10.0f) {
        float gapY = (std::rand() % 150 - 75) / 10.0f;
        pipes.push_back({ 20.0f, gapY });
    }

    // Update pipes
    for (Pipe& pipe : pipes) {
        pipe.x -= 0.1f;

        if (pipe.x < -5.5f && pipe.x > -6.0f) {
            if (shipY < pipe.gapY - pipe.gapSize / 2.0f || shipY > pipe.gapY + pipe.gapSize / 2.0f) {
                gameOver = true;
                if (score > highScore) {
                    highScore = score;
                    saveHighScore();
                }
            }
        }
    }

    // Remove off-screen pipe
    if (!pipes.empty() && pipes.front().x < -20.0f) {
        pipes.erase(pipes.begin());
        score++;
    }

    // Out of bounds
    if (shipY < -10.0f || shipY > 10.0f) {
        gameOver = true;
        if (score > highScore) {
            highScore = score;
            saveHighScore();
        }
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0, 0, 0, 0, 0, -1, 0, 1, 0);

    glDisable(GL_LIGHTING);
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < NUM_STARS; ++i) {
        glColor3f(stars[i].brightness, stars[i].brightness, stars[i].brightness);
        glVertex3f(stars[i].x, stars[i].y, stars[i].z);
    }
    glEnd();

    glEnable(GL_LIGHTING);
    drawPlanet();
    drawSpaceship();
    drawPipes();

    drawTextOverlay();

    glutSwapBuffers();
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)width / height, 1.0, 200.0);
    glMatrixMode(GL_MODELVIEW);
}

void animate(int value) {
    if (!gamePaused) {
        updateStars();
        updateGame();
    }
    glutPostRedisplay();
    glutTimerFunc(16, animate, 0);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 27: // ESC key
        exit(0);
        break;

    case 'p':
    case 'P':
        gamePaused = !gamePaused;
        break;

    case ' ':
        if (!gameOver && !gamePaused) {
            shipVelocity = 0.08f;
        }
        break;

    default:
        if (gameOver) {
            // Restart game
            shipY = 0.0f;
            shipVelocity = 0.0f;
            pipes.clear();
            pipes.push_back({ 20.0f, 0.0f });
            gameOver = false;
            score = 0;
        }
        break;
    }
}

void setupLighting() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat lightPos[] = { 0.0f, 10.0f, 10.0f, 1.0f };
    GLfloat lightAmbient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    GLfloat lightDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Flappy Spaceship");

    initializeStars();
    setupLighting();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(0, animate, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glutMainLoop();
    return 0;

}
