// FOR CLASS IMPLEMENTATION TO BE USED IN MAIN.CPP

#include<GL/glew.h>
#include<GL/freeglut_std.h>
#include<GL/gl.h>
#include<GL/glut.h>
#include<cstdio>
#include<stdlib.h>
#include<vector>

struct ShapeData{
    GLuint vboID;
    GLenum mode;
    GLsizei count;
};
struct Difficulty{
    int width;
    int height;
    int mines;
};
const Difficulty EASY = {9, 9, 10};
enum CurrentGameMode{
    MENU,
    PLAYING,
    GAME_OVER,
    GAME_WON
};

struct tile{
    bool isMine = false;
    int neighborMines = 0;
    bool isCovered = true;
    bool isFlagged = false;
};

class minesweeper{
    public:
        void scaleResetCircle();
        void setupGame();
        void handleLeftClick();
        void handleRightClick();
        void setupVBO();

    private:
        void initializeGrid();
        void calculateNeighborCounts();
        void checkWinCondition();
        void revealTile(int x, int y);
        void drawTileVBO(int x, int y, int windowWidth, int windowHeight);
        void flagRotate();
        void backgroundMotion();
};