const uint16_t gameOver[] PROGMEM = {
  900,100, 800,100, 900,100, 600,100, 500,100, 300,100, 100,100, 300,100, 500,100, 50,160,
  TONES_END };
  
void soundMove(){
  sound.tone(120, 10);
}
void soundShoot(){
  sound.tone(50, 50);
}
void soundHit(){
  sound.tone(500, 100);
}

void soundBad(){
  sound.tone(100, 100);
}
void soundGood(){
  sound.tone(900, 100);
}
void soundGameOver(){
  sound.tones(gameOver);
  while (sound.playing()) { }
  delay(50);
}
