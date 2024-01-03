#include <LiquidCrystal_I2C.h> // Library for LCD
#include <EEPROM.h> // Use for persisting the data
#include <SPI.h>
#include <RFID.h>
// D10:pin of card reader SDA. D9:pin of card reader RST.
RFID rfid(10, 9);
unsigned char status;
unsigned char str[MAX_LEN]; //MAX_LEN is 16: size of the array.

const int CARDS_TO_REMEMBER = 10;
const int TEXT_DURATION = 2000; 
// How long to display the text before turning off the screen.
const int OPEN_DURATION = 350;
const int TEXT_TICKS = 20; // Speed of the loop.
const int MAX_TICKS = 10000; // For Integer to not cause overflow.
const int rotationCycle = 128;

int openCoverButtonPin = 8;
int addCardButtonPin = 7;
int clearCardsButtonPin = 6;
bool registerCard = false;
unsigned char recognisedCards[CARDS_TO_REMEMBER][MAX_LEN];
int cardCount = 0;
// Wiring: SDA pin is connected to A4 and SCL pin to A5.
// Connect to LCD via I2C, default address of the display 0x27 (A0-A2 not jumpered).
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);
bool coverOpen = false;
int tick = 0;
bool keepOpen = false;
int closeAt = -1;
int turnOffDisplayAt = -1;
char * firstLineText = "";
char * secondLineText = "";
int firstLinePosition = 0;
int secondLinePosition = 0;
bool displayNow = false;
int firstLineSize;
int secondLineSize;

int outPorts[] = {5, 4, 3, 2};

void setup() {
   Serial.begin(9600);
   SPI.begin();
   rfid.init(); // Initialization.
   pinMode(openCoverButtonPin, INPUT);
   pinMode(addCardButtonPin, INPUT);
   pinMode(clearCardsButtonPin, INPUT);
   Serial.println("Please put the card to the induction area...");
   readPersistedCards();
   lcd.init(); // Initialize the lcd.
   lcd.backlight(); // Turn on backlight
   displayFirstLine("Put the card to the scanner!"); 
   // Print a message to the LCD

   for (int i = 0; i < 4; i++) {
      	    pinMode(outPorts[i], OUTPUT);
   }

}

bool stringEquals(char * first, char * second) {
  	int firstLen = strlen(first);
 	int secondLen = strlen(second);
  	if (firstLen != secondLen) {
    		return false;
  	}
  	for (int i = 0; i < secondLen; i++) {
    		if (first[i] != second[i]) {
      			return false;
    		}
  	}
  	return true;
}


void displayFirstLine(char * text) {
	  // Don't display the same text multiple times.
  	  if (tick < turnOffDisplayAt && stringEquals(text, firstLineText)) {
    		return;
  	  }
  firstLineText = text;
  firstLineSize = strlen(firstLineText);
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("                "); // For resetting display to blank.
  turnOffDisplayAt = (tick + TEXT_DURATION) % MAX_TICKS;
  firstLinePosition = 0;
  displayNow = true;
  displayText();
}

void displaySecondLine(char * text) {
	  // Don't display the same text multiple times.
  	  if (tick < turnOffDisplayAt && stringEquals(text, secondLineText)) {
    		return;
  	  }
  secondLineText = text;
  secondLineSize = strlen(secondLineText);
  lcd.backlight();
  lcd.setCursor(0,1);
  lcd.print("                ");
  turnOffDisplayAt = (tick + TEXT_DURATION) % MAX_TICKS;
  secondLinePosition = 0;
  displayNow = true;
  displayText();
}

void substring(char string[], int from, int to, char text[]) {
  // Extract substring from string and put into text.
  for (int i = 0; i <= to - from; i++) {
    	      text[i] = string[from + i];
  }
}

void persistCards() {
  int address = 0; // Where to store data.
  // Store the number of known cards.
  // Integer is 2 bytes, so store 2 parts separately.
  byte byte1 = cardCount >> 8;
  byte byte2 = cardCount & 0xFF;
  EEPROM.write(address++, byte1);
  EEPROM.write(address++, byte2);
  // Store the card details.
  for (int i = 0; i < cardCount; i++) {
    for (int j =  0; j < MAX_LEN; j++) {
        	       EEPROM.write(address++, recognisedCards[i][j]);
    }
  }
}

void readPersistedCards() {
  int address = 0; // Where to get data.
  // Read cardCount, which is an int consisting of 2 bytes.
  byte byte1 = EEPROM.read(address++);
  byte byte2 = EEPROM.read(address++);
  cardCount = (byte1 << 8) + byte2;
  // Read the card details.
  for (int i = 0; i < cardCount; i++) {
    for (int j =  0; j < MAX_LEN; j++) {
    	      recognisedCards[i][j] = EEPROM.read(address++);
    }
  }
}

void emptyRecognisedCards() {
  	cardCount = 0;
	persistCards();
}

void addRecognisedCard(unsigned char * card) {
  for (int i = 0; i < MAX_LEN; i++) {
    	    recognisedCards[cardCount][i] = card[i];
  }
   cardCount++;
   persistCards();
}

int isRecognisedCard(unsigned char * card) {
  for (int i = 0; i < cardCount; i++) {
    bool found = true;
    for (int j = 0; j < MAX_LEN; j++) {
        if (recognisedCards[i][j] != card[j]) {
          found = false;
        }
   }
   if (found) {
    	     return i;
    }
  }
  return -1;
}

void loop() {
  displayText();

  if (digitalRead(openCoverButtonPin) == HIGH) { 
    // If the button is pressed.
    if (coverOpen == false){
      Serial.println("Cover Opened");
      displayFirstLine("Cover Opened");
      coverOpen = true;
      // Open cover.
      // Rotate a full turn.
      moveSteps(true, 32 * rotationCycle, 2);
    } else {
      Serial.println("Cover Closed");
      displayFirstLine("Cover Closed");
      coverOpen = false;
      // Close cover.
      // Rotate a full turn towards another direction.
      moveSteps(false, 32 * rotationCycle, 2);
    }
  }
  if (digitalRead(clearCardsButtonPin) == HIGH) { 
    // If the button is pressed.
    displayFirstLine("Cleared registered cards");
    emptyRecognisedCards();
  }
  if (!registerCard) {
    if (digitalRead(addCardButtonPin) == HIGH) { 
    	       // If the button is pressed.
     	       displayFirstLine("Register card:");
    	       registerCard = true;
    }
  }
  // Search card, return card types.
  if (rfid.findCard(PICC_REQIDL, str) == MI_OK) {
      displayFirstLine("Find the card!");
      // Show card type.
      // ShowCardType(str).
      // Anti-collision detection, read card serial number.
      if (rfid.anticoll(str) == MI_OK) {
          Serial.print("The card's number is : ");
          //Display card serial number
          for (int i = 0; i < 4; i++) {
               Serial.print(0x0F & (str[i] >> 4), HEX);
               Serial.print(0x0F & str[i], HEX);
          }
          Serial.println("");
    	      }
      // Card selection (lock card to prevent redundant read, 
      // removing the line will make the sketch read cards continually).
      // rfid.selectTag(str);
      if (registerCard) {
        Serial.println("Card registered");
        addRecognisedCard(str);
        registerCard = false;
      }
      int cardNumber = isRecognisedCard(str);
      if (cardNumber != -1) {
        Serial.print("Recognised card ");
        Serial.println(cardNumber);
        Serial.println(tick);
        displayFirstLine("Recognised");
        closeAt = (tick + OPEN_DURATION) % MAX_TICKS;
        if (keepOpen == false) {
          // Rotate a full turn.
          moveSteps(true, 32 * rotationCycle, 2);
          closeAt == 0;
       }
       keepOpen = true;
      } else {
        Serial.println("Unknown card");
        displayFirstLine("Unknown");
      }
   }
   rfid.halt(); // Command the card to enter a sleeping state.
  
   if (tick > closeAt && closeAt != -1) {
      // Rotate a full turn towards another direction.
      moveSteps(false, 32 * rotationCycle, 2);
      keepOpen = false;
      closeAt = -1;
  }
}

void moveSteps(bool dir, int steps, byte ms) {
 for (int i = 0; i < steps; i++) {
      moveOneStep(dir); // Rotate a step.
      delay(ms); // Control the speed.
 }
}
void moveOneStep(bool dir) {
 // Define a variable, use four low bits to indicate the state of the port.
 static byte out = 0x01;
 // Decide the shift direction according to the rotation direction.
 if (dir) { // Ring shift left.
 	     out != 0x08 ? out = out << 1 : out = 0x01;
 } else { // Ring shift right.
 	     out != 0x01 ? out = out >> 1 : out = 0x08;
 }
 // Output signal to each port.
 for (int i = 0; i < 4; i++) {
  	      digitalWrite(outPorts[i], (out & (0x01 << i)) ? HIGH : LOW);
 }
}

void displayText() {
  // To avoid overflow.
  if (tick++ > MAX_TICKS) {
    	     tick = 0;
  }
  if (displayNow || tick % TEXT_TICKS == 0) {
    displayNow = false;
    // turns off display.
    if (turnOffDisplayAt != -1 && turnOffDisplayAt < tick) {
         firstLineText = "";
         secondLineText = "";
         turnOffDisplayAt = -1;
         lcd.clear();
         lcd.noBacklight();
    }
    if (firstLineSize > -0) {
      lcd.clear();
      if (firstLineText != "") {
          lcd.setCursor(0,0);
          if (firstLineSize <= 16) {
          lcd.print(firstLineText);
          firstLineSize = 0;
          } else {
          // How much to display the text.
          int to = firstLinePosition;
          int from = to - 16;
          if (from < 0) {
            from = 0;
            lcd.setCursor(16 - to, 0);
          }
          char text[to - from];
          // Taking the substring from the full text and placing 
          // it into the text to display on the LCD display.
          substring(firstLineText, from, to, text); 
          lcd.print(text);
          if (firstLinePosition++ > firstLineSize) {
          	 	firstLinePosition = 0;
         }
        }
      }
      if (secondLineSize > 0) {
        lcd.setCursor(0,1);
        if (secondLineSize <= 16) {
          lcd.print(secondLineText);
          secondLineSize = 0;
        } else {
          int to = secondLinePosition;
          int from = to - 16;
          if (from < 0) {
            from = 0;
            lcd.setCursor(16 - to, 1);
          }
          char text[to - from];
          substring(firstLineText, from, to, text);
          lcd.print(text);
          if (secondLinePosition++ > secondLineSize){
           	 secondLinePosition = 0;
          }
        }
      }     
     } 
   }
}

void ShowCardType(unsigned char * type) {
 Serial.print("Card type: ");
 if (type[0] == 0x04 && type[1] == 0x00) {
 	     Serial.println("MFOne-S50");
 } else if { (type[0] == 0x02 && type[1] == 0x00) {
 	     Serial.println("MFOne-S70");
 } else if { (type[0] == 0x44 && type[1] == 0x00) {
 	     Serial.println("MF-UltraLight");
 } else if { (type[0] == 0x08 && type[1] == 0x00) {
     Serial.println("MF-Pro");
 } else if { (type[0] == 0x44 && type[1] == 0x03) {
  	     Serial.println("MF Desire");
 } else { 
 	     Serial.println("Unknown");
 }
}
