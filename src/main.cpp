// Höhen und Beschleunigungssensor für Wasserraketen
// Michael Graf im Rahmen des MakerSpace-EBE
//
// 



#include <Adafruit_MPU6050.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

Adafruit_MPU6050 mpu;
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
Adafruit_BMP280 bmp; // I2C

float hoehe_anzeige, hoehe_max, hoehe_raw, hoehe_init, hoehe_null;
float acc_null, acc_anzeige, acc_raw, acc_max;
unsigned long mymil;
int state=0;

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
                  Adafruit_BMP280::STANDBY_MS_1); /* Standby time.   war 500 */


  delay(2000); // Pause for 2 seconds

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  acc_max=0;
  
  hoehe_init=bmp.readAltitude(1013.25);
  hoehe_max=0;
  hoehe_anzeige=acc_anzeige=0;
  
  mymil=millis();

  pinMode(2, INPUT_PULLUP);

}


void loop() {
  String outp;
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  acc_raw=abs(a.acceleration.z)+abs(a.acceleration.x)+abs(a.acceleration.y);
  hoehe_raw=bmp.readAltitude(1013.25);

  switch(state) {
   
  case 0:   //Am Boden
      if (acc_raw>50) state=1;
      else {
        if (acc_raw<20) {   //liegt ruhig -> Nullwerte anpassen
          hoehe_null=hoehe_raw;
          acc_null=acc_raw;
        }
        acc_max=0;
        hoehe_max=0;
      }
      break;

  case 1:   //Steigen
      if (abs(acc_raw)>abs(acc_max)) {acc_max=acc_raw;}  
      mymil=millis();
      if (acc_raw<20) state=2;
      break;    

  case 2:   //Scheitelpunkt
      if (abs(hoehe_raw)>abs(hoehe_max)) {hoehe_max=hoehe_raw;}
      if (millis()-mymil>3000) state=3;
      break;    

  case 3:   //Fallen
      hoehe_anzeige=hoehe_max-hoehe_null;
      acc_anzeige= abs(1000/36/(acc_max-acc_null));  // in 0 auf 100 in x sec :-)

      if (acc_raw<20) state=0;
      break;    

  }


  display.clearDisplay();
  display.setCursor(0, 0);

  display.print("S");
  display.print(state);

  display.print(" H:");
  outp = String(hoehe_raw-hoehe_init,1);
  display.println(outp);
  
  outp = String(hoehe_anzeige,1);
  display.print(outp);

  display.print(" ");
  outp = String(acc_anzeige,2);
  display.print(outp);
  display.display();
  
  delay(100);
}
