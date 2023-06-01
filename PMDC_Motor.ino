#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define ACSPIN A2
#define PIN_Tegangan A3
#define PIN_Proxy 3
#define PIN_Relay 8


// Tegangan Referensi
double Vref = 5000.0;   // dalam satuan milivolt

// Variabel Sensor Tegangan
float R1 = 100000.0;
float R2 = 21700.0;
double ADC_Tegangan = 0.0;
double Vdiv = 0.0;
float Vin = 0.0;
double avg_tegangan;

// Daya
double power;

// Variabel Sensor Arus
int itterasi = 1000;
double ACS_ADC = 0.0;
double avg_ACS = 0.0;
double V_ACS   = 0.0;
double V_Offset = 2500.0;   // Offset tegangan pada sensor ACS712
double I_ACS   = 0.0;

unsigned long previousMillis;
unsigned long currentMillis;
long delayTime = 500;


void setup(){
    Serial.begin(9600);

    pinMode(PIN_Proxy, INPUT);
    pinMode(PIN_Relay, OUTPUT);

    lcd.init();
    lcd.backlight();

    lcd.setCursor(0,0);
    lcd.print("DC POWER METER");

    delay(1000);
    lcd.clear();
}

void loop(){
    // Reset pembacaan ADC
    ACS_ADC = 0;
    ADC_Tegangan = 0;

    // Melakukan iterasi terhadap ADC Sensor
    for(int i=0; i<itterasi; i++){
        ACS_ADC += analogRead(ACSPIN);
        ADC_Tegangan += analogRead(PIN_Tegangan);
    }

    // Ambil data rata-rata setelah iterasi
    avg_ACS = ACS_ADC / itterasi;
    avg_tegangan = ADC_Tegangan / itterasi;

    // Mengkonversi nilai ADC ke Tegangan 5v
    V_ACS = (avg_ACS/1023.0) * Vref;

    // Mengkonversi tegangan ke arus
    I_ACS = (V_ACS - V_Offset) / 100.0;

    if(I_ACS < 0.01) {
        I_ACS = 0.0;
    }

    // Konversi adc ke tegangan 5v
    Vdiv = (avg_tegangan / 1023.0) * 5.0;

    // Perhitungan Voltage divider
    Vin = Vdiv/(R2/(R1+R2));

    // Perhitungan Daya
    power = Vin * I_ACS;

    // Mengubah kondisi output pada pin Relay berdasarkan sensor
    if(digitalRead(PIN_Proxy) == LOW) {
        digitalWrite(PIN_Relay, HIGH);
    } else {
        digitalWrite(PIN_Relay, LOW);
    }

    currentMillis = millis();
    if(currentMillis - previousMillis > delayTime) {

        // Print Tegangan
        lcd.setCursor(1,0);
        lcd.print(Vin);

        if(Vin < 10.0) {
            lcd.setCursor(5,0);
        } else {
            lcd.setCursor(6,0);
        }
        lcd.print("V ");

        // Print Arus
        lcd.setCursor(9,0);
        lcd.print(I_ACS);

        if(I_ACS < 10.0) {
            lcd.setCursor(13,0);
        } else {
            lcd.setCursor(14,0);
        }
        lcd.print("A ");

        // Print Daya
        lcd.setCursor(5,1);

        lcd.print(power);

        if(power < 10.0) {
            lcd.setCursor(9,1);
        } else if(power < 100.0) {
            lcd.setCursor(10,1);
        } else {
            lcd.setCursor(11,1);
        }
        lcd.print("W  ");


        // Menampilkan data ke Serial
        Serial.print(Vin);
        Serial.print(" | ");
        Serial.print(I_ACS);
        Serial.print(" | ");
        Serial.println(power);

        previousMillis = currentMillis;
    }
}
