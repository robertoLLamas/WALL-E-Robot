#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
//#include <DFRobotDFPlayerMini.h>

// ==============================
// WiFi Settings
// ==============================
const char* ssid = "WALL-E";
const char* password = "wallefriend";

WebServer server(80);

// ==============================
// OLED Display
// ==============================
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// ==============================
// Motor Pins (L298N)
// ==============================
int LMotorForward = 14;
int LMotorBack    = 27;
int RMotorForward = 26;
int RMotorBack    = 25;

// ==============================
// Servos
// ==============================
Servo headTilt;
Servo headSwivel;
Servo chestServo;
int headTiltPin   = 13;
int headSwivelPin = 12;
int chestServoPin = 4;

// ==============================
// DFPlayer (disabled until SD card arrives)
// ==============================
//DFRobotDFPlayerMini player;

// ==============================
// Display State
// ==============================
String lastLine1 = "";
String lastLine2 = "";
bool isIdle = true;
unsigned long idleDisplayTime = 0;

// ==============================
// Battery Animation
// ==============================
int batteryLevel = 0;
unsigned long lastAnimTime = 0;

void showBatteryAnimation() {
  if (millis() - lastAnimTime < 150) return;
  lastAnimTime = millis();

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(35, 0);
  display.println("WALL-E");

  display.drawRect(14, 20, 100, 30, WHITE);
  display.fillRect(114, 27, 6, 16, WHITE);

  int fillWidth = map(batteryLevel, 0, 100, 0, 96);
  display.fillRect(16, 22, fillWidth, 26, WHITE);

  display.setTextSize(1);
  display.setCursor(50, 56);
  display.print(batteryLevel);
  display.print("%");

  display.display();

  batteryLevel += 3;
  if (batteryLevel > 100) batteryLevel = 0;
}

// ==============================
// Show Message
// ==============================
void showMessage(String line1, String line2 = "") {
  if (line1 == lastLine1 && line2 == lastLine2) return;

  lastLine1 = line1;
  lastLine2 = line2;
  isIdle = false;
  idleDisplayTime = millis();

  display.clearDisplay();
  display.setTextColor(WHITE);

  display.setTextSize(2);
  display.setCursor(0, 10);
  display.println(line1);

  if (line2 != "") {
    display.setTextSize(1);
    display.setCursor(0, 45);
    display.println(line2);
  }
  display.display();
}

// ==============================
// Web Interface HTML
// ==============================
void handleRoot() {
  String html = "";
  html += "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body{background:#1a1a1a;display:flex;flex-direction:column;align-items:center;justify-content:center;min-height:100vh;margin:0;font-family:Arial;color:white;padding:20px;}";
  html += "h1{color:#f5a623;margin-bottom:10px;}";
  html += "p{color:#888;font-size:13px;margin-bottom:10px;}";
  html += ".grid{display:grid;grid-template-columns:repeat(3,100px);grid-template-rows:repeat(3,80px);gap:8px;margin-bottom:20px;}";
  html += ".head-grid{display:grid;grid-template-columns:repeat(3,100px);grid-template-rows:repeat(3,60px);gap:8px;margin-bottom:20px;}";
  html += ".sound-grid{display:grid;grid-template-columns:repeat(2,140px);gap:8px;margin-bottom:20px;}";
  html += ".chest-grid{display:grid;grid-template-columns:repeat(2,140px);gap:8px;margin-bottom:20px;}";
  html += "button{background:#f5a623;border:none;border-radius:10px;color:black;font-size:13px;font-weight:bold;cursor:pointer;width:100%;height:100%;padding:8px;}";
  html += "button:active{background:#c47d0e;}";
  html += ".sound-btn{height:55px;background:#2a9d8f;color:white;}";
  html += ".stop-btn{background:#e63946;color:white;}";
  html += ".chest-btn{height:55px;background:#7b2d8b;color:white;}";
  html += ".empty{background:transparent;border:none;pointer-events:none;}";
  html += "</style></head><body>";
  html += "<h1>WALL-E</h1>";

  html += "<p>Movement</p>";
  html += "<div class='grid'>";
  html += "<div class='empty'></div>";
  html += "<button ontouchstart=\"fetch('/forward')\" ontouchend=\"fetch('/stop')\">Forward</button>";
  html += "<div class='empty'></div>";
  html += "<button ontouchstart=\"fetch('/left')\" ontouchend=\"fetch('/stop')\">Left</button>";
  html += "<button class='stop-btn' onclick=\"fetch('/stop')\">STOP</button>";
  html += "<button ontouchstart=\"fetch('/right')\" ontouchend=\"fetch('/stop')\">Right</button>";
  html += "<div class='empty'></div>";
  html += "<button ontouchstart=\"fetch('/backward')\" ontouchend=\"fetch('/stop')\">Back</button>";
  html += "<div class='empty'></div>";
  html += "</div>";

  html += "<p>Head</p>";
  html += "<div class='head-grid'>";
  html += "<div class='empty'></div>";
  html += "<button onclick=\"fetch('/headup')\">Up</button>";
  html += "<div class='empty'></div>";
  html += "<button onclick=\"fetch('/headleft')\">Left</button>";
  html += "<button onclick=\"fetch('/headcenter')\">Center</button>";
  html += "<button onclick=\"fetch('/headright')\">Right</button>";
  html += "<div class='empty'></div>";
  html += "<button onclick=\"fetch('/headdown')\">Down</button>";
  html += "<div class='empty'></div>";
  html += "</div>";

  html += "<p>Chest</p>";
  html += "<div class='chest-grid'>";
  html += "<button class='chest-btn' onclick=\"fetch('/chestopen')\">Open</button>";
  html += "<button class='chest-btn' onclick=\"fetch('/chestclose')\">Close</button>";
  html += "</div>";

  html += "<p>Sounds</p>";
  html += "<div class='sound-grid'>";
  html += "<button class='sound-btn' onclick=\"fetch('/sound1')\">WALL-E!</button>";
  html += "<button class='sound-btn' onclick=\"fetch('/sound2')\">Eeeevaaaa</button>";
  html += "<button class='sound-btn' onclick=\"fetch('/sound3')\">Happy</button>";
  html += "<button class='sound-btn' onclick=\"fetch('/sound4')\">Sad</button>";
  html += "<button class='sound-btn' onclick=\"fetch('/sound5')\">Startup</button>";
  html += "<button class='sound-btn' onclick=\"fetch('/sound6')\">Hello Dolly</button>";
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}

// ==============================
// Movement Functions
// ==============================
void moveForward() {
  digitalWrite(LMotorForward, HIGH);
  digitalWrite(LMotorBack,    LOW);
  digitalWrite(RMotorForward, HIGH);
  digitalWrite(RMotorBack,    LOW);
  //player.play(7);
  showMessage("WALL-E", "Moving Fwd");
  server.send(200, "text/plain", "OK");
}

void moveBackward() {
  digitalWrite(LMotorForward, LOW);
  digitalWrite(LMotorBack,    HIGH);
  digitalWrite(RMotorForward, LOW);
  digitalWrite(RMotorBack,    HIGH);
  //player.play(7);
  showMessage("WALL-E", "Moving Back");
  server.send(200, "text/plain", "OK");
}

void turnLeft() {
  digitalWrite(LMotorForward, LOW);
  digitalWrite(LMotorBack,    HIGH);
  digitalWrite(RMotorForward, HIGH);
  digitalWrite(RMotorBack,    LOW);
  //player.play(8);
  showMessage("WALL-E", "Turning Left");
  server.send(200, "text/plain", "OK");
}

void turnRight() {
  digitalWrite(LMotorForward, HIGH);
  digitalWrite(LMotorBack,    LOW);
  digitalWrite(RMotorForward, LOW);
  digitalWrite(RMotorBack,    HIGH);
  //player.play(8);
  showMessage("WALL-E", "Turning Right");
  server.send(200, "text/plain", "OK");
}

void stopMoving() {
  digitalWrite(LMotorForward, LOW);
  digitalWrite(LMotorBack,    LOW);
  digitalWrite(RMotorForward, LOW);
  digitalWrite(RMotorBack,    LOW);
  //player.stop();
  isIdle = true;
  lastLine1 = "";
  lastLine2 = "";
  server.send(200, "text/plain", "OK");
}

// ==============================
// Head Servo Functions
// ==============================
void headLeft() {
  headSwivel.write(45);
  //player.play(9);
  showMessage("WALL-E", "Looking Left");
  server.send(200, "text/plain", "OK");
}

void headRight() {
  headSwivel.write(135);
  //player.play(9);
  showMessage("WALL-E", "Looking Right");
  server.send(200, "text/plain", "OK");
}

void headUp() {
  headTilt.write(45);
  showMessage("WALL-E", "Looking Up");
  server.send(200, "text/plain", "OK");
}

void headDown() {
  headTilt.write(135);
  showMessage("WALL-E", "Looking Down");
  server.send(200, "text/plain", "OK");
}

void headCenter() {
  headSwivel.write(90);
  headTilt.write(90);
  showMessage("WALL-E", "Head Center");
  server.send(200, "text/plain", "OK");
}

// ==============================
// Chest Functions
// ==============================
void chestOpen() {
  chestServo.write(180);
  //player.play(3);
  showMessage("WALL-E", "Chest Open!");
  server.send(200, "text/plain", "OK");
}

void chestClose() {
  chestServo.write(0);
  showMessage("WALL-E", "Chest Closed");
  server.send(200, "text/plain", "OK");
}

// ==============================
// Sound Functions (disabled until SD card arrives)
// ==============================
void sound1() { /*player.play(1);*/ showMessage("WALL-E!", ""); server.send(200, "text/plain", "OK"); }
void sound2() { /*player.play(2);*/ showMessage("Eeevaaaa", ""); server.send(200, "text/plain", "OK"); }
void sound3() { /*player.play(3);*/ showMessage("WALL-E", "Happy :)"); server.send(200, "text/plain", "OK"); }
void sound4() { /*player.play(4);*/ showMessage("WALL-E", "Sad :("); server.send(200, "text/plain", "OK"); }
void sound5() { /*player.play(5);*/ showMessage("WALL-E", "Startup!"); server.send(200, "text/plain", "OK"); }
void sound6() { /*player.play(6);*/ showMessage("Hello", "Dolly!"); server.send(200, "text/plain", "OK"); }

// ==============================
// Setup
// ==============================
void setup() {
  Serial.begin(115200);
  delay(1000);

  // OLED
  Wire.begin(21, 22);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 20);
  display.println("WALL-E");
  display.setTextSize(1);
  display.setCursor(20, 45);
  display.println("Starting up...");
  display.display();
  delay(2000);

  // Motor pins
  pinMode(LMotorForward, OUTPUT);
  pinMode(LMotorBack,    OUTPUT);
  pinMode(RMotorForward, OUTPUT);
  pinMode(RMotorBack,    OUTPUT);
  digitalWrite(LMotorForward, LOW);
  digitalWrite(LMotorBack,    LOW);
  digitalWrite(RMotorForward, LOW);
  digitalWrite(RMotorBack,    LOW);

  // Servos staggered startup
  headTilt.attach(headTiltPin);
  headTilt.write(90);
  delay(500);
  headSwivel.attach(headSwivelPin);
  headSwivel.write(90);
  delay(500);
  chestServo.attach(chestServoPin);
  chestServo.write(0);
  delay(500);

  // DFPlayer (disabled until SD card arrives)
  //Serial2.begin(9600, SERIAL_8N1, 16, 17);
  //player.begin(Serial2);
  //player.volume(20);

  // WiFi
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(
    IPAddress(192,168,4,1),
    IPAddress(192,168,4,1),
    IPAddress(255,255,255,0)
  );

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 20);
  display.println("WALL-E");
  display.setTextSize(1);
  display.setCursor(20, 45);
  display.println("WiFi Ready!");
  display.display();
  delay(1000);

  // Routes
  server.on("/",           handleRoot);
  server.on("/forward",    moveForward);
  server.on("/backward",   moveBackward);
  server.on("/left",       turnLeft);
  server.on("/right",      turnRight);
  server.on("/stop",       stopMoving);
  server.on("/headleft",   headLeft);
  server.on("/headright",  headRight);
  server.on("/headup",     headUp);
  server.on("/headdown",   headDown);
  server.on("/headcenter", headCenter);
  server.on("/chestopen",  chestOpen);
  server.on("/chestclose", chestClose);
  server.on("/sound1",     sound1);
  server.on("/sound2",     sound2);
  server.on("/sound3",     sound3);
  server.on("/sound4",     sound4);
  server.on("/sound5",     sound5);
  server.on("/sound6",     sound6);

  server.begin();

  isIdle = true;
  lastLine1 = "";
  lastLine2 = "";
  batteryLevel = 0;
}

// ==============================
// Loop
// ==============================
void loop() {
  server.handleClient();

  if (!isIdle && millis() - idleDisplayTime > 3000) {
    isIdle = true;
    lastLine1 = "";
    lastLine2 = "";
  }

  if (isIdle) {
    showBatteryAnimation();
  }
}