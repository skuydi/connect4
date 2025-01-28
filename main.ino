/*
 * Like all Arduino code - copied from somewhere else :)
 * So don't claim it as your own
 * Based on the code from botdemy -> https://www.instructables.com/Connect-4-Game-Using-Arduino-and-Neopixel/
 * Thanks to Jason A. Cox - @jasonacox for TM1637TinyDisplay library
* ---------------------------------------------------
 Modified version - 25/02 by skuydi
* adjusted matrix size from 8x8 to 10x10
* added a debounce for the buttons
* added 3 new buttons for the 2nd player 
* added a timer to make the game faster
* added a switch to control the timer
* added a random chip position on the first line
* added a switch to control the demo mode
* added random color during the play and demo mode
* added two 7 segment display to show score and animation
* added a brightness switch to select day/night mode
* ---------------------------------------------------
 * *** Player 1 ***
 * noir   = 1
 * brun   = 2   --> A0 - rightButton1
 * blanc  = 3   --> A1 - downButton1
 * bleu   = 4   --> A2 - leftButton1
 * 
 * *** Player 2 ***
 * noir   = 1
 * brun   = 2   --> D5 - rightButton1
 * blanc  = 3   --> D7 - downButton1
 * bleu   = 4   --> D4 - leftButton1
 * ---------------------------------------------------
 */

#include <TM1637Display.h>
#include <TM1637TinyDisplay.h>
#include "animations.h"

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 6
#define NUMPIXELS 100

const int MATRIX_WIDTH = 10;
const int MATRIX_HEIGHT = 10;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);

//Player 1
#define leftButton1 A0   // D4 - A0
#define downButton1 A1   // D7 - A1
#define rightButton1 A2  // D5 - A2

//Player 2
#define leftButton2 4   // A0 - D4
#define downButton2 7   // A1 - D7
#define rightButton2 5  // A2 - D5

#define demoButton 8        // Demo button
#define timeModeButton 10   // Start button
#define brightnessButton 9  // Brightness button

// Define Arduino pins for CLK and DIO
#define CLK1 2  // Clock for first display
#define DIO1 3  // Data for first display
#define CLK2 11 // Clock for second display
#define DIO2 12 // Data for second display

// Create four TM1637 objects for the two displays
TM1637Display display1_1(CLK1, DIO1);
TM1637TinyDisplay display1_2(CLK1, DIO1);
TM1637Display display2_1(CLK2, DIO2);
TM1637TinyDisplay display2_2(CLK2, DIO2);

int board[NUMPIXELS]; /* RGB Matrix Grid */

/* -Normal-
00 01 02 03 04 05 06 07 08 09
10 11 12 13 14 15 16 17 18 19
20 21 22 23 24 25 26 27 28 29
30 31 32 33 34 35 36 37 38 39
40 41 42 43 44 45 46 47 48 49
50 51 52 53 54 55 56 57 58 59
60 61 62 63 64 65 66 67 68 69
70 71 72 73 74 75 76 77 78 79
80 81 82 83 84 85 86 87 88 89
90 91 92 93 94 95 96 97 98 99
*/

/* -ZigZag-
00 01 02 03 04 05 06 07 08 09
19 18 17 16 15 14 13 12 11 10
29 28 27 26 25 24 23 22 21 20
39 38 37 36 35 34 33 32 31 30
49 48 47 46 45 44 43 42 41 40
59 58 57 56 55 54 53 52 51 50
69 68 67 66 65 64 63 62 61 60
79 78 77 76 75 74 73 72 71 70
89 88 87 86 85 84 83 82 81 80
99 98 97 96 95 94 93 92 91 90
*/

//RGB LED values for player 1 (index 0) and player 2 (index 1)
int r[2];
int g[2];
int b[2];

boolean demoMode = false; //run in demo mode until reset

int whoGoesFirst = 1;     // Player to go first.
int elapsingTime;         // Elapsing time to play
int startTimer = 2000;    // Timer befor start

bool isDemoMode;
bool buttonState = false;         // Button state (pressed or not)
bool lastButtonState = false;     // Last button state to detect changes
unsigned long lastPressTime = 0;  // Time of last button press
unsigned long timerStart = 0;     // Start time of 2-second timer
bool waitingForStart = false;     // If button is pressed, timer starts

int score = 0;
int scorePlayer1 = 0;
int scorePlayer2 = 0;

// Lap timer
int normalTime = 9000;
int nightmareTime = 3000;

// Define segments for “P”, “J”, “1”, “2”.
uint8_t P = 0b11110011;       // Segment for displaying “P”.
uint8_t J = 0b00001110;       // Segment to display “J”.
uint8_t digit1 = 0b00000110;  // segment to display “1
uint8_t digit2 = 0b01011011;  // Segment to display “2”.

int LcdBrightness = 5;
int LcdBrightnessDay = 5;       // Day mode
int LcdBrightnessNight = 1;     // Night mode
int MatrixBrightness = 75;
int MatrixBrightnessDay = 100;  // Day mode
int MatrixBrightnessNight = 10; // Night mode
  
void setup() {
  Serial.begin(9600);
  //button switches - use "PULLUP" to set open/unpressed to HIGH
  pinMode(leftButton1, INPUT_PULLUP);
  pinMode(downButton1, INPUT_PULLUP);
  pinMode(rightButton1, INPUT_PULLUP);
  pinMode(leftButton2, INPUT_PULLUP);
  pinMode(downButton2, INPUT_PULLUP);
  pinMode(rightButton2, INPUT_PULLUP);
  pinMode(demoButton, INPUT_PULLUP);
  pinMode(timeModeButton, INPUT_PULLUP);
  pinMode(brightnessButton, INPUT_PULLUP);
  
  pixels.begin(); // This initializes the NeoPixel library.
  pixels.show();  // Initialize all pixels to 'off'

  //randomSeed(analogRead(A3));
  randomSeed(analogRead(A3)*analogRead(A4));
  
  // Initialize displays - Adjust brightness (0 to 7)
  display1_1.setBrightness(LcdBrightness); 
  display1_1.showNumberDecEx(0, 0b01000000, true, 4, 4);
  display1_2.setBrightness(LcdBrightness);
  display1_2.showAnimation_P(ANIMATION1, FRAMES(ANIMATION1), TIME_MS(50));

  display2_1.setBrightness(LcdBrightness);
  display2_1.showNumberDecEx(0, 0b01000000, true, 4, 4);
  display2_2.setBrightness(LcdBrightness);
  display2_2.showAnimation_P(ANIMATION1, FRAMES(ANIMATION1), TIME_MS(50));
  
  //Serial.println("Ready!!!");
}

void loop() {
   
  if (digitalRead(brightnessButton) == LOW) {
    LcdBrightness = LcdBrightnessNight;
    MatrixBrightness = MatrixBrightnessNight;
  }
  else{
    LcdBrightness = LcdBrightnessDay;
    MatrixBrightness = MatrixBrightnessDay;
  }
  display1_1.setBrightness(LcdBrightness);
  display1_2.setBrightness(LcdBrightness);
  display2_1.setBrightness(LcdBrightness);
  display2_2.setBrightness(LcdBrightness);
  
// Read button status
  buttonState = digitalRead(timeModeButton) == LOW;  // Button is activated when connected to GND (LOW)
  bool demoMode = !digitalRead(demoButton);  // Demo mode on/off
  bool nightmareMode = !digitalRead(timeModeButton);
 
  // Nightmare mode
  if (!nightmareMode) {
    elapsingTime = normalTime;
    //Serial.print(elapsingTime);
  } else {
    elapsingTime = nightmareTime;
    //Serial.print(elapsingTime);
  }

  if (demoMode) { //run game demo
    clearScore();
    startDemo();
  } else {
    twoPlayerGame(); // two player game

    // If the button has been pressed (change of state from “not pressed” to “pressed”)
    if (buttonState && !lastButtonState) {
      lastPressTime = millis(); // Record the moment the button was pressed
      waitingForStart = true; // Start waiting for timer duration
      timerStart = millis(); // Start timer
      delay(300); // Anti-bounce to avoid multiple detections of the same a
    }
    lastButtonState = buttonState; // Save current button state   

    // If we wait for the 2 second timer after the button is pressed
    if (waitingForStart) {
      unsigned long currentTime = millis();
      if (currentTime - timerStart >= startTimer) { // If 2 seconds have elapsed
        waitingForStart = false; // Stop timer
        twoPlayerGame(); // two player game
        delay(300); // Anti-bounce to avoid multiple detections of the same press
      }
    }
  }
}

void twoPlayerGame () {
  pixels.setBrightness(MatrixBrightness);  // max 255
  
  // Choose a random color for player 1
  int index1 = random(0, 6); // Index for player 1 (0 to 10)
  int index2;
  do {
    // Choose a random color for player 2, ensuring it's different from player 1
    index2 = random(0, 6); // Index for player 2 (0 to 10)
  } while (index1 == index2); // Repeat until index2 is different from index1
    
  // Array of possible colors (R, G, B)
  int colors[6][3] = {
    {255, 0, 0},   // Red
    {0, 0, 255},   // Blue
    {0, 255, 0},   // Green
    {0, 255, 255}, // Cyan
    {255, 0, 255}, // Magenta
    {255, 165, 0}, // Orange
  };
  
  r[0] = colors[index1][0];
  g[0] = colors[index1][1];
  b[0] = colors[index1][2];

  r[1] = colors[index2][0];
  g[1] = colors[index2][1];
  b[1] = colors[index2][2];

  int whoseTurn = whoGoesFirst;

  //fillBoard(); //for testing

  //while (eachTurn(whoseTurn) == 0) {  //returns 1 if end of game.. TODO recode this
  while (!endGame(whoseTurn)) {
    if (whoseTurn == 1) {
      whoseTurn = 2;  //next turn is player 2
    } else {
      whoseTurn = 1;  //next turn is player 1
    }
  }

  melt();

  //who goes first next turn: first player has an advantage so loser goes first
  if (whoseTurn == 1) {
    whoGoesFirst = 2;  //player 2 starts the next game
  } else {
    whoGoesFirst = 1;  //player 1 starts the next game
  }
}

void runDemo() {
  // Checks if the program is in DEMO mode
  if (isDemoMode) {
    pixels.setBrightness(MatrixBrightness);

    // Array of possible colors (R, G, B)
    int colors[6][3] = {
      {255, 0, 0},   // Red
      {0, 0, 255},   // Blue
      {0, 255, 0},   // Green
      {0, 255, 255}, // Cyan
      {255, 0, 255}, // Magenta
      {255, 165, 0}, // Orange
    };

    // Choose a random color for player 1
    int index1 = random(0, 6); // Index for player 1 (0 to 10)

    // Choose a random color for player 2, ensuring it's different from player 1
    int index2;
    do {
      index2 = random(0, 6); // Index for player 2 (0 to 10)
    } while (index1 == index2); // Repeat until index2 is different from index1
  
    // Assign selected colors to players
    r[0] = colors[index1][0];
    g[0] = colors[index1][1];
    b[0] = colors[index1][2];

    r[1] = colors[index2][0];
    g[1] = colors[index2][1];
    b[1] = colors[index2][2];

/*
    // Choose a random color for each player
    int index1 = random(0, 8); // Index for player 1 (0, 1, 2)
    int index2 = random(0, 8); // Index for player 2 (0, 1, 2);
*/  

/*
    //Completely random colors
    r[0] = random(0, 256); // Random color for player 1 (Red)
    g[0] = random(0, 256); // Random color for player 1 (Green)
    b[0] = random(0, 256); // Random color for player 1 (Blue)
    r[1] = random(0, 256); // Random color for player 2 (Red)
    g[1] = random(0, 256); // Random color for player 2 (Green)
    b[1] = random(0, 256); // Random color for player 2 (Blue)
 */
  }
 
  // Run a game with a random number of rounds (e.g. 50)
  randomPlay(50); // play 50 rounds or until the end of the game
  delay(1000); // pause before “melting” the screen
  melt();
  delay(100); // pause after melt
}

void startDemo() {
  isDemoMode = true;    // Activate DEMO mode
  runDemo();            // Start DEMO mode
  isDemoMode = false;   // Disable DEMO mode after runtime
}

/*
    Find a starting spot for each player
    Some of the top row columns may be played already so find an empty spot to start
    If nothing found, return 11 (board full)
    TODO: auto play last chip
*/

int startingSpot(int player) {

  int i = i = randomColumn();
  int pixelLoc;
  int prevLoc;
  boolean foundStartLoc = false;

  if (player == 1) {  //player1 start from left
    int i = randomColumn();
    //find a starting pixel: normally at 0
    while ((i < 10) && (!foundStartLoc)) {
      if (board[i] == 0) { //found an empty spot to start
        pixelLoc = i;
        showPixel(player, pixelLoc);
        foundStartLoc = true;
      } else { //occupied, go to next
        i++;
      }
    }
  } else { //player2 starts from the right side
    int i = randomColumn();
    while ((i >= 0) && (!foundStartLoc)) {
      if (board[i] == 0) { //found an empty spot to start
        pixelLoc = i;
        showPixel(player, pixelLoc);
        foundStartLoc = true;
      } else { //occupied, go to next
        i--;
      }
    }
  }

  if (foundStartLoc) {
    return pixelLoc;
  } else {
    return 11;  // 11 means no more space to play
  }
}

boolean endGame(int player) {
  int prevLoc;
  // find a starting location for the current player
  int pixelLoc = startingSpot(player);

  if (pixelLoc == 11) { // No place to play, game over
    Serial.println("No place to play. Game over");
    return true;
  }

  int WhichButton = isButtonPressed(player); // Detects which button is pressed for the active player
  unsigned long startTime = millis(); // Start timer for 3 seconds

  while (WhichButton != 2) { // As long as it's not the down button (to “drop” the token)
    unsigned long currentTime = millis(); // Check current time
    unsigned long diff = elapsingTime-(currentTime-startTime);
    display1_1.showNumberDecEx(diff, 0b00000000, true, 4, 4); // 0b00000000 without the ":", 0b01000000 with the ":"
    display2_1.showNumberDecEx(diff, 0b00000000, true, 4, 4); // 0b00000000 without the ":", 0b01000000 with the ":"

    if (currentTime - startTime > elapsingTime) { // If more than 3 seconds have elapsed
      // Serial.println("Time up, next player.");
      // Delete the pawn of the player whose time has elapsed (reset pawn position)
      board[pixelLoc] = 0; // Reset the square to 0 (or another value representing an empty square)
      movePixel(player, pixelLoc, -1); // Move the player's pixel off-screen or to an invalid position

      return false; // Move to next player
    }

    boolean okToMove = false;  // Reset on each loop

    if (WhichButton == 1) {  // If the right button is pressed
      // Vérifier à gauche
      int chkLoc = pixelLoc - 1;
      while ((chkLoc >= 0) && (!okToMove)) {
        if (board[chkLoc] == 0) {
          prevLoc = pixelLoc;
          pixelLoc = chkLoc;  // Move left
          okToMove = true;
        } else {
          chkLoc--;
        }
      }
      movePixel(player, prevLoc, pixelLoc);
    } else if (WhichButton == 3) {  // If the right button is pressed
      // Vérifier à droite
      int chkLoc = pixelLoc + 1;
      while ((chkLoc <= 9) && (!okToMove)) {
        if (board[chkLoc] == 0) {
          prevLoc = pixelLoc;
          pixelLoc = chkLoc;  // Move right
          okToMove = true;
        } else {
          chkLoc++;
        }
      }
      movePixel(player, prevLoc, pixelLoc);
    }
    delay(1); // Short pause before checking again
    WhichButton = isButtonPressed(player); // Check button status for active player
  }

  // If the down button is pressed, play the token in the selected column
  if (WhichButton == 2) {
    int playedLoc = dropChip(pixelLoc, player);

    if (isWinningMove(player, playedLoc)) {
      // Control and display the score
      if (player == 1) {
        scorePlayer1++;
      } else if (player == 2) {
        scorePlayer2++;

      }

      if (scorePlayer1 > 2) {
        displayScore();
        displayWinner1();
      } else if (scorePlayer2 > 2) {
        displayScore();
        displayWinner2();
      } else { 
        displayScore();
      }
      return true; // The player has won

    } else {
      return false; // The game continues
    }
  }
  return false; // Returns false if no button is pressed
}

// New function to convert (x, y) into indexes in zigzag wiring
int getZigzagIndex(int x, int y) {
  if (y % 2 == 0) {
    return y * 10 + x; // Normal line (even)
  } else {
    return y * 10 + (9 - x); // Inverted line (odd)
  }
}

// Function to calculate the real index of an LED in a zigzag matrix
int zigzagIndex2(int row, int col) {
  if (row % 2 == 0) {
    // Even row: from left to right
    return row * MATRIX_WIDTH + col;
  } else {
    // Odd line: from right to left
    return row * MATRIX_WIDTH + (MATRIX_WIDTH - 1 - col);
  }
}

// Modify showPixel to use the zigzag index
void showPixel(int player, int location) {
  int x = location % 10;  // Colonne
  int y = location / 10;  // Ligne
  int zigzagIndex = getZigzagIndex(x, y);

  pixels.setPixelColor(zigzagIndex, pixels.Color(r[player - 1], g[player - 1], b[player - 1])); // Switch on the LED
  pixels.show(); // Mettre à jour
}

// MovePixel modification for zigzag wiring
void movePixel(int player, int currentLoc, int nextLoc) {
  int x1 = currentLoc % 10, y1 = currentLoc / 10; // Current coordinates
  int x2 = nextLoc % 10, y2 = nextLoc / 10;      // Next coordinates

  int currentZigzagIndex = getZigzagIndex(x1, y1);
  int nextZigzagIndex = getZigzagIndex(x2, y2);

  pixels.setPixelColor(nextZigzagIndex, pixels.Color(r[player - 1], g[player - 1], b[player - 1])); // Switch on the next one
  pixels.setPixelColor(currentZigzagIndex, pixels.Color(0, 0, 0)); // Switch off the current one
  pixels.show(); // Update
}

int isButtonPressed(int player) {
    const int debounceDelay = 25;
    auto waitForRelease = [](int buttonPin) {
        while (digitalRead(buttonPin) == LOW) {
            delay(10);
        }
    };

    auto isStablePress = [&](int buttonPin) {
        if (digitalRead(buttonPin) == LOW) {
            delay(debounceDelay);
            return digitalRead(buttonPin) == LOW;
        }
        return false;
    };

    if (player == 1) {
        if (isStablePress(leftButton1)) {
            waitForRelease(leftButton1);
            return 1;  // Gauche
        } else if (isStablePress(downButton1)) {
            waitForRelease(downButton1);
            return 2;  // Bas
        } else if (isStablePress(rightButton1)) {
            waitForRelease(rightButton1);
            return 3;  // Droite
        }
    } else if (player == 2) {
        if (isStablePress(leftButton2)) {
            waitForRelease(leftButton2);
            return 1;  // Gauche
        } else if (isStablePress(downButton2)) {
            waitForRelease(downButton2);
            return 2;  // Bas
        } else if (isStablePress(rightButton2)) {
            waitForRelease(rightButton2);
            return 3;  // Droite
        }
    }
    return 0;  // No button pressed
}

// DropChip modified to respect zigzag wiring
int dropChip(int currentLoc, int player) {
  boolean notOccupied = true;
  int checkLoc = currentLoc + 10; // Next location
  int prevLoc;

  // Light the top line first
  showPixel(player, currentLoc);

  while ((checkLoc < 100) && (notOccupied)) {
    if (board[checkLoc] == 0) { // Not occupied
      prevLoc = currentLoc;
      currentLoc = checkLoc;
      movePixel(player, prevLoc, currentLoc);
      checkLoc = checkLoc + 10; // Check lower level
    } else { // Occupé
      notOccupied = false;
    }
    delay(25); // Slow down the fall
  }

  board[currentLoc] = player;
  return currentLoc;
}

void showConnect4(int connect4[]) {
  int player = board[connect4[0]];  // Determines the associated player
  
  //Serial.print("Connected dots are : ");
  for (int l = 0; l <= 3; l++) {
    //Serial.print(connect4[l]);
    //Serial.print(" ");
  }
  //Serial.println();

  // Blink 4 times
  for (int i = 0; i < 4; i++) {
    // Switch off all connect4 LEDs
    for (int j = 0; j <= 3; j++) {
      int row = connect4[j] / MATRIX_WIDTH;
      int col = connect4[j] % MATRIX_WIDTH;
      int realIndex = zigzagIndex2(row, col);
      pixels.setPixelColor(realIndex, pixels.Color(0, 0, 0));
    }
    pixels.show();
    delay(200);

    // Switch on all connect4 LEDs
    for (int j = 0; j <= 3; j++) {
      int row = connect4[j] / MATRIX_WIDTH;
      int col = connect4[j] % MATRIX_WIDTH;
      int realIndex = zigzagIndex2(row, col);
      pixels.setPixelColor(realIndex, pixels.Color(r[player - 1], g[player - 1], b[player - 1]));
    }
    pixels.show();
    delay(200);
  }
}

/*
   clear the board matrix with 0(not occupied) and no color
*/
void clearBoard() {
  for (int i = 0; i < NUMPIXELS; i++) {
    board[i] = 0;
    pixels.setPixelColor(i, pixels.Color(0, 0, 0)); // off
  }
  pixels.show();
}

/*
   used to simulate all the chips melting.
*/

void melt() { //melting effect

  int color;

  //Serial.println("Initiate melting effect");

  for (int k = 0; k <= 9; k++) {
    //first time 55 to

    for (int i = 55; i >= 10 * k; i--) { //100-45=55 always starts from the 2nd to the bottom row

      color = board[i];

      //change row below to the same color as top
      if (color == 0) {
        pixels.setPixelColor(i + 10, pixels.Color(0, 0, 0)); // black
        board[i + 10] = 0;
      } else {
        pixels.setPixelColor(i + 10, pixels.Color(r[color - 1], g[color - 1], b[color - 1])); // set color
        board[i + 10] = color;
      }
    }

    pixels.show();
    delay(100);

    //change top rows to black //h = 0, from 9 to 0, h=1, from 15 to 10, when h=2, 23-16
    for (int i = (9 + (10 * k)); i >= (10 * k); i--) {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0)); // black
      board[i] = 0;
    }
    //Serial.println("outloop");

    pixels.show();
    //drawBoard();
    //delay(50);
  }
  delay(500);
}

/*
   Below are functions to check for connect 4
*/

boolean isWinningMove(int player, int playedLoc) {

  /*
    Serial.println("-----------------------------");
    Serial.print("isWinningMove: playedLoc = ");
    Serial.println(playedLoc);
  */

  int connected[4];  //store all connected location
  /*
    //test
    if (playedLoc < 70) {
    Serial.println("Connect4 - vertical");
    connected[0] = playedLoc;
    connected[1] = playedLoc + 10;
    connected[2] = playedLoc + 20;
    connected[3] = playedLoc + 30;
    showConnect4(connected);
    return true;
    }
  */

  //check for veritcal connect4
  if (chkVericalWin(player, playedLoc)) {
    return true;
  } else if (chkHorizontalWin(player, playedLoc)) {
    return true;
  } else if (chkDiagonalWin(player, playedLoc)) { // up & right dirction TODO: combine with below
    return true;
  } else if (chkDiagonalWin2(player, playedLoc)) { // up and left direction
    return true;
  } else {
    return false;
  }
}

boolean chkVericalWin(int player, int playedLoc) {

  int connected[4]; //init arrayf

  //Serial.println("chkVericalWin: ");

  if ((playedLoc < 70) && (board[playedLoc + 10] == player) && (board[playedLoc + 20] == player) && (board[playedLoc + 30] == player)) {
    //Serial.println("********** Connect4 - veritcal **********");
    //make array
    connected[0] = playedLoc;
    connected[1] = playedLoc + 10;
    connected[2] = playedLoc + 20;
    connected[3] = playedLoc + 30;
    showConnect4(connected);
    return true;
  } else {
    //Serial.println("  not connected");
    return false;
  }
}

boolean chkDiagonalWin2(int player, int playedLoc) {

  int connected[4] = {0, 0, 0, 0}; //init array

  int connectedIndex = 0; //starting with one connection index - self
  connected[connectedIndex] = playedLoc;

  //Serial.println("chkDiagonal  reverse");

  //check row above & left -> \ direction
  boolean keepChecking = true;
  int chkLoc;

  //check for right edge condition
  if ((playedLoc) % 10 == 0) {
    //playedLoc is on the left edge, no need to check further
    //Serial.println("left edge");
    keepChecking = false;
  } else {
    chkLoc = playedLoc - 11; // 56
  }

  while (keepChecking) {
    //Serial.print("  chkloc(up & left) = ");
    //Serial.println(chkLoc);

    if (board[chkLoc] == player) {
      connectedIndex++;
      connected[connectedIndex] = chkLoc;
      //Serial.println("  diag \ connected ");
      //Serial.println(connected[0]);
      //Serial.println(connected[1]);
      //Serial.println(connected[2]);
      //Serial.println(connected[3]);

      if (connectedIndex >= 3) {
        //Serial.println("**********Connect4 - diagonal**********");
        showConnect4(connected);
        return true;
      }

      //deal with boundary issue
      if ((chkLoc) % 10 == 0) {
        //chkLoc is on the left edge, no need to check further
        //Serial.println("left edge");
        keepChecking = false;
      } else {
        chkLoc = chkLoc - 11; // 56
      }

    } else {
      //Serial.println("  diag \ not connected ");
      keepChecking = false;
    }
  }

  // check going down diagonally in "\" direction
  keepChecking = true;

  //check for right edge condition ex: 55,49,1111
  if ((playedLoc + 1) % 10 == 0) {
    //playedLoc is on the left edge, no need to check further
    //Serial.println("right edge");
    keepChecking = false;
  } else {
    chkLoc = playedLoc + 11; //54 + 11 = 65
  }

  while ((keepChecking) && (chkLoc <= 99)) {
    //Serial.print("  chkloc(down & right) = ");
    //Serial.println(chkLoc);
    if (board[chkLoc] == player) {
      connectedIndex++;
      connected[connectedIndex] = chkLoc;
      //Serial.println("  diag \ connected ");
      //Serial.println(connected[0]);
      //Serial.println(connected[1]);
      //Serial.println(connected[2]);
      //Serial.println(connected[3]);

      if (connectedIndex >= 3) {
        //Serial.println("********** Connect4 - diagonal **********");
        showConnect4(connected);
        return true;
      }

      //edge = (playedLoc) % 10;
      if ((chkLoc + 1) % 10 == 0) {  //ex 49
        //chkLoc is on the right edge, no need to check further
        //Serial.println("right edge");
        keepChecking = false;
      } else {
        chkLoc = chkLoc + 11;
      }
    } else {
      //Serial.println("  diag \ not connected ");
      keepChecking = false;
    }
  }

  //Serial.print("connectedIndex: ");
  //Serial.println(connectedIndex);

  //drawBoard();

  return false;
}

boolean chkDiagonalWin(int player, int playedLoc) {

  int connected[4] = {0, 0, 0, 0}; //init array

  int connectedIndex = 0; //starting with one connection index - self
  connected[connectedIndex] = playedLoc;

  //Serial.println("chkDiagonal /");

  //check row above & right -> / direction
  boolean keepChecking = true;
  int chkLoc;

  //check for right edge condition
  //int edge = (playedLoc + 1) % 10;
  if ((playedLoc + 1) % 10 == 0) {
    //playedLoc is on the right edge, no need to check further
    //Serial.println("right edge");
    keepChecking = false;
  } else {
    chkLoc = playedLoc - 9; //56
  }

  while (keepChecking) {
    //Serial.print("  chkloc(up & right) = ");
    //Serial.println(chkLoc);

    if (board[chkLoc] == player) {
      connectedIndex++;
      connected[connectedIndex] = chkLoc;
      //Serial.println("  diag / connected ");
      //Serial.println(connected[0]);
      //Serial.println(connected[1]);
      //Serial.println(connected[2]);
      //Serial.println(connected[3]);

      if (connectedIndex >= 3) {
        //Serial.println("**********Connect4 - diagonal**********");
        showConnect4(connected);
        return true;
      }

      //deal with boundary issue
      if ((chkLoc + 1) % 10 == 0) {
        //playedLoc is on the right edge, no need to check further
        //Serial.println("right edge");
        keepChecking = false;
      } else {
        chkLoc = chkLoc - 9; //56
      }

    } else {
      //Serial.println("  diag / not connected ");
      keepChecking = false;
    }
  }

  keepChecking = true;

  //check for left edge condition 56 is edge
  //edge = (playedLoc) % 10;
  if ((playedLoc) % 10 == 0) {
    //playedLoc is on the left edge, no need to check further
    //Serial.println("left edge");
    keepChecking = false;
  } else {
    chkLoc = playedLoc + 9; //56
  }

  // check going down diagonally in "/" direction
  while ((keepChecking) && (chkLoc <= 1111)) {
    //Serial.print("  chkloc(down & left) = ");
    //Serial.println(chkLoc);
    if (board[chkLoc] == player) {
      connectedIndex++;
      connected[connectedIndex] = chkLoc;
      //Serial.println("  diag / connected ");
      //Serial.println(connected[0]);
      //Serial.println(connected[1]);
      //Serial.println(connected[2]);
      //Serial.println(connected[3]);

      if (connectedIndex >= 3) {
        //Serial.println("********** Connect4 - diagonal **********");
        showConnect4(connected);
        return true;
      }

      //edge = (playedLoc) % 10;
      if ((chkLoc) % 10 == 0) {
        //playedLoc is on the left edge, no need to check further
        //Serial.println("left edge");
        keepChecking = false;
      } else {
        chkLoc = chkLoc + 9; //56
      }
    } else {
      //Serial.println("  diag / not connected ");
      keepChecking = false;
    }
  }

  //drawBoard();

  return false;
}

boolean chkHorizontalWin(int player, int playedLoc) {

  int connected[4]; //init array

  int connectedIndex = 0; //starting with one connection index - self
  connected[connectedIndex] = playedLoc;

  int leftWall = int(playedLoc / 10) * 10;

  //Serial.print("chkHorizontalWin: leftWall =  ");
  //Serial.println(leftWall);

  //until hit a wall, keep checking left side
  int chkLoc = playedLoc - 1;
  boolean keepChecking = true;

  while ((leftWall <= chkLoc) && (keepChecking)) {
    //Serial.print("  chkloc = ");
    //Serial.println(chkLoc);
    if ( board[chkLoc] == player ) {
      //left connected
      connectedIndex++;
      connected[connectedIndex] = chkLoc;
      //Serial.println("  left connected ");  //33
      //Serial.println(chkLoc);
      if (connectedIndex >= 3) {
        //Serial.println("********** Connect4 - horizontal **********");
        showConnect4(connected);
        return true;
      }
    } else {
      //Serial.println("  left not connected");
      keepChecking = false;
    }
    chkLoc--; //32
  } //while to check left

  //check right: ex: for 61, right wall is 1111
  int rightWall = leftWall + 9;

  //Serial.print("chkHorizontalWin: rightWall =  ");
  //Serial.println(rightWall);

  //until hit a wall, keep checking left side
  chkLoc = playedLoc + 1;
  keepChecking = true;

  while ((rightWall >= chkLoc) && (keepChecking)) {
    //Serial.print("  chkloc = ");
    //Serial.println(chkLoc);
    if ( board[chkLoc] == player ) {
      //right connected
      connectedIndex++;  //keep accumulating connected points
      connected[connectedIndex] = chkLoc;
      //Serial.println("  right connected ");  // 33
      //Serial.println(chkLoc);
      if (connectedIndex >= 3) {
        //Serial.println("**********Connect4 - horizontal**********");
        showConnect4(connected);
        return true;
      }
    } else {
      //Serial.println("  right not connected");
      keepChecking = false;
    }
    chkLoc++; //32
  } //while to check left

  return false;
}

/*
    utilities used in testing
*/
void fillBoard() { //for testing purpose only
  int player = 2;

  for (int i = 10; i < NUMPIXELS; i++) {

    if (player == 1) {
      player = 2;
    } else {
      player = 1;
    }

    board[i] = player;
    pixels.setPixelColor(i, pixels.Color(r[player - 1], g[player - 1], b[player - 1])); // set color
  }
  pixels.show();
}

void drawBoard() {
  for (int i = 0; i < 10; i++) { //row loop

    for (int j = (0 + (10 * i)); j < (10 + (10 * i)); j++) {
      //Serial.print(j);
      Serial.print(board[j]);
      //pixels.setPixelColor(i, pixels.Color(0,0,0)); // Moderately bright green color.
      //pixels.show();
    }
    Serial.println();
  }
  Serial.println("--------");
}

/*
   Functions for play demo. This has no "winning" algorithm; it just randomly drops a chip until
   a player "accidently" wins the game.
*/

void randomPlay(int turns) {
  boolean gameOver = false;
  int player = 2;
  int i = 1;

  //Serial.println("random play");

  while ((i < turns) && (!gameOver)) {
    if (player == 1) {
      player = 2;
    } else {
      player = 1;
    }

    //pick a column that's not full
    int chooseColumn = randomColumn();
    while (board[chooseColumn] > 0) { //occupied - pick another number
      //Serial.println("@randomplay - column is full: ");
      //delay(5000);
      //pick another column
      chooseColumn = randomColumn();
    }

    //wait for down button: for debugging one turn at a time
    /*
      while (isButtonPressed() != 2 ) {
      //Serial.println("waiting for down button");
      delay(10);
      }*/

    //Serial.println(chooseColumn);
    int playedLoc = dropChip(chooseColumn, player);

    if (isWinningMove(player, playedLoc)) {
      gameOver = true;
    }
    i++;

    //pause after each turn
    delay(500);

  }
  //Serial.println("********** random play - game over **********");
}

//function to randomly pick a column to dropChip
int randomColumn() {
  int randomNum;
  //random(min, max)
  //Parameters
  //min - lower bound of the random value, inclusive (optional)
  //max - upper bound of the random value, exclusive

  //generate random # from 2 to 5 that matches led Pins
  //2 = red, 3 = blue, 4 = yellow, 5=green

  randomNum = random(0, 10);
  return randomNum;
}

// function to display the score
void displayScore() {
  // Display the score
  score = (scorePlayer1*100)+scorePlayer2;
  display1_1.showNumberDecEx(score, 0b01000000, true, 4, 4);
  display2_1.showNumberDecEx(score, 0b01000000, true, 4, 4);

  /*
  Serial.print("Score - ");
  Serial.print("Joueur 1 : ");
  Serial.println(scorePlayer1);
  Serial.print("Score - ");
  Serial.print("Joueur 2 : ");
  Serial.println(scorePlayer2);
  */
}

// Show animation winner 1
void displayWinner1() {
  display1_2.showAnimation_P(ANIMATION7, FRAMES(ANIMATION7), TIME_MS(50));
  //Serial.print("Player 1 wins");
  display1_1.clear();  // Clear display
  for (int i = 0; i < 3; i++) { 
    display1_1.setSegments(&J, 1, 0);       // Displays “J” on the first digit
    display1_1.setSegments(&digit1, 1, 1);  // Displays “1” on the second digit
    delay(500);    
    display1_1.clear();  // Clear display
    delay(500);  
  }
  display1_2.showAnimation_P(ANIMATION1, FRAMES(ANIMATION1), TIME_MS(50));
  clearScore();
  displayScore();
}

// Show animation winner 2
void displayWinner2() {
  display2_2.showAnimation_P(ANIMATION7, FRAMES(ANIMATION7), TIME_MS(50));
  //Serial.print("Player 2 wins");
  display2_1.clear();  // Clear display
  for (int i = 0; i < 3; i++) { 
    display2_1.setSegments(&J, 1, 2);       // Displays “J” on the first digit
    display2_1.setSegments(&digit2, 1, 3);  // Displays “2” on the second digit
    delay(500);    
    display2_1.clear();  // Clear display
    delay(500);  
  }
  display2_2.showAnimation_P(ANIMATION1, FRAMES(ANIMATION1), TIME_MS(50));
  clearScore();
  displayScore();
}

// Reset the score
void clearScore(){
  scorePlayer1 = 0;
  scorePlayer2 = 0;
}
