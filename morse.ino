/*
 * Morse code training on the MicroView 
 * Fredrik Hansteen
 * 2022.04.10 - First version for the Easter egg hunt
 * 
 */

#include <MicroView.h>
#include <string.h>

#define MAX_LETTERLENGTH 5    // max number of dots or dashes in a letter
#define MAX_MESSAGE 100       // max number of letters in sentence
#define NOTFOUND ' '          // character to display if unable to decode
#define X_input 15
#define Y_input 0
#define X_decode 15
#define Y_decode 15
#define Y_message 30

int LED = A3;         // declare LED as pin A3 of MicroView
int KEY = 2;          // Pin named 2 is 11 on microview
int AUDIO = A5;       // Pin named A5 is 2 on MicroView

const int pulsemin = 5;            // ms
const int soundfreq = 880;         // Hz
const int dot = 150;               // ms
const int letterspace = 500;       // ms
const int blanktime = 5000;        // ms  - blank out all input

char morseletter[MAX_LETTERLENGTH+1]; // current letter
unsigned int symbolpos;               // symbol position pointer in current letter
unsigned int letterpos;               // letter position pointer in current sentence
char decoded;                         // Latin alphapeth representation
char displaymessage[10];              // text displaymessage
char message[MAX_MESSAGE+1];          // keyed sentence
byte fontwidth;

unsigned long pulsestart, pulseend;
unsigned long previous, current;

volatile unsigned long pulse;      // new pulse duration, 0 if not available
unsigned long space;      // new space duration, 0 if not yet available
int state;                // state of key  (HIGH is open, LOW is depressed)



/*********************************************************************************/
/* Morse sequence decoder                                                        */
/*********************************************************************************/
static char* dictionary[] = {
".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", // A-I
".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", // J-R
"...", "-", "..-", "...-", ".--", "-..-", "-.--", "--.." // S-Z
};

static char* digits[] = {
"-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----." // 0-9
};

char decode(char *morse)
{
  char c = NOTFOUND;
  char base_letter = 'A';
  char base_digit = '0';
  char i;

  for(i=0; i<26; i++) {  // Check dictionary for letter
    if(strcmp(morse, dictionary[i])==0) {
      c = base_letter + i;
      break;
    }
  }
  if(c==NOTFOUND) {      // If not found among letters, check numbers
    for(i=0; i<10; i++) {
      if(strcmp(morse, digits[i])==0) {
        c = base_digit + i;
        break;
      }
    }
  }
return(c);
}

/*********************************************************************************/
/* Interrupt handler                                                             */
/*********************************************************************************/
void isr()
{
  // Debounce - read multiple times, wait for stability?
  // Most efficient seems to be a 0.1uF cap in parallell with the switch

  current = millis();
  state = digitalRead(KEY);
  
  if (state == LOW) {
    pulsestart = current;
    digitalWrite(LED, HIGH);
    tone(AUDIO, soundfreq);
  }  else {
    pulseend = current;
    pulse = pulseend - pulsestart;
    digitalWrite(LED, LOW);
    noTone(AUDIO);
  }
  
  previous = current;  // store time of this pulse
}


/*********************************************************************************/
/* Setup                                                                         */
/*********************************************************************************/
void setup() {
  // put your setup code here, to run once:
  uView.begin();
  uView.clear(PAGE);
  uView.setFontType(0);     // set font type 0, please see declaration in MicroView.cpp
  fontwidth = uView.getFontWidth() + 1;
  
  pinMode(LED, OUTPUT); // set LED pin as OUTPUT
  pinMode(KEY, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(KEY), isr, CHANGE);

  previous = millis();
  symbolpos = 0;
  letterpos = 0;
  pulse = 0;
  space = 0;
  state = HIGH;
  morseletter[MAX_LETTERLENGTH] = '\0';
  morseletter[0] = '\0';
  message[MAX_MESSAGE] = '\0';
  message[0] = '\0';
}


/*********************************************************************************/
/* Loop                                                                          */
/*********************************************************************************/
void loop() {

  char symbol;

  if(pulse > pulsemin) {         // Got a pulse - it could be a dot or a dash
    if ( pulse <= dot )
      symbol = '.';
    else
      symbol = '-';
    morseletter[symbolpos] = symbol;
    uView.drawChar(X_input + fontwidth*symbolpos, Y_input, symbol, WHITE, NORM);
    symbolpos++;
    uView.drawChar(X_input + fontwidth*symbolpos, Y_input, ' ', BLACK, XOR);  // cursor  
    pulse = 0;  // reset pulse duration timer
  }

  // measure the time between pulse events in order to decide if the symbol is complete or not
  space = millis() - previous;

  // Can we have a complete blankout timer that resets the display also of the previous letter?
  if( space >= blanktime ) {    // trick the next conditional to update
    morseletter[0]=' ';
    morseletter[1]='\0';
    symbolpos = 1;
    letterpos = 0;
    message[0] = '\0';
    uView.clear(PAGE);
  }

  if( (space >= letterspace) && (symbolpos > 0)) {   // new letter complete
    morseletter[symbolpos] = '\0';                   // update end of string
    decoded = toupper( decode(morseletter) );
    if( (decoded != NOTFOUND) && (letterpos < MAX_MESSAGE) ) {   // add letter to message if valid
      message[letterpos] = decoded;
      message[letterpos+1] = '\0';
      letterpos++;
    }
    symbolpos = 0;
    uView.setCursor(X_decode, Y_decode); uView.print("          ");
    sprintf(displaymessage, "%-5s", morseletter);
    uView.setCursor(0, Y_decode); uView.print(decoded); 
    uView.setCursor(X_decode, Y_decode); uView.print(displaymessage); 
    uView.setCursor(X_input, Y_input); uView.print("      ");
    uView.drawChar(X_input + fontwidth*symbolpos, Y_input, ' ', BLACK, XOR);  // cursor

    uView.setCursor(0, Y_message); uView.print(message);
  }

  // ignore entire symbol if too many elements are keyed
  if (symbolpos > MAX_LETTERLENGTH) {
    symbolpos = 0;
    uView.setCursor(X_input, Y_input); uView.print("       ");
    uView.drawChar(X_input + fontwidth*symbolpos, Y_input, ' ', BLACK, XOR);  // cursor
  }

  // Check if secret message has been input
  if(strcmp(message, "SOS") == 0) {
    uView.clear(PAGE);
    uView.setCursor(0, 0); uView.print("We will\nsend help!"); // 10 char per line
    uView.display();
    delay(10000);
  }

  if(strcmp(message, "EGG") == 0) {
    uView.clear(PAGE);
    uView.setCursor(0, 0); uView.print("Send the\npassword\nto receive\neggs!");
    uView.display();
    delay(10000);
  }

  if(strcmp(message, "TRE") == 0) {
    uView.clear(PAGE);
    uView.setCursor(0, 0); uView.print("Look up in\nthe trees!"); // 10 char per line
    uView.display();
    delay(10000);
  }

  if( (strcmp(message, "LISA") == 0) || (strcmp(message, "ERIK") == 0) ) {
    uView.clear(PAGE);
    uView.setCursor(0, 0); uView.print("Happy\nEaster to\nyou "); uView.print(message);  // 10 char per line
    uView.display();
    delay(10000);
  }

  uView.display();
}
