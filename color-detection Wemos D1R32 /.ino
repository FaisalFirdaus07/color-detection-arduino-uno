// ====== Pin TCS3200 (disesuaikan untuk Wemos D1 R32 / ESP32) ======
// Pilih pin digital biasa (hindari pin 6–11 karena dipakai flash)
const int S0 = 14;       // D5 pada Wemos D1 R32
const int S1 = 27;       // GPIO27
const int S2 = 26;       // GPIO26
const int S3 = 25;       // GPIO25
const int sensorOut = 33; // GPIO33 (input)


// ====== LED indikator (disesuaikan ke pin GPIO yang aman) ======
const int redLED = 18;   // GPIO18
const int greenLED = 19; // GPIO19
const int blueLED = 23;  // GPIO23


// ====== Bobot dan Bias dari Python (hasil training) ======
float weights[3][3] = {
  {5.905637288, -1.7600959720, -2.0883289398},
  {-2.4917673708, 5.9018896783, -2.4899542626},
  {-2.3446083106, -1.9130676520, 5.7831227323}
};

float bias[3] = {
  0.7323626748,
  0.2843566652,
  0.6816681843
};

// ====== Variabel Kalibrasi Hitam & Putih ======
int whiteRef[3] = {0, 0, 0}; // R, G, B untuk putih
int blackRef[3] = {0, 0, 0}; // R, G, B untuk hitam


// ====== Fungsi Pembacaan Frekuensi Warna ======
int getColorFrequency(bool s2, bool s3) {
  digitalWrite(S2, s2);
  digitalWrite(S3, s3);
  delay(50);
  return pulseIn(sensorOut, LOW);
}

// ====== Fungsi Normalisasi RGB ======
float normalize(int val, int minVal, int maxVal) {
  return ((float)(val - minVal) / (maxVal - minVal));
}

// ====== Fungsi Aktivasi (step) ======
int activation(float x) {
  return (x >= 0.5) ? 1 : 0;
}

// ====== Prediksi Warna Berdasarkan ANN ======
int predictColor(float R, float G, float B) {
  float inputs[3] = {R, G, B};
  float output[3];

  for (int i = 0; i < 3; ++i) {
    output[i] = bias[i];
    for (int j = 0; j < 3; ++j) {
      output[i] += inputs[j] * weights[j][i];
    }
    output[i] = activation(output[i]);
  }

  if (output[0] == 1 && output[1] == 0 && output[2] == 0) return 0; // Merah
  if (output[0] == 0 && output[1] == 1 && output[2] == 0) return 1; // Hijau
  if (output[0] == 0 && output[1] == 0 && output[2] == 1) return 2; // Biru
  return -1; // Tidak dikenali
}

// ====== Menyalakan LED Berdasarkan Warna ======
void showColorLED(int colorIndex) {
  digitalWrite(redLED, colorIndex == 0);
  digitalWrite(greenLED, colorIndex == 1);
  digitalWrite(blueLED, colorIndex == 2);
}

// ====== Setup ESP32 ======
void setup() {
  Serial.begin(115200);

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);

  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(blueLED, OUTPUT);

  // Set frequency scaling ke 20%
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  Serial.println("Ketik 'w' untuk sampling PUTIH, 'b' untuk sampling HITAM");
}

// ====== Loop Utama ======
void loop() {
  int redFreq = getColorFrequency(LOW, LOW);      // Red
  int greenFreq = getColorFrequency(HIGH, HIGH);  // Green
  int blueFreq = getColorFrequency(LOW, HIGH);    // Blue

  // Default range kalibrasi
  int minFreq = 20;
  int maxFreq = 300;

  // Normalisasi nilai
  float R = 1.0 - normalize(redFreq, minFreq, maxFreq);
  float G = 1.0 - normalize(greenFreq, minFreq, maxFreq);
  float B = 1.0 - normalize(blueFreq, minFreq, maxFreq);

  // Cek input serial untuk sampling
  if (Serial.available()) {
    char cmd = Serial.read();

    if (cmd == 'w') {
      whiteRef[0] = redFreq;
      whiteRef[1] = greenFreq;
      whiteRef[2] = blueFreq;
      Serial.printf("Sampel PUTIH disimpan: R=%d G=%d B=%d\n", whiteRef[0], whiteRef[1], whiteRef[2]);
    }
    else if (cmd == 'b') {
      blackRef[0] = redFreq;
      blackRef[1] = greenFreq;
      blackRef[2] = blueFreq;
      Serial.printf("Sampel HITAM disimpan: R=%d G=%d B=%d\n", blackRef[0], blackRef[1], blackRef[2]);
    }
  }

  int result = predictColor(R, G, B);

  Serial.printf("RGB Normalized: %.2f, %.2f, %.2f\n", R, G, B);

  if (result == 0) {
    Serial.println("Deteksi: MERAH");
  } else if (result == 1) {
    Serial.println("Deteksi: HIJAU");
  } else if (result == 2) {
    Serial.println("Deteksi: BIRU");
  } else {
    Serial.println("Deteksi: TIDAK DIKENALI");
  }

  showColorLED(result);
  delay(1000);
}
