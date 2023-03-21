#include <Keypad.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/*
    IO Mapping
    Kipas  (output)             14
    Heater  (output)            2
    Buzzer  (output)            13
    Motor   (output-pwm)        3
    keypad  (input)             4,5,6,7,8,9,10,11
    DS18b20 (input)             18
    Sensor  (input-interupt)    12

*/


const int heater    = 2;
const int Buzzer    = 13;
const int Motor     = 3;
const int pinSuhu   = 12;
const int IRSensor  = 18;
const int Kipas     = 14;

OneWire pin_DS18B20(pinSuhu);
DallasTemperature DS18B20(&pin_DS18B20);

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

LiquidCrystal_I2C lcd(0x27, 20, 4);

// Suhu Minimal input 
int suhuMinimal = 20;

// Variable IO
float currentSuhu;

// Variable Menu
bool resetMenu = false;
int setMode = 0;
int Menu, subMenu, sub2Menu;
int indexMenu;
String inputString;

// Variable pasteurisasi
bool Pasteurisasi = false;
int volume;
int suhuPasteurisasi;
int durasiPasteurisasi;

// Variable Fermentasi
bool fermentasi = false;
int suhuFermentasi;
int diffFermentasi;
int durasiFermentasi;

// Variable Pendingin
bool pendingin = false;
int suhuPendingin;
int diffPendingin;
int durasiPendingin;

bool clearProcess = false;
bool running = false;
bool jalan = false;

// variable timer
const long timerInterval = 1000;
unsigned long startTimer;
unsigned long currentTimer;
int detik;
int menit;
int jam;
String dispDetik;
String dispMenit;
String dispJam;

const long blinkChar = 500;
unsigned long currentCharMillis;
unsigned long startCharMillis;
bool viewChar;

// rpm
unsigned long startRpmRev;
unsigned long endRpmRev;
float RevDuration;
int count;
float rpm;
bool rpmTriggered;

unsigned long timePeriod;
unsigned long lastTimePulse;
float RPMValue;

// void isr() {
//     timePeriod = millis() - lastTimePulse ;

//     lastTimePulse = millis();
// }

void setup(){

    pinMode(heater, OUTPUT);
    pinMode(Buzzer, OUTPUT);
    pinMode(Motor, OUTPUT);
    pinMode(Kipas, OUTPUT);
    pinMode(IRSensor, INPUT);

    Serial.begin(9600);

    DS18B20.begin();

    inputString.reserve(4);

    lcd.begin();
	lcd.backlight();

    // attachInterrupt(5, rpm_fun, FALLING);
    // attachInterrupt(digitalPinToInterrupt(18), rpm_fun, RISING);

    // attachInterrupt(digitalPinToInterrupt(18),isr,RISING);

}

void loop(){

    // attachInterrupt(digitalPinToInterrupt(IRSensor), isr, FALLING);

    /******************************************
     * RPM CALCULATION
     * Perhitungan RPM memanfaatkan delay waktu
    ******************************************/
    // RPMValue = (timePeriod / 1000) * 60;

    // delay(100);
    //     Serial.println(RevDuration);

    char tombol = keypad.getKey();
    DS18B20.requestTemperatures();

    if(tombol == "c") {
        clearProcess = true;
    }

    if(resetMenu){
        inputString = "";
        lcd.clear();
        resetMenu = false;
    }

    /***************************************************************************
     * MENU UTAMA
     * Input menu pilihan
    ***************************************************************************/
    if(indexMenu == 0) {

        /* Displaynya BRO*/
        lcd.setCursor(0,0);
        lcd.print("1.Pasteurisasi");
        lcd.setCursor(0,1);
        lcd.print("2.Fermentasi");
        lcd.setCursor(0,2);
        lcd.print("3.Pendingin");

        if(tombol >= '0' && tombol <= '3'){
            lcd.clear();
            Menu = tombol - '0';

            indexMenu++;
            Serial.print("Masuk Menu ");
            Serial.println(Menu);
        }
    }
    
    else if(indexMenu == 1) {

        if(tombol >= '0' && tombol <= '9'){
            inputString += tombol;
        }
        else if(tombol == 'd') {
            resetMenu = true;
            indexMenu--;
            setMode = 0;
        }

        switch (Menu){

            /***************************************************************************
             * Menu PASTEURISASI
             * Input suhu pasteurisasi
            ***************************************************************************/
            case 1:
                if(tombol == 'a') {
                    if(inputString.toInt() > suhuMinimal) {
                        suhuPasteurisasi = inputString.toInt();

                        Serial.print("Suhu Pasteurisasi berhasil di input, ");
                        Serial.print(suhuPasteurisasi);
                        Serial.print(" Derajat Celcius");

                        indexMenu++;
                    } else {
                        Serial.println("Gagal, input suhu tidak valid");
                        inputString = "";
                    }

                    resetMenu = true;
                }

                // ------------------ Display Setting Suhu Pasteurisasi ------------------ //
                lcd.setCursor(0,0);
                lcd.print("PASTEURISASI");

                lcd.setCursor(0,1);
                lcd.print("Suhu:");
                lcd.setCursor(5,1);
                lcd.print(inputString);
            break;

            /***************************************************************************
             * Menu FERMENTASI
             * Input Suhu Fermentasi
            ***************************************************************************/
            case 2:
                if(tombol == 'a') {

                    if(inputString.toInt() > suhuMinimal) {
                        suhuFermentasi = inputString.toInt();

                        Serial.print("Suhu Fermentasi berhasil di input, ");
                        Serial.print(suhuFermentasi);
                        Serial.println(" Derajat Celcius");

                        indexMenu++;
                    } else {
                        Serial.println(" Gagal, input suhu tidak valid");
                    }

                    resetMenu = true;
                }

                // ------------------ Display Setting Suhu Fermentasi ------------------ //
                lcd.setCursor(0,0);
                lcd.print("FERMENTASI");

                lcd.setCursor(0,1);
                lcd.print("Suhu:");

                // Print Suhu Setpoint
                lcd.setCursor(5,1);
                lcd.print(inputString);
            break;

            /***************************************************************************
             * Menu PENDINGIN
             * Input suhu pendinginan
            ***************************************************************************/
            case 3:
                if(tombol == 'a') {
                    if(inputString.toInt() > suhuMinimal) {
                        suhuPendingin = inputString.toInt();

                        Serial.print("Suhu pendingin berhasil di input, ");
                        Serial.print(suhuPendingin);
                        Serial.print(" Derajat Celcius");

                        indexMenu++;
                    } else {
                        Serial.println("Gagal, input suhu tidak valid");
                        inputString = "";
                    }

                    resetMenu = true;
                }

                lcd.setCursor(0,0);
                lcd.print("PENDINGIN");

                lcd.setCursor(0,1);
                lcd.print("Suhu:");

                // Print Suhu Setpoint
                lcd.setCursor(5,1);
                lcd.print(inputString);
            break;
        }
    }


    else if(indexMenu == 2) {
        if(tombol >= '0' && tombol <= '9') {
            inputString += tombol;
        }
        else if(tombol == 'd') {
            resetMenu = true;
            indexMenu--;

        }

        switch (Menu){

        /***************************************************************************
         * Menu PASTEURISASI
         * Input Volume pasteurisasi
        ***************************************************************************/
        case 1:
            if(tombol >= '0' && tombol <= '3') {
                volume = tombol - '0';
                Serial.println(volume);

                if(volume == 1) {
                    menit = 30;
                } else if(volume == 2) {
                    menit = 40;
                } else if(volume == 3) {
                    menit = 50;
                }

                jalan = true;
                resetMenu = true;

            }

            if(jalan == false && clearProcess == false) {
                lcd.setCursor(0, 0);
                lcd.print("1.10L/30m/80RPM");
                lcd.setCursor(0, 1);
                lcd.print("2.13L/40m/90RPM");
                lcd.setCursor(0, 2);
                lcd.print("2.13L/50m/100RPM");
            }


        break;
        
        /***************************************************************************
         * Menu FERMENTASI
         * Input durasi fermentasi
        ***************************************************************************/
        case 2:
            if(tombol == 'a') {
                if(inputString.toInt() > 0) {

                    durasiFermentasi = inputString.toInt();

                    Serial.print("Durasi fermentasi berhasil di input, ");
                    Serial.print(durasiFermentasi);
                    Serial.println(" Jam");

                    jam = durasiFermentasi;
                    jalan = true;
                    resetMenu = true;

                    // proses("fermentasi", 1);


                } else {
                    Serial.println("Gagal, input durasi tidak valid ");
                }
            }

            if(jalan == false && clearProcess == false) {
                lcd.setCursor(0,0);
                lcd.print("FERMENTASI");
                lcd.setCursor(0,1);
                lcd.print("Durasi:");
                lcd.setCursor(7,1);
                lcd.print(inputString);
                lcd.setCursor(10,1);
                lcd.print("jam");
            }
        break;

        /***************************************************************************
         * Menu PENDINGIN
         * Input durasi Pendingin
        ***************************************************************************/
        case 3:
            if(tombol == 'a') {
                if(inputString.toInt() > 0) {

                    if(inputString.toInt() > 60) {
                        jam = inputString.toInt() / 60;
                        durasiPendingin = inputString.toInt() % 60;
                    } else {
                        durasiPendingin = inputString.toInt();
                    }

                    Serial.print("Durasi pendingin berhasil di input, ");
                    Serial.print(durasiPendingin);
                    Serial.println(" Menit");

                    menit = durasiPendingin;
                    jalan = true;
                    resetMenu = true;

                } else {
                    Serial.println("Gagal, input durasi tidak valid ");
                }
            }

            if(jalan == false && clearProcess == false) {
                lcd.setCursor(0,0);
                lcd.print("PENDINGIN");
                lcd.setCursor(0,1);
                lcd.print("Durasi:");
                lcd.setCursor(7,1);
                lcd.print(inputString);
                lcd.setCursor(10,1);
                lcd.print("menit");
            }
        break;
        }
    }

    /***************************************************************************
     * Clear Proses
     * Mereset Timer, Menu, dan IO ketika waktu sudah tercapai
    ***************************************************************************/
    if(clearProcess) {
        if(tombol == 'd') {
            jam = 0;
            menit = 0;
            detik = 0;
            Menu = 0;
            indexMenu = 0;
            viewChar = false;
            
            clearProcess = false;
        }
        lowOutput();

        currentCharMillis = millis();
        if(currentCharMillis - startCharMillis > blinkChar) {
            viewChar = !viewChar;

            startCharMillis = currentCharMillis;
        }

        if(viewChar) {
            lcd.setCursor(0,0);
            lcd.print("Proses Selesai");
            digitalWrite(Buzzer, HIGH);
        } else {
            resetMenu = true;
            digitalWrite(Buzzer, LOW);
        }
    }

    if(jalan) {

        currentTimer = millis();
        if(currentTimer - startTimer > timerInterval) {
            //************** DECREMENT WAKTU **************//
            detik--;
            if(detik <= 0) {
                detik = 59;
                menit--;
            }
            if(menit < 0) {
                menit = 59;
                jam--;
            }
            if(jam < 0) {
                clearProcess = true;
                jalan = false;
            }

            //************** AMBIL DATA TEMPERATUR **************//
            currentSuhu = DS18B20.getTempCByIndex(0);

            //************** ZERO PADDING **************//
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

            //************** PEMBACAAN RPM **************//
            if(digitalRead(IRSensor) == LOW && rpmTriggered == false ) {
                if(count == 0) {
                    startRpmRev = millis();
                    count++;
                } else {
                    endRpmRev = millis();
                    count = 0;
                }
                rpmTriggered = true;
            } else if(digitalRead(IRSensor) == HIGH && rpmTriggered == true) {
                RevDuration = startRpmRev - endRpmRev;
                rpm = (1 /(RevDuration / 1000)) * 60;
                if(rpm > 0) {
                    Serial.println(rpm);
                }
                rpmTriggered = false;
            }


            //************** DISPLAY **************//
            lcd.setCursor(8,1);
            lcd.print(currentSuhu);
            if(currentSuhu < 10) {
                lcd.setCursor(12,1);
                lcd.print((char)223);
                lcd.setCursor(13,1);
                lcd.print("C ");
            } else {
                lcd.setCursor(13,1);
                lcd.print((char)223);
                lcd.setCursor(14,1);
                lcd.print("C");
            }
            lcd.setCursor(0,3);
            lcd.print(dispJam);

            lcd.setCursor(2,3);
            lcd.print(":");

            lcd.setCursor(3,3);
            lcd.print(dispMenit);

            lcd.setCursor(5,3);
            lcd.print(":");

            lcd.setCursor(6,3);
            lcd.print(dispDetik);

            Serial.print(dispJam);
            Serial.print(":");
            Serial.print(dispMenit);
            Serial.print(":");
            Serial.println(dispDetik);

            startTimer = currentTimer;
        }

        //************** Proses Output **************//
        if(Menu == 1) {                                    // Untuk Output Pasteurisasi
            
            if(volume == 1) {
                analogWrite(Motor, 20);
            }
            else if(volume == 2) {
                analogWrite(Motor, 40);
            }
            else if(volume == 3) {
                analogWrite(Motor, 60);
            }

            if(currentSuhu < suhuPasteurisasi) {
                digitalWrite(heater, HIGH);
                digitalWrite(Kipas, LOW);
            } else {
                digitalWrite(heater, LOW);
                digitalWrite(Kipas, HIGH);
            }

            lcd.setCursor(0,0);
            lcd.print("PASTEURISASI");
            
            lcd.setCursor(5,1);
            lcd.print("->");

            lcd.setCursor(0,1);
            lcd.print(suhuPasteurisasi);
            if(suhuPasteurisasi < 10) {
                lcd.setCursor(2,1);
                lcd.print((char)223);
                lcd.setCursor(3,1);
                lcd.print("C ");
            } else {
                lcd.setCursor(2,1);
                lcd.print((char)223);
                lcd.setCursor(3,1);
                lcd.print("C");
            }

            lcd.setCursor(0,2);
            lcd.print("Rpm:");

            lcd.setCursor(4,2);
            lcd.print(rpm);
        }
        else if(Menu == 2) {                                // Untuk Output Fermentasi
            if(currentSuhu < suhuFermentasi) {
                digitalWrite(heater, HIGH);
            } else {
                digitalWrite(heater, LOW);
            }

            lcd.setCursor(0,0);
            lcd.print("FERMENTASI");
            
            lcd.setCursor(5,1);
            lcd.print("->");

            lcd.setCursor(0,1);
            lcd.print(suhuFermentasi);
            if(suhuFermentasi < 10) {
                lcd.setCursor(2,1);
                lcd.print((char)223);
                lcd.setCursor(3,1);
                lcd.print("C ");
            } else {
                lcd.setCursor(2,1);
                lcd.print((char)223);
                lcd.setCursor(3,1);
                lcd.print("C");
            }
        }
        else if(Menu == 3) {                                // Untuk Output Pendinginan
            if(currentSuhu > suhuPendingin) {
                digitalWrite(Kipas, HIGH);
            } else {
                digitalWrite(Kipas, LOW);
            }

            lcd.setCursor(0,0);
            lcd.print("PENDINGIN");

            lcd.setCursor(5,1);
            lcd.print("->");

            lcd.setCursor(0,1);
            lcd.print(suhuPendingin);
            if(suhuPendingin < 10) {
                lcd.setCursor(2,1);
                lcd.print((char)223);
                lcd.setCursor(3,1);
                lcd.print("C ");
            } else {
                lcd.setCursor(2,1);
                lcd.print((char)223);
                lcd.setCursor(3,1);
                lcd.print("C");
            }
        }
    }
}

void lowOutput() {
    // Setelah proses selesai, set semua output menjadi 0
    digitalWrite(heater, LOW);
    digitalWrite(Kipas, LOW);
    digitalWrite(Motor, LOW);
}
