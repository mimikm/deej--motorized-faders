#include <EEPROM.h>

const int NUM_SLIDERS = 4; // Changed to 4
const int analogInputs[NUM_SLIDERS] = {A0, A1, A2, A3}; // Adjusted to 4 analog input pins
const int motorPins[NUM_SLIDERS][2] = {
  {3, 4}, // Motor 1: {IN1, IN2}
  {5, 6}, // Motor 2: {IN1, IN2}
  {9, 10}, // Motor 3: {IN1, IN2} - Adjusted to 3 motors
  {11, 12}  // Motor 4: {IN1, IN2} - Adjusted to 4 motors
};
const int buttonPins[] = {2, 13, 14}; // Pins connected to preset buttons
const int presetMemoryAddresses[] = {0, 1, 2}; // EEPROM addresses to store preset positions

int analogSliderValues[NUM_SLIDERS];
unsigned long lastSliderUpdate = 0;
unsigned long buttonPressStartTime = 0;

void setup() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
    pinMode(analogInputs[i], INPUT);
    pinMode(motorPins[i][0], OUTPUT); // IN1 pins
    pinMode(motorPins[i][1], OUTPUT); // IN2 pins
  }

  for (int i = 0; i < 3; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP); // Set preset button pins as inputs with pull-up resistors
    attachInterrupt(digitalPinToInterrupt(buttonPins[i]), buttonPressed, FALLING); // Attach interrupts to the buttons
  }

  Serial.begin(9600);
}

void loop() {
  updateSliderValues();
  sendSliderValues(); // Actually send data (all the time)
  // printSliderValues(); // For debug
  delay(10);
}

void buttonPressed() {
  int button = -1;
  for (int i = 0; i < 3; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {
      button = i;
      break;
    }
  }
  
  if (button != -1) {
    buttonPressStartTime = millis(); // Record the time when the button was pressed
    while (digitalRead(buttonPins[button]) == LOW && millis() - buttonPressStartTime < 5000) {
      // Wait for button release or 5-second timeout
    }
    if (millis() - buttonPressStartTime >= 5000) {
      storePreset(button);
      delay(200); // Debouncing delay
    } else {
      recallPreset(button);
      delay(200); // Debouncing delay
    }
  }
}

void updateSliderValues() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
    analogSliderValues[i] = analogRead(analogInputs[i]);
  }
}

void sendSliderValues() {
  String builtString = String("");

  for (int i = 0; i < NUM_SLIDERS; i++) {
    builtString += String((int)analogSliderValues[i]);

    if (i < NUM_SLIDERS - 1) {
      builtString += String("|");
    }
  }

  Serial.println(builtString);
}

void storePreset(int index) {
  for (int i = 0; i < NUM_SLIDERS; i++) {
    int presetPosition = analogRead(analogInputs[i]); // Store current slider position as preset
    EEPROM.put(presetMemoryAddresses[index] + i * sizeof(int), presetPosition); // Store preset position in EEPROM
  }
}

void recallPreset(int index) {
  for (int i = 0; i < NUM_SLIDERS; i++) {
    int presetPosition;
    EEPROM.get(presetMemoryAddresses[index] + i * sizeof(int), presetPosition); // Retrieve preset position from EEPROM
    moveMotorToPosition(i, presetPosition); // Move motor to preset position
    analogSliderValues[i] = presetPosition; // Update slider values
  }
}

void moveMotorToPosition(int motorIndex, int targetPosition) {
  int currentPosition = analogRead(analogInputs[motorIndex]);
  if (currentPosition < targetPosition) {
    // Move motor forward
    analogWrite(motorPins[motorIndex][0], 128); // 50% duty cycle
    analogWrite(motorPins[motorIndex][1], 128);  // 50% duty cycle
  } else if (currentPosition > targetPosition) {
    // Move motor backward
    analogWrite(motorPins[motorIndex][0], 128); // 50% duty cycle
    analogWrite(motorPins[motorIndex][1], 128); // 50% duty cycle
  } else {
    // Stop motor
    analogWrite(motorPins[motorIndex][0], 128); // 50% duty cycle
    analogWrite(motorPins[motorIndex][1], 128); // 50% duty cycle

  }
}
