#include <Adafruit_MPU6050.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

Adafruit_MPU6050 mpu;
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
Adafruit_BMP280 bmp; // I2C

float Hoehe, calc_h, hmax;
float acc, acc0, accmax, acc_ms, acc_raw, acc_rawmax;
unsigned long mymil, mymil2;

void setup() {
  //Serial.begin(115200);
  
  //Serial.println("MPU6050 OLED demo");
  
  mpu.begin();
/*
  if (!mpu.begin()) {
    //Serial.println("Sensor init failed");
    while (1)
      yield();
  }
  */
  //Serial.println("Found a MPU-6050 sensor");
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    //Serial.println(F("SSD1306 allocation failed"));
    //for (;;)
    //  ; // Don't proceed, loop forever
  }
  
  display.display();
  //delay(500); // Pause for 2 seconds
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setRotation(0);

unsigned status;
  status = bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);

  if (!status) {
    //Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
    //                  "try a different address!"));
    //Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(),16);
    //Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    //Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    //Serial.print("        ID of 0x60 represents a BME 280.\n");
    //Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1) ; //delay(10);
  } //else Serial.println("BMP280 ok");
  
  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_1); /* Standby time.   war 500 */

 
 
  
  //display.display();
  delay(2000); // Pause for 2 seconds

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  acc0=abs(a.acceleration.z)+abs(a.acceleration.x)+abs(a.acceleration.y);
  accmax=acc_rawmax=0;
  Hoehe=bmp.readAltitude(1013.25);
  mymil=mymil2=millis();
  hmax=0;

  pinMode(2, INPUT_PULLUP);


}

void reset_measure(float acctemp) {
   Hoehe=bmp.readAltitude(1013.25); 
   hmax=0; 
   acc0=acctemp; 
   accmax=acc_rawmax=0;
}

void loop() {
  String outp;
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

 if (digitalRead(2)==false) {reset_measure(abs(a.acceleration.z)+abs(a.acceleration.x)+abs(a.acceleration.y));}

  calc_h=bmp.readAltitude(1013.25)-Hoehe;
  //Serial.println(calc_h);
  if (abs(calc_h)<0.06) {Hoehe=Hoehe+calc_h; calc_h=0;}
  
  if (millis()-mymil>1000) {mymil=millis(); if (abs(calc_h)<0.1) {Hoehe=Hoehe+calc_h; calc_h=0;} }

  if (abs(calc_h)>abs(hmax)) {hmax=calc_h;}

  acc_raw=abs(a.acceleration.z)+abs(a.acceleration.x)+abs(a.acceleration.y);
  acc = acc_raw - acc0;
  
  if (abs(acc)>abs(accmax)) {accmax=acc;}
  if (abs(acc_raw)>abs(acc_rawmax)) {acc_rawmax=acc_raw;}  
  acc_ms= abs(1000/36/accmax);  // in 0 auf 100 in x sec :-)


  display.clearDisplay();
  display.setCursor(0, 0);

  //display.setTextSize(2);             // Draw 2X-scale text
  //display.setTextColor(SSD1306_WHITE);
  //display.print(calc_h);
  outp = String(calc_h,1);
  display.print(outp);
  display.print(" ");
  outp = String(hmax,1);
  display.println(outp);
  outp = String(acc_raw,1);
  display.print(outp);
  display.print(" ");
  //outp = String(acc_ms,2);
  outp = String(acc_rawmax,2);
  display.println(outp);
  display.display();

/*
Serial.print("Accel:");
  Serial.print(acc);
  Serial.print(",");
  Serial.print("GyroX:");
  Serial.print(g.gyro.x);
  Serial.print(",");
  Serial.print("GyroY:");
  Serial.print(g.gyro.y);
  Serial.print(",");
  Serial.print("GyroZ:");
  Serial.print(g.gyro.z);
  Serial.print(",");
  Serial.print("Hoehe:");
  Serial.print(calc_h);
  Serial.print(",");
  Serial.print("Max:");
  Serial.print(hmax);
  Serial.println("");


  Serial.print("Accelerometer ");
  Serial.print("X: ");
  Serial.print(a.acceleration.x, 1);
  Serial.print(" m/s^2, ");
  Serial.print("Y: ");
  Serial.print(a.acceleration.y, 1);
  Serial.print(" m/s^2, ");
  Serial.print("Z: ");
  Serial.print(a.acceleration.z, 1);
  Serial.println(" m/s^2");

  display.println("Accelerometer - m/s^2");
  display.print(a.acceleration.x, 1);
  display.print(", ");
  display.print(a.acceleration.y, 1);
  display.print(", ");
  display.print(a.acceleration.z, 1);
  display.println("");

  Serial.print("Gyroscope ");
  Serial.print("X: ");
  Serial.print(g.gyro.x, 1);
  Serial.print(" rps, ");
  Serial.print("Y: ");
  Serial.print(g.gyro.y, 1);
  Serial.print(" rps, ");
  Serial.print("Z: ");
  Serial.print(g.gyro.z, 1);
  Serial.println(" rps");

  display.println("Gyroscope - rps");
  display.print(g.gyro.x, 1);
  display.print(", ");
  display.print(g.gyro.y, 1);
  display.print(", ");
  display.print(g.gyro.z, 1);
  display.println("");

  display.display(); */
  
  delay(100);
}
