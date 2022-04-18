/*
 * MSP430 G2231
 * Morse code speaking implant for easter chicken
 * 
 * Output morse code message on digital pin
 * Trigger on LDR connected to ADC
 *
 * Fredrik Hansteen
 * 2022.04.10 - First version for the Easter egg hunt
 * 
 */

#include <msp430.h>
#include <string.h>

//Define pins
#define PWMBEEP BIT4   // SH_CP -> 1.4
#define FLAG BIT6      //  1.6

#define BASE_LETTER 'A'
#define BASE_DIGIT '0'

#define DOT 250
#define DASH 750

// Declare functions
void pinWrite ( unsigned int, unsigned char );
void delay(unsigned int ms);
void tweet(char *message);
void tweetletter(char c);
char* morse_encode(char c);


static char* dictionary[] = {
".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", // A-I
".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", // J-R
"...", "-", "..-", "...-", ".--", "-..-", "-.--", "--.." // S-Z
};

static char* digits[] = {
"-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----." // 0-9
};


int main( void )
{
    unsigned int n;

    // Stop watchdog timer to prevent time out reset
    WDTCTL = WDTPW + WDTHOLD;
    P1DIR |= (PWMBEEP + FLAG);  // Setup pins as outputs


    while(1) {
        // SEE PDF FOR INFO
        // Set conversion to single channel and continuous-sampling mode
        ADC10CTL1 |= CONSEQ1;
        //Set S/H time, 10-bit converter, and continuous sampling:
        ADC10CTL0 |= ADC10SHT_3 + ADC10ON + MSC;
        //Choose P1.0 (channel 1) as an analog input pin:
        ADC10AE0 |= 1;
        // Start A/D conversion; The result will appear in the memory variable ADC10MEM
        ADC10CTL0 |= ADC10SC + ENC;
        // SET VARIABLE n TO THE RESULT OF THE A/D CONVERSION:
        n = ADC10MEM;

    	// Beep the Morse code message
        if(n<90)
            tweet("TRE");
        delay(10*DASH);		// take a break before going again
    }
}

void tweet(char *message)
{
    int i;
    for(i=0; i<strlen(message); i++)
        tweetletter(message[i]);
}

/***********************************************************/
// Output the Morse representation of character c
// on digital pins PWMBEEP and FLAG
/***********************************************************/
void tweetletter(char c)
{
    char* morse;
    int duration;
    int i, p;

    // simultaneously output a dot/dash sequence on FLAG pin
    // and a pwm audio signal on PWMBEEP pin
    // Two loops inner one is pwm, outer is flag
    // <150ms is dot (aim for 120), >is dash aim for 360
    morse = morse_encode(c);
    for(i=0; i<strlen(morse); i++){
        pinWrite(FLAG, 1);
        if(morse[i]=='.')
            duration = DOT;
        else
            duration = DASH;
        for(p=0; p<duration; p++) {
            pinWrite(PWMBEEP, 1);
            delay(1);
            pinWrite(PWMBEEP, 0);
            delay(1);
        }
        pinWrite(FLAG, 0);
        delay(DOT);
    }
    delay(5*DASH);
}


/***********************************************************/
// Set specified pin to 0 or 1
/***********************************************************/
void pinWrite( unsigned int bit, unsigned char val )
{
  if (val){
    P1OUT |= bit;
  } else {
    P1OUT &= ~bit;
  }
}

/***********************************************************/
// Delays by the specified Milliseconds
// http://www.threadabort.com/archive/2010/09/05/msp430-delay-function-like-the-arduino.aspx
/***********************************************************/
void delay(unsigned int ms)
{
  while (ms--) {
           __delay_cycles(500); // set for 16Mhz change it to 1000 for 1 Mhz
  }
}

/***********************************************************/
// Return Morse code representation of input character 
// as a string
/***********************************************************/
char* morse_encode(char c)
{
    if((c>=BASE_LETTER) && (c<=BASE_LETTER+26) )
        return(dictionary[c-BASE_LETTER]);
    else  //if((c>=BASE_DIGIT) && (c<=BASE_DIGIT+10) )
        return(digits[c-BASE_DIGIT]);
}

