#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"
#include "config.h"   // тут зберігаються SSID/PASSWORD, файл не додається в git

// ==== URL потоку інтернет-радіо ====
const char* radioURL = "http://ice1.somafm.com/groovesalad-128-mp3";

Audio audio;

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

    // Використовуємо внутрішній DAC ESP32 (GPIO25 і GPIO26)
    audio.setInternalDAC(true);    // вмикає вбудований DAC (потребує library v2.0.0)
    audio.setVolume(15);           // 0...21

    audio.connecttohost(radioURL);
}

void loop() {
    audio.loop();
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
