#include <Arduino.h>

/* Temp Sensor */
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

unsigned long previousTempMillis = 0;
const long tempInterval = 2500;
unsigned long currentTempMillis;

/* Libraries for Neopixel */
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

/* Library for Servo */
#include <Servo.h>

/* Librariers for I2C Display */
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
int coasterStarted = 0;

/* On/Off Button Digital 5 */
int onButtonPin = 5;

/* PotentioMeter Analog 0 - Divide by 17.05 to get max of 60 degrees*/
int potentionmeterPin = A0;
float potFloat = 17.05; 

/* Neopixel Digital 4 */
int neoPixelPin = 4;
int numNeoPixels = 24;
Adafruit_NeoPixel pixels(numNeoPixels, neoPixelPin, NEO_GRB + NEO_KHZ800);
unsigned long previousNeoMillis = 0;
const long neoPixelInterval = 10;
unsigned long currentNeoMillis;
unsigned long neoPixelLed = 0;
unsigned long neoPixelSet = 0;
int neoPixelNoCup = 0;
int neoPixelStartup = 0;

/* LDR Sensor Digital 12*/
int ldrPin = 11;

/* Servo Digital 3 */
Servo myservo;
int servoPin = 3;

/* Buzzer Digital 7 */
int buzzerPin = 7;

/* Relay Digital 6 */
int relayPin = 6;

int measureStart = 0;
unsigned long previousTimerMillis = 0;
unsigned long timerInterval = 1000;
unsigned long currentTimerMillis = 0;
int timerStart = 15;
int minTemp = 0;
int maxTemp = 0;
int middleMinTemp = 0;
int middleMaxTemp = 0;
int wrongMinTemp = 0;
int wrongMaxTemp = 0;

void oledDisplayCenter(String text) {
    int16_t x1;
    int16_t y1;
    uint16_t width;
    uint16_t height;

    display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);

    display.clearDisplay();
    display.setCursor((SCREEN_WIDTH - width) / 2, (SCREEN_HEIGHT - height) / 2);
    display.println(text); // text to display
    display.display();
}

void setTempDisplay(String text, String text2) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println( "S-Temp:" + text + "C" );
    display.println( "Temp:" + text2 + "C");
    display.display();
}

void heatStartDisplay(String text) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Heating in");
    display.println(text + " sec");
    display.display();
}

/* void tempDisplay(String text) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.display();
} */

void setup() {
    Serial.begin(9600);

    pinMode(onButtonPin, INPUT);
    pinMode(potentionmeterPin, INPUT);
    pinMode(ldrPin, INPUT);
    pinMode(buzzerPin, OUTPUT);
    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, HIGH);

    sensors.begin();
    // myservo.attach(servoPin);

    if( !display.begin(SSD1306_SWITCHCAPVCC, 0x3C) ) {
        for(;;);
    }

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);

    pixels.begin();
    pixels.clear();
    neoPixelLed = 0;
    neoPixelSet = 0;
    neoPixelStartup = 0;
    coasterStarted = 0;
}

void loop() {
    currentNeoMillis = millis();
    currentTempMillis = millis();
    currentTimerMillis = millis();
    int buttonState = digitalRead(onButtonPin);
    int ldrRead = digitalRead(ldrPin);
    int pot = map(analogRead(potentionmeterPin), 0, 1023, 20,70);
    int tempSens = sensors.getTempCByIndex(0);

    minTemp = pot - 5;
    maxTemp = pot + 5;
    middleMinTemp = minTemp - 5;
    middleMaxTemp = maxTemp + 5;

    int cupOnLedPixel = 0;

    if ( buttonState == 1 ) {

        if( tempSens >= minTemp && tempSens <= maxTemp && measureStart == 1 ) {
            for(int i=0; i<24; i++) {
                pixels.setPixelColor(i, pixels.Color(0, 255, 56));
                pixels.show(); 
            } 
            pixels.setBrightness(50);
        } else if( tempSens >= middleMaxTemp && measureStart == 1 ) {
            for(int i=0; i<24; i++) {
                pixels.setPixelColor(i, pixels.Color(255, 0, 0));
                pixels.show(); 
            } 
            pixels.setBrightness(50);
        } else if( tempSens <= middleMinTemp && measureStart == 1 ) {
            for(int i=0; i<24; i++) {
                pixels.setPixelColor(i, pixels.Color(0, 0, 255));
                pixels.show(); 
            }
        }

        if( tempSens <= minTemp && measureStart == 1 ) {
            digitalWrite(relayPin, LOW);
        } else if ( tempSens >= maxTemp && measureStart == 1 ) {
            digitalWrite(relayPin, HIGH);
        }

        if( currentTimerMillis - previousTimerMillis >= timerInterval && measureStart == 0 && ldrRead == 1 && coasterStarted == 1 ) {
            previousTimerMillis = currentTimerMillis;
            timerStart = timerStart - 1;
            heatStartDisplay(String(timerStart));
            Serial.println("asgasg");
            Serial.println(timerStart);
            if( timerStart == 0 ) {
                measureStart = 1;
                timerStart = 15;
            }
        }

        if( ldrRead == 0 && coasterStarted == 1 ) {
            digitalWrite(relayPin, HIGH);
            measureStart = 0;
            timerStart = 15;
            oledDisplayCenter("Place Cup");
            for(int i=0; i<24; i++) {
                pixels.setPixelColor(i, pixels.Color(255, 50, 50));
                cupOnLedPixel = cupOnLedPixel + 6;
                pixels.show(); 
                Serial.println(cupOnLedPixel);
            } 
            pixels.setBrightness(50);
        }

        if ( currentTempMillis - previousTempMillis >= tempInterval && ldrRead == 1 && measureStart == 1 ) {
            previousTempMillis = currentTempMillis;
            sensors.requestTemperatures();
            setTempDisplay(String(pot), String(tempSens)); 
        }

        if( neoPixelStartup == 0 ) {
            pixels.setPixelColor( neoPixelLed, pixels.Color(255, 50, 50)); // sit aan een vir een
            pixels.show();
            oledDisplayCenter("Loading!");
        } else if( neoPixelStartup == 1 && coasterStarted == 0 ) {
            oledDisplayCenter("Place Cup");
            coasterStarted = 1;
        }

        if ( currentNeoMillis - previousNeoMillis >= neoPixelInterval && neoPixelStartup == 0 && coasterStarted == 0 ) {
            previousNeoMillis = currentNeoMillis;
            pixels.setPixelColor( neoPixelLed, pixels.Color(0, 0, 0)); // sit aan een vir een
            pixels.show();
            neoPixelLed = neoPixelLed + 1;

            if ( neoPixelLed == 24 ) {
                neoPixelSet = neoPixelSet + 1;
                neoPixelLed = neoPixelSet;
            }

            if (neoPixelSet == 24)
            {
                pixels.setPixelColor(23, pixels.Color(255, 50, 50));
                pixels.show();
                neoPixelStartup = 1;
            }
            else
            {
                pixels.setPixelColor(neoPixelSet, pixels.Color(255, 50, 50));
                pixels.show();
            }
        }
    } else if ( buttonState == 0 ) {
        pixels.clear();
        pixels.show();
        neoPixelLed = 0;
        neoPixelSet = 0;
        neoPixelStartup = 0;
        coasterStarted = 0;
        measureStart = 0;
        timerStart = 15;
        digitalWrite(relayPin, HIGH);
        oledDisplayCenter("Off");
    }

}



