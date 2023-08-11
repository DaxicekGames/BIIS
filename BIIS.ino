#include <stdlib.h>
#include <dht.h>
#include <stdio.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <string.h>
#include <EEPROM.h>
#include <Time.h>
#include <TimeLib.h>
#include <math.h>


const char version[15] = "10.08.2023";

// Konfigurace pinů
const int buttpin = 7;
const int buttPwr = 6;
const int buzzerpin = 48;
const int lcdpwr = 8;
const int voidpin = 2;

const int dhtPin = 13;
const int dhtPwr = 12;

// Nastavení časování (ms)
const int dotTime = 80; // Tečka (Menší než)
const int dashTime = 200; // Čárka (Menší než)
const int longPressTime = dashTime; // Potvrzení/Mezera/Dlouhý stisk (Větší než)
const int errorTime = 30; // Chybný stisk (Menší než)

// Uživatelské nastavení
int toggleCursor = 1;
int toggleBuzzer = 1;
int toggleBacklight = 1;
int help = 1;

// Nastavení RTC
int set_hour = 14;
int set_minute = 0;
int set_seconds = 0;
int set_day = 9;
int set_month = 8;
int set_year = 2023;

//EEPROM Adresy
int firstStartAdr = 1;
int toggleBuzzerAdr = 2;
int toggleCursorAdr = 3;
int helpAdr = 4;
int toggleBacklightAdr = 5;

int menu = 0;
int firstStart = 1;

int presstime = 0;

char seq[20];
char charseq[20] = "";
int printpos = 0;

char currentSeq[20] = ""; // Nový řetězec pro uchování aktuální sekvence . a -

bool cursorVisible = true;
int cursorPosition = 0; // Proměnná pro uchování pozice kurzoru

// Stopwatch 
unsigned long stopwatchstart = 0;
int stopwatchactive = 0;

// Help 
int helpPage = -1;

// Dice 
int diceRoll = 0;
int diceSpeed = 1;
int rollTime = 0;

// RNG
int RNGMax = 100;
int RNGRoll = 0;

// TIMER
unsigned long timerEnd = 0;
unsigned long timerTime = 0;
int timerActive = 0;

// PLAY
int playPich = 100;

// COM
int result = 0;

//vytvořit objekt třídy dht
dht DHT; 


// inicializace knihovny displeje 
LiquidCrystal_I2C lcd(0x27,16,2); // nastavit adresu displeje na 0x27 pro 1602 displej 

void pressnoise(int pich, int duration) {
  if (toggleBuzzer == 1) {
    for (int n = 0; n < duration; n++) {
      for (int i = 0; i < pich; i++) {
        digitalWrite(buzzerpin, HIGH);
        digitalWrite(buzzerpin, LOW);
      }
      delay(1);
    }
  }
}

void startup() {
  // Úvod
  Serial.println("\n=============\nBIIS\nby Daxicek\n=============");

  lcd.print("======BIIS======");
  lcd.setCursor(0, 1);
  lcd.print("   by Daxicek   ");
  if (toggleBuzzer == 1) {
    for (int n = 0; n < 200; n++) {
      for (int i = 0; i < 100; i++) {
        digitalWrite(buzzerpin, HIGH);
        digitalWrite(buzzerpin, LOW);
      }
      delay(1);
    }
  }
  if (toggleBuzzer == 1) {
    for (int n = 0; n < 200; n++) {
      for (int i = 0; i < 75; i++) {
        digitalWrite(buzzerpin, HIGH);
        digitalWrite(buzzerpin, LOW);
      }
      delay(1);
    }
  }
  if (toggleBuzzer == 1) {
    for (int n = 0; n < 200; n++) {
      for (int i = 0; i < 50; i++) {
        digitalWrite(buzzerpin, HIGH);
        digitalWrite(buzzerpin, LOW);
      }
      delay(1);
    }
  }
  if (toggleBuzzer == 0) delay(600);

  delay(400);
  lcd.setCursor(0, 1);
  lcd.print("    Welcome!    .");
  delay(1000);
  lcd.clear();

  // Nápověda
  if (help == 1) {
    lcd.print("Submit/space:");
    lcd.setCursor(0, 1);
    lcd.print("Long press (2x-)");
    pressnoise(80, 100);
    delay(2900);
    lcd.clear();

    lcd.print("Delete char:");
    lcd.setCursor(0, 1);
    lcd.print("----");
    pressnoise(80, 100);
    delay(2900);
    lcd.clear();

    lcd.print("All commands:");
    lcd.setCursor(0, 1);
    lcd.print("HELP;");
    pressnoise(80, 100);
    delay(2900);
    lcd.clear();

  } 
  pressnoise(80, 100);
  delay(50);
  pressnoise(80, 100);
}


void setup() {
  int firstStartRead = EEPROM.read(firstStartAdr);
  if (firstStartRead != firstStart) { // pokud je hodnota různá od 1
    EEPROM.write(firstStartAdr, firstStart); // zápis do EEPROM
    EEPROM.write(toggleBuzzerAdr, toggleBuzzer);
    EEPROM.write(toggleCursorAdr, toggleCursor);
    EEPROM.write(helpAdr, help);
    EEPROM.write(toggleBacklightAdr, toggleBacklight);
  } else {
    toggleBuzzer = EEPROM.read(toggleBuzzerAdr);
    toggleCursor = EEPROM.read(toggleCursorAdr);
    help = EEPROM.read(helpAdr);
    toggleBacklight = EEPROM.read(toggleBacklightAdr);
  }

  pinMode(dhtPwr, OUTPUT);
  digitalWrite(dhtPwr, HIGH); // zapnout dht
  pinMode(dhtPin, OUTPUT);

  pinMode(buttpin, INPUT);
  pinMode(buttPwr, OUTPUT);
  digitalWrite(buttPwr, HIGH);

  pinMode(buzzerpin, OUTPUT);

  pinMode(lcdpwr, OUTPUT);
  digitalWrite(lcdpwr, HIGH);

  pinMode(voidpin, OUTPUT);

  lcd.init();
  if (toggleBacklight == 1) lcd.backlight();

  Serial.begin(9600);
  startup();
}

// Odstranit kulzor z obrazovky
void removeCursor() {
  lcd.clear();
  for (int i = 0;i < strlen(charseq);i++) {
    lcd.print(charseq[i]);
  }
}

// Funkce pro vykreslení kurzoru na aktuální pozici
void drawCursor() {
  if (cursorVisible && toggleCursor == 1) {
    lcd.setCursor(cursorPosition, 0);
    lcd.print("_");
  } else {
    lcd.setCursor(cursorPosition, 0);
    lcd.print(" ");
  }
}


// Funkce pro přidání znaku do sekvence
void addToCurrentSeq(char *str) {
  strcat(currentSeq, str);
  lcd.setCursor(0, 1);
  lcd.print(currentSeq);
}

// Funkce pro resetování proměnné currentSeq
void resetCurrentSeq() {
  memset(currentSeq, '\0', sizeof(currentSeq));
}

// Funkce pro vymazání sekvenčního řádku
void clearSeqRow() {
  resetCurrentSeq();
  lcd.setCursor(0, 1);
  for (int i = 0; i < 16; i++) {
    lcd.print(" ");
  }
  lcd.setCursor(0, 1);

  // Aktualizovat pozici kurzoru na aktuální pozici
  cursorPosition = printpos;
  drawCursor();
}

// Funkce pro odstranění posledního znaku z řetězce
void deleteLastChar(char *str) {
  int len = strlen(str);
  if (len > 0) {
    str[len - 1] = '\0';
  }
}

void gohome() {
  menu = 0;
  memset(charseq, '\0', sizeof(charseq));
  printpos = 0;
  cursorPosition = printpos;
  lcd.clear();
  removeCursor();
}

char morseToChar(char *morse) {
    if (strcmp(morse, ".-") == 0) return 'A';
    if (strcmp(morse, "-...") == 0) return 'B';
    if (strcmp(morse, "-.-.") == 0) return 'C';
    if (strcmp(morse, "-..") == 0) return 'D';
    if (strcmp(morse, ".") == 0) return 'E';
    if (strcmp(morse, "..-.") == 0) return 'F';
    if (strcmp(morse, "--.") == 0) return 'G';
    if (strcmp(morse, "....") == 0) return 'H';
    if (strcmp(morse, "..") == 0) return 'I';
    if (strcmp(morse, ".---") == 0) return 'J';
    if (strcmp(morse, "-.-") == 0) return 'K';
    if (strcmp(morse, ".-..") == 0) return 'L';
    if (strcmp(morse, "--") == 0) return 'M';
    if (strcmp(morse, "-.") == 0) return 'N';
    if (strcmp(morse, "---") == 0) return 'O';
    if (strcmp(morse, ".--.") == 0) return 'P';
    if (strcmp(morse, "--.-") == 0) return 'Q';
    if (strcmp(morse, ".-.") == 0) return 'R';
    if (strcmp(morse, "...") == 0) return 'S';
    if (strcmp(morse, "-") == 0) return 'T';
    if (strcmp(morse, "..-") == 0) return 'U';
    if (strcmp(morse, "...-") == 0) return 'V';
    if (strcmp(morse, ".--") == 0) return 'W';
    if (strcmp(morse, "-..-") == 0) return 'X';
    if (strcmp(morse, "-.--") == 0) return 'Y';
    if (strcmp(morse, "--..") == 0) return 'Z';

    if (strcmp(morse, "-----") == 0) return '0';
    if (strcmp(morse, ".----") == 0) return '1';
    if (strcmp(morse, "..---") == 0) return '2';
    if (strcmp(morse, "...--") == 0) return '3';
    if (strcmp(morse, "....-") == 0) return '4';
    if (strcmp(morse, ".....") == 0) return '5';
    if (strcmp(morse, "-....") == 0) return '6';
    if (strcmp(morse, "--...") == 0) return '7';
    if (strcmp(morse, "---..") == 0) return '8';
    if (strcmp(morse, "----.") == 0) return '9';

    if (strcmp(morse, "--..--") == 0) return ',';
    if (strcmp(morse, ".-.-.-") == 0) return '.';
    if (strcmp(morse, "---...") == 0) return ':';
    if (strcmp(morse, "-.-.-.") == 0) return ';';
    if (strcmp(morse, "..--..") == 0) return '?';
    if (strcmp(morse, "--...-") == 0) return '!';
    if (strcmp(morse, "-....-") == 0) return '-';
    if (strcmp(morse, ".-.-.") == 0) return '+';
    if (strcmp(morse, "-..-.") == 0) return '/';
    if (strcmp(morse, "...-.-") == 0) return '*';
    if (strcmp(morse, "-...-") == 0) return '=';
    if (strcmp(morse, ".-.-") == 0) return '^';
    if (strcmp(morse, "..--.-") == 0) return '_';
    if (strcmp(morse, ".-..-.") == 0) return '"';
    if (strcmp(morse, ".--.-.") == 0) return '@';
    if (strcmp(morse, ".---.---.-") == 0) return '%';

}


void longPress() {
  Serial.println("LONG");
  if (strlen(seq) == 0) {
    removeCursor();
    strcat(charseq, " ");
    printpos += 1;
    cursorPosition = printpos;
  } else if (strcmp(seq, "----") == 0) {
    // Pokud byl zadán řetězec "----", vymazat poslední znak
    deleteLastChar(charseq);
    deleteLastChar(currentSeq); // Smazat také poslední znak z aktuální sekvence
    printpos -= 1;
    lcd.setCursor(printpos, 0);
    lcd.print(" "); // Vymazat znak na pozici kurzoru
    clearSeqRow();

    // Při smazání sekvence nastavit seq na prázdný řetězec
    seq[0] = '\0';
    cursorPosition = printpos;
    removeCursor();
  } else {
    char letter = morseToChar(seq);

    char tempStr[2];
    tempStr[0] = letter;
    tempStr[1] = '\0';

    strcat(charseq, tempStr);
    Serial.println(charseq);

    lcd.setCursor(printpos, 0);
    lcd.print(letter);

    addToCurrentSeq(tempStr); // Přidat znak do aktuální sekvence

    printpos += 1;
    cursorPosition = printpos;
    drawCursor();

    seq[0] = '\0';
    resetCurrentSeq(); // Resetovat currentSeq před každým novým zadáním
    clearSeqRow();
    if (strcmp(charseq, "TEMP;") == 0) {
      pressnoise(80, 100);
      lcd.setCursor(0, 0);
      lcd.print("Loading...");
      memset(charseq, '\0', sizeof(charseq));
      int chk = DHT.read11(dhtPin);
      int temp = DHT.temperature;
      int hum = DHT.humidity;
      delay(200);
      menu = 1;
    }
    if (strcmp(charseq, "OFF;") == 0) {
      pressnoise(80, 100);
      memset(charseq, '\0', sizeof(charseq));
      menu = 2;
    }
    if (strcmp(charseq, "SW;") == 0) {
      pressnoise(80, 100);
      memset(charseq, '\0', sizeof(charseq));
      lcd.setCursor(0, 0);
      lcd.print("Time: 00:00:00.0");
      delay(500);
      presstime = 0;
      menu = 3;
    }
    if (strcmp(charseq, "CURSOR;") == 0) {
      pressnoise(80, 100);
      memset(charseq, '\0', sizeof(charseq));
      menu = 4;
    }
    if (strcmp(charseq, "BUZZER;") == 0) {
      pressnoise(80, 100);
      memset(charseq, '\0', sizeof(charseq));
      menu = 5;
    }
    if (strcmp(charseq, "ABOUT;") == 0) {
      pressnoise(80, 100);
      memset(charseq, '\0', sizeof(charseq));
      menu = 6;
    }
    if (strcmp(charseq, "SLEEP;") == 0) {
      pressnoise(80, 100);
      memset(charseq, '\0', sizeof(charseq));
      lcd.clear();
      lcd.noBacklight();
      lcd.noDisplay();
      delay(2000);
      menu = 7;
    }
    if (strcmp(charseq, "HELP;") == 0) {
      pressnoise(80, 100);
      memset(charseq, '\0', sizeof(charseq));
      menu = 8;
    }
    if (charseq[0] == 'P' && charseq[1] == 'L' && charseq[2] == 'A' && charseq[3] == 'Y') { // PLAY
      for (int i = 0;i < 20;i++) {
        if (charseq[i] == ';') {
          char *pcomm;
          char *ppar;
          pcomm = strtok(charseq, " ");
          ppar = strtok(NULL, " ");
          for (int n = 0;n < 20;n++) {
            if (ppar[n] == ';') ppar[n] = '\0';
          }
          playPich = atoi(ppar);
          Serial.print("Pich: ");
          Serial.println(playPich);
          pressnoise(80, 100);
          presstime = 0;
          memset(charseq, '\0', sizeof(charseq));
          lcd.clear();
          lcd.print("  Press button  .");
          lcd.setCursor(0, 1);
          lcd.print(" to play buzzer .");
          delay(2000);
          pressnoise(80, 100);
          lcd.setCursor(0, 0);
          lcd.print("   Long press   .");
          lcd.setCursor(0, 1);
          lcd.print("    to exit!    .");
          delay(2000);
          lcd.clear();
          pressnoise(80, 100);
          menu = 9;
        }
      }
    }
    if (strcmp(charseq, "SHELP;") == 0) {
      pressnoise(80, 100);
      memset(charseq, '\0', sizeof(charseq));
      menu = 10;
    }
    if (strcmp(charseq, "BKLIGHT;") == 0) {
      pressnoise(80, 100);
      memset(charseq, '\0', sizeof(charseq));
      menu = 11;
    }
    if (strcmp(charseq, "RESET;") == 0) {
      pressnoise(80, 100);
      memset(charseq, '\0', sizeof(charseq));
      menu = 12;
    }
    if (strcmp(charseq, "RESTART;") == 0) {
      pressnoise(80, 100);
      memset(charseq, '\0', sizeof(charseq));
      menu = 13;
    }
    if (strcmp(charseq, "DICE;") == 0) {
      pressnoise(80, 100);
      presstime = 0;
      memset(charseq, '\0', sizeof(charseq));
      lcd.clear();
      lcd.print("Press button to .");
      lcd.setCursor(0, 1);
      lcd.print("  roll a dice!  .");
      delay(2000);
      lcd.setCursor(0, 0);
      lcd.print("  ROLL A DICE   .");
      lcd.setCursor(0, 1);
      lcd.print("      =0=       .");
      pressnoise(80, 100);
      menu = 14;
    }
    if (charseq[0] == 'R' && charseq[1] == 'N' && charseq[2] == 'G') { // RNG
      for (int i = 0;i < 20;i++) {
        if (charseq[i] == ';') {
          char *rcomm;
          char *rpar;
          rcomm = strtok(charseq, " ");
          rpar = strtok(NULL, " ");
          for (int n = 0;n < 20;n++) {
            if (rpar[n] == ';') rpar[n] = '\0';
          }
          RNGMax = atoi(rpar);
          Serial.print("RNG Max: ");
          Serial.println(RNGMax);
          pressnoise(80, 100);
          presstime = 0;
          memset(charseq, '\0', sizeof(charseq));
          lcd.clear();
          lcd.print("Press button to .");
          lcd.setCursor(0, 1);
          lcd.print("  generate num  .");
          delay(2000);
          lcd.clear();
          lcd.print("RNG-MAX: ");
          lcd.print(RNGMax);
          lcd.setCursor(0, 1);
          lcd.print("0");
          pressnoise(80, 100);
          menu = 15;
        }
      }
    }
    if (charseq[0] == 'S' && charseq[1] == 'T' && charseq[2] == 'I' && charseq[3] == 'M' && charseq[4] == 'E' && charseq[5] == 'R' ) { // STIMER
      for (int i = 0;i < 20;i++) {
        if (charseq[i] == ';') {
          char *stcomm;
          char *stpar;
          stcomm = strtok(charseq, " ");
          stpar = strtok(NULL, " ");
          for (int n = 0;n < 20;n++) {
            if (stpar[n] == ';') stpar[n] = '\0';
          }
          timerTime = atoi(stpar) * 1000;
          Serial.print("TIMER: ");
          Serial.println(timerTime);
          pressnoise(80, 100);
          memset(charseq, '\0', sizeof(charseq));
          presstime = 0;
          lcd.clear();
          lcd.print("Time: ");
          lcd.print(millisToTimeString(timerTime));
          pressnoise(80, 100);
          menu = 16;
        }
      }
    }
    if (charseq[0] == 'M' && charseq[1] == 'T' && charseq[2] == 'I' && charseq[3] == 'M' && charseq[4] == 'E' && charseq[5] == 'R' ) { // MTIMER
      for (int i = 0;i < 20;i++) {
        if (charseq[i] == ';') {
          char *mtcomm;
          char *mtpar;
          mtcomm = strtok(charseq, " ");
          mtpar = strtok(NULL, " ");
          for (int n = 0;n < 20;n++) {
            if (mtpar[n] == ';') mtpar[n] = '\0';
          }
          timerTime = atoi(mtpar) * 60000;
          Serial.print("TIMER: ");
          Serial.println(timerTime);
          pressnoise(80, 100);
          memset(charseq, '\0', sizeof(charseq));
          presstime = 0;
          lcd.clear();
          lcd.print("Time: ");
          lcd.print(millisToTimeString(timerTime));
          pressnoise(80, 100);
          menu = 16;
        }
      }
    }
    if (charseq[0] == 'C' && charseq[1] == 'O' && charseq[2] == 'M') { // COM
      for (int i = 0;i < 20;i++) {
        if (charseq[i] == ';') {
          char *Ccomm;
          char *Cnum1;
          char *Copearation;
          char *Cnum2;

          Ccomm = strtok(charseq, " ");
          Cnum1 = strtok(NULL, " ");
          Copearation = strtok(NULL, " ");
          Cnum2 = strtok(NULL, " ");

          for (int n = 0;n < 20;n++) {
            if (Cnum2[n] == ';') Cnum2[n] = '\0';
          }
          pressnoise(80, 100); 
          presstime = 0;
          
          if (Copearation[0] == '+') result = atoi(Cnum1) + atoi(Cnum2);
          if (Copearation[0] == '-') result = atoi(Cnum1) - atoi(Cnum2);
          if (Copearation[0] == '*' || Copearation[0] == '.') result = atoi(Cnum1) * atoi(Cnum2);
          if (Copearation[0] == '/' || Copearation[0] == ':') result = atoi(Cnum1) / atoi(Cnum2);
          if (Copearation[0] == '%') result = atoi(Cnum1) % atoi(Cnum2);
          if (Copearation[0] == 'V') result = pow(atoi(Cnum2), 1/atoi(Cnum1));
          if (Copearation[0] == '^') result = pow(atoi(Cnum1), atoi(Cnum2));

          sprintf(charseq, "COM %d", result);
          printpos = 4 + sizeof(result);
          cursorPosition = printpos;
          Serial.print("Printpos: ");
          Serial.println(printpos);
          removeCursor();
        }
      }
    }
  }
}

void tempmenu1() {
  int chk = DHT.read11(dhtPin);
  int temp = DHT.temperature;
  int hum = DHT.humidity;

  if (hum > 100 || hum < 0) {
    lcd.setCursor(0, 0);
    lcd.print("     ERROR!     ");
    lcd.setCursor(0, 1);
    lcd.print(" DHT not found! ");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temp); // zobrazit teplotu na displeji
    lcd.print(char(223)); // zobrazit znak "°"
    lcd.print("C");

    if (temp > 17) {
      if (temp < 26) {
            lcd.print("  GOOD  .");
      } else {
        lcd.print("  BAD  .");
      }
    } else {
      lcd.print("  BAD  .");
    }


    lcd.setCursor(0, 1);
    lcd.print("Hum : ");
    lcd.print(hum); //zobrazit vlhkost na displeji
    lcd.print(" %"); 
    if (hum > 40) {
      if (hum < 85) {
            lcd.print("  GOOD  .");
      } else {
        lcd.print("  BAD  .");
      }
    } else {
      lcd.print("  BAD  .");
    }


    Serial.println(" ");
    Serial.print("Temp: ");
    Serial.println(temp);
    Serial.print("Hum : ");
    Serial.println(hum);
  }
}

// Funkce pro převod milisekund na textový řetězec ve formátu "HH:MM:SS"
unsigned long seconds;
unsigned long minutes;
unsigned long hours;
unsigned long miliseconds;
String millisToTimeString(unsigned long milli) {
  seconds = milli / 1000;
  minutes = seconds / 60;
  hours = minutes / 60;
  miliseconds = milli % 1000;

  // Vypočítání zbývajících minut a sekund po odečtení hodin
  minutes %= 60;
  seconds %= 60;

  
  char buffer[14];
  sprintf(buffer, "%02lu:%02lu:%02lu.%02lu", hours, minutes, seconds, miliseconds);
  return String(buffer);
}

// přidaní "0" u čísel do 10
void print2digits(int number ) { 
  if (number >= 0 && number < 10) {
    lcd.write('0');
  }
    lcd.print(number);
}


void helpmenu(int page) {
      if (page == 0) {
      lcd.setCursor(0, 0);
      lcd.print("   Help menu:   .");
      lcd.setCursor(0, 1);
      lcd.print("  Press button  .");
    }
    if (page == 1) {
      lcd.setCursor(0, 0);
      lcd.print("TEMP;       1/18.");
      lcd.setCursor(0, 1);
      lcd.print("Show temp & hum .");
    }
    if (page == 2) {
      lcd.setCursor(0, 0);
      lcd.print("OFF;        2/18.");
      lcd.setCursor(0, 1);
      lcd.print("Shutdown BIIS   .");
    }
    if (page == 3) {
      lcd.setCursor(0, 0);
      lcd.print("SW;         3/18.");
      lcd.setCursor(0, 1);
      lcd.print("Stopwatch       .");
    }
    if (page == 4) {
      lcd.setCursor(0, 0);
      lcd.print("CURSOR;     4/18.");
      lcd.setCursor(0, 1);
      lcd.print("Toggle cursor   .");
    }
    if (page == 5) {
      lcd.setCursor(0, 0);
      lcd.print("BUZZER;     5/18.");
      lcd.setCursor(0, 1);
      lcd.print("Toggle buzzer   .");
    }
    if (page == 6) {
      lcd.setCursor(0, 0);
      lcd.print("ABOUT;      6/18.");
      lcd.setCursor(0, 1);
      lcd.print("Show version    .");
    }
    if (page == 7) {
      lcd.setCursor(0, 0);
      lcd.print("SLEEP;      7/18.");
      lcd.setCursor(0, 1);
      lcd.print("Turn off lcd    .");
    }
    if (page == 8) {
      lcd.setCursor(0, 0);
      lcd.print("HELP;       8/18.");
      lcd.setCursor(0, 1);
      lcd.print("Show commands   .");
    }
    if (page == 9) {
      lcd.setCursor(0, 0);
      lcd.print("PLAY pich;  9/18.");
      lcd.setCursor(0, 1);
      lcd.print("Play with buzzer.");
    }
    if (page == 10) {
      lcd.setCursor(0, 0);
      lcd.print("SHELP;     10/18.");
      lcd.setCursor(0, 1);
      lcd.print("Togg start help .");
    }
    if (page == 11) {
      lcd.setCursor(0, 0);
      lcd.print("BKLIGHT;   11/18.");
      lcd.setCursor(0, 1);
      lcd.print("Togg backlight  .");
    }
    if (page == 12) {
      lcd.setCursor(0, 0);
      lcd.print("RESET;     12/18.");
      lcd.setCursor(0, 1);
      lcd.print("Reset memory    .");
    }
    if (page == 13) {
      lcd.setCursor(0, 0);
      lcd.print("RESTART;   13/18.");
      lcd.setCursor(0, 1);
      lcd.print("Restart BIIS    .");
    }
    if (page == 14) {
      lcd.setCursor(0, 0);
      lcd.print("DICE;      14/18.");
      lcd.setCursor(0, 1);
      lcd.print("Roll simple dice.");
    }
    if (page == 15) {
      lcd.setCursor(0, 0);
      lcd.print("RNG  max;  15/18.");
      lcd.setCursor(0, 1);
      lcd.print("Gen random num  .");
    }
    if (page == 16) {
      lcd.setCursor(0, 0);
      lcd.print("STIMER s;  16/18.");
      lcd.setCursor(0, 1);
      lcd.print("Timer (sec)     .");
    }
    if (page == 17) {
      lcd.setCursor(0, 0);
      lcd.print("MTIMER m;  17/18.");
      lcd.setCursor(0, 1);
      lcd.print("Timer (min)     .");
    }
    if (page == 18) {
      lcd.setCursor(0, 0);
      lcd.print("TIME;      18/18.");
      lcd.setCursor(0, 1);
      lcd.print("Show date & time.");
    }
    if (page > 18) {
      helpPage = -1;
      gohome();
    }
}

void loop() {
  if (menu == 0) {
    unsigned long currentTime = millis();
    static unsigned long prevCursorTime = currentTime;
    const unsigned long cursorInterval = 500; // Interval blikání kurzoru (v ms)

    if (currentTime - prevCursorTime >= cursorInterval) {
      prevCursorTime = currentTime;
      cursorVisible = !cursorVisible; // Invertovat stav kurzoru
      drawCursor(); // Vykreslit kurzor na aktuální pozici
    }

    if (digitalRead(buttpin) == HIGH) {
      presstime += 1;
      if (presstime < 200) {
        if (toggleBuzzer == 1) {
          for (int i = 0; i < 50; i++) {
            digitalWrite(buzzerpin, HIGH);
            digitalWrite(buzzerpin, LOW);
          }
        } else {
          for (int i = 0; i < 50; i++) {
            digitalWrite(voidpin, HIGH);
            digitalWrite(voidpin, LOW);
          }
        }
      }
    } else {
      if (presstime > errorTime) {
        Serial.print("Presstime: ");
        Serial.println(presstime);
        if (presstime < dotTime) {
          Serial.println(".");
          strcat(seq, ".");
          addToCurrentSeq(".");
        } else if (presstime < dashTime) {
            Serial.println("-");
            strcat(seq, "-");
            addToCurrentSeq("-");
          }
        } else if (presstime > 1) {
          Serial.println("Error, short press");
          Serial.print("Presstime: ");
          Serial.println(presstime);
        }
        presstime = 0;
      } 

    if (presstime == longPressTime) longPress();
  } else if (menu == 1) { //MENU 1 TEPLOTA - LOOP!
    tempmenu1();
    delay(199);
    if (digitalRead(buttpin) == HIGH) {
      gohome();
    }
  } else if (menu == 2) { //MENU 2 OFF - LOOP!
    lcd.clear();
    lcd.print(" Shutdown in 5s ");
    delay(1000);
    for (int i = 0;i <= 4001;i++) {
      if (digitalRead(buttpin) == HIGH) {
        gohome();
        break;
      }
      if (i == 4001) {
        digitalWrite(lcdpwr, LOW);
        digitalWrite(dhtPwr, LOW);
        if (toggleBuzzer == 1) {
          for (int n = 0; n < 100; n++) {
            for (int i = 0; i < 50; i++) {
              digitalWrite(buzzerpin, HIGH);
              digitalWrite(buzzerpin, LOW);
            }
          delay(1);
          }
        }
        exit(0);

      }
      delay(1);
    }
  } else if (menu == 3) { //MENU 3 STOPWATCH - LOOP!
    if (digitalRead(buttpin) == HIGH) {
      presstime++;
    } else {
      if (presstime > errorTime) {
        pressnoise(80, 100);
        if (presstime < 1000) {
          stopwatchactive = !stopwatchactive;
          if (stopwatchactive == 1) stopwatchstart = millis();
        } else {
          stopwatchstart = 0;
          stopwatchactive = 0;
          presstime = 0;
          gohome();
        }
        presstime = 0;
      }
    }
    if (millis()%100 == 0 && stopwatchactive == 1) {
      lcd.setCursor(6, 0);
      lcd.print(millisToTimeString(millis() - stopwatchstart));
    }
  } else if (menu == 4) { //MENU 4 TOGGLE CURSOR
    toggleCursor = !toggleCursor;
    EEPROM.write(toggleCursorAdr, toggleCursor);
    removeCursor();
    gohome();
  } else if (menu == 5) { //MENU 5 TOGGLE BUZZER
    toggleBuzzer = !toggleBuzzer;
    EEPROM.write(toggleBuzzerAdr, toggleBuzzer);
    gohome();
  } else if (menu == 6) { //MENU 6 ABOUT - LOOP!
    lcd.setCursor(0, 0);
    lcd.print("BIIS  by Daxicek .");
    lcd.setCursor(0, 1);
    lcd.print("Ver:  ");
    for (int i = 0;i < strlen(version);i++) {
      lcd.print(version[i]);
    }
    lcd.print("       .");
    if (digitalRead(buttpin) == HIGH) gohome();
  } else if (menu == 7) { //MENU 7 SLEEP - LOOP!
    if (digitalRead(buttpin) == HIGH) {
      if (toggleBacklight == 1) lcd.backlight();
      lcd.display();
      startup();
      gohome();
    }
  } else if (menu == 8) { //MENU 8 HELP - LOOP!
    if (digitalRead(buttpin) == HIGH) {
      presstime++;
    } else if (presstime > errorTime) {
      Serial.print("Presstime: ");
      Serial.println(presstime);
      helpPage++;
      presstime = 0;
      helpmenu(helpPage);
      pressnoise(80, 100);
    }
  } else if (menu == 9) { //MENU 9 PLAY - LOOP!
    if (digitalRead(buttpin) == HIGH) {
      presstime++;
      for (int i = 0; i < playPich; i++) {
        digitalWrite(buzzerpin, HIGH);
        digitalWrite(buzzerpin, LOW);
      }
    } else if (presstime > 0 && presstime < 4999) {
      lcd.setCursor(0, 0);
      lcd.print("Presstime: ");
      lcd.print(presstime);
      Serial.print("Presstime:");
      Serial.println(presstime);
      presstime = 0;
    }
    if (presstime == 2000) {
      delay(1000);
      presstime = 0;
      gohome();
    }
  } else if (menu == 10) { //MENU 10 TOGGLE START HELP
    help = !help;
    EEPROM.write(helpAdr, help);
    gohome();
  } else if (menu == 11) { //MENU 11 TOGGLE BACKLIGHT
    toggleBacklight = !toggleBacklight;
    EEPROM.write(toggleBacklightAdr, toggleBacklight);
    if (toggleBacklight == 1) lcd.backlight();
    if (toggleBacklight == 0) lcd.noBacklight();
    gohome();
  } else if (menu == 12) { //MENU 12 RESET
    lcd.clear();
    lcd.noBacklight();
    lcd.noDisplay();
    toggleBacklight = 1;
    toggleBuzzer = 1;
    toggleCursor = 1;
    firstStart = 1;
    help = 1;
    EEPROM.write(toggleBacklightAdr, toggleBacklight);
    EEPROM.write(toggleBuzzerAdr, toggleBuzzer);
    EEPROM.write(toggleCursorAdr, toggleCursor);
    EEPROM.write(firstStartAdr, firstStart);
    EEPROM.write(helpAdr, help);
    delay(1000);
    lcd.display();
    lcd.backlight();
    startup();
    gohome();
  } else if (menu == 13) { //MENU 13 RESTART
    lcd.clear();
    lcd.noBacklight();
    lcd.noDisplay();
    delay(1000);
    lcd.display();
    lcd.backlight();
    startup();
    gohome();
  } else if (menu == 14) { //MENU 14 DICE - LOOP!
    srand(millis());
    rollTime = rand() % 50;
    if (rollTime < 30) rollTime += 35;
    if (digitalRead(buttpin) == HIGH) {
      presstime++; 
    } else {
      if (presstime > errorTime) {
        if (presstime < 1000) {
          diceSpeed = 1;
          for (int i = 0;i < rollTime;i++) {
            if (diceRoll < 6) {
              diceRoll++;
            } else {
              diceRoll = 1;
            }
            lcd.setCursor(7, 1);
            pressnoise(20, 20);
            lcd.print(diceRoll);
            if (diceSpeed > 10) delay(diceSpeed-10);
            diceSpeed += rollTime/20;
          }
          Serial.print("Presstime: ");
          Serial.println(presstime);
          presstime = 0;
        } else {
          Serial.print("Presstime: ");
          Serial.println(presstime);
          presstime = 0;
          diceRoll = 0;
          gohome();
        }
      } 
    }
  } else if (menu == 15) { //MENU 15 RNG - LOOP!
    srand(millis());
    if (digitalRead(buttpin) == HIGH) {
      presstime++; 
    } else {
      if (presstime > errorTime) {
        pressnoise(80, 100);
        if (presstime < 1000) {
          RNGRoll = rand() % RNGMax;
          Serial.print("Presstime: ");
          Serial.println(presstime);
          lcd.setCursor(0, 1);
          lcd.print(RNGRoll);
        } else {
          Serial.print("Presstime: ");
          Serial.println(presstime);
          presstime = 0;
          gohome();
        }
      } 
      presstime = 0;
    }
  } else if (menu == 16) { //MENU 16 & 17 TIMER - LOOP!
    if (digitalRead(buttpin) == HIGH) {
      presstime++;
    } else {
      if (presstime > errorTime) {
        if (presstime < 1000) {
          timerActive = !timerActive;
          if (timerActive == 1) timerEnd = millis() + timerTime;
        } else {
          timerEnd = 0;
          timerActive = 0;
          presstime = 0;
          gohome();
        }
        presstime = 0;
      }
    }
    if (timerActive == 1) {
      lcd.setCursor(6, 0);
      lcd.print(millisToTimeString(timerEnd - millis()));
      Serial.print("Time left:");
      Serial.println(timerEnd - millis());
      if (millis() >= timerEnd) {
        timerActive = 0;
        lcd.print("Time:            .");
        for (int i = 0;i < 3;i++) {
          lcd.setCursor(0, 0);
          lcd.print("Time:           .");
          pressnoise(100, 500);
          lcd.setCursor(0, 0);
          lcd.print("Time: 00:00:00.0");
          delay(500);
        }
      }
    } 
  } else if (menu == 18) {  // MENU 18 COM 
  }

  delay(1);
}
