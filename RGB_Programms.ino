/*
 * Pallets
 */
DEFINE_GRADIENT_PALETTE( Red_wave ) {
    0, 255, 0, 0,
   55, 0, 0,  0,
  200, 0, 0,  0,
  255, 255, 0,  0};    

DEFINE_GRADIENT_PALETTE( Red_Orange_gp ) {
    0, 206, 20, 14,
   58, 113, 11,  5,
  130, 208, 21, 16,
  198, 171, 15,  8,
  255, 128, 12,  6};

/*
 * Patern lists
 */
typedef void (*SimplePatternList[])();
SimplePatternList basePatternGully =    { rainbowGully, rainbowGullyWithGlitter,  juggleGully,        sinelonGully,   confettiGully}; 
SimplePatternList basePattersWall =     { rainbowWall,  rainbowWall,              randomWallColors,   sinelonWall,    confettiWall}; 


uint8_t gCurrentPatternNumber = 0; 
uint8_t gHue = 0;
bool newPatern = false;
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void setLeds(){
  if(!state){ //Off
    FastLED.clear();
  }else{ //On
    EVERY_N_MILLISECONDS( 20 ) { gHue++; } 
    
    if(effect == "initiation"){
      effect_initiation( false );
    }else if(effect == "demo"){
      basePatternGully[gCurrentPatternNumber]();
      basePattersWall[gCurrentPatternNumber]();
      EVERY_N_SECONDS( 10 ) { nextPattern(); } 
    }else if(effect == "pre-movie"){
      colorConfettiGully(CRGB::OrangeRed);
      pre_movie( false );
    }else if(effect == "movie"){ 
      leds_wall.fadeToBlackBy(5);
      fadeTowardColor( leds_bar, 1, CHSV(0,255,brightness), 40);
      gullyWaveFromPallet(Red_wave);    
    }else if(effect == "movie-pauze"){ 
      theaterGully();
      fadeTowardColor( leds_bar, 1, leds_left[0], 10);
      cascadeWallToColor( CRGB::OrangeRed );  
    }else if(effect == "post-movie"){ 
      post_movie( false );
    }else if(effect == "game"){ 
      game( false );
    }else if(effect == "poweroff"){ 
      poweroff();
    }else if(effect == "color"){ 
      //Solid Color 
      fill_solid(leds_left, NUM_LEDS_DIGITAL, CRGB(color)); 
      fill_solid(leds_right, NUM_LEDS_DIGITAL, CRGB(color)); 
      fill_solid(leds_analog, NUM_LEDS_ANALOG, CRGB(color));     

      //Collor correction between the wall strips
      leds_wall[0] -= CRGB( 100, 20, 0); //Less red
      leds_wall[1] -= CRGB( 20, 20, 10); //Less red
      leds_wall[2] -= CRGB( 100, 20, 0); //Less red
      leds_wall[3] -= CRGB( 20, 20, 10); //Less red
      leds_wall[4] -= CRGB( 100, 20, 0); //Less red
  
    }else{
      //Keep as is
    }
  }

  FastLED.delay(1000 / FRAMES_PER_SECOND);
}


/*
 * Led Patersn for analog an digital 
 */
void effect_initiation( boolean reset ){
  static unsigned long previousStep = 0;
  unsigned long currentMillis = millis();
  int sepDelay  = 1500;
  static int phase = 0;
  String return_effect = "demo";

  // initiate
  if(reset){
    previousStep = currentMillis;
    FastLED.clear ();
    phase = 1;
    exit;
  }

  if (currentMillis - previousStep >= sepDelay) {  
    previousStep = currentMillis;
    phase++;
  }

  switch (phase) {
    case 1:
      leds_analog[0] = CRGB::Red;
    case 2:
      leds_analog[0] = CRGB::Green;
      leds_analog[1] = CRGB::Red;
      break;
    case 3:
      leds_analog[0] = CRGB::Blue;
      leds_analog[1] = CRGB::Green;
      leds_analog[2] = CRGB::Red;
      break;
    case 4:
      leds_analog[0] = CRGB::Red;
      leds_analog[1] = CRGB::Blue;
      leds_analog[2] = CRGB::Green;
      leds_analog[3] = CRGB::Red;
      break;
    case 5:
      leds_analog[1] = CRGB::Green;
      leds_analog[2] = CRGB::Blue;
      leds_analog[3] = CRGB::Green;
      leds_analog[4] = CRGB::Red;
      fill_solid(leds_left, NUM_LEDS_DIGITAL, CRGB::Red);
      fill_solid(leds_right, NUM_LEDS_DIGITAL, CRGB::Red);
      break;
    case 6:
      leds_analog[2] = CRGB::Blue;
      leds_analog[3] = CRGB::Blue;
      leds_analog[4] = CRGB::Green;
      leds_analog[5] = CRGB::Red;
      fill_solid(leds_left, NUM_LEDS_DIGITAL, CRGB::Green);
      fill_solid(leds_right, NUM_LEDS_DIGITAL, CRGB::Green);
      break;
    case 7:
      leds_analog[3] = CRGB::Red;
      leds_analog[4] = CRGB::Blue;
      leds_analog[5] = CRGB::Green;
      fill_solid(leds_left, NUM_LEDS_DIGITAL, CRGB::Blue);
      fill_solid(leds_right, NUM_LEDS_DIGITAL, CRGB::Blue);
      break;
    case 8:
      fill_gradient(leds_analog, NUM_LEDS_ANALOG, CHSV(9,255,brightness), CHSV(200,255,brightness), FORWARD_HUES);
      fill_gradient(leds_left, NUM_LEDS_DIGITAL, CHSV(9,255,brightness), CHSV(200,255,brightness), FORWARD_HUES);
      fill_gradient(leds_right, NUM_LEDS_DIGITAL, CHSV(9,255,brightness), CHSV(200,255,brightness), FORWARD_HUES);
      break;
    default:
      //Leave initiation effect
      FastLED.clear ();
      phase = 1;
      effect = return_effect;
      sendState();
      break;
  }
}

void poweroff(){
  /* This function is blocking by design */
  CRGB bar_colors[6] = {CRGB::Red, CRGB::Red, CRGB::Green, CRGB::Green, CRGB::Blue, CRGB::Blue};
  
  fill_gradient_RGB(leds_left, NUM_LEDS_DIGITAL, CRGB::Red, CRGB::Black);
  fill_gradient_RGB(leds_right, NUM_LEDS_DIGITAL,CRGB::Red, CRGB::Black);
  brightness = 200;
  FastLED.setBrightness(brightness);
  FastLED.show(); 
  sendState();
  delay(1000); 

  for ( uint8_t i = 0; i <= NUM_LEDS_WALL; i++) {
    leds_analog[i] = bar_colors[i];
    FastLED.show(); 
    delay(1000); 
  }
  
  effect = "stream";
  sendState();
}

void post_movie( boolean reset ){
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  
  if(reset){
    gHue = 0;
    previousMillis = currentMillis;
    brightness=10;
    sendState();
    exit;
  }
  
  EVERY_N_MILLISECONDS( 50 ) {
    if( brightness < brightness_mem ){
      brightness++;
    }
  }
  
  cascadeWallToColor( CHSV(gHue, 255, brightness ) );  
  fadeTowardColor( leds_left_all, NUM_LEDS_DIGITAL, CHSV(gHue, 255, brightness ), 1);
  fadeTowardColor( leds_right_all, NUM_LEDS_DIGITAL, CHSV(gHue, 255, brightness ), 1);
  
  if(currentMillis - previousMillis >= 8000){
    gCurrentPatternNumber = 1;
    effect = "demo";
    sendState();
  }  
}

void game( boolean reset ) {
  EVERY_N_SECONDS( 60 ) {
    rgbSetNext();
  }

  for ( uint8_t i = 0; i < NUM_LEDS_WALL; i++) {
    fadeTowardColor( leds_wall[i], color, 5+(i*3) );
  }
      
  fadeTowardColor( leds_left_all, NUM_LEDS_DIGITAL, color, 10 );
  fadeTowardColor( leds_right_all, NUM_LEDS_DIGITAL, color, 10 );
  leds_bar = color;
  
  processSound();      
}

/*
 * Led Patersn for the gully  
 */
void theaterGully() {  
  uint8_t snakeSize = 3;
  uint8_t spaceSize = 2; 
  uint8_t cycleSize = snakeSize + spaceSize;
  static uint8_t cyclePos = 0;

  EVERY_N_MILLISECONDS( 300 ) {
    //Shift al leds
    for ( uint8_t i = NUM_LEDS_DIGITAL; i > 0; i--) { 
      leds_left_all[i] = leds_left_all[i-1];
    }

    //Set the first led
    leds_left[0] = CRGB::Black;
    if(cyclePos < snakeSize){
      leds_left[0] = CRGB::Red;
    }

    //Set the next position
    if(cyclePos >= cycleSize - 1){
      cyclePos = 0;
    }else{
      cyclePos++;
    }

    //Set the right bar
    leds_right_all = leds_left_all;
  }
}

void colorConfettiGully(CRGB color){ 
  fadeToBlackBy( leds_left, NUM_LEDS_DIGITAL, 10);
  fadeToBlackBy( leds_right, NUM_LEDS_DIGITAL, 10);
  int pos = random16( NUM_LEDS_DIGITAL * 2 );

  if(pos > NUM_LEDS_DIGITAL){
    pos = pos - NUM_LEDS_DIGITAL;
    leds_left[pos] += CRGB::Red;
  }else{
    leds_right[pos] += CRGB::Red;
  }
}

void confettiGully() {
  int pos;
  
  // random colored speckles that blink in and fade smoothly, LEFT
  fadeToBlackBy( leds_left, NUM_LEDS_DIGITAL, 10);
  pos = random16(NUM_LEDS_DIGITAL);
  leds_left[pos] += CHSV( gHue + random8(64), 200, 255);
  
  //random colored speckles that blink in and fade smoothly, RIGHT
  fadeToBlackBy( leds_right, NUM_LEDS_DIGITAL, 10);
  pos = random16(NUM_LEDS_DIGITAL);
  leds_right[pos] += CHSV( gHue + random8(64), 200, 255);
}

void rainbowGully(){
  // FastLED's built-in rainbow generator
  fill_rainbow( leds_left, NUM_LEDS_DIGITAL, gHue, 7);
  
  //mirror to right strip
  leds_right_all = leds_left_all;

  //Add to bar led
  leds_bar = leds_left[0];
}

void gullyWaveFromPallet(CRGBPalette16 palette){
    CRGB newColor;
    int fadeAmount = 40;
    
    for( int i = 0; i < NUM_LEDS_DIGITAL; i++) {
        newColor = ColorFromPalette( palette, i, brightness, LINEARBLEND);
        fadeTowardColor( leds_left[i], newColor, fadeAmount);
        fadeTowardColor( leds_right[i], newColor, fadeAmount);
    }
}

void rainbowGullyWithGlitter(){
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbowGully();
  addGlitter(10);
}

void addGlitter( fract8 chanceOfGlitter){
  if( random8() < chanceOfGlitter) {
    leds_left[ random16(NUM_LEDS_DIGITAL) ] += CRGB::White;
  }
  if( random8() < chanceOfGlitter) {
    leds_right[ random16(NUM_LEDS_DIGITAL) ] += CRGB::White;
  }
}

void sinelonGully(){
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds_left, NUM_LEDS_DIGITAL, 20);
  int pos = beatsin16(13,0,NUM_LEDS_DIGITAL);
  leds_left[pos] += CHSV( gHue, 255, 192);

  //mirror to right strip
  leds_right_all = leds_left_all;
}

void juggleGully() {
  fadeToBlackBy( leds_left, NUM_LEDS_DIGITAL, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds_left[beatsin16(i+7,0,NUM_LEDS_DIGITAL)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
  
  //mirror to right strip
  leds_right_all = leds_left_all;
}

void processSound(){
  if(sound > 0){
    sound = 0;
    fill_solid(leds_left_all, NUM_LEDS_DIGITAL, leds_left_all[0] + CHSV( gHue , 200, brightness ) );
    fill_solid(leds_right_all, NUM_LEDS_DIGITAL, leds_left_all[0] + CHSV( gHue , 200, brightness ) );
  }
}

/*
 * Led Patersn for the wall  
 */
void cascadeWallToColor( CRGB bgColor ) {
  static uint8_t pixel = 0;

  if(leds_wall[pixel] == bgColor){
    if(NUM_LEDS_WALL > pixel+1){
      pixel++;
    }else{
      pixel = 0;
    }
  }else{
    fadeTowardColor( leds_wall[pixel], bgColor, 10);
  }
}

void randomWallColors(){
  EVERY_N_SECONDS(4) { 
    for ( int i = 0; i < NUM_LEDS_WALL; i++) {
      leds_wall[i] = CHSV(random8(),255,255);
    }
  }
}

void confettiWall() {
  const CRGB primary [] = {CRGB::Red, CRGB::Green, CRGB::Blue};
  
  if (newPatern) {
    leds_wall = CRGB::Black;
    leds_wall[0] += CRGB::Red;
    newPatern = false;
  }

  fadeToBlackBy( leds_wall, NUM_LEDS_WALL, 5 ); 
  
  EVERY_N_SECONDS( 1 ) {
    leds_wall[random(0,4)] += primary[random(0,2)];
  }
}

void rainbowWall(){
  fill_rainbow( leds_analog, NUM_LEDS_ANALOG, gHue, 20);
}

void pre_movie( boolean reset ) {  
  static const int steps = 15;
  static const int virtual_length = 8;
  static int led_pos = 1;
  static const int primary [] = {255, 85, 170};
  static int pulse_color = 0;
  
  static int wall_brightness = 150;
  static int pulse_brightness = 250;
  static int timer_delay = 200;

  if(reset){
    wall_brightness = 150;
    pulse_brightness = 250;
    timer_delay = 300;
    led_pos = 1;
    pulse_color = 0;
  }
  
  static const int wall_brightness_step = wall_brightness/steps;
  static const int pulse_brightness_step = pulse_brightness / 30;
  static const int timer_delay_step = timer_delay/steps;
  
  fadeTowardColor( leds_wall, 5, CHSV(30,255,wall_brightness), 5);
  
  EVERY_N_MILLISECONDS_I( thistimer, timer_delay ) {    
    if(led_pos > virtual_length){
      led_pos = 1;
      
      pulse_color++;
      if(pulse_color > 2 ){pulse_color=0;}      
      timer_delay = constrain(timer_delay - timer_delay_step , 0, 1000);
      wall_brightness = constrain(wall_brightness - wall_brightness_step , 0, 255);
      pulse_brightness = constrain(pulse_brightness - pulse_brightness_step , 0, 255);
      thistimer.setPeriod(timer_delay);
    }

    if(led_pos <= 5 ){
      leds_analog_all[led_pos] += CHSV(primary[pulse_color],255,pulse_brightness);
    }

    if(pulse_brightness < 1){
      effect = "movie";
      sendState();
    }
    
    led_pos++;
  }
}

void sinelonWall() {
  // a colored dot sweeping back and forth, with fading trails
  if (newPatern) {
    leds_analog_all = CRGB::Black;
  }

  fadeToBlackBy( leds_analog_all, NUM_LEDS_WALL, 20);
  int pos = beatsin16(13, 0, NUM_LEDS_WALL - 1);
  leds_wall[pos] += CHSV( gHue, 255, 192);
}

/* 
 * Wifi connection function
 */
void toggle_gully(){
  static boolean gully_on = false;
  unsigned long previousMillis; 
  unsigned long currentMillis = millis();
  
  FastLED.clear();

  if(gully_on){
    fill_solid(leds_left_all,  NUM_LEDS_DIGITAL, CRGB::Red); 
    fill_solid(leds_right_all, NUM_LEDS_DIGITAL, CRGB::Red); 
  }

  //toggle
  if(currentMillis - previousMillis > 500) {
    previousMillis = currentMillis;   
    gully_on = !gully_on;
  }
  
  FastLED.show(); 
}

/*
 * 
 * Led helper functions 
 * 
 */
void nextPattern(){
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( basePatternGully );
  newPatern = true;
}

void rgbSetNext(){
  if(color == CRGB(brightness,0,0) ){
    color = CRGB(0,240,0);
  }else if(color == CRGB(0,brightness,0)){
    color = CRGB(0,0,brightness);
  }else{
    color = CRGB(brightness,0,0);
  }
}

// Helper function that blends one uint8_t toward another by a given amount
void nblendU8TowardU8( uint8_t& cur, const uint8_t target, uint8_t amount) {
  if ( cur == target) return;
  if ( cur < target ) {
    uint8_t delta = target - cur;
    delta = scale8_video( delta, amount);
    cur += delta;
  } else {
    uint8_t delta = cur - target;
    delta = scale8_video( delta, amount);
    cur -= delta;
  }
}

// Blend one CRGB color toward another CRGB color by a given amount.
// Blending is linear, and done in the RGB color space.
// This function modifies 'cur' in place.
CRGB fadeTowardColor( CRGB& cur, const CRGB& target, uint8_t amount) {
  nblendU8TowardU8( cur.red,   target.red,   amount);
  nblendU8TowardU8( cur.green, target.green, amount);
  nblendU8TowardU8( cur.blue,  target.blue,  amount);
  return cur;
}

// Fade an entire array of CRGBs toward a given background color by a given amount
// This function modifies the pixel array in place.
void fadeTowardColor( CRGB* L, uint16_t N, const CRGB& bgColor, uint8_t fadeAmount) {
  for ( uint16_t i = 0; i < N; i++) {
    fadeTowardColor( L[i], bgColor, fadeAmount);
  }
}
