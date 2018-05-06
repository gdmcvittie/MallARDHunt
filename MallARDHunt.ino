
/*
MallARD Hunt v0.1
2018 PixelPlatforms
 */
#include <Arduboy2.h>
#include <ArduboyTones.h>
#include <Sprites.h>
#include "bitmaps.h"

Arduboy2 arduboy;
ArduboyTones sound(arduboy.audio.enabled);
Sprites sprites;

/*
* constants
*/
int GRAVITY = 0;
int MIN_X = 0;
int MAX_X = 118;
int MIN_Y = 0;
int MAX_Y = 45;

int PLAYER_W = 10;
int PLAYER_H = 10;

int MAX_LIVES = 3;
int MAX_HEALTH = 3;

int MAX_BULLETS = 6;

int ENEMY_W = 13;
int ENEMY_H = 14;
int MAX_DUCKS = 4;

int DOG_Y = 33;

int MAX_REPEATS = 36; /* dog laughs :) */
/*
 * game state
 * 0 = brand
 * 1 = intro
 * 2 = play
 * 3 = level done
 * 4 = game over
 */
int game_state = 0;
bool sound_enabled = true;
int current_level = 0;
int time = 10;

/*
 * easy mode
 * true = normal (6 bullets, 4 ducks, ducks start slow and get faster)
 * false = hard (4 bullets, 4 ducks, ducks start fast and get faster)
 */
int easy_mode = true;

int score = 0;

int player_x = 59;
int player_y = 28;

bool player_hurt = false;
bool player_die = false;

int player_speed = 6;

int player_lives = 3;
int player_health = 3;

int player_bullets = 6;

int enemy_x = 2;
int enemy_y = 56;
int enemy_direction = 0; //0 = right, 1 = left
int enemy_speed = 1;
int enemy_hit = false;
int enemy_top_hits = 0;

int ducks_left = 4;
int ducks_hit = 0;

int dog_y = 56;

int repeats = 0;

bool _reload = false;

bool win = false;

bool sound_played = false;

bool collide = false;


/*
 * boot up
 */
void setup() {
  arduboy.boot();
  arduboy.systemButtons();
  arduboy.audio.begin();
  arduboy.setFrameRate(60); 
  arduboy.initRandomSeed();
}

/*
* main game loop
*/
void loop() {
  if (!(arduboy.nextFrame()))
    return;

  //clean slate
  arduboy.clear();
  
  //build level
  buildLevel();
    
  //buttons
  arduboy.pollButtons();
}


/*
* build the level
*/
void buildLevel(){
  switch(game_state){
  
	//brand
    case 0:
      arduboy.drawSlowXYBitmap(24, 12, brand, 88, 20, WHITE);
    break;
	
	//intro / title
	case 1:
      arduboy.drawSlowXYBitmap(0, 0, title, 128, 64, WHITE);
      if(sound_enabled){
        arduboy.drawSlowXYBitmap(27, 39, sound_on, 5, 7, BLACK);
      }
      if(easy_mode){
        arduboy.drawSlowXYBitmap(64, 39, easy, 26, 7, BLACK);
      }else{
        arduboy.drawSlowXYBitmap(64, 39, hard, 26, 7, BLACK);
      }
    break;
	
	//gameplay
    case 2:
      arduboy.drawSlowXYBitmap(0, 0, game, 128, 64, WHITE);      
      //player
      addPlayer();
      //enemy
      addEnemy();      
      //add hud
      addHud();
    break;

    //level done
    case 3:
        arduboy.drawSlowXYBitmap(0, 0, game, 128, 64, WHITE);
        
        if(win){
          if(dog_y > DOG_Y){
            dog_y = dog_y - 2;
          }
          arduboy.drawSlowXYBitmap(56, dog_y, dogY, 28, 18, BLACK);
        } else {
          if(dog_y > DOG_Y){
            dog_y = dog_y - 2;
          }
          arduboy.drawSlowXYBitmap(56, dog_y, dogN, 16, 19, BLACK);
          if(arduboy.everyXFrames(2)){
            if(sound_enabled && repeats < MAX_REPEATS){
              repeats++;
              soundMove();
              dog_y = dog_y + 3;
            }            
          }
        }
        //add hud
        addHud();
    break;
	
	//gameover
    case 4:
        arduboy.drawSlowXYBitmap(0, 0, game, 128, 64, WHITE);
        if(dog_y > DOG_Y){
          dog_y = dog_y - 2;
        }
        arduboy.drawSlowXYBitmap(56, 33, dogN, 16, 19, BLACK);
        if(arduboy.everyXFrames(2)){
          if(sound_enabled && repeats < MAX_REPEATS){
            repeats++;
            soundMove();
            dog_y = dog_y + 3;
          }
        }
		    arduboy.drawSlowXYBitmap(48, 6, game_over, 32, 17, BLACK);
        //score
        arduboy.setCursor(51,54);
        arduboy.print(score);
    break;
  }
  //button presses
  //delay(20);
  handleButtons();
  arduboy.display();
}


/*
 * button handling
 */
 void handleButtons(){
  switch(game_state){
  
	//brand
    case 0:
      if ( arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON) || arduboy.justPressed(UP_BUTTON) || arduboy.justPressed(DOWN_BUTTON) || arduboy.justPressed(LEFT_BUTTON) || arduboy.justPressed(RIGHT_BUTTON) ){
        game_state = 1;
      }
      delay(60);
    break;
	
    //intro
    case 1:
      if ( arduboy.justPressed(A_BUTTON) ){
          //toggle sound
        if(sound_enabled){
          sound_enabled = false;
          arduboy.audio.off();
        } else {
          sound_enabled = true;
          arduboy.audio.on();
          soundGood();
        }
      }
      //game mode (easy/hard)
      if( arduboy.justPressed(UP_BUTTON) || arduboy.justPressed(DOWN_BUTTON) ){
        if(easy_mode == true){
          easy_mode = false;
          MAX_BULLETS = 4;
          player_bullets = 4;
          enemy_speed = 2;
        } else {
          easy_mode = true;
          MAX_BULLETS = 6;
          player_bullets = 6;
          enemy_speed = 1;
        }
      }
      //play game
      if ( arduboy.justPressed(B_BUTTON) ){
        game_state = 2; 
      }
      delay(60);
    break;
		
    //playing
    case 2:
      if(!collide){
        if ( arduboy.pressed(RIGHT_BUTTON) ){
          if(player_x < MAX_X){
            player_x = player_x + player_speed;
          }
        }
        if ( arduboy.pressed(LEFT_BUTTON) ){
          if(player_x > MIN_X){
            player_x = player_x - player_speed;       
          }
        }
        if ( arduboy.pressed(UP_BUTTON) ){
          if(player_y > MIN_Y){
            player_y = player_y - player_speed;
          }
        }
        if ( arduboy.pressed(DOWN_BUTTON) ){
          if(player_y < MAX_Y){
            player_y = player_y + player_speed;
          }
        }
        if( arduboy.justPressed(B_BUTTON) ){
          onPressB();
        }     
        if( arduboy.justPressed(A_BUTTON) ){
          onPressA();
        }
      } else {
        if ( arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON) ){
          current_level = 1;
          reset(); 
        }   
      }
    break;

    //level done
    case 3:
      if ( arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON) ){
        if(win){
          enemy_speed++;
          player_lives = MAX_LIVES;
        }
        dog_y = 56;
        player_bullets = MAX_BULLETS;   
        ducks_left = MAX_DUCKS;     
        ducks_hit = 0;
        game_state = 2;
      }
    break;
	
	//game over
    case 4:
      if ( arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON) ){
        reset(); 
      }
    break;
  }
 }


/*
 * player
 */
void addPlayer(){
  arduboy.drawBitmap(player_x, player_y, player, 10, 10, BLACK);
}

/*
 * enemy
 */
void addEnemy(){
  if(ducks_left>0){
    if(!enemy_hit){
      if(enemy_direction == 0 && enemy_x < MAX_X){
        enemy_x = enemy_x + enemy_speed;
      } else if(enemy_x >= MAX_X){
        enemy_direction = 1;
      }
      if(enemy_direction == 1 && enemy_x > MIN_X){
        enemy_x = enemy_x - enemy_speed;
      } else if(enemy_x <= MIN_X){
        enemy_direction = 0;
      }
      if(enemy_y > 0){
        enemy_y--;
        if(sound_enabled){
          soundMove();
        }
        if(enemy_direction == 1){
          arduboy.drawSlowXYBitmap(enemy_x, enemy_y, duckL, 13, 14, BLACK);
        } else {
          arduboy.drawSlowXYBitmap(enemy_x, enemy_y, duckR, 13, 14, BLACK);
        }
      } else {
        if(sound_enabled && !sound_played){
          sound_played = true;
          soundBad();
        }
        respawn();
      }      
    } else {
      if(enemy_y<MAX_Y){
        enemy_y++;
        arduboy.drawSlowXYBitmap(enemy_x, enemy_y, duckD, 6, 17, BLACK);
      } else {
        int del = random(20,40);
        if(arduboy.everyXFrames(del)){
          if(ducks_hit<MAX_DUCKS){
            ducks_hit++;
            respawn();
          } else if(ducks_hit == MAX_DUCKS){
            win = true;
            game_state = 3;   
          }
        } 
      }
    }
  } else {
    if(player_bullets>0 && ducks_hit == MAX_DUCKS){
      win = true;
      game_state = 3; 
    } else {
      if(player_lives>0){
        player_lives--;
        dog_y = 56;
        win = false;
        repeats = 0;
        game_state = 3;
      } else {
        dog_y = 56;
        repeats = 0;
        win = false;
        game_state = 4;
      }      
    }
  }
}


void respawn(){
  enemy_hit = false;
  int dir = random(0,100);
  if(dir % 2 == 0){
    enemy_direction = 0;
  } else {
    enemy_direction = 1;
  }
  sound_played = false;
  enemy_y = MAX_Y;
  enemy_x = random(MIN_X,MAX_X);
  ducks_left--; 
}

/*
 * hud
 */
void addHud(){
    //life
    for(int i=0; i<player_lives; i++){
      arduboy.drawSlowXYBitmap( (i==0) ? 103 : 103+(i * 8) , 2, heart, 7, 7, BLACK);
    }
    
    //bullets
    for(int i=0; i<player_bullets; i++){
      arduboy.drawSlowXYBitmap( (i==0) ? 2 : 2+(i * 4) , 54, bullet, 3, 7, WHITE);
    }

    //ducks left
    for(int i=0; i<ducks_left; i++){
      arduboy.drawSlowXYBitmap( (i==0) ? 92 : 92+(i * 9) , 54, duck, 8, 7, WHITE);
    }

    //ducks hit
    for(int i=0; i<ducks_hit; i++){
      arduboy.drawSlowXYBitmap( (i==0) ? 17 : 17+(i * 9) , 1, duck, 8, 7, BLACK);
    }

    //reload needed
    if(_reload && arduboy.everyXFrames(2)){
      arduboy.drawSlowXYBitmap( 2 , 54, reload, 23, 7, WHITE);
    }

    //score
    arduboy.setCursor(51,54);
    arduboy.print(score);
}


/*
 * on press A (during gameplay)
 */
 void onPressA(){
  if(_reload){
      score = 0;
      ducks_left = MAX_DUCKS;
      player_bullets = MAX_BULLETS;
      _reload = false;
  }
 }


/*
 * on press B (during gameplay)
 */
void onPressB(){
  if(player_bullets>0){
    if(sound_enabled){
      soundShoot();
    }
    player_bullets--;
    fireBullet();
  }
}

void fireBullet(){
   if( player_x+4 >= enemy_x+2 && player_x+4 <= enemy_x+10 && player_y+4 >= enemy_y+2 && player_y+4 <= enemy_y+12 ){
    if(sound_enabled){
      soundHit();
    }
    enemy_hit = true;
    score += 100;
   }
}


/*
 * reset game
 */
void reset(){
  player_x = 59;
  player_y = 28;
  player_die = false;
  player_hurt = false;
  player_bullets = MAX_BULLETS;
  player_health = MAX_HEALTH;
  player_lives = MAX_LIVES;
  ducks_left = MAX_DUCKS;
  ducks_hit = 0;
  _reload = false;
  win = false;
  dog_y = 56;
  score = 0;
  collide = false;
  game_state = 1;
  delay(200); 
}

void trace(String txt){
  arduboy.setCursor(40,16);
  arduboy.print(txt);
}
