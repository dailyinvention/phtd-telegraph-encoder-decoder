#include <Lewis.h>

Lewis Morse;

// Initiate the Morse Code Library
void setup() {
  Morse.begin(6, 6, 20, false);
}

// Prints each char in sentence in morse code
void createSentence(String stringToConvert) {
  // Gets length of string
  unsigned int stringLength = stringToConvert.length();
  // Loops through string, converting each letter to morse code
  for(int i = 0; i < stringLength; i++) {
    Morse.print(stringToConvert.charAt(i));
    Serial.print(stringToConvert.charAt(i));
    delay(1000);
  }
}

// Runs loop every ten seconds to output sentence in morse code
void loop() {
  createSentence("Welcome to the Port Hope Train Depot!");
  delay(10000);

}
