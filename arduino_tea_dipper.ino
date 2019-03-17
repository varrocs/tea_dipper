#include <Servo8Bit2.h>

// Needs 8MHz clock

Servo8Bit2 arm;

const int PIN_SERVO=3;
const int PIN_BUZZER=0; 
const int PIN_MOTOR_SWITCH=4;

const int POS_DIPPED=2400;
const int POS_RAISED=1800;
const int POS_FINISHED=1750;

const uint32_t TIME_DIPPING = 240000ul; // 4 minutes
const uint32_t TIME_NOTICE  = 3000;
const uint32_t DEPLETED_VOLTAGE = 3300;

long Vrail_leo() { 
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif 

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH 
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000   
                         
  return result;
 
}


void play_depleted_tune() {
      for (byte j = 0; j < 100; ++j) {
        digitalWrite(PIN_BUZZER, HIGH);
        _delay_ms(1);
        digitalWrite(PIN_BUZZER, LOW);
        _delay_ms(1);
      }
      delay(500);    
}


void depleted() {
  while (true) {
    play_depleted_tune();
    delay(TIME_NOTICE);
  }
}

void check_voltage() {
 long voltage =  Vrail_leo();
 if (voltage < DEPLETED_VOLTAGE) {
  depleted();
 }
}

void move_arm(int from, int to, uint16_t delay_start, uint16_t delay_end) {
  int delta = (from > to) ? (-1) : (1);

  for (int i = from; i != to; i += delta) {
    arm.writeMicroseconds(i);
    uint16_t d;
    d = map(i, from, to, delay_start, delay_end);
    delayMicroseconds(d); 
   }
}

void play_ready_tune() {
    for (byte i = 0; i < 3; ++i) {
      for (byte j = 0; j < 100; ++j) {
        digitalWrite(PIN_BUZZER, HIGH);
        _delay_ms(1);
        digitalWrite(PIN_BUZZER, LOW);
        _delay_ms(1);
      }
      delay(200);    
    }
}


void dip() {
  move_arm(POS_FINISHED, POS_RAISED, 2000, 1000);
  digitalWrite(PIN_MOTOR_SWITCH, HIGH);  
 
  while (millis() < TIME_DIPPING) {
   move_arm(POS_RAISED, POS_DIPPED, 3000, 500);
   move_arm(POS_DIPPED, POS_RAISED, 500, 3000);
  }
}

void park() {
  move_arm(POS_RAISED, POS_FINISHED, 2000, 1000);
  arm.detach();
  digitalWrite(PIN_MOTOR_SWITCH, LOW);
}



void ready() {
  while (true) {
    play_ready_tune();
    delay(TIME_NOTICE);
    check_voltage();
  }
}

void setup() {  
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_MOTOR_SWITCH, OUTPUT);
  digitalWrite(PIN_MOTOR_SWITCH, HIGH);
  
  arm.write(POS_FINISHED);
  delay(10); 
  arm.attach(PIN_SERVO);
}

void loop() {
  check_voltage();
  dip();
  check_voltage();
  park();
  ready();
  
  while(1) {
  }
  
}
