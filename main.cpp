#include<GL/glew.h>
#include<GL/freeglut_std.h>
#include<GL/gl.h>
#include<GL/glut.h>
#include<cstdio>
#include<cstdlib>
#include<stdlib.h>
#include "minesweeper.h" // implements the minesweeper class
#include<vector>
#include <iostream>




MinesweeperGame game;
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    game.draw();
    glutSwapBuffers();
}

void update(int value) {
    game.updateAnimations();
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void mouse(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        if (button == 3) game.handleZoom(1);
        else if (button == 4) game.handleZoom(-1);
        else game.handleInput(button, state, x, y);
    }
    glutPostRedisplay();
}

void reshape(int w, int h) {
    windowWidth = w; windowHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 1, 0, 1);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 800);
    glutCreateWindow("Minesweeper Final");
    glutFullScreen();

    if (glewInit() != GLEW_OK) return -1;

    game.setupVBO();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutTimerFunc(0, update, 0);
    glutMainLoop();
    return 0;
}