/*
    WeatherGirl
    by Eliza Weisman
    http://elizas.website

    A cute little DIY weather station (with fancy lighting!)

 */

#include <Arduino.h>

// ThreeClor.h, by Eliza Weisman
// for easy HSV control of RGB LED modules
#include <ThreeClor.h>

// Adafruit_Sensor and DHT libraries, by Adafruit
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

// liquid-crystal display
#include <LiquidCrystal.h>
#include <math.h>

// Degree symbol bitmap
byte degree[8] = {
  B01000,
  B10100,
  B01000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
};


// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(9, 8, 4, 5, 6, 7);

const int DHT_PIN  = 3
        , DHT_TYPE = DHT11
        , TEMP_MIN = 32
        , TEMP_MAX = 122;

CommonCathodeLed<11,10, 9> rgb_led = CommonCathodeLed<11, 10, 9>();
DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
    // create the degree symbol bitmap
    lcd.createChar(0, degree);
    // set up the LCD's number of columns and rows
    lcd.begin(16, 2);
    // Print a message to the LCD.
    lcd.print("WeatherGirl <3");

    Serial.begin(9600);
    // Print a message to the LCD.
    Serial.print("WeatherGirl started! <3");

    dht.begin();
}

void loop() {
    // Wait a few seconds between measurements.
    delay(2000);

    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
      lcd.clear();
      lcd.print("DHT sensor error!");
      return;
    }

    // int hue = map((long)f, TEMP_MIN, TEMP_MAX, 0, 255);
    // // set the LED color
    // rgb_led.color = HSVColor(hue);
    // rgb_led.show();

    // Compute heat index in Fahrenheit (the default)
    float hif = dht.computeHeatIndex(f, h);
    // Compute heat index in Celsius (isFahreheit = false)
    float hic = dht.computeHeatIndex(t, h, false);


    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" °C ");
    Serial.print(f);
    Serial.print(" °F\t");
    Serial.print("Heat index: ");
    Serial.print(hic);
    Serial.print(" °C ");
    Serial.print(hif);
    Serial.println(" °F");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("temp: ");
    lcd.print((int)round(t), 10);
    lcd.write(byte(0));
    lcd.print("C ");
    lcd.print((int)round(f), 10);
    lcd.write(byte(0));
    lcd.print("F");
    lcd.setCursor(0,1);
    lcd.print("humidity: ");
    lcd.print(h);
    lcd.print("%");
}
