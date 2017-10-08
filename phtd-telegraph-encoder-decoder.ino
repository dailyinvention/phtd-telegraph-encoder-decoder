#include <ArduinoJson.h>
#include <WiFi101.h>
#include <SPI.h>
#include <Lewis.h>

char ssid[] = "S-Link1";
char pass[] = "Wolfgang04";

int status = WL_IDLE_STATUS;

Lewis Morse;
WiFiClient client;

IPAddress server(192, 168, 1, 121);

unsigned long lastConnectionTime = 0;
const unsigned long postingInterval = 10L * 1000L;

// Initiate the Morse Code Library
void setup() {
  Morse.begin(6, 6, 20, false);

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to WiFi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  printWiFiStatus();
}

// Prints each char in sentence in morse code
void createSentence(String stringToConvert) {
  // Gets length of string
  unsigned int stringLength = stringToConvert.length();
  // Loops through string, converting each letter to morse code
  for (std::size_t i = 0; i < stringLength; i++) {
    Morse.print(stringToConvert.charAt(i));
    Serial.print(stringToConvert.charAt(i));
    delay(1000);
  }
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

// this method makes a HTTP connection to the server:
void httpRequest() {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection:
  if (client.connect(server, 3000)) {

    Serial.println("connecting...");
    // send the HTTP PUT request:
    client.println("GET /get-controls");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
  }
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

String extractJSON(String readString) {
  String jsonPart;
  // Extracts JSON from response
  for (std::size_t i = 0; i < readString.length(); i++) {
    if (readString.substring(i, i + 1) == "{") {
      jsonPart = "{" + readString.substring(i + 1);
      break;
    }
  }
  return jsonPart;
}

// Runs loop every ten seconds to output sentence in morse code
void loop() {
  String readString, jsonPart;
  while (client.available()) {
    char c = client.read();
    readString += c;
  }

  jsonPart = extractJSON(readString);
  Serial.print(jsonPart);

  // if ten seconds have passed since your last connection,
  // then connect again and send data:
  if (millis() - lastConnectionTime > postingInterval) {
    httpRequest();
  }

  createSentence("Welcome to the Port Hope Train Depot!");
  delay(10000);

}
