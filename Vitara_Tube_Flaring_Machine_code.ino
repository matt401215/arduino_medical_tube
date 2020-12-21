////////////////////////////////////////////////////////////////////////////////
// THIS IS THE GOOD ONE!
//
// VITARA Medical Tube Flaring Machine
// Matthew Gilroy
// Fall 2020
// Uses a solid state relay to control a rope heater for use in medical tube
// flaring. Monitors the temperature of the heater with a k-type thermocouple.
// Display relevant information on OLED screen.
//
/////////////////////////////////////////////////////////////////////////////////
#include <Adafruit_MAX31856.h>         // Thermocouple
#include <SPI.h>                       // OLED Display
#include <Wire.h>                      // OLED Display
#include <Adafruit_GFX.h>              // OLED Display
#include <Adafruit_SSD1306.h>          // OLED Display
#define SCREEN_ADDRESS 0x3C            // Found using address scanner code
#define SCREEN_WIDTH 128               // OLED display width, in pixels
#define SCREEN_HEIGHT 64               // OLED display height, in pixels
#define OLED_RESET    -1               // Reset pin # (or -1 if sharing Arduino reset pin)
#define DRDY_PIN 5                     // Tells MC when another reading is ready
float fahrenheitTip = 0;
float fahrenheitChip = 0;
const uint8_t switch_pin = 2;
const uint8_t ssr_pin = 6;
uint8_t ssr_state = LOW; //HIGH; //LOW;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_MAX31856 maxthermo = Adafruit_MAX31856(10, 11, 12, 13);


void setup() {
  Serial.begin(9600);
  pinMode(switch_pin, INPUT_PULLUP);
  pinMode(ssr_pin, OUTPUT);
  pinMode(DRDY_PIN, INPUT);    // DRDY tells arduino when another reading is ready (goes low)
  attachInterrupt(digitalPinToInterrupt(switch_pin), ssr, CHANGE);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000);                // Pause for 2 seconds to allow for initialization
  display.clearDisplay();     // Clear the buffer
  initialize_thermo();
  display.clearDisplay();
}


////////////////////////////////////////////////////////////////////////////////////////
//
// Main Loop
// Sit in loop until a new thermocouple reading is ready. Convert the
// new reading and display on screen.
// Check what state the SSR is in and update display.
//
///////////////////////////////////////////////////////////////////////////////////////
void loop() {
  // The DRDY output goes low when a new conversion result is available
  // while loop runs a loop doing nothing important until DRDY pin goes low
  int count = 0;
  while (digitalRead(DRDY_PIN)) {
    if (count++ > 200) {
      count = 0;
    }
  }

  // Convert celsius into fahrenheit and display
  fahrenheitTip = (maxthermo.readThermocoupleTemperature() * (9 / 5)) + 32;
  //display.setTextSize(1);
  display.setCursor(0, 28);
  //display.println("Temp (F): ");
  display.setTextSize(3);
  display.println(fahrenheitTip);
  display.setTextSize(1);
  display.println("Temp (F)");
  //display.setTextSize(1);
  //display.setCursor(0,55);
  //fahrenheitChip = (maxthermo.readCJTemperature() * (9/5)) + 32;
  //display.print("Chip temp: ");
  //display.println(fahrenheitChip);
  //display.display();

  display.setTextSize(2);
  display.setCursor(0, 0);
  if (ssr_state == HIGH) {
    display.print("Heater ON");
  } else {
    display.print("Heater OFF");
  }
  display.display();

  delay(1000);                    // Delay next reading
  display.clearDisplay();
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Interrupt loop
// When switch is throw, change the state of the SSR.
//
////////////////////////////////////////////////////////////////////////////////////////
void ssr() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200){
    ssr_state = !ssr_state;
    digitalWrite(ssr_pin, ssr_state);
  }
  last_interrupt_time = interrupt_time;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Initialize thermocouple
// Check if thermocouple is there, if not fall into loop until it
// is found. Then display initialization screen.
// Set the type of thermocouple.
// Set thermocouple to read in continuous mode
//
///////////////////////////////////////////////////////////////////////////////////////
void initialize_thermo() {
  display.clearDisplay();

  // What happens if program can't find thermocouple
  if (!maxthermo.begin()) {
    display.println("Could not initialize thermocouple.");
    display.display();
    while (1) delay(10);
  }

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("MAX31856");
  display.setTextSize(1);
  display.println("Thermocouple \nInitialized");
  display.display();
  delay(2000);

  // Declare what type of thermocouple you are using (change K if necessary)
  maxthermo.setThermocoupleType(MAX31856_TCTYPE_K);

  // Set thermocouple to read in continuous mode
  maxthermo.setConversionMode(MAX31856_CONTINUOUS);
}
