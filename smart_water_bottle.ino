#include <Time.h>
#include <TimeLib.h>
#include <Wire.h>
#include <DS1307RTC.h>
#include "LowPower.h"
#include "OneButton.h"

OneButton button(2, true);

const byte interruptPin = 2;
volatile int state = 0;

const int trigPin = 4;
const int echoPin = 5;

int piezoPin = 3;

const int digit[4] = {9,6,7,8};
int digit_value[4];
int digit_value1[4];

int button_press_count = 1;
const int segment[8] = {16,10,12,14,15,17,11,13};
const byte number[10][8] = {{1,1,1,1,1,1,0,0}, //0
                            {0,1,1,0,0,0,0,0}, //1
                            {1,1,0,1,1,0,1,0}, //2
                            {1,1,1,1,0,0,1,0}, //3
                            {0,1,1,0,0,1,1,0}, //4
                            {1,0,1,1,0,1,1,0}, //5
                            {1,0,1,1,1,1,1,0}, //6
                            {1,1,1,0,0,0,0,0}, //7
                            {1,1,1,1,1,1,1,0}, //8
                            {1,1,1,1,0,1,1,0}}; //9

const byte d[8] = {0,1,1,1,1,0,1,1};
const byte a[8] = {1,1,1,0,1,1,1,1};
const byte r[8] = {0,0,0,0,1,0,1,1};
const byte t[8] = {0,0,0,1,1,1,1,1};

int seconds, minutes, hours;

int water_in_ounch[15];
int water_intake_ounch[15];
int water_intake_days[7];
int water_intake_times = 0;
int previous_water_amount = 0;
int total_water_intake_today = 0;
int average_intake_last_week = 0;
int inatke_day = 1;
float average_water_level = 0; //store average of multiple reading
int water_amount_in_ounce = 0; //store calculated amount of water

int idle_time = 0;
int intake_day = 1;
int previous_value = 0;

void setup() {
  Serial.begin(9600);
  pinMode(interruptPin, INPUT_PULLUP);
  // put your setup code here, to run once:
  for(int i=6; i<=17; i++)
      pinMode(i, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  button.attachClick(pressed);
  button.attachDoubleClick(doubleclick);
  button.attachLongPressStart(longPressStart);
  button.attachDuringLongPress(longPress);
}


long previous_state = millis();
int count = 1;
int daily_intake = 0;
int weekly_intake = 0;
long sleep_time = millis();

void loop() {
  read_time();
  button.tick(); // keep watching the push buttons:
  calculation();
  daily_intake = total_water_intake_in_day();
  weekly_intake = average_water_intake_last_week();
  if(button_press_count == 1){
    display_d();
    display_number(daily_intake);
  }
  else if(button_press_count == 2){
    display_a();
    display_number(weekly_intake);
  }
  else if(button_press_count == 3){
    display_r();
    display_number(water_amount_in_ounce);
  }
  else if(button_press_count == 4){
    display_first_2(hours);
    display_last_2(minutes);
  }
  if(idle_time>=120){
    alert();
    alert();
    }
  if((millis() - sleep_time) >= 15000){
    display_off();
    attachInterrupt(digitalPinToInterrupt(interruptPin), blank, FALLING);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    detachInterrupt(digitalPinToInterrupt(interruptPin));
    sleep_time = millis();
    }  
}


void display_digit(int digit){
  for(int i=0; i<8; i++){
    digitalWrite(segment[i], number[digit][i]);
    }
  }


void display_number(int number){
  int i=0;
  while(number>0){
    digit_value[2-i] = number%10;
    number = number/10;
    i++;
    }
  digitalWrite(digit[1], HIGH);
  digitalWrite(digit[2], LOW);
  digitalWrite(digit[3], LOW);
  display_digit(digit_value[0]);
  delay(5);

  digitalWrite(digit[1], LOW);
  digitalWrite(digit[2], HIGH);
  digitalWrite(digit[3], LOW);
  display_digit(digit_value[1]);
  delay(5);

  digitalWrite(digit[1], LOW);
  digitalWrite(digit[2], LOW);
  digitalWrite(digit[3], HIGH);
  display_digit(digit_value[2]);
  delay(5);
  digitalWrite(digit[3], LOW);
  digit_value[0] = 0;
  digit_value[1] = 0;
  digit_value[2] = 0;
  }


void display_first_2(int number){
  digitalWrite(digit[2], LOW);
  digitalWrite(digit[3], LOW);
  int i=0;
  while(number>0){
    digit_value[1-i] = number%10;
    number = number/10;
    i++;
    }
  digitalWrite(digit[0], HIGH);
  digitalWrite(digit[1], LOW);
  display_digit(digit_value[0]);
  delay(3);

  digitalWrite(digit[0], LOW);
  digitalWrite(digit[1], HIGH);
  display_digit(digit_value[1]);
  delay(3);  
  }


void display_last_2(int number){
  digitalWrite(digit[0], LOW);
  digitalWrite(digit[1], LOW);
  int i=0;
  while(number>0){
    digit_value1[1-i] = number%10;
    number = number/10;
    i++;
    }
  digitalWrite(digit[2], HIGH);
  digitalWrite(digit[3], LOW);
  display_digit(digit_value1[0]);
  delay(3);
  digitalWrite(digit[2], LOW);
  digitalWrite(digit[3], HIGH);
  display_digit(digit_value1[1]);
  delay(3);
  }


void display_d(){
  digitalWrite(digit[0], HIGH);
  for(int i=0; i<8; i++){
    digitalWrite(segment[i], d[i]);
    }
  delay(5);
  digitalWrite(digit[0], LOW);
  }


void display_a(){
  digitalWrite(digit[0], HIGH);
  for(int i=0; i<8; i++){
    digitalWrite(segment[i], a[i]);
    }
  delay(5);
  digitalWrite(digit[0], LOW);
  }

  
void display_r(){
  digitalWrite(digit[0], HIGH);
  for(int i=0; i<8; i++){
    digitalWrite(segment[i], r[i]);
    }
  delay(5);
  digitalWrite(digit[0], LOW);
  }

  
void display_t(){
  digitalWrite(digit[0], HIGH);
  for(int i=0; i<8; i++){
    digitalWrite(segment[i], t[i]);
    }
  delay(5);
  digitalWrite(digit[0], LOW);
  }

void display_off(){
  digitalWrite(digit[0], LOW);
  digitalWrite(digit[1], LOW);
  digitalWrite(digit[2], LOW);
  digitalWrite(digit[3], LOW);
  for(int i=0; i<8; i++){
    digitalWrite(segment[i], LOW);
    }
  delay(5);
  }

void read_time() {
 tmElements_t tm;

 if (RTC.read(tm)) {
 seconds = tm.Second;
 minutes = tm.Minute;
 hours = tm.Hour;
  } 
}


int distance_in_cm(){
  long duration, cm;
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  
  duration = pulseIn(echoPin, HIGH);

  // convert the time into a distance
  cm = microsecondsToCentimeters(duration); 
  return cm;
  }

long microsecondsToCentimeters(long microseconds)
  {
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
  }

void alert(){
  tone(piezoPin, 2000, 50);
  tone(piezoPin, 2000, 200);
  //delay(10);
  }


void blank() {
  //tone(piezoPin, 2000, 100);
  //state++;
}

// This function will be called when the button1 was pressed 1 time (and no 2. button press followed).
void pressed() {
  //Serial.println("Button 1 click.");
  button_press_count++;
  alert();
  if(button_press_count == 5){
    button_press_count = 1;
    }
} // click


// This function will be called when the button1 was pressed 2 times in a short timeframe.
void doubleclick() {
  Serial.println("Button 1 doubleclick.");
} // doubleclick


// This function will be called once, when the button1 is pressed for a long time.
void longPressStart() {
  Serial.println("Button 1 longPress start");
} // longPressStart


// This function will be called often, while the button1 is pressed for a long time.
void longPress() {
  Serial.println("Button 1 longPress...");
  water_intake_ounch[water_intake_times - 1] = 0; //ignore last value
} // longPress

void calculation(){
  float water_level = 0;// store level in every step
  int read_value = 0; //read sensor reading in cm

  for(int i=0; i<5; i++){ //take five reading
      read_value = distance_in_cm();
      if(read_value>16 || read_value<3){// unstable reading
          return; //return to calling function because reading is unstable
        }
      else if(read_value<=16 && read_value>=3){//valid value      
          water_level = water_level + read_value;
        }
      delay(10);
   }
  
  average_water_level = 17 - water_level/5; //find average from five reading, 17 = botole height
  water_amount_in_ounce = int(average_water_level*1.87);//16 cm water level = 30 ounch
  if(water_intake_times==0){
     previous_water_amount = water_amount_in_ounce;
     water_intake_times = 1;
    }
  if((water_amount_in_ounce < previous_water_amount-1) && (hours < 24)){//some water is consumed
    water_intake_ounch[water_intake_times - 1] = previous_water_amount - water_amount_in_ounce;
    water_intake_times++;
    previous_water_amount = water_amount_in_ounce;
    idle_time = 0;
    }
  else if(water_amount_in_ounce > previous_water_amount){ //water is refilled
    //water refil here
    previous_water_amount = water_amount_in_ounce;
    }
  else if(water_amount_in_ounce == previous_water_amount){ //no water consumed or reafill
    idle_time+=1;
    }

  if(hours==23 && minutes==59){ // a day is over and all values start from zero for new day
    for(int i=0; i<15; i++){
      water_intake_ounch[i] = 0;
      }
    water_intake_times = 0;
    intake_day++;
    if(intake_day==8){
      intake_day = 1;
      }
    }
}

int total_water_intake_in_day(){//calculate total water intake in a day
  total_water_intake_today = 0;
  for(int i=0; i<water_intake_times; i++){
    total_water_intake_today = total_water_intake_today + water_intake_ounch[i];
    } 
    water_intake_days[intake_day] = total_water_intake_today;
    return total_water_intake_today;
  }

int average_water_intake_last_week(){//calculate average water intake last week
  for(int i=1; i<=intake_day; i++){
    average_intake_last_week = average_intake_last_week + water_intake_days[i-1];
    }
  average_intake_last_week = average_intake_last_week/intake_day;
  return average_intake_last_week;
  }
