// FOR CLASS IMPLEMENTATION TO BE USED IN MAIN.CPP

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

int windowWidth = 800;
int windowHeight = 800;

const float MAX_GRID_SIZE = 0.85f;
const float ZOOM_SPEED = 0.1f;

float bgScrollX = 0.0f;
float bgScrollY = 0.0f;
float gridScale = 1.0f;
float flagWaveTime = 0.0f;

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
        void drawText(float x, float y, const char *str, void *font);
        void drawTextCentered(float x, float y, const char *str, void *font = GLUT_BITMAP_HELVETICA_18);
};

MinesweeperGame::~MinesweeperGame(){
    if (tileShape.vboID !=0){
        glDeleteBuffers(1, &tileShape.vboID);
    }
}
void MinesweeperGame::setupVBO() {
    // Simple Square (0,0) to (1,1)
    GLfloat tile_vertices[] = {
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
    };
    tileShape.mode = GL_TRIANGLES;
    tileShape.count = 6;
    glGenBuffers(1, &tileShape.vboID);
    glBindBuffer(GL_ARRAY_BUFFER, tileShape.vboID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tile_vertices), tile_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void MinesweeperGame::setupGame(const Difficulty& diff) {
    gridWidth = diff.width;
    gridHeight = diff.height;
    totalMines = diff.mines;
    gameGrid.assign(gridHeight, std::vector<Tile>(gridWidth));
    initializeGrid();
    gridScale = 1.0f;
    currentState = PLAYING;
}

void MinesweeperGame::initializeGrid() {
    for (auto& row : gameGrid) for (auto& t : row) t = Tile{};
    srand((unsigned int)time(0));
    int placed = 0;
    while (placed < totalMines) {
        int rx = rand() % gridWidth;
        int ry = rand() % gridHeight;
        if (!gameGrid[ry][rx].isMine) {
            gameGrid[ry][rx].isMine = true;
            placed++;
        }
    }
    calculateNeighborCounts();
}

void MinesweeperGame::calculateNeighborCounts() {
    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            if (gameGrid[y][x].isMine) continue;
            int count = 0;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    int nx = x + dx; int ny = y + dy;
                    if (nx >= 0 && nx < gridWidth && ny >= 0 && ny < gridHeight) {
                        if (gameGrid[ny][nx].isMine) count++;
                    }
                }
            }
            gameGrid[y][x].neighborMines = count;
        }
    }
}

void MinesweeperGame::updateAnimations() {
    // Update Flag Wave
    flagWaveTime += 0.2f;

    // Update Background Scroll
    bgScrollX += 0.001f;
    bgScrollY += 0.001f;
    // Wrap around to keep numbers small
    if (bgScrollX > 1.0f) bgScrollX -= 1.0f;
    if (bgScrollY > 1.0f) bgScrollY -= 1.0f;

    // Update Flip Animation
    for (auto& row : gameGrid) {
        for (auto& t : row) {
            if (t.isFlipping) {
                t.flipAngle -= 10.0f;
                if (t.flipAngle <= 0.0f) {
                    t.flipAngle = 0.0f;
                    t.isFlipping = false;
                }
            }
        }
    }
}

void MinesweeperGame::revealTile(int x, int y) {
    if (x < 0 || x >= gridWidth || y < 0 || y >= gridHeight) return;
    Tile& t = gameGrid[y][x];
    if (!t.isCovered || t.isFlagged) return;

    t.isCovered = false;
    t.isFlipping = true;
    t.flipAngle = 180.0f;

    if (t.isMine) {
        currentState = GAME_OVER;
        return;
    }

    if (t.neighborMines == 0) {
        for (int dy = -1; dy <= 1; ++dy)
            for (int dx = -1; dx <= 1; ++dx)
                if (dx != 0 || dy != 0) revealTile(x + dx, y + dy);
    }
    checkWinCondition();
}

void MinesweeperGame::checkWinCondition() {
    int safeTotal = (gridWidth * gridHeight) - totalMines;
    int revealed = 0;
    for (const auto& row : gameGrid)
        for (const auto& t : row)
            if (!t.isCovered && !t.isMine) revealed++;

    if (revealed == safeTotal) currentState = GAME_WON;
}

void MinesweeperGame::drawButton(float x, float y, float w, float h, const char* label, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();

    glColor3f(1, 1, 1);
    drawTextCentered(x + w / 2.0f, y + h / 2.0f - 0.01f, label, GLUT_BITMAP_TIMES_ROMAN_24);
}

void MinesweeperGame::drawBackground() {
    // Clear background with a dark blue
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindBuffer(GL_ARRAY_BUFFER, tileShape.vboID);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, (void*)0);

    // Draw semi-transparent shapes
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 1.0f, 1.0f, 0.05f); // Very faint white

    // Loop to create a grid of background particles
    for (int i = -2; i < 8; i++) {
        for (int j = -2; j < 8; j++) {
            glPushMatrix();

            // Calculate position with wrapping offset
            float spacing = 0.3f;
            float xPos = (i * spacing) + bgScrollX;
            float yPos = (j * spacing) + bgScrollY;

            // Wrapping logic: If it goes off screen, map it back
            // fmodf creates a repeating loop
            float wrappedX = fmodf(xPos, 1.5f);
            float wrappedY = fmodf(yPos, 1.5f);
            if (wrappedX < 0) wrappedX += 1.5f; // Handle negative loop
            if (wrappedY < 0) wrappedY += 1.5f;

            // Center offset (to cover full screen)
            glTranslatef(wrappedX - 0.2f, wrappedY - 0.2f, 0.0f);

            // Scale them down
            glScalef(0.1f, 0.1f, 1.0f);

            // Rotate them slightly for style
            glRotatef(45.0f, 0.0f, 0.0f, 1.0f);

            glDrawArrays(tileShape.mode, 0, tileShape.count);
            glPopMatrix();
        }
    }

    glDisable(GL_BLEND);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MinesweeperGame::drawMenu() {
    // Draw Background first
    drawBackground();

    glColor3f(1, 1, 1);
    drawTextCentered(0.5f, 0.7f, "MINESWEEPER", GLUT_BITMAP_TIMES_ROMAN_24);

    drawButton(0.35f, 0.5f, 0.3f, 0.1f, "PLAY", 0.0f, 0.6f, 0.0f);
    drawButton(0.35f, 0.35f, 0.3f, 0.1f, "EXIT", 0.8f, 0.0f, 0.0f);
}

void MinesweeperGame::drawGameOver() {
    // Overlay transparent black
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(1, 0); glVertex2f(1, 1); glVertex2f(0, 1);
    glEnd();
    glDisable(GL_BLEND);

    if (currentState == GAME_WON) {
        glColor3f(0, 1, 0);
        drawTextCentered(0.5f, 0.7f, "YOU WON!", GLUT_BITMAP_TIMES_ROMAN_24);
    }
    else {
        glColor3f(1, 0, 0);
        drawTextCentered(0.5f, 0.7f, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);
    }

    drawButton(0.35f, 0.5f, 0.3f, 0.1f, "RETRY", 0.0f, 0.0f, 0.8f);
    drawButton(0.35f, 0.35f, 0.3f, 0.1f, "MAIN MENU", 0.4f, 0.4f, 0.4f);
}

void MinesweeperGame::drawTile(int x, int y, float xPos, float yPos, float size) {
    const Tile& t = gameGrid[y][x];

    glPushMatrix();
    glTranslatef(xPos + size / 2, yPos + size / 2, 0.0f);

    // TRANSFORMATION: Tile Rotation
    if (t.isFlipping) {
        glRotatef(t.flipAngle, 0.0f, 1.0f, 0.0f);
    }
    else if (!t.isCovered) {
        glRotatef(0.0f, 0.0f, 1.0f, 0.0f);
    }

    glScalef(size, size, 1.0f);
    glTranslatef(-0.5f, -0.5f, 0.0f);

    if (t.isCovered) glColor3f(0.6f, 0.6f, 0.6f);
    else glColor3f(0.85f, 0.85f, 0.85f);

    if (t.isMine && !t.isCovered) glColor3f(1.0f, 0.2f, 0.2f);

    glBindBuffer(GL_ARRAY_BUFFER, tileShape.vboID);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, (void*)0);
    glDrawArrays(tileShape.mode, 0, tileShape.count);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glPopMatrix();

    // Contents
    float centerX = xPos + size / 2;
    float centerY = yPos + size / 2;

    if (t.isCovered && t.isFlagged) {
        glPushMatrix();
        // TRANSFORMATION: Flag Waving (Translate)
        float waveY = sin(flagWaveTime + x) * 0.005f;
        glTranslatef(0.0f, waveY, 0.0f);

        glColor3f(1, 0, 0);
        glBegin(GL_TRIANGLES);
        glVertex2f(centerX - size * 0.2f, centerY - size * 0.2f);
        glVertex2f(centerX - size * 0.2f, centerY + size * 0.3f);
        glVertex2f(centerX + size * 0.3f, centerY + size * 0.05f);
        glEnd();
        glColor3f(0, 0, 0);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glVertex2f(centerX - size * 0.2f, centerY - size * 0.3f);
        glVertex2f(centerX - size * 0.2f, centerY + size * 0.3f);
        glEnd();
        glPopMatrix();
    }
    else if (!t.isCovered && t.isMine) {
        glColor3f(0, 0, 0);
        glPointSize(5.0f * gridScale);
        glBegin(GL_POINTS); glVertex2f(centerX, centerY); glEnd();
    }
    else if (!t.isCovered && t.neighborMines > 0) {
        void* font = GLUT_BITMAP_TIMES_ROMAN_24;
        if (t.neighborMines == 1) glColor3f(0, 0, 1);
        else if (t.neighborMines == 2) glColor3f(0, 0.5, 0);
        else glColor3f(1, 0, 0);
        char num[2]; sprintf(num, "%d", t.neighborMines);
        drawText(centerX - size * 0.15f, centerY - size * 0.15f, num, font);
    }
}

void MinesweeperGame::drawGame() {
    // Draw the moving background first
    drawBackground();

    // TRANSFORMATION: Grid Zoom (Scale)
    glPushMatrix();
    glTranslatef(0.5f, 0.5f, 0.0f);
    glScalef(gridScale, gridScale, 1.0f);
    glTranslatef(-0.5f, -0.5f, 0.0f);

    float margin = 0.1f;
    float drawableSize = 1.0f - (margin * 2);
    float maxDim = (float)std::max(gridWidth, gridHeight);
    float tileSize = drawableSize / maxDim;
    float gap = tileSize * 0.05f;
    float actualSize = tileSize - gap;

    float totalW = gridWidth * tileSize;
    float totalH = gridHeight * tileSize;
    float startX = (1.0f - totalW) / 2.0f;
    float startY = (1.0f - totalH) / 2.0f;

    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            drawTile(x, y, startX + x * tileSize + gap / 2, startY + y * tileSize + gap / 2, actualSize);
        }
    }

    glPopMatrix(); // End Zoom

    drawButton(0.85f, 0.9f, 0.1f, 0.05f, "R", 0.7f, 0.7f, 0.7f);
    drawButton(0.05f, 0.9f, 0.1f, 0.05f, "X", 0.7f, 0.0f, 0.0f);
}

void MinesweeperGame::draw() {
    if (currentState == MENU) drawMenu();
    else {
        drawGame();
        if (currentState == GAME_OVER || currentState == GAME_WON) {
            drawGameOver();
        }
    }
}
void MinesweeperGame::drawText(float x, float y, const char* str, void* font = GLUT_BITMAP_HELVETICA_18) {
    glRasterPos2f(x, y);
    while (*str) {
        glutBitmapCharacter(font, *str++);
    }
}
void MinesweeperGame::drawTextCentered(float x, float y, const char* str, void* font) {
    int len = 0;
    const char* p = str;
    while (*p++) len++;
    float offset = len * 0.01f;
    drawText(x - offset, y, str, font);
}
bool MinesweeperGame::checkButton(float mx, float my, float bx, float by, float bw, float bh) {
    return (mx >= bx && mx <= bx + bw && my >= by && my <= by + bh);
}

bool MinesweeperGame::getGridCoords(float mx, float my, int& gx, int& gy) {
    float worldX = (mx - 0.5f) / gridScale + 0.5f;
    float worldY = (my - 0.5f) / gridScale + 0.5f;

    float margin = 0.1f;
    float drawableSize = 1.0f - (margin * 2);
    float maxDim = (float)std::max(gridWidth, gridHeight);
    float tileSize = drawableSize / maxDim;

    float totalW = gridWidth * tileSize;
    float totalH = gridHeight * tileSize;
    float startX = (1.0f - totalW) / 2.0f;
    float startY = (1.0f - totalH) / 2.0f;

    if (worldX < startX || worldX > startX + totalW || worldY < startY || worldY > startY + totalH)
        return false;

    gx = (int)((worldX - startX) / tileSize);
    gy = (int)((worldY - startY) / tileSize);
    return (gx >= 0 && gx < gridWidth && gy >= 0 && gy < gridHeight);
}

void MinesweeperGame::handleZoom(int direction) {
    if (currentState != PLAYING && currentState != GAME_WON && currentState != GAME_OVER) return;
    if (direction > 0) gridScale += ZOOM_SPEED;
    else gridScale -= ZOOM_SPEED;
    if (gridScale < 0.5f) gridScale = 0.5f;
    if (gridScale > 3.0f) gridScale = 3.0f;
}

void MinesweeperGame::handleInput(int button, int state, int x, int y) {
    float mx = (float)x / windowWidth;
    float my = 1.0f - (float)y / windowHeight;

    if (currentState == MENU) {
        if (checkButton(mx, my, 0.35f, 0.5f, 0.3f, 0.1f)) setupGame(MEDIUM);
        if (checkButton(mx, my, 0.35f, 0.35f, 0.3f, 0.1f)) exit(0);
        return;
    }

    if (currentState == GAME_OVER || currentState == GAME_WON) {
        if (checkButton(mx, my, 0.35f, 0.5f, 0.3f, 0.1f)) setupGame(MEDIUM);
        if (checkButton(mx, my, 0.35f, 0.35f, 0.3f, 0.1f)) currentState = MENU;
        return;
    }

    if (checkButton(mx, my, 0.85f, 0.9f, 0.1f, 0.05f)) { setupGame(MEDIUM); return; }
    if (checkButton(mx, my, 0.05f, 0.9f, 0.1f, 0.05f)) { currentState = MENU; return; }

    int gx, gy;
    if (getGridCoords(mx, my, gx, gy)) {
        if (button == GLUT_LEFT_BUTTON) revealTile(gx, gy);
        else if (button == GLUT_RIGHT_BUTTON) gameGrid[gy][gx].isFlagged = !gameGrid[gy][gx].isFlagged;
    }
}