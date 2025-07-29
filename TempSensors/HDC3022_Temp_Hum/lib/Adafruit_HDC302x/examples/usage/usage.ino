// Example usage for Adafruit_HDC302x library by Ishman.

#include "Adafruit_HDC302x.h"

// Initialize objects from the lib
Adafruit_HDC302x adafruit_HDC302x;

void setup() {
    // Call functions on initialized library objects that require hardware
    adafruit_HDC302x.begin();
}

void loop() {
    // Use the library's initialized objects and functions
    adafruit_HDC302x.process();
}
