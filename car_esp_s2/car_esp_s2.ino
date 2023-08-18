
//#define MY_DEBUG // gpad inputs
#define MY_DEBUG1 // speed

#include "util.h"
GamepadData gamepadData;  
static uint8_t buf_rf[64];
static bool b_frm_start;
static bool b_rcvFrm;
static uint8_t frm_buf_len;

static int16_t cur_spd;

#define RF Serial1

// motor 관련 설정
#include "ServoESC.h"

#define SERVO_PIN 3
#define ESC_PIN 5
#define RF_RX 9
#define RF_TX 10

static ServoESC esc_servo;

static unsigned long cur;
static int enforce_stop;
static unsigned long prev_frame;

void setup() {
  Serial.begin(115200);
  RF.begin(9600, SERIAL_8N1, RF_RX, RF_TX);
  esc_servo.attach(SERVO_PIN, ESC_PIN); //attach the servo to the needed pins

  delay(3000);
}


// loop1 : core 1
void loop() {
  cur=millis();

  if(cur-prev_frame>600){  //when nothing is recieved from the controller for 600 msec STOP
    enforce_stop=5;       
    esc_servo.SetEscMicros(0);    
  }else{
/*    if(enforce_stop)
      Serial.println("rcv sig!");
*/
//    enforce_stop=false;
  }
  
  // RF-receicver
  if (RF.available() > 0){
    frm_buf_len += RF.readBytes(&buf_rf[frm_buf_len],RF.available()); //if the RF is available is open then take the things from the 
    /*for(int i=0; i<frm_buf_len; i++){
      Serial.print(buf_rf[i],HEX);
      Serial.print(",");
    }
    Serial.println("");
    */
    if(frm_buf_len > sizeof(GamepadData)){ // if the length of the data recieved reset everything
      b_frm_start=false;                   
      frm_buf_len=0;
    }
    
    if(frm_buf_len>2){ // check the startframe that the first two bytes start with 0xfff and if recieved set b_frm_start
      if(buf_rf[0]==0xff && buf_rf[1]==0xff){   
        b_frm_start=true;
      }else{ // if the first two are not 0xff keep on resetting
        frm_buf_len=0;  
        b_frm_start=false;
        if(enforce_stop) enforce_stop=5;
      }
    }

    // the reason the +3 is there to subtract the frame start and the checksum
    // the FRAME_DATA_SIZE is used to calculate the checksum
    if(b_frm_start && frm_buf_len == FRAME_DATA_SIZE+3){
      uint8_t chkSum=0;
      for(int i=0; i<FRAME_DATA_SIZE; i++){
        chkSum+=buf_rf[2+i];
      }

      // Compare the calculated checksum with the received checksum
      if(chkSum == buf_rf[FRAME_DATA_SIZE+2]){ // if the frame is correct then the flag b_rcvFrm as recieved 
        memncpy(buf_rf, gamepadData.buf, frm_buf_len);
        b_rcvFrm=true;
      }else{  // if it is wrong then reset
        if(enforce_stop) enforce_stop=5;        
        Serial.printf("ChkSum Err %x, %x\n", chkSum, buf_rf[FRAME_DATA_SIZE+2]);
        for(int i=0; i<sizeof(GamepadData)-1; i++)
          Serial.printf("%d, ", buf_rf[i]);
        Serial.println();
      }
      b_frm_start=false;
      frm_buf_len=0;
    } // end of chksum process
  } // end of channel_A process the reception (RF Transiver) receiver


  // through the RF module recieve the frame
  if(b_rcvFrm){
#ifdef MY_DEBUG //made for debugging if MY_DEBUG IS defined then go into the the debugging
    Serial.printf("[%d, %d-%d] %d-%d, %d, spd:%d\n",
        gamepadData.data.lx, gamepadData.data.rx,gamepadData.data.ry, gamepadData.data.l_trigger,
        gamepadData.data.r_trigger,  gamepadData.data.btns.var, esc_servo.GetCurSpeed());//encoder.rpm );
    
#endif
    prev_frame=cur;
    
    if(gamepadData.data.btns.btn_set.y){ // y: Toggle Cruise     
    }else if(gamepadData.data.btns.btn_set.b){  // B : Kp 0.1증가
    }else if(gamepadData.data.btns.btn_set.a){ // A : Kp 0.1감소
    }else if(gamepadData.data.btns.btn_set.up){  //up-pad : Ki 0.01증가
    }else if(gamepadData.data.btns.btn_set.dn){  //down-pad : Ki 0.01감소
    }else if(gamepadData.data.btns.btn_set.right){  //right-pad : Kd 0.01증가
    }else if(gamepadData.data.btns.btn_set.left){  //left-pad : Kd 0.01감소
    }else if(gamepadData.data.btns.btn_set.lb){ // left bumper: cruise speed step dn
    }else if(gamepadData.data.btns.btn_set.rb){ // right bumper: cruise speed step up
    }

    // Left-Right 0~199. 0:Left, 199:Right, 100:Center
    esc_servo.SetServoMicros(uint16_t(gamepadData.data.lx-100));
#ifdef MY_DEBUG1  //debug options
        Serial.println(gamepadData.data.lx-100);
#endif
    if(enforce_stop==0){ // cruise is off
      if (gamepadData.data.r_trigger && gamepadData.data.l_trigger==0){ // r-trigger only pressing
        cur_spd=gamepadData.data.r_trigger;
        esc_servo.SetEscMicros(cur_spd);
#ifdef MY_DEBUG1        
        Serial.println(gamepadData.data.r_trigger);
#endif
      }else if (gamepadData.data.r_trigger==0 && gamepadData.data.l_trigger){ // l-trigger only
        cur_spd=-1*gamepadData.data.l_trigger;
        esc_servo.SetEscMicros(cur_spd);
#ifdef MY_DEBUG1        
        Serial.println(-1*gamepadData.data.l_trigger);
#endif
      }else if (gamepadData.data.r_trigger && gamepadData.data.l_trigger){ // both trigger 
        // do it later
      }else{ // none trigger pressing
        esc_servo.SetEscMicros(0);
        cur_spd=0;
#ifdef MY_DEBUG1        
        Serial.println(0);
#endif
        // do it later
      }
    }else{ 
      enforce_stop--; // need receive 5 continuous inputs for make enforce_stop 0.
    }
    b_rcvFrm=false;
  }
  //yield;
}
