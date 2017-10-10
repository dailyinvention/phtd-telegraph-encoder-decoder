#include <ArduinoJson.h>
#include <WiFi101.h>
#include <SPI.h>
#include <Lewis.h>

// Wifi SSID
char ssid[] = "S-Link1";
// Wifi Password
char pass[] = "Wolfgang04";

int status = WL_IDLE_STATUS;

Lewis Morse;
WiFiClient client;

IPAddress server(192, 168, 1, 121);

unsigned long lastConnectionTime = 0;
unsigned int messageCount = 0;
unsigned long int delayValue;
unsigned int messageArraySize = 0;
bool isLoaded = false;
const unsigned long postingInterval = 10L * 1000L;

// Initiate the Morse Code Library
void setup() {
  Morse.begin(6, 6, 20, false);

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // Attempt to connect to WiFi network:
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
  signed int stringLength = stringToConvert.length();
  // Loops through string, converting each letter to morse code
  for (signed int i = 0; i < stringLength; i++) {
    Morse.print(stringToConvert.charAt(i));
    Serial.print(stringToConvert.charAt(i));
    delay(1000);
  }
}

// this method makes a get request to the REST api.
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

// Checks the wifi status.
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

// Extracts JSON from response.
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

// Returns the value for a particular option.
const char *returnOptionsProp(JsonArray& options, const char *prop) {
  const char *value = "";
  for (auto option : options) {
    const char *name = option["name"];
    if (strcmp(name, prop) == 0) {
      value = option["value"];
    }
  }
  return value;
}

// Returns array of messages from JSON.
const char **returnMessages(JsonArray& messages) {
  const char **finalMessages;
  messageArraySize = 0;
  finalMessages = new const char * [sizeof(messages)];
  int order = 0;
  // Builds array or messages
  for (JsonObject& message : messages) {
    order = message["order"].as<int>();
    const char *messageText = message["message"];
    finalMessages[order - 1] = messageText;
    messageArraySize++;
  }
  return finalMessages;
}

// Runs loop every ten seconds to output sentence in morse code.
void loop() {
  String readString, jsonPart;

  while (client.available()) {
    char c = client.read();
    readString += c;
    isLoaded = true;
  }

  // Checks to see if read information is complete before parsing
  if (isLoaded == true) {
    // Parses JSON
    DynamicJsonBuffer jsonBuffer;
    jsonPart = extractJSON(readString);
    
    // Separate controls from message JSON
    JsonObject& jsonObj = jsonBuffer.parseObject(jsonPart);
    JsonArray& controls = jsonObj["controls"];
    JsonArray& messages = jsonObj["messages"];

    // Sets the delay time.
    delayValue = strtoul(returnOptionsProp(controls, (char *)"messagesDelay"), NULL, 0);

    const char **allMessages = returnMessages(messages);
    
    // Runs sentences through Morse Code processor
    createSentence(allMessages[messageCount]);
    Serial.println("\n");

    // Checks if the current message index has exceeded the total messages array count and either iterates the count or resets it to zero.
    if (messageCount < messageArraySize - 1) {
      messageCount++;
      //isLoaded = false;
    } else {
      messageCount = 0;
      isLoaded = false;
    }
  }

  if (millis() - lastConnectionTime > postingInterval) {
    httpRequest();
  }
  
  // Sets delay value.
  delay(delayValue);
}
