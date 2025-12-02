// ====== Pin TCS3200 ======
const int S0 = 4;
const int S1 = 5;
const int S2 = 6;
const int S3 = 7;
const int sensorOut = 8;

// ====== LED indikator ======
const int redLED = 9;
const int greenLED = 10;
const int blueLED = 11;

// ====== Bobot dan Bias ANN (3 kelas: Merah, Hijau, Biru) ======
float weights[3][3] = {
  { 6.8569887739,  -2.2708413727, -2.5288990341 },
  { -3.0367085985,  6.8301814022, -2.8828011587 },
  { -2.8068484963, -2.5318621745,  6.8640854405 }
};

float bias[3] = {
  0.7584940364,
  0.3032519731,
  0.6368209147
};

// ====== Kalibrasi ======
int whiteRef[3] = {0, 0, 0};
int blackRef[3] = {0, 0, 0};

// ====== Fungsi Pembacaan Frekuensi ======
int getColorFrequency(bool s2, bool s3) {
  digitalWrite(S2, s2);
  digitalWrite(S3, s3);
  delay(50);
  return pulseIn(sensorOut, LOW);
}

// ====== Normalisasi ======
float normalize(int val, int minVal, int maxVal) {
  return ((float)(val - minVal) / (maxVal - minVal));
}

// ====== ANN Predict Warna ======
int predictColor(float R, float G, float B) {
  float inputs[3] = {R, G, B};
  float output[3];

  for (int i = 0; i < 3; i++) {
    output[i] = bias[i];
    for (int j = 0; j < 3; j++) {
      output[i] += inputs[j] * weights[j][i];
    }
  }

  int idx = 0;
  float maxv = output[0];
  for (int i = 1; i < 3; i++) {
    if (output[i] > maxv) {
      maxv = output[i];
      idx = i;
    }
  }

  return idx;  // 0=Merah, 1=Hijau, 2=Biru
}

// ====== LED Output ======
void showColorLED(int colorIndex) {
  if (colorIndex == 3) { // KUNING
    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, HIGH);
    digitalWrite(blueLED, LOW);
    return;
  }

  digitalWrite(redLED,  colorIndex == 0);
  digitalWrite(greenLED, colorIndex == 1);
  digitalWrite(blueLED,  colorIndex == 2);
}

// ====== DETEKSI KUNING (TUNED) ======
bool isYellow(float R, float G, float B) {

  // Threshold khusus hasil tuning
  bool cond1 = (R > 0.55);
  bool cond2 = (G > 0.50);
  bool cond3 = (B < 0.38);

  // Tambahan: selisih warna (lebih stabil)
  bool cond4 = ((R - B) > 0.25);
  bool cond5 = ((G - B) > 0.20);

  return (cond1 && cond2 && cond3 && cond4 && cond5);
}

// ====== Setup ======
void setup() {
  Serial.begin(9600);

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);

  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(blueLED, OUTPUT);

  // TCS3200 scaling 20%
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  Serial.println("Ketik 'w' untuk kalibrasi PUTIH, 'b' untuk HITAM");
}

// ====== Loop ======
void loop() {

  // Baca frekuensi RGB
  int redFreq   = getColorFrequency(LOW, LOW);
  int greenFreq = getColorFrequency(HIGH, HIGH);
  int blueFreq  = getColorFrequency(LOW, HIGH);

  // Rentang normal (bisa disesuaikan)
  int minFreq = 20;
  int maxFreq = 300;

  // Normalisasi
  float R = 1.0 - normalize(redFreq, minFreq, maxFreq);
  float G = 1.0 - normalize(greenFreq, minFreq, maxFreq);
  float B = 1.0 - normalize(blueFreq, minFreq, maxFreq);

  // === Deteksi Kuning (lebih akurat) ===
  if (isYellow(R, G, B)) {
    Serial.print("RGB Normalized: ");
    Serial.print(R); Serial.print(", ");
    Serial.print(G); Serial.print(", ");
    Serial.println(B);

    Serial.println("Deteksi: KUNING");
    showColorLED(3);
    delay(600);
    return;
  }

  // === ANN untuk Merah/Hijau/Biru ===
  int result = predictColor(R, G, B);

  Serial.print("RGB Normalized: ");
  Serial.print(R); Serial.print(", ");
  Serial.print(G); Serial.print(", ");
  Serial.println(B);

  if (result == 0) Serial.println("Deteksi: MERAH");
  else if (result == 1) Serial.println("Deteksi: HIJAU");
  else if (result == 2) Serial.println("Deteksi: BIRU");

  showColorLED(result);
  delay(600);
}
