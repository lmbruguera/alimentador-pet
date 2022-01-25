#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <TimeLib.h>
#include <Servo.h>
#include <LittleFS.h>
#include <ArduinoJson.h>


Servo servo;
int pos = 0;

const size_t JSON_SIZE = JSON_OBJECT_SIZE(5);

const byte WEBSERVER_PORT = 80;
const char* WEBSERVER_HEADER_KEYS[] = {"User-Agent"};

ESP8266WebServer  server(WEBSERVER_PORT);


struct Data {
  char ssid[50];
  char pass[50];
  int time1;
  int time2;
  int time3;
};

Data data;

boolean configLoad() {
  File file = LittleFS.open("/data.txt", "r");
  if (!file) {
    Serial.println(F("File not found!"));
    return false;
  }

  StaticJsonDocument<192> doc;

  DeserializationError error = deserializeJson(doc, file);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return false;
  }

  strlcpy(data.ssid, doc["ssid"] | "MY NETWORK SSID", sizeof(data.ssid));
  strlcpy(data.pass, doc["pass"] | "MY NETWORK PASS", sizeof(data.pass));
  data.time1 = doc["time1"];
  data.time2 = doc["time2"];
  data.time3 = doc["time3"];

  return true;

}

boolean configSave() {
  File file = LittleFS.open("/data.txt", "r");
  if (!file) {
    Serial.println(F("File not found!"));
    return false;
  }

  StaticJsonDocument<128> doc;

  doc["ssid"] = data.ssid;
  doc["pass"] = data.pass;
  doc["time1"] = data.time1;
  doc["time2"] = data.time2;
  doc["time3"] = data.time3;

  serializeJson(doc, file);
  file.close();

  return true;
}


void handleHome() {
  File file = LittleFS.open(F("index.html"), "r");
  if (!file.isFile()) {
    Serial.println("index.html file not found");
    return;
  }
  if (file) {
    file.setTimeout(500);
    String s = file.readString();
    file.close();

    s.replace(F("#ssid"), WiFi.status() == WL_CONNECTED ? data.ssid : "PET-FEED");
    s.replace(F("#time1"), String(data.time1));
    s.replace(F("#time2"), String(data.time2));
    s.replace(F("#time3"), String(data.time3));

    server.send(200, F("text/html"), s);
  } else {
    server.send(500, F("text/plain"), F("Home - Error 500"));
  }
}

void handleConfigSave() {

  if (server.args() == 6) {


    String s;

    s = server.arg("ssid");
    s.trim();
    if ( s != data.ssid) {
      s.toCharArray(data.ssid, s.length() + 1);
    }

    s = server.arg("pass");
    s.trim();
    if ( s != "") {
      s.toCharArray(data.pass, s.length() + 1);
    }

    s = server.arg("time1");
    s.trim();
    if ( s != String(data.time1)) {
      data.time1 = s.toInt();
    }

    s = server.arg("time2");
    s.trim();
    if ( s != String(data.time2)) {
      data.time2 = s.toInt();
    }

    s = server.arg("time3");
    s.trim();
    if ( s != String(data.time3)) {
      data.time3 = s.toInt();
    }

    if (configSave()) {
      server.send(200, F("text/html"), F("<html><meta charset='UTF-8'><script>alert('Config save Success.');history.back()</script></html>"));
      configLoad();
    } else {
      server.send(200, F("text/html"), F("<html><meta charset='UTF-8'><script>alert('Fail to save.');history.back()</script></html>"));
      Serial.println(F("ConfigSave - ERRO salvando Config"));
    }
  } else {
    server.send(200, F("text/html"), F("<html><meta charset='UTF-8'><script>alert('Param error');history.back()</script></html>"));
  }
}


void handleReboot() {
  // Reboot
  ESP.restart();

  server.send(200, F("text/html"), F("<html><meta charset='UTF-8'><script>alert('Rebooting.');window.location = '/';history.back()</script></html>"));
}

void setup() {

  Serial.begin(74880);
  Serial.println("Serial begun");

  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
  }
  configLoad();

  WiFi.hostname("PET-FEEDER");
  WiFi.softAP("PET-FEED", "12345678");

  WiFi.begin(data.ssid, data.pass);

  byte b = 0;
  while (WiFi.status() != WL_CONNECTED && b < 60) {
    b++;
    Serial.print(".");
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Wifi connected - IP: ");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.println("Wifi not connected");
    Serial.print("SOFTAP IP: ");
    Serial.println(WiFi.softAPIP());
  }

  servo.attach(2);

  server.on(F("/index.html"), handleHome);
            server.on(F("/configSave"), handleConfigSave);
            server.on(F("/reboot"), handleReboot);
            server.onNotFound(handleHome);
            server.collectHeaders(WEBSERVER_HEADER_KEYS, 1);
            server.begin();

}

void loop() {

  yield();

  

}
