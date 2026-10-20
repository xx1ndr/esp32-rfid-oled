#include <Arduino.h>
#include <WebServer.h>
#include <AutoConnect.h>
#include <MFRC522.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

namespace {
// Wiring ESP32 DevKit:
// LED built-in: GPIO2
// RC522 (SPI): SDA/SS=GPIO5, SCK=GPIO18, MOSI=GPIO23, MISO=GPIO19, RST=GPIO27, VCC=3V3, GND=GND
// OLED SSD1306 (I2C): SDA=GPIO21, SCL=GPIO22, address=0x3C
// RC522 signals and power must use 3.3V logic; do not connect it to 5V.
constexpr uint8_t LED_PIN = 2;
constexpr char API_TOKEN[] = "123";
constexpr uint8_t RFID_SS_PIN = 5;
constexpr uint8_t RFID_RST_PIN = 27;
constexpr uint8_t OLED_WIDTH = 128;
constexpr uint8_t OLED_HEIGHT = 64;
constexpr uint8_t OLED_ADDRESS = 0x3C;

WebServer server(80);
AutoConnect portal(server);
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
bool ledOn = false;
bool displayReady = false;

void showCardId(const String& cardId) {
    if (!displayReady) return;

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Kartu terdeteksi");
    display.println("ID:");
    display.println(cardId);
    display.display();
}

void readRfidCard() {
    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

    String cardId;
    for (byte i = 0; i < rfid.uid.size; ++i) {
        if (i > 0) cardId += ':';
        if (rfid.uid.uidByte[i] < 0x10) cardId += '0';
        cardId += String(rfid.uid.uidByte[i], HEX);
    }
    cardId.toUpperCase();

    Serial.print("ID kartu: ");
    Serial.println(cardId);
    showCardId(cardId);

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
}

bool isAuthorized() {
    String authorization = server.header("Authorization");
    return authorization == String("Bearer ") + API_TOKEN;
}

void sendUnauthorized() {
    server.send(401, "application/json", "{\"error\":\"unauthorized\"}");
}

void handleControlPage() {
    static const char page[] PROGMEM = R"HTML(
<!doctype html><html lang="id"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 LED</title>
<style>
body{font-family:system-ui,sans-serif;max-width:32rem;margin:3rem auto;padding:0 1rem;background:#eef2f3;color:#172126}
main{background:#fff;padding:2rem;border-radius:12px;box-shadow:0 8px 24px #17212622}
button,input{font:inherit;padding:.7rem;margin:.25rem 0;border:1px solid #b7c2c5;border-radius:6px}
button{cursor:pointer;background:#176b87;color:#fff;border:0;min-width:8rem}button.off{background:#b33a3a}
input{box-sizing:border-box;width:100%}#status{margin:1rem 0;font-weight:600}
</style></head><body><main><h1>ESP32 LED</h1>
<label>API token<input id="token" type="password" autocomplete="off"></label>
<p id="status">Status belum dibaca.</p>
<button onclick="setLed(true)">Nyalakan</button>
<button class="off" onclick="setLed(false)">Matikan</button>
<script>
const token=document.querySelector('#token');
async function request(path,options={}){options.headers={...(options.headers||{}),Authorization:'Bearer '+token.value};const r=await fetch(path,options);if(!r.ok)throw Error(await r.text());return r.json()}
async function refresh(){try{const s=await request('/api/status');document.querySelector('#status').textContent=`LED: ${s.led?'ON':'OFF'} | Wi-Fi: ${s.ip}`}catch(e){document.querySelector('#status').textContent='Token salah atau ESP32 tidak dapat dihubungi.'}}
async function setLed(value){try{await request('/api/led',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({on:value})});refresh()}catch(e){alert('Perintah ditolak. Periksa token.')}}
token.addEventListener('change',refresh);
</script></main></body></html>)HTML";
    server.send(200, "text/html; charset=utf-8", page);
}

void handleStatus() {
    if (!isAuthorized()) { sendUnauthorized(); return; }
    String json = String("{\"led\":") + (ledOn ? "true" : "false") +
                                ",\"ip\":\"" + WiFi.localIP().toString() + "\"}";
    server.send(200, "application/json", json);
}

void handleLed() {
    if (!isAuthorized()) { sendUnauthorized(); return; }
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"JSON body required\"}");
        return;
    }

    String body = server.arg("plain");
    if (body.indexOf("\"on\":true") >= 0) ledOn = true;
    else if (body.indexOf("\"on\":false") >= 0) ledOn = false;
    else {
        server.send(400, "application/json", "{\"error\":\"on must be boolean\"}");
        return;
    }

    digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
    server.send(200, "application/json", ledOn ? "{\"led\":true}" : "{\"led\":false}");
}
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    const char* headerKeys[] = {"Authorization"};
    server.collectHeaders(headerKeys, 1);

    server.on("/control", HTTP_GET, handleControlPage);
    server.on("/api/status", HTTP_GET, handleStatus);
    server.on("/api/led", HTTP_POST, handleLed);

    displayReady = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
    if (displayReady) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.println("Tempelkan kartu");
        display.println("RFID...");
        display.display();
    } else {
        Serial.println("Gagal menginisialisasi OLED. Periksa kabel dan alamat I2C.");
    }

    SPI.begin();
    rfid.PCD_Init();
    const byte rfidVersion = rfid.PCD_ReadRegister(rfid.VersionReg);
    if (rfidVersion == 0x00 || rfidVersion == 0xFF) {
        Serial.println("Gagal menginisialisasi RFID RC522. Periksa kabel dan catu daya.");
    } else {
        Serial.println("RFID RC522 siap. Tempelkan kartu.");
    }

    AutoConnectConfig config;
    config.apid = "ESP32-Setup";
    config.psk = "esp32setup";
    config.autoReconnect = true;
    config.reconnectInterval = 10;
    portal.config(config);

    if (portal.begin()) {
        Serial.print("Wi-Fi connected. Control page: http://");
        Serial.print(WiFi.localIP());
        Serial.println("/control");
    } else {
        Serial.println("Wi-Fi setup failed; restarting...");
        delay(3000);
        ESP.restart();
    }
}

void loop() {
    portal.handleClient();
    readRfidCard();
}