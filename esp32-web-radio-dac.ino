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
String nowPlaying = "---";
bool isPlaying = true;

Audio audio(true, I2S_DAC_CHANNEL_BOTH_EN);   // true = internal DAC, both channels
WebServer server(80);

// ==== HTML-сторінка веб-інтерфейсу (стиль на кшталт yoRadio) ====
String buildPage() {
    String html = "<!DOCTYPE html><html><head><meta charset='utf-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<meta http-equiv='refresh' content='5'>";
    html += "<title>ESP32 Web Radio</title>";
    html += "<style>";
    html += "*{box-sizing:border-box;}";
    html += "body{font-family:'Segoe UI',sans-serif;background:linear-gradient(135deg,#0f0f1a,#1a1a2e);";
    html += "color:#eee;text-align:center;padding:0;margin:0;min-height:100vh;}";
    html += ".card{max-width:420px;margin:0 auto;padding:24px 16px;}";
    html += "h1{font-size:1.1em;letter-spacing:2px;color:#7fdbca;text-transform:uppercase;margin-bottom:4px;}";
    html += ".now{background:#20203a;border-radius:14px;padding:18px;margin:16px 0;";
    html += "box-shadow:0 4px 20px rgba(0,0,0,0.4);}";
    html += ".now .label{font-size:0.75em;color:#7fdbca;letter-spacing:1px;text-transform:uppercase;}";
    html += ".now .track{font-size:1.15em;margin-top:6px;min-height:1.4em;word-wrap:break-word;}";
    html += ".stations{margin-top:20px;}";
    html += "button.station{display:flex;align-items:center;justify-content:space-between;";
    html += "width:100%;margin:8px 0;padding:14px 18px;font-size:1em;border:1px solid #333;";
    html += "border-radius:12px;background:#20203a;color:#eee;transition:0.2s;}";
    html += "button.station.active{background:linear-gradient(135deg,#7fdbca,#3fa796);color:#0f0f1a;font-weight:bold;border:none;}";
    html += ".vol-row{display:flex;align-items:center;gap:12px;margin-top:26px;}";
    html += ".vol-row span{font-size:1.3em;}";
    html += "input[type=range]{flex:1;height:6px;-webkit-appearance:none;background:#333;border-radius:3px;outline:none;}";
    html += "input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:20px;height:20px;";
    html += "border-radius:50%;background:#7fdbca;cursor:pointer;}";
    html += "</style></head><body>";
    html += "<div class='card'>";
    html += "<h1>ESP32 Web Radio</h1>";

    html += "<div class='now'>";
    html += "<div class='label'>Зараз грає</div>";
    html += "<div class='track'>" + String(stations[currentStation].name) + "</div>";
    html += "<div class='track' style='font-size:0.85em;opacity:0.7;'>" + nowPlaying + "</div>";
    html += "</div>";

    html += "<div class='stations'>";
    for (int i = 0; i < 4; i++) {
        html += "<button class='station " + String(i == currentStation ? "active" : "") + "' ";
        html += "onclick=\"fetch('/station?id=" + String(i) + "').then(()=>location.reload())\">";
        html += "<span>" + String(stations[i].name) + "</span>";
        html += "<span>" + String(i == currentStation ? "\u25B6" : "") + "</span>";
        html += "</button>";
    }
    html += "</div>";

    html += "<div class='vol-row'>";
    html += "<span>&#128265;</span>";
    html += "<input type='range' min='0' max='21' value='" + String(currentVolume) + "' ";
    html += "onchange=\"fetch('/volume?v='+this.value)\">";
    html += "<span>&#128266;</span>";
    html += "</div>";

    html += "</div></body></html>";
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
            nowPlaying = "---";
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

// Проста сторінка статусу для AJAX-опитування (без перезавантаження, опційно)
void handleNowPlaying() {
    server.send(200, "text/plain", nowPlaying);
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
    server.on("/nowplaying", handleNowPlaying);
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
    nowPlaying = String(info);
}
