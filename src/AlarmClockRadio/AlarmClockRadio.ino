#include <LiquidCrystal.h>
#include <string.h>

int second = 0; // this is to keep track of the seconds

/* parameters for the normal clock */
String   am_pm = "AM";               // this variable is for the clock am or pm 
volatile uint16_t minute = 0x01;     // these are in hex so that it is easier ot display
volatile uint8_t  hour   = 0x01;     // this for the clock minute
String   day    = "Monday";          // this variable is for the day. init it to monday

/* parameters for the alarm */
String   alarm_am_pm = "AM";               // this variable is for the alarm am or pm
volatile uint16_t alarm_minute = 0x00;     // this variable is for the alarm minute
volatile uint8_t  alarm_hour   = 0x0;      // this variable is for the alarm hour

volatile uint8_t  count        = 0x0;   // count is to know what time variable we are going to increment: minute, hour, day, am/pm
volatile uint8_t  screen       = 0xff;  // variable is to know what screen we are on. default to clock screen
uint8_t alarm_off = 0x00;     // signal to check if the alarm is off

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// init the lcd
void lcdInit(){
  lcd.begin(16, 2);     // set up the LCD's numbeer of column and rows
  lcd.clear();          // clear the screen
// default appearance
  lcd.print(day);       // print default day
  lcd.setCursor(0, 1);  // set cursor
  lcd.print(" 1:01 ");  // default time
  lcd.print(am_pm);     // default time
}

// enable interrupts
void interruptEnable(){
  pinMode(2, INPUT_PULLUP);                     // this is the external interrupt pin on the Arduino UNO R3
  pinMode(3, INPUT_PULLUP);                     // thi is an external interrupt pin on the Arduino UNO R3
  attachInterrupt(0, setTime, RISING);          // this will enable pin 2 as an interrupt, ISR will be setTime function, interrupt occurs on rising edge of button press
  attachInterrupt(1, modeChange, RISING);       // this will enable pin 3 as an interrupt, ISR will change which feature we will be incrementing (min., hour, day, am/pm)

  // set up the timer to interrupt every second
  TCCR1A = 0;     // clear control register A for timer 1
  TCCR1B = 0;     // clear control register B for timer 1
  TCNT1 = 3036;   // set value to count up to to equal 1 second
  TCCR1B |= (1<<CS12);    // set the clock prescaler to be 256
  TIMSK1 |= (1<<TOIE1);   // enable the overflow interrupt for timer 1. when the coutner exceeds TCNT1 there will be an interrupt
}

// this is to display the alarm screen
void alarmScreen(){
  lcd.setCursor(0,0);       // set the cursor
  lcd.print("ALARM    ");   // print alarm to indicate alarm screen
  lcd.setCursor(0,1);       // set cursor
  if(alarm_hour < 0x0A){    // if the alarm hour is less than 10 we need to put blank space
    lcd.print(" ");         // print a blank space
    lcd.print(alarm_hour);  // print the alarm hour
  }
  else
    lcd.print(alarm_hour);  // print the alarm hour
  lcd.setCursor(3, 1);      // set the cursor
  if(alarm_minute < 0x0A){  // if the alarm minute is less than 10 we need to put a 0
    lcd.print("0");         // print a 0 
    lcd.print(alarm_minute);// print the alarm minute
  }
  else
    lcd.print(alarm_minute);// print the alarm minute
  lcd.setCursor(6, 1);      // set the cursor
  lcd.print(alarm_am_pm);   // print am or pm
}

// this is to display the clock screen
void clockScreen(){
  lcd.setCursor(0,0);       // set the cursor
  lcd.print(day);           // print the day it is 
  lcd.setCursor(0,1);       // set the cursor
  if(alarm_hour < 0x0A){    // if the hour is less than 10 we need to put a blank space
    lcd.print(" ");         // print a blank space
    lcd.print(hour);        // print the hour
  }
  else
    lcd.print(hour);        // print the hour
  lcd.setCursor(3, 1);      // set the cursor
  if(minute < 0x0A){        // if the minute is less than 10 we need to put a 0
    lcd.print("0");         // print a 0
    lcd.print(minute);      // print the minute
  }
  else
    lcd.print(minute);      // print the minute
  lcd.setCursor(6, 1);      // set the cursor
  lcd.print(am_pm);         // print am or pm
}

void setup() {
  Serial.begin(9600);
  pinMode(13, INPUT_PULLUP);  // connect to button 3
  pinMode(15, OUTPUT);        // use as radio input voltage
  digitalWrite(15, LOW);      // set the radio to be off initially
  lcdInit();                  // initialize the LCD
  interruptEnable();          // initialize interupts
} 

// main loop
void loop() {
  if((alarm_hour == hour) && (alarm_minute == minute) && (alarm_am_pm == am_pm) && (alarm_off == 0x00)){  // check if it is time to turn on the radio
    digitalWrite(15, HIGH);   //  turn on the radio
    if(digitalRead(13))       // this button will turn off the alarm
      alarm_off = 0xff;
  }
  else if((alarm_hour == hour) && (alarm_minute == minute) && (alarm_am_pm == am_pm) && (alarm_off == 0xff)){ // check if the user turned off the radio
    digitalWrite(15, LOW);    // turn off the radio
    alarm_off = 0xff;         // keep the radio off
  }
  else{
    digitalWrite(15, LOW);    // turn off the radio
    alarm_off = 0x00;         // init signal for when the alarm is set to go off again
  }
}

// this will iterate through the days
void changeDay(){
  lcd.setCursor(0,0);           // set cursor
  lcd.print("          ");      // clear row
  lcd.setCursor(0,0);           // set cursor
  if(day == "Monday")           // if monday, change to tuesday   
    day = "Tuesday";
  else if (day == "Tuesday")    // if tuesday, change to wednesday
    day = "Wednesday";
  else if (day == "Wednesday")  // if wednesday, change to thursday
    day = "Thursday";
  else if (day == "Thursday")   // if thursday, change to friday
    day = "Friday";
  else if (day == "Friday")     // if friday, change to saturday
    day = "Saturday";
  else if (day == "Saturday")   // if saturday, change to sunday
    day = "Sunday";
  else if (day == "Sunday")     // if sunday, change to monday
    day = "Monday";
  lcd.print(day);               // print the day on the lcd
}

// this ISR will increment one of the time components
void setTime(){
  if(screen == 0xff && !digitalRead(13)){ // if we are on the clock screen and button 3 is not being pressed we will increment the time when this interrupt occurs
    if(count == 0x00){                    // if the count is 0x00 we will increment the minute
      lcd.setCursor(3, 1);                // set cursor
      minute = (minute + 0x01) % 0x3C;    // increment the minute
      if(minute < 0x0A){                  // if the minute is less than 10 we will put a 0 first
        lcd.print("0");                   // print a 0
        lcd.print(minute);                // print the minute
      }
      else
        lcd.print(minute);                // print the minute
    } 
    else if(count == 0x01){               // if the count is 0x01 we will increment the hours
      hour = ((hour + 0x1) % 0xD);        // when hou reaches 12 we will go back to 0
      if (hour == 0x00)                   // if hour is 0 go to 1
        hour = 0x01; 
      lcd.setCursor(0, 1);                // set cursor
      if(hour < 0x0A){                    // if hour is less than 10 we need to put a blank
        lcd.print(" ");                   // print a blank
        lcd.print(hour);                  // print the hour
      }
      else{
        lcd.print(hour);                  // print the hour
      }
    } 
    else if(count == 0x02){               // if the count is 0x02 we will change the day
      changeDay();                        // this function will change the day
    }
    else if(count == 0x03){               // if the count is 0x03 we will change from AM to PM or PM to AM
      lcd.setCursor(6, 1);                // set the cursor
      if (am_pm == "AM")                  // if we are in AM change to PM
        am_pm = "PM";                     // set PM
      else                                // if we are not in AM change to AM
        am_pm = "AM";                     // set AM     
      lcd.print(am_pm);                   // print AM or PM
    }
  }
  else if(screen == 0x00 && !digitalRead(13)){      // if the we are on the alarm screen and button 3 is not being pressed we will increment the alarm values
    if(count == 0x00){                              // if the count is 0x00 we will increment the alarm minute
      lcd.setCursor(3, 1);                          // set the cursor for tha alarm minute
      alarm_minute = (alarm_minute + 0x01) % 0x3C;  // increment the alarm minute
      if(alarm_minute < 0x0A){                      // if the alarm minute is less than 10 we will put a 0 first
        lcd.print("0");                             // print a 0 
        lcd.print(alarm_minute);                    // print the alarm minute
      }
      else
        lcd.print(alarm_minute);                    // print the alarm minute
    } 
    else if(count == 0x01){                         // if the count is 0x01 we will increment the alarm hour
      alarm_hour = ((alarm_hour + 0x1) % 0xD);      // increment the alarm hour
/*  i commented this out so the user can use the clock without the alarm by setting the alarm hour value to 1
      if (alarm_hour == 0x00)
        alarm_hour = 0x01; 
*/
      
      lcd.setCursor(0, 1);                          // set the cursor
      if(alarm_hour < 0x0A){                        // if the alarm hour is less than 10 we need to put a blank space
        lcd.print(" ");                             // print the blank space
        lcd.print(alarm_hour);                      // print the alarm hour
      }
      else
        lcd.print(alarm_hour);                      // print the alarm hour
    }
    else if(count == 0x02){}                        // do nothing if the count is 0x02
    else if(count == 0x03){                         // if the count is 0x03 we will change alarm from AM to PM or PM to AM 
      lcd.setCursor(6, 1);                          // set the cursor
      if (alarm_am_pm == "AM")                      // if alarm is AM change to PM 
        alarm_am_pm = "PM";                         // set PM
      else                                          // if alarm is not AM change to AM 
        alarm_am_pm = "AM";                         // set AM 
      lcd.print(alarm_am_pm);                       // print AM 
    }
  }
  else if (digitalRead(13)){                        // if button 3 is being pressed we will change the screen
    if(screen == 0xff){                             // if on the clock screen change to the alarm screen
      alarmScreen();                                // display the alarm screen
      screen = 0x00;                                // change signal to prepare to change to clock screen
    }
    else{                                           // if not on the clock screen change to the clock screen
      clockScreen();                                // display the clock screen
      screen = 0xff;                                // change the signal to prepare to change to alarm screen              
    } 
  }
}

// this will go through each time component
void modeChange(){
  count = (count + 0x1) % 0x4;
}

// this ISR is called when there is an overflow for timer1
ISR(TIMER1_OVF_vect){
  Serial.println(second);
  TCNT1 = 3036; // set the value to count up to equal 1 second
  second = (second + 1) % 0x3C;  // 0x3c is equal to 60. when the seconds have reached 60 we go back to 0

  // when seconds equal 0x00 we know that a minute has now passed so we will have to increment the minute value and change update the LCD display
  if(second == 0x00){
    minute = (minute + 0x01) % 0x3C;  // 0x3c us equal to 60. when the minutes have reached 59, the next value will be 00
    lcd.setCursor(3, 1);
    if(minute < 0x0A){  // if the minute value is less than 10 we have to make sure that there is a '0' in the tens column
      lcd.print("0");
      lcd.print(minute);  // print the minute value
    }
    else
      lcd.print(minute);  // print the minute value
  }
  // if the minute is equal to 0x00 and seconds are equal to 0x00 this means that an hour has just passed so we need to update this
  if(minute == 0x00 && second == 0x00){
    hour = (hour + 0x01) % 0xD;   // update the hour by 1. after reaching 12 the hour will go back to 0
    if (hour == 0x00)   // set the hour to 1 if the hour is 0
        hour = 0x01;
    lcd.setCursor(0, 1);  // set position of lcd cursor
      if(hour < 0x0A){  // if hour is less than 10 we need to make sure that there is a blank value in the beginning
        lcd.print(" ");   // print blank value to the lcd
        lcd.print(hour);  // print the hour value on the lcd
      }
      else{
        lcd.print(hour);  // print the hour value on the lcd
      }
  }
  if((hour == 0xD) && minute  == 0x00 && second == 0x00){   // if the hour reaches 0x0D and the minutes and seconds are 0 this means that 12 hours have passed so we to change from AM to PM
    if(am_pm == "AM")
      am_pm = "PM";   // if AM we are now in PM
    else
      am_pm = "AM";   // if not AM we are now in AM
    lcd.setCursor(6, 1);  // set lcd cursor
    lcd.print(am_pm);     // display AM or PM
  }
  
  if(hour == 0x0B && minute == 0x3B && am_pm == "PM" && second == 0x3B) // change the day when we are in PM and 12 hours have passed
    changeDay();
}
