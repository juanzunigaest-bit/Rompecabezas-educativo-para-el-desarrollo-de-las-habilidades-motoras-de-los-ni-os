/***************************************************
DFPlayer - A Mini MP3 Player For Arduino
 <https://www.dfrobot.com/product-1121.html>
 
 ***************************************************
 This example shows the basic function of library for DFPlayer.
 
 Created 2016-12-07
 By [Angelo qiao](Angelo.qiao@dfrobot.com)
 
 GNU Lesser General Public License.
 See <http://www.gnu.org/licenses/> for details.
 All above must be included in any redistribution
 ****************************************************/

/***********Notice and Trouble shooting***************
 1.Connection and Diagram can be found here
 <https://www.dfrobot.com/wiki/index.php/DFPlayer_Mini_SKU:DFR0299#Connection_Diagram>
 2.This code is tested on Arduino Uno, Leonardo, Mega boards.
 ****************************************************/

#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"

#define FPSerial Serial1

const uint8_t hallPins[20] = {
  13, 12, 11, 10, 9,  8,  7,  6,  4,  5,   // HALL1..HALL10 
  25, 27, 29, 33, 31, 35, 37, 39, 41 ,43   // HALL11..HALL20
};

const uint8_t BUSY_PIN = 23;

const uint8_t N_HALL = 16;

uint8_t lastState[N_HALL];
unsigned long buzzerOnUntil = 0;

unsigned long tPressStart[N_HALL];
bool holdArmed[N_HALL];
bool songPlayed[N_HALL];

// Asigna aquí qué canción corresponde a cada entrada (HALL1..HALL16)
uint16_t songForHall[N_HALL] = {
  1,  2,  3,  4,
  5,  6,  7,  8,
  9, 10, 11, 12,
  13, 14, 15, 16
};

const uint8_t BUZZER_PIN = A0;   // AD0
const uint8_t LED1_PIN = A15;    // AD0

// ================== VOLUMEN FIJO (CALIBRACIÓN) ==================
// ✅ ÚNICO CAMBIO: variable para pruebas (0 a 30)
uint8_t VOLUMEN_FIJO = 20;

DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

void setup()
{
#if (defined ESP32)
  FPSerial.begin(9600, SERIAL_8N1, /*rx =*/D3, /*tx =*/D2);
#else
  FPSerial.begin(9600);
#endif

  pinMode(BUSY_PIN, INPUT);   // BUSY normalmente es open-collector / activo en LOW
  for (uint8_t i = 0; i < N_HALL; i++) {
    pinMode(hallPins[i], INPUT);   // o INPUT, según tu hardware
    lastState[i] = digitalRead(hallPins[i]);
    holdArmed[i] = false;
    songPlayed[i] = false;
    tPressStart[i] = 0;
  }

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED1_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  Serial.begin(115200);

  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  
  if (!myDFPlayer.begin(FPSerial, /*isACK = */true, /*doReset = */true)) {  //Use serial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println(F("DFPlayer Mini online."));
  
  // ✅ ÚNICO CAMBIO: aplicar volumen fijo desde variable
  myDFPlayer.volume(VOLUMEN_FIJO);  //Set volume value. From 0 to 30
  Serial.print(F("Volumen fijo configurado: "));
  Serial.println(VOLUMEN_FIJO);

  // myDFPlayer.play(5);  //Play the first mp3
}

void loop()
{
  for (uint8_t i = 0; i < N_HALL; i++) {
    uint8_t current = digitalRead(hallPins[i]);

    // Flanco LOW -> HIGH (según tu código)
    if (lastState[i] == LOW && current == HIGH) {
      buzzerOnUntil = millis() + 500;

      tPressStart[i] = millis();
      holdArmed[i]   = true;
      songPlayed[i]  = false;
    }

    // Si se suelta (vuelve a LOW), reset
    if (current == LOW) {
      holdArmed[i]  = false;
      songPlayed[i] = false;
    }

    // Si se mantiene HIGH por 1 segundo, reproduce 1 vez
    if (holdArmed[i] && !songPlayed[i] && current == HIGH) {
      if (millis() - tPressStart[i] >= 1000UL) {
        uint16_t CANCION = songForHall[i];
        // ===== Serial: sensor presionado =====
        Serial.print("Sensor presionado: HALL");
        Serial.print(i + 1);                 // HALL1..HALL16
        Serial.print("  Pin: ");
        Serial.println(hallPins[i]);
        digitalWrite(BUZZER_PIN, HIGH);
        delay(50);
        digitalWrite(BUZZER_PIN, LOW);
        Serial.print("CANCION: ");
        Serial.print(CANCION);      
        myDFPlayer.play(CANCION);
        songPlayed[i] = true;
        delay(100);
        while (digitalRead(BUSY_PIN) == LOW) {
          // espera bloqueante mientras está “aplastado/ocupado”
          digitalWrite(LED1_PIN, HIGH);
          delay(100);
          digitalWrite(LED1_PIN, LOW);
          delay(100);
        }
      }
    }

    lastState[i] = current;
  }
}
