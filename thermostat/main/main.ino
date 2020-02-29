#include <math.h>

int pinA = 3;  // Connected to CLK on KY-040
int pinB = 4;  // Connected to DT on KY-040
int desiredTemperature = 20; 
int minTemperature = 0;
int maxTemperature = 30;
int pinALast;  
int aVal;
int payloadOut = 13;

int currentTempKatodPins[] = {A5, A4};
int desiredTempKatodPins[] = {A3, A2};

int segmentsPins[] = {6, 7, 8, 9, 10, 11, 12};

//{A, B, C, D, E, F, G} - распиновка сегментов
int digits[10][7] = {
  {1, 1, 1, 1, 1, 1, 0}, //Цифра 0
  {0, 1, 1, 0, 0, 0, 0}, //Цифра 1
  {1, 1, 0, 1, 1, 0, 1}, //Цифра 2
  {1, 1, 1, 1, 0, 0, 1}, //Цифра 3
  {0, 1, 1, 0, 0, 1, 1}, //Цифра 4
  {1, 0, 1, 1, 0, 1, 1}, //Цифра 5
  {1, 0, 1, 1, 1, 1, 1}, //Цифра 6
  {1, 1, 1, 0, 0, 0, 0}, //Цифра 7
  {1, 1, 1, 1, 1, 1, 1}, //Цифра 8
  {1, 1, 1, 1, 0, 1, 1}  //Цифра 9
};

void setup() {
  pinMode(pinA, INPUT);
  pinMode(pinB, INPUT);
  pinMode(payloadOut, OUTPUT);
  // Read Pin A
  // Whatever state it's in will reflect the last position   
  pinALast = digitalRead(pinA);   

  for (int i = 0; i < 2; i++) {
    pinMode(currentTempKatodPins[i], OUTPUT);
    digitalWrite(currentTempKatodPins[i], HIGH);
  }

  for (int i = 0; i < 2; i++) {
    pinMode(desiredTempKatodPins[i], OUTPUT);
    digitalWrite(desiredTempKatodPins[i], HIGH);
  }
  
  for (int i = 0; i < 7; i++) {
    pinMode(segmentsPins[i], OUTPUT);
  }
  
  Serial.begin(9600);
}

double lastTemp = -999;
int averageSpeed = 50.0;

double readThermister() {
  int RawADC = 1023 - analogRead(0);
  double temp;
  temp = log(((10240000/RawADC) - 10000));
  temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * temp * temp ))* temp ); // Уравнение Стейнхарта-Харта
  temp = temp - 273.15; // Кельвин -> Цельсий 

  if (lastTemp != -999) {
    temp = ((averageSpeed - 1) * lastTemp + temp) / averageSpeed;
  }
  
  lastTemp = temp;  
  return temp;
}

bool readEncoder() { 
  aVal = digitalRead(pinA);
  bool changed = false;
   
  if (aVal != pinALast){ // Means the knob is rotating
    // if the knob is rotating, we need to determine direction
    // We do that by reading pin B.
    if (digitalRead(pinB) != aVal) {  // Means pin A Changed first - We're Rotating Clockwise
      desiredTemperature--;

      if (desiredTemperature < minTemperature) {
        desiredTemperature = minTemperature;
      }
    } else {// Otherwise B changed first and we're moving CCW
      desiredTemperature++;

      if (desiredTemperature > maxTemperature) {
        desiredTemperature = maxTemperature;
      }
    }
          
    changed = true;
  } 
  pinALast = aVal;

  return changed;
}

void displayTemp(int value, int katodPins[]) {
  value %= 100;
  int values[] = {value / 10, value % 10};

  for (int i = 0; i< 2; i++) {    
    for (int k = 0; k < 7; k++) {// Каждый сегмент по очереди - исходя из заданной карты
      digitalWrite(segmentsPins[k], ((digits[values[i]][k] == 1) ? HIGH : LOW));
    }
    
    digitalWrite(katodPins[i], LOW);
    delay(1); 
    digitalWrite(katodPins[i], HIGH); 
  }
}

void loop() {
  double currentTemperature = readThermister();
  bool changed = readEncoder();

  if (changed) {
    Serial.print("Desired temperature: ");
    Serial.print(desiredTemperature);    

    Serial.print(" Current temperature: ");
    Serial.println(currentTemperature);
  }

  displayTemp(currentTemperature, currentTempKatodPins);
  displayTemp(desiredTemperature, desiredTempKatodPins);

  if (desiredTemperature > currentTemperature) {
    digitalWrite(payloadOut, 1);
  } else {
    digitalWrite(payloadOut, 0);
  }

  delay(10);
}


// 514 => 25 ?
// 523 => 24
// 523 => 23
// 546 => 22
// 557 => 21
// 563 => 20
// 575 => 19
