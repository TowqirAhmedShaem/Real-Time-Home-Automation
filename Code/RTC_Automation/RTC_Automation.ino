/*
 * Automatic Home  Appliance Controlling On Basis of Time
 * Author : Towqir Ahmed Shaem
 * This project is under development
 */
#include <Wire.h>
#include "ds3231.h"

#define BUFF_MAX 128

uint8_t time[8];
char recv[BUFF_MAX];
unsigned int recv_size = 0;
unsigned long prev, interval = 3000;

void parse_cmd(char *cmd, int cmdsize);
char tempF[6];
float temperature;
struct ts t;
bool firstFlag = true;

// Multiplug connected with arduino pin
#define firstPlug   5
#define secondPlug  4
#define thirdPlug   3
#define fourthPlug  2

// Plug stop time interval Like Stop 2:48 -  2:50
#define firstPlughour1  5
#define firstPlugmin1   1
#define firstPlughour2  17
#define firstPlugmin2   30

//Filter
#define secondPlughour1  9
#define secondPlugmin1   30
#define secondPlughour2  11
#define secondPlugmin2   0

#define secondPlughour3  14
#define secondPlugmin3   45
#define secondPlughour4  15
#define secondPlugmin4   1

//Light
#define thirdPlughour1  5
#define thirdPlugmin1   1
#define thirdPlughour2  17
#define thirdPlugmin2   30

//Filter
#define fourthPlughour1  9
#define fourthPlugmin1   30
#define fourthPlughour2  11
#define fourthPlugmin2   0

//Filter
#define fourthPlughour3  14
#define fourthPlugmin3   45
#define fourthPlughour4  15
#define fourthPlugmin4   1

void setup()
{

  Serial.begin(9600);
  Wire.begin();
  
  DS3231_init(DS3231_CONTROL_INTCN);
  
  memset(recv, 0, BUFF_MAX);
  Serial.println("GET time");


  //Serial.println("Setting time");
  //parse_cmd("T305902116022019",16);
  //           TssmmhhWDDMMYYYY

  pinMode(firstPlug, OUTPUT);
  pinMode(secondPlug, OUTPUT);
  pinMode(thirdPlug, OUTPUT);
  pinMode(fourthPlug, OUTPUT);

  digitalWrite(firstPlug, HIGH);
  digitalWrite(secondPlug, HIGH);
  digitalWrite(thirdPlug, HIGH);
  digitalWrite(fourthPlug, HIGH);
  delay(1000);
}

void loop()
{
  char in;
  char buff[BUFF_MAX];
  unsigned long now = millis();

  // show time once in a while
  if ((now - prev > interval) && (Serial.available() <= 0)) {
    DS3231_get(&t);
    temperature = DS3231_get_treg(); //Get temperature
    dtostrf(temperature, 5, 1, tempF);
    Serial.print("Temp : ");
    Serial.print(tempF);
    Serial.println(" C ");

    Serial.print("Year : ");
    Serial.println(t.year);
    Serial.print("Month : ");
    Serial.println(t.mon);
    Serial.print("Day : ");
    Serial.println(t.mday);
    Serial.print("Week Day : ");
    Serial.println(t.wday);


    Serial.print("Hour : ");
    Serial.println(t.hour);
    Serial.print("Minute : ");
    if (t.min < 10)
    {
      Serial.print("0");
    }
    Serial.println(t.min);


    snprintf(buff, BUFF_MAX, "%d.%02d.%02d %02d:%02d:%02d", t.year,
             t.mon, t.mday, t.hour, t.min, t.sec);
    Serial.println();
    Serial.println(buff);
    prev = now;

    plugChecking();

  }

  if (Serial.available() > 0) {
    in = Serial.read();

    if ((in == 10 || in == 13) && (recv_size > 0)) {
      parse_cmd(recv, recv_size);
      recv_size = 0;
      recv[0] = 0;
    } else if (in < 48 || in > 122) {
      ;       // ignore ~[0-9A-Za-z]
    } else if (recv_size > BUFF_MAX - 2) {   // drop lines that are too long
      // drop
      recv_size = 0;
      recv[0] = 0;
    } else if (recv_size < BUFF_MAX - 2) {
      recv[recv_size] = in;
      recv[recv_size + 1] = 0;
      recv_size += 1;
    }

  }
}



void plugChecking() {
  
  // First Plug
  if ( checkOnOFFState(t.hour, t.min, firstPlughour1, firstPlugmin1, firstPlughour2, firstPlugmin2) ) {
    digitalWrite(firstPlug, HIGH);
    firstFlag = true;
  }
  else {
    if (firstFlag) {
      delay(2000);
      digitalWrite(firstPlug, LOW);
      delay(2000);
      digitalWrite(firstPlug, HIGH);
      delay(2000);
      //digitalWrite(firstPlug, LOW);
      firstFlag = false;

    }
    digitalWrite(firstPlug, LOW);

  }

  // Second Plug
  if ( checkOnOFFState(t.hour, t.min, secondPlughour1, secondPlugmin1, secondPlughour2, secondPlugmin2) ) {
    digitalWrite(secondPlug, HIGH);
  }
    if ( checkOnOFFState(t.hour, t.min, secondPlughour3, secondPlugmin3, secondPlughour4, secondPlugmin4) ) {
    digitalWrite(secondPlug, HIGH);
  }
  else {
    digitalWrite(secondPlug, LOW);
  }

  // Third Plug
  if ( checkOnOFFState(t.hour, t.min, thirdPlughour1, thirdPlugmin1, thirdPlughour2, thirdPlugmin2) ) {
    digitalWrite(thirdPlug, HIGH);
  }
  else {
    digitalWrite(thirdPlug, LOW);
  }

  // Fourth Plug
  Serial.println("Fourth Plug");

  if ( checkOnOFFState(t.hour, t.min, fourthPlughour1, fourthPlugmin1, fourthPlughour2, fourthPlugmin2)) {
    digitalWrite(fourthPlug, HIGH);
  }
    if ( checkOnOFFState(t.hour, t.min, fourthPlughour3, fourthPlugmin3, fourthPlughour4, fourthPlugmin4)) {
    digitalWrite(fourthPlug, HIGH);
  }
  else {
    digitalWrite(fourthPlug, LOW);
  }
}


bool checkOnOFFState( int presenthour, int presentminute, int primaryhour, int primaryminute, int secondaryhour, int secondaryminute ) {
  if ( presenthour > primaryhour && presenthour < secondaryhour) {
    return true;
  }
  else {
    if ( presenthour == primaryhour && primaryhour == secondaryhour ) {
      if ( presentminute >= primaryminute && presentminute <= secondaryminute) {
        return true;
      }
    }
    if ( presenthour == primaryhour && primaryhour != secondaryhour) {
      if ( presentminute >= primaryminute ) {
        return true;
      }
      else
      return false;
    }
    if ( presenthour == secondaryhour && primaryhour != secondaryhour ) {
      if ( presentminute <= secondaryminute ) {
        return true;
      }
      else
      return false;
    }
    else {
      return false;
    }

  }

}

void parse_cmd(char *cmd, int cmdsize)
{
  uint8_t i;
  uint8_t reg_val;
  char buff[BUFF_MAX];
  struct ts t;

  //snprintf(buff, BUFF_MAX, "cmd was '%s' %d\n", cmd, cmdsize);
  //Serial.print(buff);

  // TssmmhhWDDMMYYYY aka set time
  if (cmd[0] == 84 && cmdsize == 16) {
    //T355720619112011
    t.sec = inp2toi(cmd, 1);
    t.min = inp2toi(cmd, 3);
    t.hour = inp2toi(cmd, 5);
    t.wday = cmd[7] - 48;
    t.mday = inp2toi(cmd, 8);
    t.mon = inp2toi(cmd, 10);
    t.year = inp2toi(cmd, 12) * 100 + inp2toi(cmd, 14);
    DS3231_set(t);
    Serial.println("OK");
  } else if (cmd[0] == 49 && cmdsize == 1) {  // "1" get alarm 1
    DS3231_get_a1(&buff[0], 59);
    Serial.println(buff);
  } else if (cmd[0] == 50 && cmdsize == 1) {  // "2" get alarm 1
    DS3231_get_a2(&buff[0], 59);
    Serial.println(buff);
  } else if (cmd[0] == 51 && cmdsize == 1) {  // "3" get aging register
    Serial.print("aging reg is ");
    Serial.println(DS3231_get_aging(), DEC);
  } else if (cmd[0] == 65 && cmdsize == 9) {  // "A" set alarm 1
    DS3231_set_creg(DS3231_CONTROL_INTCN | DS3231_CONTROL_A1IE);
    //ASSMMHHDD
    for (i = 0; i < 4; i++) {
      time[i] = (cmd[2 * i + 1] - 48) * 10 + cmd[2 * i + 2] - 48; // ss, mm, hh, dd
    }
    uint8_t flags[5] = { 0, 0, 0, 0, 0 };
    DS3231_set_a1(time[0], time[1], time[2], time[3], flags);
    DS3231_get_a1(&buff[0], 59);
    Serial.println(buff);
  } else if (cmd[0] == 66 && cmdsize == 7) {  // "B" Set Alarm 2
    DS3231_set_creg(DS3231_CONTROL_INTCN | DS3231_CONTROL_A2IE);
    //BMMHHDD
    for (i = 0; i < 4; i++) {
      time[i] = (cmd[2 * i + 1] - 48) * 10 + cmd[2 * i + 2] - 48; // mm, hh, dd
    }
    uint8_t flags[5] = { 0, 0, 0, 0 };
    DS3231_set_a2(time[0], time[1], time[2], flags);
    DS3231_get_a2(&buff[0], 59);
    Serial.println(buff);
  } else if (cmd[0] == 67 && cmdsize == 1) {  // "C" - get temperature register
    Serial.print("temperature reg is ");
    Serial.println(DS3231_get_treg(), DEC);
  } else if (cmd[0] == 68 && cmdsize == 1) {  // "D" - reset status register alarm flags
    reg_val = DS3231_get_sreg();
    reg_val &= B11111100;
    DS3231_set_sreg(reg_val);
  } else if (cmd[0] == 70 && cmdsize == 1) {  // "F" - custom fct
    reg_val = DS3231_get_addr(0x5);
    Serial.print("orig ");
    Serial.print(reg_val, DEC);
    Serial.print("month is ");
    Serial.println(bcdtodec(reg_val & 0x1F), DEC);
  } else if (cmd[0] == 71 && cmdsize == 1) {  // "G" - set aging status register
    DS3231_set_aging(0);
  } else if (cmd[0] == 83 && cmdsize == 1) {  // "S" - get status register
    Serial.print("status reg is ");
    Serial.println(DS3231_get_sreg(), DEC);
  } else {
    Serial.print("unknown command prefix ");
    Serial.println(cmd[0]);
    Serial.println(cmd[0], DEC);
  }
}
