// Höhen und Beschleunigungssensor für Wasserraketen
// Michael Graf im Rahmen des MakerSpace-EBE-Wasserraketen-Projektes
// 27.08.2024
// V1



#include <Adafruit_MPU6050.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

Adafruit_MPU6050 mpu;
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
Adafruit_BMP280 bmp; // I2C

float hoehe_raw, hoehe_init, hoehe_null, hoehe_max, hoehe_anzeige;
float acc_raw, acc_null, acc_max, acc_anzeige;
unsigned long mymil;
int state=0;

#define ACC_GRENZE_RUHIG 20  // m/s2
#define ACC_GRENZE_START 50  // m/s2
#define DAUER_SCHEITEL 3000  // ms

void setup() {

  mpu.begin();
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
  
  display.display();

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setRotation(0);

  bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_1);   /* Standby time.   war 500 */


  delay(2000); // Pause for 2 seconds

  acc_max=0;
  hoehe_max=0;
  hoehe_anzeige=acc_anzeige=0;

  hoehe_init=bmp.readAltitude(1013.25);           //Festlegen des Initialwertes um die aktuelle Höhe anzuzeigen - für Messung nicht relevant
  mymil=millis();

  pinMode(2, INPUT_PULLUP);

}


void loop() {
  String outp;
  sensors_event_t a, g, temp;

  //delay(100);                                                            //kein delay - maximale Messfrequenz ! 

  mpu.getEvent(&a, &g, &temp);                                             // bei jedem Durchlauf Messwerte erfassen
  acc_raw=abs(a.acceleration.z)+abs(a.acceleration.x)+abs(a.acceleration.y);   //Summe aller Richtungen
  hoehe_raw=bmp.readAltitude(1013.25);                                     // die 1013.25 ist laut Lib ein Kalibrierwert für Meereshöhe in Europa ??? nur übernommen

  switch(state) {
   
  case 0:   //Rakete am Boden
      if (acc_raw>ACC_GRENZE_START) state=1;                               // Bei grosser Beschleunigung sofort Neustart der Messung .... sonst
      else {
        if (acc_raw<ACC_GRENZE_RUHIG) {                                    // liegt ruhig -> Nullwerte anpassen, aber nicht wenn sie bewegt/getragen wird
          hoehe_null=hoehe_raw;
          acc_null=acc_raw;
        }
        acc_max=0;                                                         // gemessene Maximalwerte löschen
        hoehe_max=0;
      }
      break;

  case 1:   //steigen
      if (abs(acc_raw)>abs(acc_max)) acc_max=acc_raw;                      // maximale Beschleunigung erfassen
      mymil=millis();
      if (acc_raw<ACC_GRENZE_RUHIG) state=2;                               // wenn Beschleunigung gering, quasi wie ruhig liegen -> Scheitelpunktphase 
      break;    

  case 2:   //Scheitelpunktphase
      if (abs(hoehe_raw)>abs(hoehe_max)) hoehe_max=hoehe_raw;              // max. Höhe erfassen (Luftdruck durch Beschleunigung jetzt hoffentlich vernachlässigbar)
      if (millis()-mymil>DAUER_SCHEITEL) state=3;                          // nach einigen Sekunden Übergang in den freien Fall - anderen Messwerten habe ich nicht getraut
      break;                                                               // funktioniert nur bei Starts nach oben 

  case 3:   //fallen
      hoehe_anzeige=hoehe_max-hoehe_null;                                  // Werte zur Anzeige ermitteln da Messung jetzt fertig
      acc_anzeige= abs(1000/36/(acc_max-acc_null));                        // in 0 auf 100 in x sec :-) - versteht man besser wie Meter pro Sekunde zum Quadrat
      state=4;
      break;

  case 4:   //warten auf Aufschlag    
      if (acc_raw<ACC_GRENZE_RUHIG) state=0;                               // wenn wieder ruhig (wieder am Boden) dann von vorne
      break;    

  }                                                                        // Anzeigewerte bleiben bestehen und werden dauerhaft angezeigt bis zum nächsten Start = Durchlauf der States


  display.clearDisplay();
  display.setCursor(0, 0);

  display.print("S");
  display.print(state);

  display.print(" H:");                                                     //aktuelle Höhe im Vergleich zum Einschalten - keine Bedeutung für die Messung
  outp = String(hoehe_raw-hoehe_init,1);
  display.println(outp);
  
  outp = String(hoehe_anzeige,1);                                           //maximal erreichte Höhe
  display.print(outp);

  display.print(" ");
  outp = String(acc_anzeige,2);                                             //maximale Beschleunigung
  display.print(outp);
  display.display();
  

}
