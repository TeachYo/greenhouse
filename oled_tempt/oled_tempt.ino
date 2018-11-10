// Include the correct display library
// For a connection via I2C using Wire include
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`

// Initialize the OLED display using Wire library
SSD1306Wire  display(0x3c, 21, 18);

#include <OneWire.h>
OneWire  ds(15);  // on pin 10 (a 4.7K resistor is necessary)
#include "DHT.h"

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

int DHTPIN = 17;
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);

  // Initialising the UI will init the display too.
  display.init();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  dht.begin();
}

float tempt_ds(){
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
  
  if ( !ds.search(addr)) {
    Serial.println("No more addresses.");
    ds.reset_search();
    delay(250);
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(750);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  //Serial.print("  Temperature = ");
  //Serial.print(celsius);
  //Serial.print(" Celsius, ");
  //Serial.print(fahrenheit);
  //Serial.println(" Fahrenheit");

  return celsius;
}

float Tempt(float *h, float *t, float *f){
    // Wait a few seconds between measurements.
  delay(1000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  *h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  *t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  *f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(*h) || isnan(*t) || isnan(*f)) {
    Serial.println("Failed to read from DHT sensor!");
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(*f, *h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(*t, *h, false);

  Serial.print("Humidity: ");
  Serial.print(*h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(*t);
  Serial.print(" *C ");
  Serial.print(*f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");
}

void loop() {
  float h,t,f;
  Tempt(&h, &t, &f);

  float a = tempt_ds();

  // clear the display
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 5, String(t) + " *C Temp");
  delay(1000);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 15, String(h) + " % Humidity");
  delay(1000);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 25, String(a) + " C Temp Air");
  delay(1000);
  display.display();
}
