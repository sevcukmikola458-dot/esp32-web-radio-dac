#include "Arduino.h"
#include "WiFi.h"
#include "WebServer.h"
#include "Audio.h"
#include "config.h"   // тут зберігаються SSID/PASSWORD, файл не додається в git

// ==== Список радіостанцій (можна змінити URL-адреси на свої) ====
struct Station {
    const char* name;
    const char* url;
};

Station stations[4] = {
    {"SomaFM Groove Salad",  "http://ice1.somafm.com/groovesalad-128-mp3"},
    {"SomaFM Drone Zone",    "http://ice1.somafm.com/dronezone-128-mp3"},
    {"SomaFM Indie Pop Rocks", "http://ice1.somafm.com/indiepop-128-mp3"},
    {"SomaFM Beat Blender",  "http://ice1.somafm.com/beatblender-128-mp3"}
};

int currentStation = 0;
int currentVolume = 15;   // 0...21

Audio audio(true, I2S_DAC_CHANNEL_BOTH_EN);   // true = internal DAC, both channels
WebServer server(80);

// ==== HTML-сторінка веб-інтерфейсу ====
String buildPage() {
    String html = "<!DOCTYPE html><html><head><meta charset='utf-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>ESP32 Web Radio</title>";
    html += "<style>";
    html += "body{font-family:sans-serif;background:#1a1a1a;color:#eee;text-align:center;padding:20px;}";
    html += "h1{font-size:1.4em;}";
    html += "button{display:block;width:90%;margin:8px auto;padding:14px;font-size:1em;";
    html += "border:none;border-radius:8px;background:#333;color:#eee;}";
    html += "button.active{background:#4caf50;}";
    html += "input[type=range]{width:90%;margin:20px 0;}";
    html += "</style></head><body>";
    html += "<h1>ESP32 Web Radio</h1>";

    for (int i = 0; i < 4; i++) {
        html += "<button class='" + String(i == currentStation ? "active" : "") + "' ";
        html += "onclick=\"fetch('/station?id=" + String(i) + "').then(()=>location.reload())\">";
        html += String(stations[i].name) + "</button>";
    }

    html += "<p>Гучність: <span id='volLabel'>" + String(currentVolume) + "</span></p>";
    html += "<input type='range' min='0' max='21' value='" + String(currentVolume) + "' ";
    html += "oninput=\"document.getElementById('volLabel').innerText=this.value\" ";
    html += "onchange=\"fetch('/volume?v='+this.value)\">";

    html += "</body></html>";
    return html;
}

void handleRoot() {
    server.send(200, "text/html", buildPage());
}

void handleStation() {
    if (server.hasArg("id")) {
        int id = server.arg("id").toInt();
        if (id >= 0 && id < 4) {
            currentStation = id;
            audio.connecttohost(stations[currentStation].url);
        }
    }
    server.send(200, "text/plain", "OK");
}

void handleVolume() {
    if (server.hasArg("v")) {
        int v = server.arg("v").toInt();
        if (v >= 0 && v <= 21) {
            currentVolume = v;
            audio.setVolume(currentVolume);
        }
    }
    server.send(200, "text/plain", "OK");
}

void setup() {
    Serial.begin(115200);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Підключення до WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(300);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.println("Відкрийте веб-інтерфейс за цією IP-адресою в браузері");

    audio.setVolume(currentVolume);
    audio.connecttohost(stations[currentStation].url);

    server.on("/", handleRoot);
    server.on("/station", handleStation);
    server.on("/volume", handleVolume);
    server.begin();
}

void loop() {
    audio.loop();
    server.handleClient();
}

void audio_info(const char *info) {
    Serial.print("info        "); Serial.println(info);
}
void audio_id3data(const char *info) {
    Serial.print("id3data     "); Serial.println(info);
}
void audio_showstation(const char *info) {
    Serial.print("station     "); Serial.println(info);
}
void audio_showstreamtitle(const char *info) {
    Serial.print("streamtitle "); Serial.println(info);
}
