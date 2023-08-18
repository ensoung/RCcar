/*
 * https://github.com/kanestoboi/AS5600
 */

#ifndef AS5600_h
#define AS5600_h

#include "Arduino.h"
#include <Wire.h>
#include "kalman.h"

class AS5600
{
public:
    //uint16_t prev_rpm=0;
    Kalman kal_rpm=Kalman(1, 1, 0.1, 1000);
    bool en;//, fwd;
    int16_t cur_pos, prev_pos, diff_pos;
    int16_t cur_rpm;

    //static const uint8_t buf_num=10;//8;
    //int16_t rpm_buf[buf_num]; // save last 4 rpm 
    //int16_t rpm_avg;
    //uint8_t rpm_buf_idx;
    //unsigned long cur_prev;
    // bool dir; // 해상도가 낮아서 방향을 알수는 없음. 실제 rpm이 댑다 빠르면 읽을 수있는 rpm도 정확도가 꽤 떨이질지도...
    AS5600(){ prev_pos=cur_rpm=0;};
    //AS5600(){ prev_pos=cur_rpm=0;};
    bool begin(){ Wire.begin(); en=isConnected(); return en; };

    uint16_t getRpm(int16_t speed_cur){
      /*
      if(en==false) {
        Serial.print(" encoder fail ");
        return prev_pos;
      }
      */
      return getAngle16();
      
      if(speed_cur) 
        cur_pos = getAngle16();
      else // speed가 0일때는 강제로 변화없음 상태로 유지.
        cur_pos=prev_pos=0;
        
      if(cur_pos==0xffff) { // check I2C Busy
        return prev_pos;
      }

      diff_pos = cur_pos-prev_pos;
      
      if(speed_cur>=0){// 전진중. cur_rpm가 감소함.
        if(diff_pos>0) diff_pos-=4096; // 0~15범위 넘어갔으므로 제대로 맞춰줌.
      } else { // 후진중. cur_rpm가 증가함.
        if(diff_pos<0) diff_pos+=4096; // 0~15범위 넘어갔으므로 제대로 맞춰줌.
      }
//      if(abs(speed_cur)<5 && abs(diff_rpm)>10) // 이걸 왜 넣는지 모르겠음.
//        diff_rpm=diff_rpm/abs(diff_rpm);
        return diff_pos;
        //cur_rpm = kal_rpm.updateEstimate(diff_pos);
        return cur_rpm;
    }
    
    //uint16_t getRpm() { return cur_rpm; }
    
    // return 0~15
    uint8_t getAngle() {
      return _getRegister(_ANGLEAddressMSB);//, _ANGLEAddressLSB);
    }

    // return 0~4095(0xfff)
    uint16_t getAngle16() {
      return _getRegister16(_ANGLEAddressMSB);//, _ANGLEAddressLSB);
    }

    uint8_t _getRegister(byte register1) {  
        uint8_t _b=0;
        //if (Wire.available()) { return 0xff; } // return 0xff when i2c is busy!!!!!!  should Check it.
        Wire.beginTransmission(_AS5600Address);
        Wire.write(register1);
        Wire.endTransmission();
      
        Wire.requestFrom(_AS5600Address, 1);
      
        //while (Wire.available() == 0) { }
        _b = Wire.read();
      
        return _b;
    }
    
    uint16_t _getRegister16(byte register1) {  
        uint16_t ret;
        Serial.print("1");
        Wire.beginTransmission(_AS5600Address);
        Wire.write(register1);
        Wire.endTransmission();
        Serial.print("2");
        Wire.requestFrom(_AS5600Address, 2); // MSB+LSB
        Serial.print("3");
        ret = Wire.read();
        ret<<=8;
        Serial.print("4");
        ret += Wire.read();
        //ret &= 0xfff;
        Serial.println(ret);
        return ret;
    }
    
    bool isConnected()
    { 
      Wire.beginTransmission(_AS5600Address);
      return ( Wire.endTransmission() == 0);
    }
    

private:
  int _AS5600Address = 0x36; // I2C address for AS5600
  byte _RAWANGLEAddressMSB = 0x0C;
  byte _RAWANGLEAddressLSB = 0x0D;
  byte _ANGLEAddressMSB = 0x0E;
  byte _ANGLEAddressLSB = 0x0F;
  byte _STATUSAddress = 0x0B;

};

#endif
