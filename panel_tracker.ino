#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ESP32Servo.h>

// --- 1. PENGATURAN WIFI & TELEGRAM ---
const char* ssid = "";           // Hotspot HP kamu
const char* password = "";      // Password hotspot
#define BOTtoken "" // Token Bot R.E.N.G
#define CHAT_ID ""             // ID Chat Telegram kamu

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// --- 2. PENGATURAN PIN KOMPONEN ---
const int pinLDRKiri = 34;   // Baris 19 (G34)
const int pinLDRKanan = 35;  // Baris 20 (G35)
const int pinServo = 13;     // Baris 28 (D13)
const int pinBaterai = 32;   // Baris 27 (G32) - Dari tengah 3 Resistor

Servo myservo;

// --- 3. VARIABEL LOGIKA ---
int posisiServo = 90;        // Posisi awal servo di tengah (90 derajat)
unsigned long waktuTerakhir = 0;
const unsigned long jedaKirim = 3000; // DIUBAH JADI 3 DETIK SEKALI BIAR PAS DEMO LANGSUNG MUNCUL!

// --- 4. FUNGSI HITUNG PERSENTASE BATERAI ---
int hitungPersenBaterai(float volt) {
  if (volt < 3.0) {
    return 0; 
  }
  int mVolt = volt * 1000; 
  int persen = map(mVolt, 3300, 4200, 0, 100);
  return constrain(persen, 0, 100); 
}

void setup() {
  Serial.begin(115200);
  
  // Setup Pin Analog
  pinMode(pinLDRKiri, INPUT);
  pinMode(pinLDRKanan, INPUT);
  pinMode(pinBaterai, INPUT);
  
  // Hubungkan Servo
  myservo.attach(pinServo);
  myservo.write(posisiServo); 
  
  // Mengabaikan verifikasi SSL Telegram agar koneksi lancar
  client.setInsecure();
  
  // --- PROSES KONEKSI WIFI ---
  Serial.print("Menghubungkan ke WiFi...");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi Terhubung Sukses!");
  
  // Kirim notifikasi pertama
  bot.sendMessage(CHAT_ID, "🤖 Sistem Maket R.E.N.G Berhasil Dinyalakan Mandiri!", "");
  delay(2000); 
}

void loop() {
  // --- 1. BACA NILAI SENSOR LDR ---
  int nilaiKiri = analogRead(pinLDRKiri);
  int nilaiKanan = analogRead(pinLDRKanan);
  
  Serial.print("LDR Kiri: "); Serial.print(nilaiKiri);
  Serial.print(" | LDR Kanan: "); Serial.println(nilaiKanan);
  
  // --- 2. LOGIKA PERGERAKAN SERVO ---
  int toleransi = 150; 
  
  if ((nilaiKanan - nilaiKiri) > toleransi) {
    if (posisiServo < 180) posisiServo += 3; // Bergerak ke kiri
  } 
  else if ((nilaiKiri - nilaiKanan) > toleransi) {
    if (posisiServo > 0) posisiServo -= 3;   // Bergerak ke kanan
  }
  
  myservo.write(posisiServo);
  delay(20); 
  
  // --- 3. BACA VOLTASE BATERAI & KIRIM KE TELEGRAM ---
  if (millis() - waktuTerakhir > jedaKirim) {
    int nilaiADC = analogRead(pinBaterai);
    
    // Konversi ADC ke Voltase murni asli baterai
    float voltase = (nilaiADC / 4095.0) * 3.3 * 2.0; 
    int persentase = hitungPersenBaterai(voltase);
    
    // SUSUN FORMAT PESAN FORMAT BARU (SUDAH DIPERBAIKI)
    String pesan = "========================\n";
    pesan += "🤖 LAPORAN SOLAR TRACKER\n";
    pesan += "========================\n";
    pesan += "📐 Posisi Servo : " + String(posisiServo) + "°\n";
    pesan += "---\n";
    pesan += "💡 LDR Kiri     : " + String(nilaiKiri) + "\n";   
    pesan += "💡 LDR Kanan    : " + String(nilaiKanan) + "\n";  
    pesan += "---\n";
    pesan += "🔋 Voltase Saat Ini : " + String(voltase, 2) +  "%)\n";
    pesan += "========================";
    
    // Eksekusi kirim ke Telegram
    if (bot.sendMessage(CHAT_ID, pesan, "Markdown")) {
      Serial.println(">>> Laporan Status Terkirim ke Telegram!");
    } else {
      Serial.println(">>> Gagal kirim ke Telegram, cek jaringan Hotspot!");
    }
    
    waktuTerakhir = millis();
  }
} 