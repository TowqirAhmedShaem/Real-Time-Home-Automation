/*
   Automatic Home  Appliance Controlling On Basis of Time
   Author : Towqir Ahmed Shaem
   This project is under development
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

//#define debug true

// Multiplug connected with arduino pin
#define firstPlug   5
#define secondPlug  4
#define thirdPlug   3
#define fourthPlug  2

const int row = 6;
const int column = 7;

int timeschedule[row][column] = {

//#define firstPlug   5
//#define secondPlug  4
//#define thirdPlug   3
//#define fourthPlug  2

  {5,  05, 15,  18, 30, 1, 1 },
  {4,  10, 15,  11, 35, 2, 1 },
  {4,  22, 30,  23, 30, 3, 1 },  
  {3,  05, 15,  18, 30, 4, 1 },
  {2,  22, 30,  23, 30, 5, 1 },
  {2,  10, 15,  11, 35, 6, 1 }
  
};

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


  for ( int i = 0; i < row; i++) {

    int plug          = timeschedule[i][0];
    int primaryTime   = ( timeschedule[i][1] * 100 + timeschedule[i][2] );
    int secondaryTime = ( timeschedule[i][3] * 100 + timeschedule[i][4] );
    int presentTime    = ( t.hour * 100 + t.min );
    
#ifdef debug
    Serial.print("Plug NUmber");
    Serial.println(plug);
    Serial.print("Primary TIme");
    Serial.println(primaryTime);
    Serial.print("Secondary Time : ");
    Serial.println(secondaryTime);
    Serial.print("Present Time : ");
    Serial.println(presentTime);
#endif

    if ( presentTime >= primaryTime && presentTime <= secondaryTime ) {
      timeschedule[i][6] = 1;
      
#ifdef debug
      Serial.print(plug);
      Serial.println(" : HIGH");
#endif
    }
    else {
      timeschedule[i][6] = 0;
      
#ifdef debug
      Serial.print(plug);
      Serial.println(" : LOW");
#endif
    }
  }

  bool flag = false;
  
   for( int i = 0; i< row; i++){
    for ( int j = 0; j< row; j++){
      if( timeschedule[i][0] == timeschedule[j][0] ){
  
          if( timeschedule[i][6] == timeschedule[j][6] &&  timeschedule[i][6] == 0){
            flag = false;
          }
          else{
            flag = true;
            break;
          }
        }
 
    }

    if(flag){
      digitalWrite(timeschedule[i][0], HIGH);
    }
    else{
      digitalWrite(timeschedule[i][0], LOW);

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
