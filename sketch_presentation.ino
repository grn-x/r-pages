#include <WiFi.h>
#include <WebServer.h>
#include <vector>
#include <map>

const char* ssid = "";
const char* password = "";
const char* consoleUser = "";
const char* consolePass = "";

const int pinGreenLED = 27;
const int pinButtonReset = 32; 
int buttonState = 0;

std::vector<String> taskOneClients;

WebServer server(80);




void handlePing() {
  String json = "{\"status\":\"alive\"}";
  server.send(200, "application/json", json);

  digitalWrite(pinGreenLED, HIGH);
  delay(1000);
  digitalWrite(pinGreenLED, LOW);
}


void task_one() {
  const char* solution = "matchme";
  if (!server.hasArg("param")) {
    server.send(400, "application/json", "{\"error\": \"Missing required parameter 'param'\"}");
    return;
  }

  String paramValue = server.arg("param");
  String clientName = server.hasArg("name") ? server.arg("name") : "anonymous";
  unsigned long timestamp = millis();

  if (paramValue == solution) {
    String logEntry = clientName + " at " + String(timestamp / 1000) + "s";
    taskOneClients.push_back(logEntry);

    server.send(200, "application/json", "{\"result\":true}");
  } else {
    server.send(200, "application/json", "{\"result\":false}");
  }
}



std::map<String, String> correctAnswers = {
  {"c1", "5"},
  {"c2", "active"},
  {"c3", "obj"},
  {"c4", "1"},
  {"c5", "\"ALICE\""},
  {"c6", "8"},
  {"c7", "30"},
  {"c9", "2"},
  {"c10", "\"olleh\""},
  {"c1b", "Equals"},
  {"c2b", "Null"},
  {"c3b", "Same"},
  {"c4b", "ArrayEquals"},
  {"c5b", "Equals"},
  {"c6b", "True"},
  {"c7b", "Equals"},
  {"c9b", "Throws"},
  {"c10b", "False"}
};

std::vector<String> taskAssertionsLog;

void task_assertions() {
  if (!server.hasArg("number") || !server.hasArg("param")) {
    server.send(400, "application/json", "{\"error\": \"Missing required parameter 'number' or 'param'\"}");
    return;
  }

  String number = server.arg("number");
  String paramValue = server.arg("param");
  String clientName = server.hasArg("name") ? server.arg("name") : "anonymous";
  unsigned long timestamp = millis();

  // Check if the number exists
  if (correctAnswers.find(number) == correctAnswers.end()) {
    server.send(404, "application/json", "{\"error\": \"Unknown problem number\"}");
    return;
  }

  String correctAnswer = correctAnswers[number];

  if (paramValue == correctAnswer) {
    String logEntry = clientName + " solved " + number + " at " + String(timestamp / 1000) + "s";
    taskAssertionsLog.push_back(logEntry);
    server.send(200, "application/json", "{\"result\":true}");
  } else {
    server.send(200, "application/json", "{\"result\":false}");
  }
}



std::map<String, String> correctAnswersDebug = {
  {"c1", "cft{solution_one}"},
  {"c2", "secret"},
  {"c3", "552575653"},
  {"c4", "227086596"},
  {"c5", "320200509"}
};

      //cft{solution_one}; c1
      //secret c2
      //end value //552575653 c3
      //find out value at eecallstack: 227086596 c4
      //what would the value be if c callstack was 468 -> 320200509 c5

std::vector<String> taskDebugLog;
void task_debug() {
  if (!server.hasArg("number") || !server.hasArg("param")) {
    server.send(400, "application/json", "{\"error\": \"Missing required parameter 'number' or 'param'\"}");
    return;
  }

  String number = server.arg("number");
  String paramValue = server.arg("param");
  String clientName = server.hasArg("name") ? server.arg("name") : "anonymous";
  unsigned long timestamp = millis();

  // Check if the number exists
  if (correctAnswersDebug.find(number) == correctAnswersDebug.end()) {
    server.send(404, "application/json", "{\"error\": \"Unknown problem number\"}");
    return;
  }

  String correctAnswer = correctAnswersDebug[number];

  if (paramValue == correctAnswer) {
    String logEntry = clientName + " solved " + number + " at " + String(timestamp / 1000) + "s";
    taskDebugLog.push_back(logEntry);
    server.send(200, "application/json", "{\"result\":true}");
  } else {
    server.send(200, "application/json", "{\"result\":false}");
  }
}


void handleWebConsole() {
  if (!server.authenticate(consoleUser, consolePass)) {
    return server.requestAuthentication();
  }

  String html = "<!DOCTYPE html><html><head><meta http-equiv=\"refresh\" content=\"10\">";
  html += "<title>Admin Console</title></head><body>";
  
  html += "<h1>Task One Completions</h1><ul>";
  for (String entry : taskOneClients) {
    html += "<li>" + entry + "</li>";
  }
  html += "</ul>";

  html += "<h1>Assertion Task Log</h1><ul>";
  for (String entry : taskAssertionsLog) {
    html += "<li>" + entry + "</li>";
  }
  html += "</ul>";

  html += "</body></html>";

  server.send(200, "text/html", html);
}



void handleResetButton(){
  buttonState = digitalRead(pinButtonReset);
  if(buttonState ==LOW){
    delay(200);  // debounce delay
    Serial.println("Reset-Button pressed");
    ESP.restart();
  }
}

void setup() {
  Serial.begin(115200);

  Serial.println("Setup Executed");
  pinMode(pinGreenLED, OUTPUT);
  digitalWrite(pinGreenLED, HIGH);
  //for led
  pinMode(pinButtonReset, INPUT_PULLUP);


  WiFi.begin(ssid, password);
  int wifiDisconnectCounter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    wifiDisconnectCounter ++;
    delay(500);
    Serial.println("Connecting...");
    Serial.print(WiFi.status());

    handleResetButton();

    if(wifiDisconnectCounter > 10){
      wifiDisconnectCounter = 0;
      Serial.println("Resetting wifi");
        WiFi.disconnect();
        WiFi.begin(ssid, password);

    }

  }

  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());
  server.enableCORS();
  server.on("/ping", handlePing);
  server.on("/task_one", task_one);
    server.on("/login", handleWebConsole);
    server.on("/assert", task_assertions);
    server.on("/debug", task_debug());

  server.begin();
}

void loop() {
  server.handleClient();
  handleResetButton();
}

