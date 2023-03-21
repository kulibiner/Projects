#include <Keypad.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/************* IOPin **************/
const int heater    = 2;
const int Buzzer    = 13;
const int Motor     = 3;
const int pinSuhu   = 12;
const int IRSensor  = 18;
const int Kipas     = 14;

/************* KEYPAD **************/
const byte ROWS = 4;
const byte COLS = 4;
byte colPins[COLS]  = {7, 6, 5, 4};
byte rowPins[ROWS]  = {11, 10, 9, 8};

char keymap[ROWS][COLS] = {
    {'1', '2', '3', 'a'},
    {'4', '5', '6', 'b'},
    {'7', '8', '9', 'c'},
    {'*', '0', '#', 'd'}
};

Keypad  keypad = Keypad( makeKeymap(keymap), rowPins, colPins, ROWS, COLS);

/************* DS18B20 **************/
OneWire pin_DS18B20(pinSuhu);
DallasTemperature DS18B20(&pin_DS18B20);

/************* LCD **************/
LiquidCrystal_I2C lcd(0x27, 20, 4);


/************* Sensing Variable **************/
float currentSuhu;
float rps;
int rpm;
int pulsePerRevolution = 1;
unsigned long lastPulse;
long pulsePeriod;

/************* Variable Menu **************/
bool resetMenu      = false;
int Menu            = 0;
int indexMenu       = 1;
int setMode         = 0;
String inputString  = "";

/************* Variable Pasteurisasi **************/
int suhuPasteurisasi;
int volume;
int PWMMotor;

/************* Variable Pendinginan **************/
int suhuPendinginan;
int durasiPendinginan;

/************* Variable Fermentasi **************/
int suhuFermentasi;
int durasiFermentasi;

/************* Variable Timer **************/
bool running             = false;
bool clearProcess        = false;
const long timerInterval = 1000;
unsigned long startTimer;
unsigned long currentTimer;
int detik;
int menit;
int jam;
String dispDetik;
String dispMenit;
String dispJam;

/************* Variable Blinking Display **************/
const long blinkInterval = 500;
unsigned long startBlink;
unsigned long currentBlink;
bool viewChar;

void isr() {
    pulsePeriod = millis() - lastPulse;
    lastPulse = millis();
}

void setup(){

    /************* SET IO **************/
    pinMode(heater, OUTPUT);
    pinMode(Buzzer, OUTPUT);
    pinMode(Motor, OUTPUT);
    pinMode(Kipas, OUTPUT);
    pinMode(IRSensor, INPUT);

    /************* SET BAUDRATE **************/
    Serial.begin(9600);

    /************* SET Sensor Suhu **************/
    DS18B20.begin();

    /************* SET LCD **************/
    lcd.begin();
    lcd.backlight();

    inputString.reserve(4);
}

void loop() {
    // Baca input dari keypad
    char tombol = keypad.getKey();

    attachInterrupt(digitalPinToInterrupt(IRSensor), isr, RISING);
    rps = (pulsePeriod * pulsePerRevolution) / 1000.00;
    rpm = rps * 60;

    // Reset Display dan Variable
    if(resetMenu) {
        lcd.clear();
        inputString = "";
        resetMenu = false;
    }

    if(tombol == 'c') {
        running = false;

        lcd.setCursor(0,0);
        lcd.print("RESETING...");

        digitalWrite(Buzzer, HIGH);
        delay(1500);
        digitalWrite(Buzzer, LOW);

        jam = 0;
        menit = 0;
        detik = 0;
        Menu = 0;
        indexMenu = 1;

    }

    switch (Menu){
    /*******************************************
    * MENU UTAMA
    ********************************************/
    case 0:
        if(tombol >= '0' && tombol <= '3'){
            Menu = tombol - '0';
            Serial.print("Masuk Menu ");
            Serial.println(Menu);
            resetMenu = true;
        }

        lcd.setCursor(0,0);
        lcd.print("1. Pasteurisasi");
        lcd.setCursor(0,1);
        lcd.print("2. Pendinginan");
        lcd.setCursor(0,2);
        lcd.print("3. Fermentasi");
    break;
    
    /*******************************************
    * PASTEURISASI
    ********************************************/
    case 1:
        if(tombol >= '0' && tombol <= '9') {
            inputString += tombol;
        }

        // IndexMenu 1 Pasteurisasi, Input Suhu
        if(indexMenu == 1) {
            if(tombol == 'a') {
                if(inputString.toInt() > 20) {
                    suhuPasteurisasi = inputString.toInt();
                    Serial.println("Input Suhu Pasteurisasi Berhasil");

                    resetMenu = true;
                    indexMenu++;
                } else {
                    Serial.println("Gagal, Input suhu tidak valid");
                    resetMenu = true;
                }
            } else if(tombol == 'd') {
                    Menu = 0;
            }

            lcd.setCursor(0,0);
            lcd.print("Pasteurisasi");
            lcd.setCursor(0,1);
            lcd.print("Suhu:");
            lcd.setCursor(5,1);
            lcd.print(inputString);
        }

        // IndexMenu 2 Pasteurisasi, Input Volume Susu
        else if(indexMenu == 2) {
            if(tombol == '1') {
                menit = 30;
                PWMMotor = 50;
                resetMenu = true;
                indexMenu++;
            } else if(tombol == '2') {
                PWMMotor = 100;
                menit = 40;
                resetMenu = true;
                indexMenu++;
            } else if(tombol == '3') {
                PWMMotor = 150;
                menit = 50;
                resetMenu = true;
                indexMenu++;
            } else if(tombol == 'd') {
                resetMenu = true;
                indexMenu--;
            }

            lcd.setCursor(0,0);
            lcd.print("1.10l/30M/80RPM");
            lcd.setCursor(0,1);
            lcd.print("2.13l/40M/90RPM");
            lcd.setCursor(0,2);
            lcd.print("3.15l/50M/100RPM");
        }

        // IndexMenu 3  Pasteurisasi , Akan mengaktifkan timer dan output yang diperlukan
        else if(indexMenu == 3) {

            // Set PWM Motor 75 dari 255
            analogWrite(Motor, PWMMotor);

            // Akan dieksekusi ketika suhu dibawah setpoint
            if(suhuPasteurisasi > currentSuhu) {
                digitalWrite(heater, HIGH);
                digitalWrite(Kipas, LOW);
            } else {    // Sebaliknya
                digitalWrite(heater, LOW);
                digitalWrite(Kipas, HIGH);
            }


            running = true;

            lcd.setCursor(0,0);
            lcd.print("Pasteurisasi");
            lcd.setCursor(0,1);
            lcd.print(suhuPasteurisasi);
            if(suhuPasteurisasi > 10) {
                lcd.setCursor(2,1);
                lcd.print((char)223);
                lcd.setCursor(3,1);
                lcd.print("C ->");
            } else {
                lcd.setCursor(1,1);
                lcd.print((char)223);
                lcd.setCursor(2,1);
                lcd.print("C  ->");
            }

            lcd.setCursor(0,2);
            lcd.print("RPM =");
            lcd.setCursor(5,2);
            lcd.print(rpm);
        }
    break;

    /*******************************************
    * PENDINGINAN
    ********************************************/
    case 2:
        if(tombol >= '0' && tombol <= '9') {
            inputString += tombol;
        }
        
        //indexMenu Pendinginan, Input Setpoint Suhu Pendinginan
        if(indexMenu == 1) {
            if(tombol == 'a') {
                if(inputString.toInt() > 20) {
                    suhuPendinginan = inputString.toInt();
                    Serial.println("Input Suhu Pasteurisasi Berhasil");

                    resetMenu = true;
                    indexMenu++;
                } else {
                    Serial.println("Gagal, Input suhu tidak valid");
                    resetMenu = true;
                }
            } else if(tombol == 'd') {
                    Menu = 0;
            }


            lcd.setCursor(0,0);
            lcd.print("Pendinginan");
            lcd.setCursor(0,1);
            lcd.print("Suhu:");
            lcd.setCursor(5,1);
            lcd.print(inputString);
        }
        
        //indexMenu 2 Pendinginan, Input Setpoint Durasi Pendinginan
        else if(indexMenu == 2) {
            if(tombol == 'a') {
                if(inputString.toInt() > 0) {
                    if(inputString.toInt() > 60) {
                        durasiPendinginan = inputString.toInt();
                        jam = durasiPendinginan / 60;
                        menit = durasiPendinginan % 60;
                        resetMenu = true;
                        indexMenu++;
                    } else {
                        durasiPendinginan = inputString.toInt();
                        menit = durasiPendinginan;
                        resetMenu = true;
                        indexMenu++;
                    }
                } else {
                    Serial.println("Input Durasi Pendingin Gagal");
                        resetMenu = true;
                }
            } else if(tombol == 'd') {
                resetMenu = true;
                indexMenu--;
            }

            lcd.setCursor(0,0);
            lcd.print("Pendinginan");
            lcd.setCursor(0,1);
            lcd.print("Durasi:");
            lcd.setCursor(7,1);
            lcd.print(inputString);
            lcd.setCursor(10,1);
            lcd.print("menit");
        }

        // IndexMenu 3  Pendinginan , Akan mengaktifkan timer dan output yang diperlukan
        else if(indexMenu == 3) {

            // Akan dieksekusi ketika suhu dibawah setpoint
            if(suhuPendinginan < currentSuhu) {
                digitalWrite(Kipas, HIGH);
            } else {    // Sebaliknya
                digitalWrite(Kipas, LOW);
            }

            running = true;

            lcd.setCursor(0,0);
            lcd.print("Pendinginan");
            lcd.setCursor(0,1);
            lcd.print(suhuPendinginan);
            if(suhuPendinginan > 10) {
                lcd.setCursor(2,1);
                lcd.print((char)223);
                lcd.setCursor(3,1);
                lcd.print("C ->");
            } else {
                lcd.setCursor(1,1);
                lcd.print((char)223);
                lcd.setCursor(2,1);
                lcd.print("C  ->");
            }
        }
    break;

    /*******************************************
    * FERMENTASI
    ********************************************/
    case 3:
        if(tombol >= '0' && tombol <= '9') {
            inputString += tombol;
        }

        //indexMenu 1 Fermentasi, Input Setpoint Suhu Fermentasi
        if(indexMenu == 1) {
            if(tombol == 'a') {
                if(inputString.toInt() > 20) {
                    suhuFermentasi = inputString.toInt();
                    Serial.println("Input Suhu Pasteurisasi Berhasil");

                    resetMenu = true;
                    indexMenu++;
                } else {
                    Serial.println("Gagal, Input suhu tidak valid");
                    resetMenu = true;
                }
            } else if(tombol == 'd') {
                    Menu = 0;
            }

            lcd.setCursor(0,0);
            lcd.print("Fermentasi");
            lcd.setCursor(0,1);
            lcd.print("Suhu:");
            lcd.setCursor(5,1);
            lcd.print(inputString);
        }

        //indexMenu 2 Fermentasi, Input Setpoint Durasi Fermentasi
        else if(indexMenu == 2) {
            if(tombol == 'a') {
                if(inputString.toInt() > 0) {
                    durasiFermentasi = inputString.toInt();
                    jam = durasiFermentasi;
                    Serial.println("Input durasi fermentasi berhasil");
                    resetMenu = true;
                    indexMenu++;
                } else {
                    Serial.println("Input durasi fermentasi berhasil");
                    resetMenu = true;
                }
            } else if(tombol == 'd') {
                resetMenu = true;
                indexMenu--;
            }

            lcd.setCursor(0,0);
            lcd.print("Fermentasi");
            lcd.setCursor(0,1);
            lcd.print("Durasi:");
            lcd.setCursor(7,1);
            lcd.print(inputString);
            lcd.setCursor(10,1);
            lcd.print("jam");
        }

        // IndexMenu 3  Fermentasi , Akan mengaktifkan timer dan output yang diperlukan
        else if(indexMenu == 3) {

            // Akan dieksekusi ketika suhu dibawah setpoint
            if(suhuFermentasi > currentSuhu) {
                digitalWrite(heater, HIGH);
            } else {    // Sebaliknya
                digitalWrite(heater, LOW);
            }

            running = true;

            lcd.setCursor(0,0);
            lcd.print("Fermentasi");
            lcd.setCursor(0,1);
            lcd.print(suhuFermentasi);

            if(suhuFermentasi > 10) {
                lcd.setCursor(2,1);
                lcd.print((char)223);
                lcd.setCursor(3,1);
                lcd.print("C ->");
            } else {
                lcd.setCursor(1,1);
                lcd.print((char)223);
                lcd.setCursor(2,1);
                lcd.print("C  ->");
            }
        }
    break;

    /*******************************************
    * ALARM SELESAI - DIEKSEKUSI SETELAH TIMER SUDAH SELESAI
    ********************************************/
    case 4:
        if(tombol == 'a') {
            jam = 0;
            menit = 0;
            detik = 0;
            Menu = 0;
            indexMenu = 1;
            viewChar = false;
            
            Menu = 0;
        }

        lowOutput();

        currentBlink = millis();
        if(currentBlink - startBlink > blinkInterval) {
            viewChar = !viewChar;

            startBlink = currentBlink;
        }

        if(viewChar) {
            lcd.setCursor(0,0);
            lcd.print("Proses Selesai");
            digitalWrite(Buzzer, HIGH);
        } else {
            resetMenu = true;
            digitalWrite(Buzzer, LOW);
        }
    break;

    }

    /*******************************************
    * TIMER - Timer menghitung Mundur
    ********************************************/
    if(running) {
        // Zero Padding
        if(detik < 10) {
            dispDetik = "0";
            dispDetik += detik;
        } else {
            dispDetik = detik; 
        }
        if(menit < 10) {
            dispMenit = "0";
            dispMenit += menit;
        } else {
            dispMenit = menit; 
        }
        if(jam < 10) {
            dispJam = "0";
            dispJam += jam;
        } else {
            dispJam = jam; 
        }

        currentTimer = millis();
        if(currentTimer - startTimer > timerInterval) {
            detik--;
            if(detik < 0) {
                detik = 59;
                menit--;
            }
            if(menit < 0) {
                menit = 59;
                jam--;
            }
            if(jam < 0) {
                Menu = 4;
                running = false;
            }

            Serial.print(dispJam);
            Serial.print(":");
            Serial.print(dispMenit);
            Serial.print(":");
            Serial.println(dispDetik);

            lcd.setCursor(8,1);
            lcd.print(currentSuhu);
            
            lcd.setCursor(0,3);
            lcd.print(dispJam);
            lcd.setCursor(2,3);
            lcd.print(":");
            lcd.setCursor(3,3);
            lcd.print(dispMenit);
            lcd.setCursor(5,3);
            lcd.print(":");
            lcd.setCursor(6,3);
            lcd.print(dispMenit);

            DS18B20.requestTemperatures();
            currentSuhu = DS18B20.getTempCByIndex(0);

            startTimer = currentTimer;
        }

    }

}

/*******************************************
* SET SEMUA OUTPUT LOW
********************************************/
void lowOutput() {
    digitalWrite(heater, LOW);
    digitalWrite(Kipas, LOW);
    digitalWrite(Motor, LOW);
}
