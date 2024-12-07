#include "DisplayControl.h"

// Arrays defining GPIO pins for each digit
const uint8_t num2_msd_pins[] = {18, 16, 17, 5};   // GPIO pins for num1 Most Significant Digit (MSD)
const uint8_t num2_lsd_pins[] = {4, 15, 2, 0};   // GPIO pins for num1 Least Significant Digit (LSD)
const uint8_t num1_msd_pins[] = {27, 13, 12, 14}; // GPIO pins for num2 Most Significant Digit (MSD)
const uint8_t num1_lsd_pins[] = {32, 26, 25, 33}; // GPIO pins for num2 Least Significant Digit (LSD)
const uint8_t* pinsArray[] = {num1_msd_pins, num1_lsd_pins, num2_msd_pins, num2_lsd_pins};

void setupDisplayGPIO() {
    // Initialize all pins as OUTPUT and set LOW
    for (uint8_t i = 0; i < 4; i++) {
        for (uint8_t j = 0; j < 4; j++) {
            pinMode(pinsArray[i][j], OUTPUT);
            digitalWrite(pinsArray[i][j], HIGH);
        }
    }
}

// Function to display score using two numbers
void displayScore(uint8_t num1, uint8_t num2) {
    
    // Extract digits
  uint8_t num1_digits[] = {num1 / 10, num1 % 10}; // MSD and LSD of num1
  uint8_t num2_digits[] = {num2 / 10, num2 % 10}; // MSD and LSD of num2



  if(num1_digits[0] == 0){
    num1_digits[0] = ~num1_digits[0];
  }
  if(num2_digits[0] == 0){
    num2_digits[0] = ~num2_digits[0];
  }


  for (uint8_t i = 0; i < 4; i++) {
    // Write the LSB to the corresponding pins in pinsArray
    digitalWrite(pinsArray[0][i], (num1_digits[0] & 1) ? HIGH : LOW);  // Write to num1_lsd_pins
    num1_digits[0] >>= 1;  // Shift the bits for next iteration 
  }
  for (uint8_t i = 0; i < 4; i++) {
    // Write the LSB to the corresponding pins in pinsArray
    digitalWrite(pinsArray[1][i], (num1_digits[1] & 1) ? HIGH : LOW);  // Write to num1_lsd_pins
    num1_digits[1] >>= 1;  // Shift the bits for next iteration 
  }
  for (uint8_t i = 0; i < 4; i++) {
    // Write the LSB to the corresponding pins in pinsArray
    digitalWrite(pinsArray[2][i], (num2_digits[0] & 1) ? HIGH : LOW);  // Write to num1_lsd_pins
    num2_digits[0] >>= 1;  // Shift the bits for next iteration 
  }
  for (uint8_t i = 0; i < 4; i++) {
    // Write the LSB to the corresponding pins in pinsArray
    digitalWrite(pinsArray[3][i], (num2_digits[1] & 1) ? HIGH : LOW);  // Write to num1_lsd_pins
    num2_digits[1] >>= 1;  // Shift the bits for next iteration 
  }

}
