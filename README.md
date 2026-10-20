# ESP32 Wi-Fi LED Control

Project IoT sederhana berbasis ESP32 untuk mengatur LED melalui halaman web. Konfigurasi Wi-Fi dilakukan menggunakan library AutoConnect, sedangkan kontrol LED tersedia melalui web server ESP32.

## Fitur

- Captive portal AutoConnect untuk konfigurasi Wi-Fi.
- Scan jaringan Wi-Fi dan pemilihan SSID melalui portal.
- Penyimpanan konfigurasi Wi-Fi oleh AutoConnect.
- Halaman kontrol LED pada `/control`.
- API status LED dan koneksi Wi-Fi.
- API untuk menyalakan dan mematikan LED.
- Pembacaan UID kartu RFID RC522 dan penampilannya pada OLED SSD1306.
- Proteksi API menggunakan Bearer token.
- Dapat diakses dari internet menggunakan ngrok yang berjalan pada komputer atau Raspberry Pi di jaringan yang sama.

## Perangkat Keras

- ESP32 DevKit.
- LED bawaan board pada GPIO2, atau LED eksternal dengan resistor.
- Modul RFID RC522.
- OLED SSD1306 I2C 128×64 dengan alamat default `0x3C`.
- Kabel USB untuk upload firmware dan Serial Monitor.

> GPIO2 adalah pin boot-strapping pada beberapa board ESP32. Pastikan rangkaian LED tidak mengganggu proses boot. Jika LED board menggunakan pin atau polaritas berbeda, ubah `LED_PIN` dan logika `digitalWrite` pada `src/main.cpp`.

### Wiring ESP32, RFID, dan OLED

Hubungkan RC522 ke ESP32 menggunakan bus SPI (default ESP32), dan OLED menggunakan bus I2C:

| Modul | Pin modul | Pin ESP32 | Keterangan |
|---|---|---|---|
| LED bawaan / LED eksternal | Anoda / input | GPIO2 | Pin LED yang dikendalikan firmware; LED eksternal memerlukan resistor. |
| RC522 | SDA / SS | GPIO5 | Chip select SPI; pin ini bukan SDA I2C. |
| RC522 | SCK | GPIO18 | Clock SPI. |
| RC522 | MOSI | GPIO23 | Data ESP32 ke RC522. |
| RC522 | MISO | GPIO19 | Data RC522 ke ESP32. |
| RC522 | RST | GPIO27 | Reset modul RFID. |
| RC522 | 3.3V | 3V3 | Catu daya 3,3 V. |
| RC522 | GND | GND | Ground bersama. |
| OLED SSD1306 I2C | SDA | GPIO21 | Data I2C. |
| OLED SSD1306 I2C | SCL | GPIO22 | Clock I2C. |
| OLED SSD1306 I2C | VCC | Sesuai spesifikasi modul (3V3 disarankan) | Cek tegangan yang didukung modul. |
| OLED SSD1306 I2C | GND | GND | Ground bersama. |

Pastikan GND ESP32, RC522, dan OLED terhubung bersama. **RC522 hanya menggunakan catu daya dan logika 3,3 V; jangan hubungkan modul RC522 ke 5 V.** Firmware menampilkan UID kartu pada OLED dan Serial Monitor. Alamat OLED default adalah `0x3C`; jika alamat modul berbeda, ubah `OLED_ADDRESS` di `src/main.cpp`. Pin SS dan RST RC522 dapat disesuaikan melalui `RFID_SS_PIN` dan `RFID_RST_PIN`. Pin SPI dan I2C yang berbeda dari default board perlu disesuaikan pada kode inisialisasi bus.

## Struktur Project

```text
|-- platformio.ini
|-- src/
|   -- main.cpp
|-- README.md
```

## Persiapan Software

Install PlatformIO melalui PlatformIO IDE atau PlatformIO Core. Project ini menggunakan:

- Board: `esp32dev`
- Framework: Arduino
- Library: `hieromon/AutoConnect`, `miguelbalboa/MFRC522`, `adafruit/Adafruit SSD1306`, dan `adafruit/Adafruit GFX Library`

Build project:

```powershell
pio run
```

Jika perintah `pio` belum tersedia, gunakan:

```powershell
platformio run
```

Upload firmware:

```powershell
pio run -t upload
```

Buka Serial Monitor:

```powershell
pio device monitor -b 115200
```

## Konfigurasi Token API

Sebelum upload, ubah token pada `src/main.cpp`:

```cpp
constexpr char API_TOKEN[] = "CHANGE_THIS_TO_A_LONG_RANDOM_TOKEN";
```

Gunakan token acak yang panjang, misalnya minimal 32 karakter. Jangan membagikan token dan jangan mengunggah token asli ke repository publik.

Contoh format:

```cpp
constexpr char API_TOKEN[] = "ganti-dengan-token-acak-yang-panjang";
```

## Konfigurasi Wi-Fi Pertama Kali

1. Upload firmware ke ESP32.
2. Buka Serial Monitor pada baud rate `115200`.
3. Hubungkan laptop atau smartphone ke Wi-Fi:

   ```text
   SSID      : ESP32-Setup
   Password  : esp32setup
   ```

4. AutoConnect biasanya membuka captive portal secara otomatis.
5. Jika portal tidak muncul, buka alamat yang ditampilkan oleh AutoConnect, biasanya alamat gateway access point ESP32.
6. Pilih jaringan Wi-Fi rumah atau kantor.
7. Masukkan password Wi-Fi.
8. Simpan konfigurasi dan tunggu ESP32 terhubung.
9. Catat alamat IP ESP32 dari Serial Monitor.

Setelah terhubung, Serial Monitor menampilkan alamat seperti:

```text
Wi-Fi connected. Control page: http://192.168.1.50/control
```

## Kontrol LED Lokal

Buka alamat berikut dari perangkat yang berada pada jaringan Wi-Fi yang sama:

```text
http://IP_ESP32/control
```

Masukkan API token pada halaman tersebut, lalu gunakan tombol **Nyalakan** atau **Matikan**.

## API

Semua endpoint API memerlukan header berikut:

```http
Authorization: Bearer API_TOKEN
```

### Membaca Status

```http
GET /api/status
```

Contoh PowerShell:

```powershell
$headers = @{ Authorization = "Bearer API_TOKEN" }
Invoke-RestMethod -Uri "http://192.168.1.50/api/status" -Headers $headers
```

Contoh response:

```json
{
  "led": true,
  "ip": "192.168.1.50"
}
```

### Menyalakan LED

```http
POST /api/led
Content-Type: application/json
Authorization: Bearer API_TOKEN

{"on":true}
```

PowerShell:

```powershell
$headers = @{
    Authorization = "Bearer API_TOKEN"
    "Content-Type" = "application/json"
}

Invoke-RestMethod `
    -Uri "http://192.168.1.50/api/led" `
    -Method Post `
    -Headers $headers `
    -Body '{"on":true}'
```

### Mematikan LED

```http
POST /api/led
Content-Type: application/json
Authorization: Bearer API_TOKEN

{"on":false}
```

Request tanpa token yang benar akan mendapatkan response `401 Unauthorized`.

## Akses dari Internet dengan ngrok

ESP32 tidak menjalankan agent ngrok secara langsung. Jalankan ngrok pada komputer atau Raspberry Pi yang berada pada jaringan Wi-Fi yang sama dengan ESP32.

Misalnya IP ESP32 adalah `192.168.1.50`:

```powershell
ngrok http 192.168.1.50:80
```

Ngrok akan memberikan URL publik, misalnya:

```text
https://contoh.ngrok-free.app
```

Halaman kontrol dapat dibuka pada:

```text
https://contoh.ngrok-free.app/control
```

API juga dapat dipanggil melalui URL tersebut:

```text
https://contoh.ngrok-free.app/api/status
https://contoh.ngrok-free.app/api/led
```

Komputer yang menjalankan ngrok harus tetap menyala dan tetap terhubung ke jaringan yang sama dengan ESP32.

## Keamanan

- Ganti `API_TOKEN` sebelum firmware digunakan.
- Jangan membagikan URL ngrok secara terbuka tanpa perlindungan tambahan.
- Jangan memasukkan password Wi-Fi ke dalam source code.
- Jangan mengekspos portal konfigurasi AutoConnect ke internet.
- Hentikan proses ngrok jika akses remote sudah tidak diperlukan.
- Untuk penggunaan produksi, pertimbangkan reverse proxy dengan autentikasi tambahan atau server cloud.
- Endpoint kontrol saat ini menggunakan HTTP lokal. Perlindungan TLS berasal dari koneksi ngrok antara client dan ngrok, sedangkan koneksi ngrok ke ESP32 tetap berada di jaringan lokal.

## Troubleshooting

### AP `ESP32-Setup` tidak terlihat

- Pastikan firmware berhasil di-upload.
- Buka Serial Monitor dengan baud rate `115200`.
- Tekan tombol reset pada ESP32.
- Pastikan board yang dipilih adalah `esp32dev`.
- Hapus konfigurasi Wi-Fi AutoConnect jika perangkat terus mencoba jaringan lama.

### Captive portal tidak muncul

- Hubungkan kembali perangkat ke `ESP32-Setup`.
- Buka alamat gateway ESP32 secara manual.
- Matikan sementara koneksi VPN pada laptop atau smartphone.
- Coba browser lain.

### LED tidak menyala

- Pastikan LED terhubung ke GPIO2.
- Periksa apakah LED active-low. Jika iya, balik nilai `HIGH` dan `LOW` pada `digitalWrite`.
- Pastikan resistor digunakan untuk LED eksternal.
- Periksa response dari `/api/status`.

### API menghasilkan `401 Unauthorized`

- Pastikan token sama persis dengan `API_TOKEN` di firmware.
- Pastikan format header benar:

  ```http
  Authorization: Bearer API_TOKEN
  ```

- Pastikan firmware terbaru sudah di-upload.

### Ngrok tidak dapat mengakses ESP32

- Pastikan komputer ngrok dan ESP32 berada pada jaringan yang sama.
- Uji terlebih dahulu `http://IP_ESP32/control` secara lokal.
- Pastikan alamat IP ESP32 benar.
- Pastikan ngrok diarahkan ke port `80`.
- Pastikan komputer tidak berpindah jaringan setelah ngrok dijalankan.

## Status Implementasi

Sudah tersedia:

- AutoConnect Wi-Fi provisioning.
- Pembacaan UID RFID RC522 dan tampilan ID kartu pada OLED SSD1306.
- Web server ESP32.
- Halaman kontrol LED.
- API status dan kontrol LED.
- Autentikasi Bearer token.
- Kompilasi PlatformIO untuk board `esp32dev`.

Belum termasuk:

- Kontrol multi-user.
- Database cloud.
- Penyimpanan token melalui portal.
- HTTPS langsung pada ESP32.
- OTA firmware update.
#   e s p 3 2 - r f i d - o l e d  
 