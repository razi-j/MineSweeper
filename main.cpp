#include<GL/glew.h>
#include<GL/freeglut_std.h>
#include<GL/gl.h>
#include<GL/glut.h>
#include<cstdio>
#include<stdlib.h>
#include "minesweeper.h" // implements the minesweeper class
#include<vector>


int main(int argc, char **argv){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE| GLUT_RGB| GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("MineSweeper?!");

    glutMainLoop();
    return 0;
}