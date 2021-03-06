/*
    WeatherGirl
    by Eliza Weisman
    http://elizas.website

    A cute little DIY weather station (with fancy lighting!)

    Circuit:
    * Ethernet shield attached to pins 10, 11, 12, 13
    * DHT11 data wire connected to pin 3
    * LCD:
        - RS pin connected to pin 9
        - E pin connected to pin 8
        - D4, D5, D6, and D7 connected to pins 4, 5, 6, and 7

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

#include <SPI.h>
#include <Ethernet.h>

#include <SD.h>


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

IPAddress ip(192, 168, 1, 177);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);


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
        , TEMP_MAX = 122
        , CHIP_SELECT = 2;
//
// float h, t, f, hif, hic;
bool sd = false;
CommonCathodeLed<11,10, 9> rgb_led = CommonCathodeLed<11, 10, 9>();
DHT dht(DHT_PIN, DHT_TYPE);

class State {
private:
    float h, t, f, hif, hic;
    unsigned long time;
public:

    /* Construct a new State from humidity, Celcius, and Fahrenheit */
    State(float h, float t, float f)
        : h { h }
        , t { t }
        , f { f }
        , hif { dht.computeHeatIndex(f, h) }
        , hic { dht.computeHeatIndex(t, h, true) }
        , time { millis() }
        { }

    State(DHT d)
        : h { d.readHumidity() }
        , t { d.readTemperature() }
        , f { d.readTemperature(true) }
        , hif { d.computeHeatIndex(f, h) }
        , hic { d.computeHeatIndex(t, h, true) }
        , time { millis() }
        { }

    unsigned long timestamp(void) {
        return this->time;
    }

    void lcd_output(LiquidCrystal l) {
        l.clear();
        l.setCursor(0, 0);
        l.print("temp: ");
        l.print((int)round(this->t), 10);
        l.write(byte(0));
        l.print("C ");
        l.print((int)round(this->f), 10);
        l.write(byte(0));
        l.print("F");
        l.setCursor(0,1);
        l.print("humidity: ");
        l.print(this->h);
        l.print("%");
    }

    void serial_output(void) {
        Serial.print("Humidity: ");
        Serial.print(this->h);
        Serial.print(" %\t");
        Serial.print("Temperature: ");
        Serial.print(this->t);
        Serial.print(" °C ");
        Serial.print(this->f);
        Serial.print(" °F\t");
        Serial.print("Heat index: ");
        Serial.print(this->hic);
        Serial.print(" °C ");
        Serial.print(this->hif);
        Serial.println(" °F");
    }

    void html_output(EthernetClient client) {

        // print the current readings, in HTML format:
        client.print("Temperature: ");
        client.print(this->t);
        client.print(" °C / ");
        client.print(this->f);
        client.print(" °F ");
        client.println("<br />");

        client.print("Heat index: ");
        client.print(this->hic);
        client.print(" °C / ");
        client.print(this->hif);
        client.print(" °F ");
        client.println("<br />");

        client.print("Humidity: " + String(h) + " %");
        client.println("<br />");
    }

    void csv_output(File dataFile) {
        // if the file is available, write to it:
      if (dataFile) {
        String dataString = String(this->time)
                          + "," + String(this->t)
                          + "," + String(this->h);
        dataFile.println(dataString);
        dataFile.close();
      }
    }

};

State state = State(0,0,0);

void setup(void) {
    // start the SPI library:
    SPI.begin();

    // start the Ethernet connection and the server:
    Ethernet.begin(mac, ip);
    server.begin();

    // create the degree symbol bitmap
    lcd.createChar(0, degree);
    // set up the LCD's number of columns and rows
    lcd.begin(16, 2);
    // Print a message to the LCD.
    lcd.print("WeatherGirl <3");

    Serial.begin(9600);
    // Print a message to the LCD.
    Serial.print("WeatherGirl started! <3");
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
    }


    Serial.print("Initializing SD card...");

    // see if the card is present and can be initialized:
    if (!SD.begin(CHIP_SELECT)) {
        Serial.println("Card failed, or not present");
        lcd.print("no SD card");
    } else {
        Serial.println("card initialized.");
        sd = true;
    }

    dht.begin();
}

// void measure(void) {
//     // TODO: refactor this to a class method
//     // Reading temperature or humidity takes about 250 milliseconds!
//     // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
//     float h_temp = dht.readHumidity()
//           // Read temperature as Celsius (the default)
//         , t_temp = dht.readTemperature()
//           // Read temperature as Fahrenheit (isFahrenheit = true)
//         , f_temp = dht.readTemperature(true)
//         ;
//
//     // Check if any reads failed and exit early (to try again).
//     if (isnan(h_temp) || isnan(t_temp) || isnan(f_temp)) {
//       Serial.println("Failed to read from DHT sensor!");
//       lcd.clear();
//       lcd.print("DHT sensor error!");
//       return;
//   } else {
//       h = h_temp;
//       t = t_temp;
//       f = f_temp;
//       hif = dht.computeHeatIndex(f, h);
//       hic = dht.computeHeatIndex(t, h, false);
//   }
//
// }

void serve_ethernet(void) {
    // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("Got a client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
            // send a standard http response header
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println();
            state.html_output(client);
            break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }
}


void loop(void) {
    // Wait a few seconds between measurements.
    if (millis() - state.timestamp() > 2000) {
        state = State(dht);
        state.lcd_output(lcd);
        state.serial_output();

        // if a SD card was found, log data
        if (sd) {
            File dataFile = SD.open("weather.csv", FILE_WRITE);
            state.csv_output(dataFile);
            dataFile.close();
        }

    }
    serve_ethernet();

}
