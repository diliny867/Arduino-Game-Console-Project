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
/// 0-8 bytes - top left
/// 8-16 bytes - top right
/// 16-24 bytes - bottom left
/// 24-32 bytes - bottom right
*/
i8 renderBuffer[32];
void drawPixel(i8 x, i8 y, bool value){ //convoluted bit math
  i8 pos = (y/8)*16+y%8 + (x/8)*8;
  renderBuffer[pos]|=value<<(x%8);
  //renderBuffer[pos]&=~(i8(!value)<<(x%8));
}
void drawLine8(i8 x, i8 y, i8 line){ //convoluted bit math
  i8 pos = (y/8)*16+y%8 + (x/8)*8;
  renderBuffer[pos]|=(line<<(x%8));
  if(x%16<8){
    renderBuffer[pos+8]|=(line>>(8-(x%8)));
  }
}
i8 getPixel(i8 x, i8 y){ //convoluted bit math
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
      if(y+i >= 16){
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
};

class Menu{
public:
  //Menu Code Here
  void Init(){

  }
  void Draw(){

  }
}
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
    bullets.push_back(playerPos);
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
	class Snake{
		Pos moveDelta = Pos(1,0);
		arx::vector<Pos> body;
	};
	ull score = 0;
	Snake snake;
	//arx::vector<Pos> apples;
  Pos apple;
  bool dieOnWall = false;
	void Init(){
		score = 0;
		moveDelta = Pos(1,0);
    SpawnApple();
		snake.body.clear();
		snake.body.push_back(Pos(7,7));
	}
	void SpawnApple(){
    bool goodSpawn;
    do{ //bad code, but i kinda dont care
      goodSpawn = true;
      apple = Pos(random(16),random(16));
      //apples.push_back(Pos(random(16),random(16)));
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
		
		const lastPos = snake.body.front();
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
      score++;
      SpawnApple();
    }
		//for(auto& it=apples.begin();it<apples.end();){
		//	if(*it==snake.body.back()){
		//		snake.push_back(lastPos);
		//		it = apples.erase(it);
		//		score++;
		//	}else{
		//		++it;
		//	}
		//}
	}
	void Draw(){
		for(auto& snakePart: snake.body){
			drawPixel(snakePart.x,snakePart.y,1);
		}
    drawPixel(apple.x,apple.y,1);
		//for(auto& apple: apples){
		//	drawPixel(apple.x,apple.y,1);
		//}
	}
};
SnakeGame snakeGame;


ull lastMillis = 0;
ull lastMillis2 = 0;

void setState(State s){
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
  setState(State::SNAKE);
  mx.begin();
  mx.clear(); // clear the display
}

i8 playerDX = 1;
void loop() {
  clearRenderBuffer();
  switch(state){
////////////////////////////////////////////////////////////////////
    case(State::MENU):
      if(millis()-lastMillis >= 500){
        im.x++;
        if(im.x>=32){
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
        spaceInvaders.playerPos.x+=playerDX;
        if(spaceInvaders.playerPos.x == 12 || spaceInvaders.playerPos.x == 4){
          playerDX*=-1;
        }
        lastMillis = millis();
      }
      if(millis()-lastMillis2 >= 1100){
        spaceInvaders.Shoot();
        lastMillis2 = millis();
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
        snakeGame.SpawnApple();
        lastMillis2 = millis();
      }
	  	snakeGame.Draw();
	  break;
  }
  showOnScreen();
}