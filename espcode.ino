#include <WiFi.h>
#include <ESPAsyncWebSrv.h>
#include <DNSServer.h>
#include <AsyncTCP.h>

// WiFi Access Point
const char* esp_ssid = "ESP32-Trigger";

// Pin für das Airbag-Signal (z.B. GPIO 18)
int airbagPin = 18;

// ESP32 Webserver
DNSServer dnsServer;
AsyncWebServer server(80);

// HTML-Seite für das Captive Portal
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Airbag Trigger</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      padding: 20px;
    }
    button {
      padding: 15px 30px;
      font-size: 20px;
      background-color: #4CAF50;
      color: white;
      border: none;
      cursor: pointer;
    }
    button:hover {
      background-color: #45a049;
    }
  </style>
</head>
<body>
  <h1>ESP32 Airbag Trigger</h1>
  <p>Click the button below to trigger the airbag signal:</p>
  <button onclick="triggerAirbag()">Trigger Airbag</button>

  <script>
    function triggerAirbag() {
      fetch('/trigger', { method: 'POST' })
        .then(response => response.text())
        .then(data => {
          alert(data);  // Show feedback message
        });
    }
  </script>
</body>
</html>
)rawliteral";

// Setup für den Webserver
void setupServer() {
  // Hauptseite (GET-Anfrage)
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html);
  });

  // Trigger-Auslösung (POST-Anfrage)
  server.on("/trigger", HTTP_POST, [](AsyncWebServerRequest* request){
    // Hier wird der Airbag ausgelöst (GPIO-Pin aktiviert)
    digitalWrite(airbagPin, HIGH);  // Signal zum Auslösen des Airbags
    delay(100);                     // Airbag bleibt für eine kurze Zeit aktiv
    digitalWrite(airbagPin, LOW);   // Rücksetzen des Signals

    // Rückmeldung an den Client
    request->send(200, "text/plain", "Airbag ausgelöst!");
  });
}

void setup() {
  // Serielle Kommunikation starten
  Serial.begin(115200);

  // Airbag-Ausgangspins festlegen
  pinMode(airbagPin, OUTPUT);
  digitalWrite(airbagPin, LOW);  // Starten mit "aus" (LOW)

  // ESP32 im Access-Point-Modus einrichten
  WiFi.mode(WIFI_AP);
  WiFi.softAP(esp_ssid);
  Serial.println("ESP32 Webserver gestartet");

  // IP-Adresse des ESP32 ausgeben
  Serial.print("ESP32 IP-Adresse: ");
  Serial.println(WiFi.localIP());

  // Webserver einrichten
  setupServer();

  // DNS-Server starten (für Captive Portal)
  Serial.println("Starte DNS-Server...");
  dnsServer.start(53, "*", WiFi.softAPIP());

  // Webserver starten
  server.begin();
  Serial.println("Webserver läuft!");
}

void loop() {
  // DNS-Anfragen verarbeiten (wird für das Captive Portal benötigt)
  dnsServer.processNextRequest();
}
