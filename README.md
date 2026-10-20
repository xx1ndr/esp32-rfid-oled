# ESP32 RFID OLED

Project IoT berbasis ESP32 untuk mengontrol LED melalui web, membaca UID kartu RFID RC522, dan menampilkan hasilnya pada OLED SSD1306.

## Features

- Wi-Fi configuration menggunakan AutoConnect.
- Web interface untuk kontrol LED.
- REST API dengan autentikasi Bearer token.
- Pembacaan UID kartu RFID RC522.
- Tampilan UID pada OLED SSD1306.
- Remote access menggunakan ngrok.

## Hardware

- ESP32 DevKit
- RFID RC522
- OLED SSD1306 I2C 128×64
- LED bawaan atau LED eksternal
- Kabel USB

## Wiring

| Module | Pin Module | ESP32 |
|---|---|---|
| LED | Signal | GPIO2 |
| RC522 | SDA/SS | GPIO5 |
| RC522 | SCK | GPIO18 |
| RC522 | MOSI | GPIO23 |
| RC522 | MISO | GPIO19 |
| RC522 | RST | GPIO27 |
| RC522 | VCC | 3V3 |
| RC522 | GND | GND |
| OLED | SDA | GPIO21 |
| OLED | SCL | GPIO22 |
| OLED | VCC | 3V3* |
| OLED | GND | GND |

\* Pastikan tegangan sesuai dengan spesifikasi modul. RC522 menggunakan logika 3,3 V.

## Requirements

- [PlatformIO](https://platformio.org/)
- ESP32 Arduino framework

Libraries:
- AutoConnect
- MFRC522
- Adafruit SSD1306
- Adafruit GFX

## Installation

Clone repository:

```bash
git clone https://github.com/xx1ndr/esp32-rfid-oled.git
cd esp32-rfid-oled
```

Build project:

```bash
pio run
```

Upload firmware:

```bash
pio run -t upload
```

Open Serial Monitor:

```bash
pio device monitor -b 115200
```

## Wi-Fi Setup

1. Upload firmware ke ESP32.
2. Hubungkan perangkat ke access point yang dibuat ESP32.
3. SSID default: `ESP32-Setup`
4. Password default: `esp32setup`
5. Buka captive portal dan pilih jaringan Wi-Fi.
6. Setelah terhubung, catat IP ESP32 dari Serial Monitor.

## LED Control

Buka halaman kontrol melalui browser:

```text
http://ESP32_IP/control
```

Masukkan API token untuk mengontrol LED.

## API

Semua endpoint API memerlukan header:

```http
Authorization: Bearer YOUR_API_TOKEN
```

| Method | Endpoint | Description |
|---|---|---|
| GET | `/api/status` | Membaca status LED dan koneksi |
| POST | `/api/led` | Mengontrol LED |

Contoh menyalakan LED menggunakan PowerShell:

```powershell
$headers = @{
    Authorization = "Bearer YOUR_API_TOKEN"
    "Content-Type" = "application/json"
}

Invoke-RestMethod `
    -Uri "http://ESP32_IP/api/led" `
    -Method Post `
    -Headers $headers `
    -Body '{"on":true}'
```

Gunakan `"on":false` untuk mematikan LED.

## Remote Access

Untuk mengakses ESP32 dari internet, jalankan ngrok pada komputer atau Raspberry Pi yang berada di jaringan yang sama:

```bash
ngrok http 192.168.1.50:80
```

Ganti IP tersebut dengan IP ESP32 kamu.

Gunakan URL HTTPS yang diberikan ngrok untuk mengakses `/control` atau endpoint API.

## Security

- Ganti `API_TOKEN` di `src/main.cpp` sebelum digunakan.
- Jangan unggah token asli atau password Wi-Fi ke repository publik.
- Jangan mengekspos captive portal konfigurasi ke internet.
- Hentikan ngrok jika akses remote tidak diperlukan.
- Gunakan autentikasi tambahan jika project diakses secara publik.

## Project Structure

```text
esp32-rfid-oled/
├── platformio.ini
├── src/
│   └── main.cpp
└── README.md
```

## License

Tambahkan file `LICENSE` jika ingin menentukan lisensi penggunaan project ini.
