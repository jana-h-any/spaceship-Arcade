#include <GL/glut.h>
#include <cstdlib>
#include <ctime>

const int NUM_STARS = 1000;
float movementSpeed = 0.1f;

struct Star {
    float x, y, z;
    float brightness;
    float speed;
};

Star stars[NUM_STARS];

enum AppState {
    MENU,
    FLAPPY,
    DEFENDER
};

AppState currentState = MENU;

void initializeStars() {
    srand(time(0));
    for (int i = 0; i < NUM_STARS; ++i) {
        stars[i].x = (rand() % 2000 - 1000) / 100.0f;
        stars[i].y = (rand() % 2000 - 1000) / 100.0f;
        stars[i].z = (rand() % 1000) / -10.0f - 15.0f;
        stars[i].brightness = (rand() % 100) / 100.0f * 0.5f + 0.5f;
        stars[i].speed = movementSpeed * (0.5f + stars[i].brightness);
    }
}

void updateStars() {
    for (int i = 0; i < NUM_STARS; ++i) {
        stars[i].z += stars[i].speed;
        if (stars[i].z > 0.0f) {
            stars[i].x = (rand() % 2000 - 1000) / 100.0f;
            stars[i].y = (rand() % 2000 - 1000) / 100.0f;
            stars[i].z = -15.0f;
            stars[i].brightness = (rand() % 100) / 100.0f * 0.5f + 0.5f;
            stars[i].speed = movementSpeed * (0.5f + stars[i].brightness);
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

void drawText(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    for (int i = 0; text[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
}


void displayMenu() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0, 0, 0, 0, 0, -1, 0, 1, 0);

    updateStars();
    drawStarfield();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    drawText(-0.4f, 0.6f, "  Welcome to Spaceship Arcade ");
    drawText(-0.5f, 0.48f, "=================================");
    drawText(-0.4f, 0.25f, "Press 1 - Play Flappy Spaceship");
    drawText(-0.4f, 0.0f, "Press 2 - Play Spaceship Defender");
    drawText(-0.45f, -0.4f, " Press ESC at any time to return to Menu");
    drawText(-0.25f, -0.6f, "~ Powered by Pixel ~");


    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glutSwapBuffers();
}

void display() {
    if (currentState == MENU)
        displayMenu();
}

void timer(int value) {
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void keyboard(unsigned char key, int x, int y) {
    if (currentState == MENU) {
        if (key == '1') {
            system("\"Flappy Spaceship.exe\"");
        }
        else if (key == '2') {
            system("\"Spaceship Defender.exe\"");
        }
    }
    else if (key == 27) {
        currentState = MENU;
    }
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)w / h, 0.1, 100);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Spaceship Menu");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING); 
    glEnable(GL_LIGHT0);  

    initializeStars();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}


