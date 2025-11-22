// FOR EASIER IMPORTING TO MAIN.CPP

#ifndef MINESWEEEPER_H
#define MINESWEEEPER_H

#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cstdio> 
#include <string>

#include<GL/glew.h>
#include<GL/freeglut_std.h>
#include<GL/gl.h>
#include<GL/glut.h>

// holds vbo data
struct ShapeData{
    GLuint vboID;
    GLenum mode;
    GLsizei count;
};

struct Difficulty {
    int width;
    int height;
    int mines;
};
// toggles the game state
enum CurrentGameMode{
    MENU,
    PLAYING,
    GAME_OVER,
    GAME_WON
};
// tile data for the game
struct Tile{
    bool isMine = false;
    int neighborMines = 0;
    bool isCovered = true;
    bool isFlagged = false;

    float flipAngle = 180.0f; // Starts "face down"
    bool isFlipping = false;
};

const Difficulty MEDIUM = { 10, 10, 15 }; // 10x10 grid for good visibility

extern int windowWidth;
extern int windowHeight;

const float MAX_GRID_SIZE = 0.85f;
const float ZOOM_SPEED = 0.1f;

extern float bgScrollX ;
extern float bgScrollY ;
extern float gridScale ;
extern float flagWaveTime ;

class MinesweeperGame{
    public:
        // initialize values
        CurrentGameMode currentState = MENU; 
        int gridWidth = 0;
        int gridHeight = 0;
        int totalMines = 0;
        std::vector<std::vector<Tile>> gameGrid;

        // contain the shape data into a struct
        ShapeData tileShape;

        // Constructor and Deconstructor
        MinesweeperGame() = default;
        ~MinesweeperGame();

        void setupGame(const Difficulty &diff);
        void resetGame();
        void updateAnimations();

        // Input Handling
        void handleInput(int button, int state, int x, int y);
        void handleZoom(int direction);

        // Drawing
        void setupVBO();
        void draw();

    private:
        void initializeGrid();
        void calculateNeighborCounts();
        void revealTile(int x, int y);
        void checkWinCondition();
        bool getGridCoords(float mouseX, float mouseY, int &gridX, int &gridY);

        bool checkButton(float mx, float my, float bx, float by, float bw, float bh);

        // Rendering Helpers
        void drawBackground(); // NEW: Moving Background
        void drawMenu();
        void drawGameOver();
        void drawGame();
        void drawTile(int x, int y, float xPos, float yPos, float size);
        void drawButton(float x, float y, float w, float h, const char *label, float r, float g, float b);
        void drawText(float x, float y, const char *str, void *font = GLUT_BITMAP_HELVETICA_18);
        void drawTextCentered(float x, float y, const char *str, void *font = GLUT_BITMAP_HELVETICA_18);
};
#endif