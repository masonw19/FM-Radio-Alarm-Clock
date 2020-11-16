// lcd.home() this will set the cursor to (0, 0)
// lcd.setCursor(2, 1) // move the cursor right 2 and down 1
#include <LiquidCrystal.h>
#include <string.h>

int second = 0; // this may not actually be necessary

/* parameters for the normal clock */
String   am_pm = "AM";
volatile uint16_t minute = 0x01;     // these are in hex so that it is easier ot display
volatile uint8_t  hour   = 0x01;
volatile uint16_t test   = 0x00;

/* parameters for the alarm */
String   alarm_am_pm = "AM";
volatile uint16_t alarm_minute = 0x00;     // these are in hex so that it is easier ot display
volatile uint8_t  alarm_hour   = 0x0;   
volatile uint16_t alarm_test   = 0x00;
volatile uint8_t  alarm_count  = 0x00;

volatile uint8_t  count        = 0x0;
volatile uint8_t  screen       = 0xff;
String   day    = "Monday";
uint8_t alarm_off = 0x00;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
//LiquidCrystal lcd(6, 7, 8, 15, 2, 0);

void lcdInit(){
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.clear();
// default appearance
  lcd.print(day);
  lcd.setCursor(0, 1);
  lcd.print("00:00 ");
  lcd.print(am_pm);
}

void interruptEnable(){
  pinMode(2, INPUT_PULLUP);                     // this is the external interrupt pin on the Arduino UNO
  pinMode(3, INPUT_PULLUP);
  attachInterrupt(0, setTime, RISING);          // this will enable pin 2 as an interrupt, ISR will be setTime function, interrupt occurs on rising edge of button press
  attachInterrupt(1, modeChange, RISING);
}

// this is to display the alarm screen
void alarmScreen(){
  lcd.setCursor(0,0);
  lcd.print("ALARM    ");
  lcd.setCursor(0,1);
  if(alarm_hour < 0x0A){
    lcd.print(" ");
    lcd.print(alarm_hour);
  }
  else
    lcd.print(alarm_hour);
  lcd.setCursor(3, 1);
  if(alarm_minute < 0x0A){
    lcd.print("0");
    lcd.print(alarm_minute);
  }
  else
    lcd.print(alarm_minute);
  lcd.setCursor(6, 1);
  lcd.print(alarm_am_pm);
}

// this is to display the clock screen
void clockScreen(){
  lcd.setCursor(0,0);
  lcd.print(day);
  lcd.setCursor(0,1);
  if(alarm_hour < 0x0A){
    lcd.print(" ");
    lcd.print(hour);
  }
  else
    lcd.print(hour);
  lcd.setCursor(3, 1);
  if(minute < 0x0A){
    lcd.print("0");
    lcd.print(minute);
  }
  else
    lcd.print(minute);
  lcd.setCursor(6, 1);
  lcd.print(am_pm);
}

void setup() {
  Serial.begin(9600);
  pinMode(13, INPUT_PULLUP);  
  pinMode(14, INPUT_PULLUP);
  pinMode(15, OUTPUT);
  digitalWrite(15, LOW);
  lcdInit();                  // initialize the LCD
  interruptEnable();          // initialize interupts

  // set up the timer to interrupt every second
  TCCR1A = 0;
  TCCR1B = 0; 
  TCNT1 = 3036;
  TCCR1B |= (1<<CS12);
  TIMSK1 |= (1<<TOIE1);
  
} 

// ISR to handle the clock
ISR(TIMER1_OVF_vect){
  Serial.println(second);
  TCNT1 = 3036;
  second = (second + 1) % 0x3C;  

  if(second == 0x00){
    minute = (minute + 0x01) % 0x3C;
    lcd.setCursor(3, 1);
    if(minute < 0x0A){
      lcd.print("0");
      lcd.print(minute);
    }
    else
      lcd.print(minute);
  }
  if(minute == 0x00 && second == 0x00){
    hour = (hour + 0x01) % 0xD; 
    lcd.setCursor(0, 1);
      if(hour < 0x0A){
        lcd.print(" ");
        lcd.print(hour);
      }
      else{
        lcd.print(hour);
      }
  }
  if((hour == 0xD) && minute  == 0x00 && second == 0x00){
    if(am_pm == "AM")
      am_pm = "PM";
    else
      am_pm = "AM";
    lcd.setCursor(6, 1);
    lcd.print(am_pm);
  }
  
  if(hour == 0x0B && minute == 0x3B && am_pm == "PM" && second == 0x3B)
    changeDay();
}

void loop() {
  if((alarm_hour == hour) && (alarm_minute == minute) && (alarm_am_pm == am_pm) && (alarm_off == 0x00)){  // check if it is time to turn on the radio
    digitalWrite(15, HIGH)  //  turn on the radio
    if(digitalRead(14))     // this button will turn off the alarm
      alarm_off = 0xff;
  }
  else if((alarm_hour == hour) && (alarm_minute == minute) && (alarm_am_pm == am_pm) && (alarm_off == 0xff)){ // check if the user turned off the radio
    digitalWrite(15, LOW);  // turn off the radio
    alarm_off = 0xff;       // keep the radio off
  }
  else{
    digitalWrite(15, LOW);  // turn off the radio
    alarm_off = 0x00;       // init signal for when the alarm is set to go off again
  }
}

// this will iterate through the days
void changeDay(){
  lcd.setCursor(0,0);
  lcd.print("          ");
  lcd.setCursor(0,0);
  if(day == "Monday")          
    day = "Tuesday";
  else if (day == "Tuesday")
    day = "Wednesday";
  else if (day == "Wednesday")
    day = "Thursday";
  else if (day == "Thursday")
    day = "Friday";
  else if (day == "Friday")
    day = "Saturday";
  else if (day == "Saturday")
    day = "Sunday";
  else if (day == "Sunday")
    day = "Monday";
  lcd.print(day);
}

// this ISR will increment one of the time components
void setTime(){
  if(digitalRead(14)){     /* CLOCK STUFF */
    if(count == 0x00){
      lcd.setCursor(3, 1);
      minute = (minute + 0x01) % 0x3C;
      if(minute < 0x0A){
        lcd.print("0");
        lcd.print(minute);
      }
      else
        lcd.print(minute);
    } 
    else if(count == 0x01){
      hour = ((hour + 0x1) % 0xD);  // when hours reaches 12 we will have 
      lcd.setCursor(0, 1);
      if(hour < 0x0A){
        lcd.print(" ");
        lcd.print(hour);
      }
      else{
        lcd.print(hour);
      }
    }
    else if(count == 0x02){
      changeDay();
    }
    else if(count == 0x03){
      lcd.setCursor(6, 1);
      if (am_pm == "AM") 
        am_pm = "PM";
      else
        am_pm = "AM";
      lcd.print(am_pm);
    }
  }
  else if(digitalRead(13)){                               /* ALARM CLOCK STUFF */
    if(count == 0x00){
      lcd.setCursor(3, 1);
      alarm_minute = (alarm_minute + 0x01) % 0x3C;
      if(alarm_minute < 0x0A){
        lcd.print("0");
        lcd.print(alarm_minute);
      }
      else
        lcd.print(alarm_minute);
    } 
    else if(count == 0x01){
      alarm_hour = ((alarm_hour + 0x1) % 0xD);  // when hours reaches 12 we will have 
      if(alarm_hour < 0x0A){
        lcd.setCursor(0, 1);
        lcd.print(" ");
        lcd.print(alarm_hour);
      }
      else{
        lcd.setCursor(0,1);
        lcd.print(alarm_hour);
      }
    }
    else if(count == 0x02){}
    else if(count == 0x03){
      lcd.setCursor(6, 1);
      if (alarm_am_pm == "AM") 
        alarm_am_pm = "PM";
      else
        alarm_am_pm = "AM";
      lcd.print(alarm_am_pm);
    }
  }
  else{                                     /* THIS WILL BE FOR CHANGING THE SCREEN */
    if(screen == 0xff){
      alarmScreen();
      screen = 0x00;
    }
    else{
      clockScreen();
      screen = 0xff;
    } 
  }
}

// this will go through each time component
void modeChange(){
  count = (count + 0x1) % 0x4;
}
