// This is an ultra simple software servo driver. For best
// results, use with a timer0 interrupt to refresh() all
// your servos once every 20 milliseconds!
// Written by Limor Fried for Adafruit Industries, BSD license

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "ServoESC.h"


/**
 * @brief Construct a new ServoESC::ServoESC object
 *
 */
ServoESC::ServoESC(void) {
  isAttached = false;
  servo_pos=-101;
  escPin=servoPin = 255;
}

/**
 * @brief Attacht to a supplied pin
 *
 * @param pin The pin to attach to for controlling the servo
**/
void ServoESC::attach(uint8_t sPin, uint8_t ePin) {
  servoPin = sPin;
  escPin = ePin;

  ledcSetup(SERVO_PWM_CH, 50, 12);  // 50hz, 12bit resolution=0~4095  1ms=205 1.5ms=308, 2ms=410
  ledcAttachPin(sPin, SERVO_PWM_CH);

  ledcSetup(ESC_PWM_CH, 50, 12);  // use the led pwm since it means we don't need to make a pwm manually
  ledcAttachPin(ePin, ESC_PWM_CH);
  
  isAttached = true;
  
  SetServoMicros(0);
  SetEscMicros(0);
}


/**
 * @brief Update the servo's angle setting and the corresponding pulse width
 *
 * @param a The target servo angle
 */
void ServoESC::SetServoMicros(int16_t a) 
{
  if(servo_pos==a)
    return;
  servo_pos=a;
  // 12 bit resolution=0~4095  1ms=205(0 degree) 1.5ms=308(90 degree), 2ms=410(180 degree)
  micros_servo=SERVO_MID;
  if(a<0)
    micros_servo = map(a, -1, -100, SERVO_MID+1, SERVO_RIGHT_MAX); // 357이 대략 135라 보는데 수정해야함.
  else
    micros_servo = map(a, 1, 100, SERVO_MID-1, SERVO_LEFT_MAX); // 256이 대략 45라 보는데, 수정해야함.
  
  ledcWrite(SERVO_PWM_CH, micros_servo);
}


void ServoESC::SetEscMicros(int16_t spd) 
{
  speed_cur=spd;

  if(spd!=0)
    Serial.println(spd); 
  if(spd>0){ // 1ms=205 1.5ms=308, 2ms=410
    micros_esc = map(spd, 0, 255, 308, 368); // R-Trigger 0~100
  }else if(spd<0){ // R-Trigger가 0일때만 후진임.(R-Trigger가 눌려있으면 RTrig(acel)-LTrig(brk_rev)('-'면 0)값이 speed임.
    micros_esc = map(spd, -255,0, 227, 307); 
  }else{
    micros_esc = 308; // 1511~1522 : stop
  }
  if(spd!=0)
    Serial.println(micros_esc);
  ledcWrite(ESC_PWM_CH, micros_esc);

}
