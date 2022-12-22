#include <LiquidCrystal_PCF8574.h>
#include <Wire.h>

uint8_t START_STOPP_PIN = 12;  // Pins deklarieren u d initialisieren
uint8_t ZEIT_RESET_PIN = 13;

int stateStartStopp; // Status fuer Start-Stopp-Button
int stateZeitReset; // Status fuer Zeit-Reset-Button
int letzterStateStartStopp; // letzter Status vom Start-Stopp-Button
int letzterStateZeitReset; // letzter Status vom Zeit-Reset-Button
int stateProgramm = 0;  // Status fuer Programmablauf

bool anzeigeZwei = 0; // Ob 1. oder 2. Reihe des Displays angezeigt werden soll, funktioniert nicht so gut ;(

unsigned long startZeit = 0; // Zeiten
long zwischenZeit = 0;
long zeitAngehalten = 0;

LiquidCrystal_PCF8574 lcd(0x27); // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;
  Wire.beginTransmission(0x27);
  lcd.begin(16, 2);

  pinMode(START_STOPP_PIN, INPUT_PULLUP); // Start - Stopp
  pinMode(ZEIT_RESET_PIN, INPUT_PULLUP); // Zwischenzeit - Reset
}

// Funktion zur Anzeige der Zeiten
void zeigeZeit(long Zeit, bool woAnzeigen) {

  unsigned long anzeigeZeit = Zeit - startZeit;
  int zehntelsek = (anzeigeZeit / 100) % 10;
  int sekunden = (anzeigeZeit / 1000) % 60;
  int minuten = ((anzeigeZeit / 1000) / 60) % 60;

  lcd.setBacklight(16);
  lcd.home();
  lcd.clear();
  if (woAnzeigen) {
    lcd.setCursor(0, 1);
  } else {
    lcd.setCursor(0, 0);
  }
  if (minuten < 10 && minuten < 1) {
    lcd.print("00");
  } else if (minuten < 10) {
    lcd.print("0");
    lcd.print(minuten);
  } else {
    lcd.print(minuten);
  }
  lcd.print(":");
  if (sekunden < 10) {
    lcd.print("0");
    lcd.print(sekunden);
  } else {
    lcd.print(sekunden);
  }
  lcd.print(":");
  lcd.print(zehntelsek);
}

void loop()
{
  delay(10); // Prellen abfangen
  stateStartStopp = digitalRead(START_STOPP_PIN); // aktuellen Status der Buttons festhalten
  stateZeitReset = digitalRead(ZEIT_RESET_PIN);

  switch (stateProgramm) {
    case 0: // gestoppt
      startZeit = 0;
      // start
      if ((stateStartStopp == LOW) && (stateStartStopp != letzterStateStartStopp)) {
        startZeit = millis();
        stateProgramm = 1;
      }
      break;
    case 1: // Uhr laeuft
      anzeigeZwei = 0;
      zeigeZeit(millis(), anzeigeZwei);
      // Zwischenzeit wird gesetzt
      if ((stateZeitReset == LOW) && (stateZeitReset != letzterStateZeitReset)) {
        zwischenZeit = millis();
        stateProgramm = 2;
      }
      // Zeit wurde angehalten
      if ((stateStartStopp == LOW) && (stateStartStopp != letzterStateStartStopp)) {
        zeitAngehalten = millis();
        stateProgramm = 3;
      }
      break;
    case 2: // Zwischenzeit
      anzeigeZwei = 1;
      zeigeZeit(zwischenZeit, anzeigeZwei);
      // zwischenzeit ausblenden
      if ((stateZeitReset == LOW) && (stateZeitReset != letzterStateZeitReset)) {
        stateProgramm = 1;
      }
      break;
    case 3: // Gestoppt
      // weiter laufen lassen
      if ((stateZeitReset == LOW) && (stateZeitReset != letzterStateZeitReset)) {
        startZeit = startZeit + (millis() - zeitAngehalten);
        stateProgramm = 1;
      }
      // Zeit loeschen
      if ((stateStartStopp == LOW) && (stateStartStopp != letzterStateStartStopp)) {
        stateProgramm = 0;
      }
      break;
  }
  letzterStateStartStopp = stateStartStopp;
  letzterStateZeitReset = stateZeitReset;
}
