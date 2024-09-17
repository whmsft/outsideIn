#include "raylib.h"
#include <vector>
#if defined(PLATFORM_WEB)
  #include <emscripten/emscripten.h>
#endif

// Screen and player properties
int screenWidth, screenHeight, playerX, playerY, fallVelocity, moveSpeed;
bool playerAbove, playerAboveLastFrame = true;
int frame, objectsAboveCooldown, objectsBelowCooldown, SCORE = 0;
std::vector<std::vector<int>> objectsAbove, objectsBelow;
int SCREEN = 0; // 0: start screen; 1: game screen; 2: game over
int otherVariables[] = {0};

// Colors
Color PRIMARY = WHITE;
Color SECONDARY = BLACK;
Color PLAYER = BLACK;

// Function declarations
void UpdateDraw(void);

// Collision functions
bool collide(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
  return (x1 <= x2 + w2 && x1 + w1 >= x2 && y1 <= y2 + h2 && y1 + h1 >= y2);
}
bool collideSideways(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
  int yOffset = playerAbove ? -1 : 1;
  return (x1 <= x2 + w2 && x1 + w1 >= x2 && y1 + yOffset <= y2 + h2 && y1 + yOffset + h1 >= y2);
}

int main(void) {
  //SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  playerAbove = true;
  #if defined(PLATFORM_WEB)
    InitWindow(320, 640, "outsideIn");
    emscripten_set_main_loop(UpdateDraw, 0, 1);
  #else
    InitWindow(320, 640, "outsideIn");
    //InitWindow(GetMonitorWidth(GetCurrentMonitor()), GetMonitorHeight(GetCurrentMonitor()), "outsideIn");
    //ToggleFullscreen();
    SetTargetFPS(30);
    while (!WindowShouldClose()) {
      UpdateDraw();
    }
  #endif

  CloseWindow();
  return 0;
}

void UpdateDraw(void) {
  screenWidth = GetScreenWidth();
  screenHeight = GetScreenHeight();
  if (SCREEN == 0) {
    BeginDrawing();
    ClearBackground(WHITE);
    DrawText("outsideIn",screenWidth/2-MeasureText("outsideIn",screenWidth/5)/2,screenHeight/4-screenHeight/20,screenHeight/10,SECONDARY);
		DrawRectangle(0,screenHeight/2,screenWidth,screenHeight/2,SECONDARY);
		DrawText("PLAY",screenWidth/2-MeasureText("PLAY",screenHeight/10)/2,0.75*screenHeight-screenWidth/10,screenHeight/10,PRIMARY);
    EndDrawing();
		if (collide(GetMouseX(), GetMouseY(), 1, 1, 0, screenHeight/2, screenWidth, screenHeight/2) && (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))) {
      SCREEN = 1;
      objectsAbove.clear();
      objectsBelow.clear();
      playerX = screenWidth / 2 - screenWidth / 40;
      playerY = screenHeight / 3;
      moveSpeed = screenWidth / 100;
    }
  } else if (SCREEN == 1) {
    fallVelocity += playerAbove ? screenWidth/80 : -screenWidth/80;

    // Collision with platform
    if (collide(playerX, playerY, screenWidth / 20, screenWidth / 20, screenWidth / 4, screenHeight / 2 - screenWidth / 20, screenWidth / 2, screenWidth / 10)) {
      fallVelocity = 0;
      playerY = playerAbove ? (screenHeight / 2) - screenWidth / 10 : (screenHeight / 2) + screenWidth / 20;
    }
  
    // Update player color and position
    PLAYER = (playerY >= screenHeight / 2) ? PRIMARY : SECONDARY;
    playerAbove = (playerY + screenWidth / 40 <= screenHeight / 2);
    playerY += fallVelocity / 5;
  
    // Mouse/Touch/Keyboard input
    bool isRightHalf = (GetMouseX() >= screenWidth / 2 || GetTouchX() >= screenWidth / 2);
    bool isLeftHalf = (GetMouseX() <= screenWidth / 2 || GetTouchX() <= screenWidth / 2); 
    bool collideRight = collideSideways(playerX-moveSpeed, playerY, screenWidth/20, screenWidth/20, screenWidth/4, screenHeight/2-screenWidth/20, screenWidth/2, screenWidth/10);
    bool collideLeft = collideSideways(playerX+moveSpeed, playerY, screenWidth/20, screenWidth/20, screenWidth/4, screenHeight/2-screenWidth/20, screenWidth/2, screenWidth/10);
    if ((((IsGestureDetected(GESTURE_HOLD) || IsMouseButtonDown(MOUSE_LEFT_BUTTON)) && isRightHalf) || IsKeyDown(KEY_RIGHT)) && !collideLeft) {
      playerX += moveSpeed;
    }
    if ((((IsGestureDetected(GESTURE_HOLD) || IsMouseButtonDown(MOUSE_LEFT_BUTTON)) && isLeftHalf) || IsKeyDown(KEY_LEFT)) && !collideRight) {
      playerX -= moveSpeed;
    }
    if (playerX>screenWidth-screenWidth/40) playerX = -screenWidth/40;
    if (playerX<-screenWidth/40) playerX = screenWidth-screenWidth/40;
  
    // Object generation
    if (objectsAboveCooldown == 0) {
      objectsAbove.push_back({ GetRandomValue(-screenWidth/20,screenWidth), 0, GetRandomValue(1,5) });
      objectsAboveCooldown = GetRandomValue(1,40);
    }  
    if (objectsBelowCooldown == 0) {
      objectsBelow.push_back({ GetRandomValue(-screenWidth/20,screenWidth), screenHeight, GetRandomValue(1,5) });
      objectsBelowCooldown = GetRandomValue(1,40);
    }
    
    BeginDrawing();
    ClearBackground(WHITE);
  
    // Draw game elements
    DrawRectangle(0, screenHeight / 2, screenWidth, screenHeight / 2, SECONDARY);
    DrawRectangle(screenWidth / 4, screenHeight / 2, screenWidth / 2, screenWidth / 20, PRIMARY);
    DrawRectangle(screenWidth / 4, screenHeight / 2 - screenWidth / 20, screenWidth / 2, screenWidth / 20, SECONDARY);
    DrawRectangle(playerX, playerY, screenWidth / 20, screenWidth / 20, PLAYER);
  
    // Draw/Update objects
    for (auto i = 0; i < objectsBelow.size(); ) {auto& object = objectsBelow[i];
      DrawRectangle(object[0], object[1], screenWidth / 20, screenWidth / 20, PRIMARY);
      object[1] -= object[2]*moveSpeed;
      if (object[1] < screenHeight/2) {objectsBelow.erase(objectsBelow.begin() + i);} else {++i;}
      if (collide(object[0],object[1],screenWidth/20,screenWidth/20,playerX,playerY,screenWidth/20,screenWidth/20)) {SCORE=0;frame=-1;SCREEN=2;otherVariables[0]=screenHeight;}
    }
    for (auto i = 0; i < objectsAbove.size(); ) {auto& object = objectsAbove[i];
      DrawRectangle(object[0], object[1], screenWidth / 20, screenWidth / 20, SECONDARY);
      object[1] += object[2]*moveSpeed;
      if (object[1] > screenHeight/2-screenWidth/20) {objectsAbove.erase(objectsAbove.begin() + i);} else {++i;}
      if (collide(object[0],object[1],screenWidth/20,screenWidth/20,playerX,playerY,screenWidth/20,screenWidth/20)) {SCORE=0;frame=-1;SCREEN=2;otherVariables[0]=screenHeight;}
    }
  
    // Debug info
    DrawText(TextFormat("Score: %i",SCORE),5*moveSpeed,5*moveSpeed,10*moveSpeed,SECONDARY);
  
    EndDrawing();
    frame++;
    objectsBelowCooldown--;
    objectsAboveCooldown--;
    if (playerAboveLastFrame!=playerAbove) SCORE++;
    playerAboveLastFrame=playerAbove;   
  } else if (SCREEN == 2) {
    BeginDrawing();
    DrawRing(Vector2{playerX+screenWidth/40,playerY+screenWidth/40}, otherVariables[0], screenHeight, 0, 360, 640, PLAYER);
    DrawText("Click anywhere",screenWidth/2-MeasureText("Click anywhere",screenWidth/10)/2,0.75*screenHeight,screenWidth/10,(playerY >= screenHeight / 2) ? SECONDARY : PRIMARY);
    DrawText(TextFormat("Score: %i",SCORE),5*moveSpeed,5*moveSpeed,10*moveSpeed,(playerY >= screenHeight / 2) ? SECONDARY : PRIMARY);
    EndDrawing();
    if (otherVariables[0]>screenWidth/5) {otherVariables[0]-=screenWidth/10;} else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {SCREEN=0;}
  }
}
