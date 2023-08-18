// This is an ultra simple software servo driver. For best
// results, use with a timer0 interrupt to refresh() all
// your servos once every 20 milliseconds!
// Written by Limor Fried for Adafruit Industries, BSD license

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

/**
 * @brief Class for basic software servo control
 *
 */
class ServoESC {
public:
  ServoESC(void); 
  void attach(uint8_t spin, uint8_t epin);
  boolean attached(){ return isAttached; };
  void SetServoMicros(int16_t a); // 1~100 speed를 servo pwm length(usec) 로 변환.
  void SetEscMicros(int16_t s); // -100~100 servo angle을 servo pwm length(usec) 로 변환.
  int16_t GetCurSpeed() { return speed_cur; }

  boolean isAttached;

private:
  const int SERVO_PWM_CH=0;
  const int ESC_PWM_CH=1;
  int servo_pos;
  const int SERVO_MID=308;
  const int SERVO_LEFT_MAX=257;
  const int SERVO_RIGHT_MAX=359;
  
  uint8_t servoPin, escPin;
  int16_t speed_cur;

  uint16_t micros_esc, micros_servo;
  
};
