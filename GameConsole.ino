
        #include <ArxContainer.h>
#include <MD_MAX72xx.h>
//#include <MD_Parola.h>
#include <SPI.h>

typedef uint8_t ui8;
typedef int8_t i8;
typedef unsigned long long ull; // uint 32

#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW
//#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW
#define MAX_DEVICES 4

// BUTTONS__________________________
// define buttons for control
#define UP_BUTTON 2
#define DOWN_BUTTON 3
#define LEFT_BUTTON 4
#define RIGHT_BUTTON 5
#define ACTION_BUTTON 6
#define EXIT_BUTTON 7

enum ButtonState{
  PRESS,
  RELEASE
};

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, 12, 11, 10, MAX_DEVICES);
//MD_Parola parola = MD_Parola(HARDWARE_TYPE, 12, 11, 10, MAX_DEVICES);

enum State{
  MENU,
  SPACEINVADERS,
  SNAKE
};
State state = MENU;

void setState(State s); //for classes to know about this function

/*
/// 0-8 bytes - bottom right
/// 8-16 bytes - bottom left
/// 16-24 bytes - tor right
/// 24-32 bytes - top left
*/
i8 renderBuffer[32];
void drawPixel(i8 x, i8 y, bool value){ //convoluted math
  i8 pos = (y/8)*16+y%8 + (x/8)*8;

  /*
  i8 pos;
  if(y<8){
    pos = y%8+(x/8)*8;
    renderBuffer[pos]|=value<<(7-x%8);
  }else{
    pos = 16+y%8+(x/8)*8;
    renderBuffer[pos]|=value<<(x%8);
  }*/
  
  //renderBuffer[pos]&=~(i8(!value)<<(x%8));
}

void drawLine8(i8 x, i8 y, i8 line){ //convoluted math
  /*
  i8 pos = (y/8)*16+y%8 + (x/8)*8;
  //Serial.println(String(pos));
  if(x<8||x>=16&&x<24){
    renderBuffer[pos]|=(line<<(8-x%8));
  }else if(x>=8&&x<16||x>=24){
    renderBuffer[pos]|=(line>>(x%8));
  }
  */
  i8 pos = (y/8)*16+y%8 + (x/8)*8;
  renderBuffer[pos]|=(line<<(x%8));
  if(x%16<8){
    renderBuffer[pos+8]|=(line>>(8-(x%8)));
  }
}
i8 getPixel(i8 x, i8 y){ //convoluted math
  i8 pos = (y/8)*16+y%8 + (x/8)*8;
  return (renderBuffer[pos]>>(x%8))&1;
}
void clearRenderBuffer(){
  memset(renderBuffer, 0, sizeof(renderBuffer));
}
void showOnScreen(){
  mx.setBuffer(7, 8, (ui8*)renderBuffer);
  mx.setBuffer(15, 8, (ui8*)renderBuffer+8);
  mx.setBuffer(23, 8, (ui8*)renderBuffer+16);
  mx.setBuffer(31, 8, (ui8*)renderBuffer+24);
}


class Img{
public:
  Img(i8 x0, i8 y0):x(x0),y(y0){}
  Img():Img(0,0){}
  i8 x;
  i8 y;
  i8* sprite;
  void Draw(){
    for(i8 i=0; i<8; i++){
      if(y+i >= 32){
        break;
      }
        drawLine8(x,y+i,sprite[i]);
    }
  }
};
Img im;

struct Pos{
    Pos(i8 x_, i8 y_):x(x_),y(y_){}
    Pos():Pos(0,0){}
    i8 x;
    i8 y;
    friend bool operator==(Pos& lhs, Pos& rhs){
      return lhs.x==rhs.x&&lhs.y==rhs.y;
    }
    friend bool operator!=(Pos lhs, Pos rhs){
      return lhs.x!=rhs.x||lhs.y!=rhs.y;
    }
};

class Menu{
public:
  //Menu Code Here
  void Init(){
    
  }
  void Draw(){
    
  }
};
Menu menu;

class SpaceInvaders{
public:
  Pos playerPos = Pos(7,1);
  arx::vector<Pos> bullets;
  arx::vector<Pos> enemies;
  i8 enemyY = 0;
  i8 enemyDX = 1;
  ull tickTime = 0;
  void Init(){
    playerPos = Pos(7,1);
    enemyY = 0;
    enemyDX = 1;
    tickTime = 0;
    enemies.clear();
    enemies.push_back(Pos(5,15));
    enemies.push_back(Pos(7,15));
    enemies.push_back(Pos(9,15));
    enemies.push_back(Pos(11,15));
    enemies.push_back(Pos(6,14));
    enemies.push_back(Pos(8,14));
    enemies.push_back(Pos(10,14));
    enemies.push_back(Pos(12,14));
    enemies.push_back(Pos(5,13));
    enemies.push_back(Pos(7,13));
    enemies.push_back(Pos(9,13));
    enemies.push_back(Pos(11,13));
  }
  void GameOver(bool win){
    if(win){

    }else{

    }
    setState(State::MENU);
  }
  void Shoot(){
    if(bullets.back()!=playerPos){
      bullets.push_back(playerPos);
    }
  }
  void Draw(){
    drawPixel(playerPos.x,playerPos.y,1);
    for(auto& bullet: bullets){
      drawPixel(bullet.x,bullet.y,1);
    }
    for(auto& enemy: enemies){
      drawPixel(enemy.x,enemy.y,1);
    }
  }
  void Tick(){
    if(enemies.empty()){
      GameOver(true);
      return;
    }
    for(auto it = bullets.begin();it<bullets.end();){
      it->y+=1;
      if(it->y>=16){
        it = bullets.erase(it);
      }else{
        ++it;
      }
    }
    bool enemyWasDeleted = false;
    for(auto it = enemies.begin();it<enemies.end();){
      if(*it == playerPos){
        GameOver(false);
        return;
      }
      enemyWasDeleted = false;
      it->x+=enemyDX;
      for(auto it2 = bullets.begin();it2<bullets.end();){
        if(it->x == it2->x && it->y == it2->y){
          enemyWasDeleted = true;
          //bulletWasDeleted = true;
          it = enemies.erase(it);
          it2 = bullets.erase(it2);
          break;
        }else{
          ++it2;
        }
      }
      if(!enemyWasDeleted){
        ++it;
      }
    }
    if(tickTime%4==0){
      enemyDX*=-1;
    }
    if(tickTime%8==0 && tickTime!=0){
      if(enemyY>=12){
        GameOver(false);
        return;
      }
      for(auto& enemy: enemies){
        enemy.y-=1;
      }
      enemyY++;
    }
    tickTime++;
  }
};
SpaceInvaders spaceInvaders;

class SnakeGame{
public:
	struct Snake{
		Pos moveDelta = Pos(1,0);
		arx::vector<Pos> body;
	};
  Pos moveDelta;
	ull score = 0;
	Snake snake;
  Pos apple;
  bool dieOnWall = false;
	void Init(){
		score = 0;
		moveDelta = Pos(1,0);
		snake.body.clear();
		snake.body.push_back(Pos(3,4));
    snake.body.push_back(Pos(2,4));
    SpawnApple();
	}
	void SpawnApple(){
    bool goodSpawn;
    do{ //bad code, but i kinda dont care
      goodSpawn = true;
      apple = Pos(random(16),random(16));
      for(auto& snakePart: snake.body){
		  	if(snakePart == apple){
          goodSpawn = false;
          break;
        }
		  }
    }while(!goodSpawn);
	}
	void GameOver(bool win){
    if(win){

    }else{
      
    }
    setState(State::MENU);
	}
	void Tick(){
    if(snake.body.size()==256){
      GameOver(true);
      return;
    }
    if(dieOnWall){
      if(snake.body.back().x>15 || snake.body.back().x<0 || snake.body.back().y>15 || snake.body.back().y<0){
		  	GameOver(false);
        return;
		  }
    }else{
      if(snake.body.back().x>15){
        snake.body.back().x=0;
      }
      if(snake.body.back().x<0){
        snake.body.back().x=15;
      }
      if(snake.body.back().y>15){
        snake.body.back().y=0;
      }
      if(snake.body.back().y<0){
        snake.body.back().y=15;
      }
    }
		
		const Pos lastPos = snake.body.front();
		for(i8 i=0;i<snake.body.size()-1;i++){
			if(snake.body.back()==snake.body[i]){
				GameOver(false);
        return;
			}
			snake.body[i]=snake.body[i+1];
		}
		snake.body.back().x+=snake.moveDelta.x;
		snake.body.back().y+=snake.moveDelta.y;

    if(snake.body.back() == apple){
      snake.body.push_back(lastPos);
      score++;
      SpawnApple();
    }
	}
	void Draw(){
		for(auto& snakePart: snake.body){
			drawPixel(snakePart.x,snakePart.y,1);
		}
    drawPixel(apple.x,apple.y,1);
	}
};
SnakeGame snakeGame;


ull lastMillis = 0;
ull lastMillis2 = 0;

void setState(State s){
  if(s==MENU){return;}
  lastMillis = 0;
  lastMillis2 = 0;
  state = s;
  clearRenderBuffer();
  switch(s){
    case(State::MENU):
      menu.Init();
    break;
    case(State::SPACEINVADERS):
      spaceInvaders.Init();
    break;
    case(State::SNAKE):
      snakeGame.Init();
    break;
  }
}

class Button{
public:
  unsigned long long lastChangeTime = 0;
  int pin;
  ButtonState state;
  Button(int pin_):pin(pin_){
    pinMode(pin, INPUT);
  }
  void CheckState(){
    if(millis()-lastChangeTime > 10){
      if(digitalRead(pin) == HIGH){
        state = PRESS;
      }
      if(digitalRead(pin) == LOW){
        state = RELEASE;
      }
      lastChangeTime = millis();
    }
  }
};
Button upBttn(UP_BUTTON);
Button downBttn(DOWN_BUTTON);
Button leftBttn(LEFT_BUTTON);
Button rightBttn(RIGHT_BUTTON);
Button actionBttn(ACTION_BUTTON);
Button exitBttn(EXIT_BUTTON);
void checkButtons(){
  upBttn.CheckState();
  downBttn.CheckState();
  leftBttn.CheckState();
  rightBttn.CheckState();
  actionBttn.CheckState();
  exitBttn.CheckState();
}

void setup() {
  im.sprite = new i8[8]{
    0b00000000,
    0b00011000,
    0b00111100,
    0b01111110,
    0b11111111,
    0b00100100,
    0b01000010,
    0b10000001
  };
  setState(State::SPACEINVADERS);
  Serial.begin(9600);
  mx.begin();
  mx.clear(); // clear the display
  checkButtons();
  
}

//i8 playerDX = 1;
void loop() {
  clearRenderBuffer();
  checkButtons();

/*
  for(int i =0;i<8;i++){
    renderBuffer[i+24]=255;
  }
  showOnScreen();
  
  return;*/
  if(state != State::MENU){
    if(exitBttn.state == PRESS){
      Serial.println("exit");
      setState(State::MENU);
    }
  }
  switch(state){
////////////////////////////////////////////////////////////////////
    case(State::MENU):
      if(millis()-lastMillis >= 500){
        im.x++;
        if(im.x>=16){
          im.x=0;
        }
        lastMillis = millis();
      }
      im.Draw();
      break;
////////////////////////////////////////////////////////////////////
    case(State::SPACEINVADERS):
      if(millis()-lastMillis >= 500){
        spaceInvaders.Tick();
        /*
        spaceInvaders.playerPos.x+=playerDX;
        if(spaceInvaders.playerPos.x == 12 || spaceInvaders.playerPos.x == 4){
          playerDX*=-1;
        }*/
        lastMillis = millis();
      }
      //if(millis()-lastMillis2 >= 1100){
      //  spaceInvaders.Shoot();
      //  lastMillis2 = millis();
      //}
      if(leftBttn.state == PRESS){
        Serial.println("left");
        if(spaceInvaders.playerPos.x>0){
          spaceInvaders.playerPos.x--;
        }
      }
      if(rightBttn.state == PRESS){
        Serial.println("right");
        if(spaceInvaders.playerPos.x<15){
          spaceInvaders.playerPos.x++;
        }
      }
      if(actionBttn.state == PRESS){
        Serial.println("action");
        spaceInvaders.Shoot();
      }
      spaceInvaders.Draw();
    break;
////////////////////////////////////////////////////////////////////
	  case(State::SNAKE):
	  	if(millis()-lastMillis >= 500){
	  		snakeGame.Tick();
	  		lastMillis = millis();
	  	}
	  	if(millis()-lastMillis2 >= 1100){
        lastMillis2 = millis();
      }
      if(upBttn.state == PRESS){
        Serial.println("up");
        if(snakeGame.snake.moveDelta!=Pos(0,-1)){
          snakeGame.snake.moveDelta = Pos(0,1);
        }
      }
      if(downBttn.state == PRESS){
        Serial.println("down");
        if(snakeGame.snake.moveDelta!=Pos(0,1)){
          snakeGame.snake.moveDelta = Pos(0,-1);
        }
      }
      if(leftBttn.state == PRESS){
        Serial.println("left");
        if(snakeGame.snake.moveDelta!=Pos(1,0)){
          snakeGame.snake.moveDelta = Pos(-1,0);
        }
      }
      if(rightBttn.state == PRESS){
        Serial.println("right");
        if(snakeGame.snake.moveDelta!=Pos(-1,0)){
          snakeGame.snake.moveDelta = Pos(1,0);
        }
      }
	  	snakeGame.Draw();
	  break;
  }
  showOnScreen();
}
