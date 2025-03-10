#include <WiFi.h>

//  Replace with your WiFi credentials:
const char *ssid = "iPhone";        
const char *password = "shimalakas";  

WiFiServer server(80);  // HTTP server on port 80

// Relay and Switch pin assignments
const int relayPins[] = {33, 26, 27};   
const int switchPins[] = {18, 19, 21};  

// Relay states: LOW means relay is ON, HIGH means OFF (assuming active LOW relays)
bool relayStates[3] = {HIGH, HIGH, HIGH};  

// For edge detection on physical (rocker) switches
bool lastSwitchState[3] = {HIGH, HIGH, HIGH};

void setup() {
  Serial.begin(115200);

  // Set up relay pins as output and switch pins as input with pull-up
  for (int i = 0; i < 3; i++) {
    pinMode(relayPins[i], OUTPUT);
    pinMode(switchPins[i], INPUT_PULLUP);
    digitalWrite(relayPins[i], relayStates[i]); // Initialize relay state
  }

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());
  server.begin();  // Start the HTTP server
}

void loop() {
  // Handle physical rocker switch input with edge detection
  for (int i = 0; i < 3; i++) {
    bool currentSwitchState = digitalRead(switchPins[i]);

    // Check if the physical switch state has changed
    if (currentSwitchState != lastSwitchState[i]) {
      if (i == 1) {  
        //  Invert the logic for GPIO 19
        relayStates[i] = (currentSwitchState == HIGH) ? LOW : HIGH; 
      } else {
        relayStates[i] = (currentSwitchState == LOW) ? LOW : HIGH; 
      }

      Serial.printf("Physical switch %d turned %s\n", i + 1, relayStates[i] == LOW ? "ON" : "OFF");

      lastSwitchState[i] = currentSwitchState;  // Update last state
      digitalWrite(relayPins[i], relayStates[i]); // Update relay immediately
    }
  }

  // Handle Web Server Requests
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New Client Connected");
    String request = client.readStringUntil('\r');  // Read the HTTP request
    client.flush();

    //  Fix: Allow web commands to control relays at all times
    if (request.indexOf("/ON1") != -1) {
      relayStates[0] = LOW;
      Serial.println("Web: Relay 1 ON");
    }
    if (request.indexOf("/OFF1") != -1) {
      relayStates[0] = HIGH;
      Serial.println("Web: Relay 1 OFF");
    }
    if (request.indexOf("/ON2") != -1) {
      relayStates[1] = LOW;
      Serial.println("Web: Relay 2 ON");
    }
    if (request.indexOf("/OFF2") != -1) {
      relayStates[1] = HIGH;
      Serial.println("Web: Relay 2 OFF");
    }
    if (request.indexOf("/ON3") != -1) {
      relayStates[2] = LOW;
      Serial.println("Web: Relay 3 ON");
    }
    if (request.indexOf("/OFF3") != -1) {
      relayStates[2] = HIGH;
      Serial.println("Web: Relay 3 OFF");
    }

    // After processing web commands, update all relays
    for (int i = 0; i < 3; i++) {
      digitalWrite(relayPins[i], relayStates[i]);
    }

    // Send a simple HTML page back to the client
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("<html><head>");
    client.println("<style>");
    client.println("body { font-family: Arial, sans-serif; text-align: center; background-color: #f4f4f4; padding: 20px; }");
    client.println(".container { background: white; padding: 30px; border-radius: 10px; display: inline-block; }");
    client.println(".button { font-size: 20px; padding: 10px 20px; margin: 10px; border: none; border-radius: 5px; cursor: pointer; }");
    client.println(".on { background-color: green; color: white; }");
    client.println(".off { background-color: red; color: white; }");
    client.println("</style>");
    client.println("</head><body>");
    client.println("<div class='container'>");
    client.println("<h1>ESP32 Relay Control</h1>");

    for (int i = 0; i < 3; i++) {
      client.print("<p>Relay ");
      client.print(i + 1);
      client.print(" is <b>");
      client.print(relayStates[i] == LOW ? "ON" : "OFF");
      client.println("</b></p>");
      client.print("<a href='/ON");
      client.print(i + 1);
      client.print("'><button class='button on'>Turn ON</button></a>");
      client.print("<a href='/OFF");
      client.print(i + 1);
      client.print("'><button class='button off'>Turn OFF</button></a>");
    }

    client.println("</div></body></html>");
    client.stop();
  }

  delay(50);  // Short debounce delay
}
