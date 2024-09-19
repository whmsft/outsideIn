#include "include/raylib.h"
#include <vector>
#if defined(PLATFORM_WEB)
  #include <emscripten/emscripten.h>
#endif

// Screen and player properties
int screenWidth, screenHeight, playerX, playerY, fallVelocity;
bool playerAbove, playerAboveLastFrame = true;
int frame, objectsAboveCooldown, objectsBelowCooldown, SCORE = 0;
std::vector<std::vector<int>> objectsAbove, objectsBelow;
int SCREEN = 0; // 0: start screen; 1: game screen; 2: game over
int HIGHSCORE = 0;
int otherVariables[] = {0,0,0,0};

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
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  playerAbove = true;
  #if defined(PLATFORM_WEB)
    InitWindow(320, 640, "outsideIn");
    emscripten_set_main_loop(UpdateDraw, 30, 1);
  #else
    //InitWindow(360, 720, "outsideIn");
    InitWindow(GetMonitorWidth(GetCurrentMonitor()), GetMonitorHeight(GetCurrentMonitor()), "outsideIn");
    ToggleFullscreen();
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
  BeginDrawing();
  if (SCREEN == 0) {
    ClearBackground(WHITE);
    DrawRectangle(0,screenHeight-otherVariables[2],screenWidth,screenHeight,SECONDARY);
    if (otherVariables[2]<screenHeight/2){otherVariables[2]+=screenWidth/20;} else {otherVariables[2]=screenHeight/2;}
    if (otherVariables[2]==screenHeight/2) {
      if (objectsAboveCooldown == 0 && otherVariables[3]==0) {
        objectsAbove.push_back({ GetRandomValue(-screenWidth/20,screenWidth), 0, GetRandomValue(2,8) });
        objectsBelow.push_back({ GetRandomValue(-screenWidth/20,screenWidth), screenHeight, GetRandomValue(2,8) });
        objectsAboveCooldown = GetRandomValue(1,30);
      }
      for (auto i = 0; i < objectsBelow.size(); ) {auto& object = objectsBelow[i];
        DrawRectangle(object[0], object[1], screenWidth / 20, screenWidth / 20, PRIMARY);
        object[1] -= object[2]*screenWidth / 100;
        if (otherVariables[3]) object[2]*=1.5;
        if (object[1] < screenHeight/2) {objectsBelow.erase(objectsBelow.begin() + i);} else {++i;}
      }
      for (auto i = 0; i < objectsAbove.size(); ) {auto& object = objectsAbove[i];
        DrawRectangle(object[0], object[1], screenWidth / 20, screenWidth / 20, SECONDARY);
        object[1] += object[2]*screenWidth / 100;
        if (otherVariables[3]) object[2]*=1.5;
        if (object[1] > screenHeight/2-screenWidth/20) {objectsAbove.erase(objectsAbove.begin() + i);} else {++i;}
      }
      objectsAboveCooldown--;
    }
    DrawText("outsideIn",screenWidth/2-MeasureText("outsideIn",screenWidth/5)/2,screenHeight/4-screenHeight/20+screenHeight/2-otherVariables[2],screenWidth/5,SECONDARY);
    DrawText("PLAY",screenWidth/2-MeasureText("PLAY",screenHeight/10)/2,0.75*screenHeight-screenWidth/10+screenHeight/2-otherVariables[2],screenHeight/10,PRIMARY);
    DrawText(TextFormat("Highscore: %i",HIGHSCORE),screenWidth/2-MeasureText(TextFormat("Highscore: %i",HIGHSCORE),8*(screenWidth/100))/2,0.75*screenHeight+screenWidth/10+screenHeight/2-otherVariables[2],8*(screenWidth/100),PRIMARY);
    DrawText("a game by whmsft",screenWidth/2-MeasureText("a game by whmsft",8*(screenWidth/100))/2,screenHeight-8*(screenWidth/100)-screenWidth/20,8*(screenWidth/100),PRIMARY);
    if ((otherVariables[2]==screenHeight/2)&& (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsGestureDetected(GESTURE_TAP))) otherVariables[3]=1;
    if (otherVariables[3]==1 && objectsAbove.size()==0 && objectsBelow.size()==0) {
      SCREEN = 1;
      objectsAbove.clear();
      objectsBelow.clear();
      playerX = screenWidth / 2 - screenWidth / 40;
      playerY = screenHeight / 3;
      otherVariables[2]=0;
      otherVariables[3]=0;
      objectsAboveCooldown=30;
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
    bool isLeftHalf = !(isRightHalf);
    bool collideRight = collideSideways(playerX-(screenWidth/100), playerY, screenWidth/20, screenWidth/20, screenWidth/4, screenHeight/2-screenWidth/20, screenWidth/2, screenWidth/10);
    bool collideLeft = collideSideways(playerX+(screenWidth/100), playerY, screenWidth/20, screenWidth/20, screenWidth/4, screenHeight/2-screenWidth/20, screenWidth/2, screenWidth/10);
    if ((((IsGestureDetected(GESTURE_HOLD) || IsMouseButtonDown(MOUSE_LEFT_BUTTON)) && isRightHalf) || IsKeyDown(KEY_RIGHT)) && !collideLeft) {
      playerX += (screenWidth/100);
    }
    if ((((IsGestureDetected(GESTURE_HOLD) || IsMouseButtonDown(MOUSE_LEFT_BUTTON)) && isLeftHalf) || IsKeyDown(KEY_LEFT)) && !collideRight) {
      playerX -= (screenWidth/100);
    }
    if (playerX>screenWidth-screenWidth/40) playerX = -screenWidth/40;
    if (playerX<-screenWidth/40) playerX = screenWidth-screenWidth/40;
  
    // Object generation
    if (objectsAboveCooldown == 0) {
      objectsAbove.push_back({ GetRandomValue(-screenWidth/20,screenWidth), 0, GetRandomValue(2,8) });
      objectsAboveCooldown = GetRandomValue(1,30);
    }  
    if (objectsBelowCooldown == 0) {
      objectsBelow.push_back({ GetRandomValue(-screenWidth/20,screenWidth), screenHeight, GetRandomValue(2,8) });
      objectsBelowCooldown = GetRandomValue(1,30);
    }
    
    ClearBackground(WHITE);
  
    // Draw game elements
    DrawRectangle(0, screenHeight / 2, screenWidth, screenHeight / 2, SECONDARY);
    DrawRectangle(playerX, playerY, screenWidth / 20, screenWidth / 20, PLAYER);
  
    // Draw/Update objects
    for (auto i = 0; i < objectsBelow.size(); ) {auto& object = objectsBelow[i];
      object[1] -= object[2]*(screenWidth/100);
      DrawRectangle(object[0], object[1], screenWidth / 20, screenWidth / 20, PRIMARY);
      if (object[1] < screenHeight/2) {objectsBelow.erase(objectsBelow.begin() + i);} else {++i;}
      if (collide(object[0],object[1],screenWidth/20,screenWidth/20,playerX,playerY,screenWidth/20,screenWidth/20)) {SCORE=0;frame=-1;SCREEN=2;otherVariables[0]=screenHeight;if(HIGHSCORE<SCORE)HIGHSCORE=SCORE;}
    }
    for (auto i = 0; i < objectsAbove.size(); ) {auto& object = objectsAbove[i];
      object[1] += object[2]*(screenWidth/100);
      DrawRectangle(object[0], object[1], screenWidth / 20, screenWidth / 20, SECONDARY);
      if (object[1] > screenHeight/2-screenWidth/20) {objectsAbove.erase(objectsAbove.begin() + i);} else {++i;}
      if (collide(object[0],object[1],screenWidth/20,screenWidth/20,playerX,playerY,screenWidth/20,screenWidth/20)) {SCORE=0;frame=-1;SCREEN=2;otherVariables[0]=screenHeight;if(HIGHSCORE<SCORE)HIGHSCORE=SCORE;}
    }
    DrawRectangle(screenWidth / 4, screenHeight / 2, screenWidth / 2, screenWidth / 20, PRIMARY);
    DrawRectangle(screenWidth / 4, screenHeight / 2 - screenWidth / 20, screenWidth / 2, screenWidth / 20, SECONDARY);
    // Debug info
    DrawText(TextFormat("Score: %i",SCORE),5*(screenWidth/100),5*(screenWidth/100),10*(screenWidth/100),SECONDARY);
  
    frame++;
    objectsBelowCooldown--;
    objectsAboveCooldown--;
    if (playerAboveLastFrame!=playerAbove) SCORE++;
    playerAboveLastFrame=playerAbove;   
  } else if (SCREEN == 2) {
    ClearBackground(WHITE);
    DrawRectangle(0, screenHeight / 2, screenWidth, screenHeight / 2, SECONDARY);
    DrawRectangle(playerX, playerY, screenWidth / 20, screenWidth / 20, PLAYER);
    for (auto i = 0; i < objectsBelow.size(); ) {auto& object = objectsBelow[i];DrawRectangle(object[0], object[1], screenWidth / 20, screenWidth / 20, PRIMARY);}
    for (auto i = 0; i < objectsAbove.size(); ) {auto& object = objectsAbove[i];DrawRectangle(object[0], object[1], screenWidth / 20, screenWidth / 20, SECONDARY);}
    DrawRectangle(screenWidth / 4, screenHeight / 2, screenWidth / 2, screenWidth / 20, PRIMARY);
    DrawRectangle(screenWidth / 4, screenHeight / 2 - screenWidth / 20, screenWidth / 2, screenWidth / 20, SECONDARY);
    DrawRing(Vector2{playerX+screenWidth/40.0f,playerY+screenWidth/40.0f}, otherVariables[0], screenHeight, 0.0f, 360.0f, screenHeight, PLAYER);
    DrawText("Click anywhere",screenWidth/2-MeasureText("Click anywhere",screenWidth/10)/2,0.75*screenHeight,screenWidth/10,(playerY >= screenHeight / 2) ? SECONDARY : PRIMARY);
    DrawText(TextFormat("Score: %i",SCORE),5*(screenWidth/100),5*(screenWidth/100),10*(screenWidth/100),(playerY >= screenHeight / 2) ? SECONDARY : PRIMARY);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsGestureDetected(GESTURE_TAP)) otherVariables[1]=1;
    if (otherVariables[0]>screenWidth/4) otherVariables[0]-=screenWidth/10;
    if (otherVariables[1]==1) otherVariables[0]-=screenWidth/20;
    if (otherVariables[0]<screenWidth/20) {SCREEN=0;otherVariables[0]=0;otherVariables[1]=0;}
  }
  EndDrawing();
}
