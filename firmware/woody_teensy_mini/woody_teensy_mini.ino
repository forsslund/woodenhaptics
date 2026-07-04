#include <Encoder.h>

/*
    Polhem Teensy Mini edition

    Will send as raw values to computer as meaningful

    enc_home not used, will set all encoders to 0 when calibrations button is pressed.
    instead, it expects the receiving computer to compute the offset on all messages
    for reference: constexpr int enc_home[] = {8312, -10366, 19764, 0, 30, 0};
                   correct_value = received_value + enc_home

*/

// -------- SETTINGS -----------------------------------------------
constexpr float firmware_version = 2.0; // 2022-10-31 bumped to 1.5
constexpr int escon_max_milliamps = 6000;
constexpr bool VINTAGE = false; // set to true for vintage Phantom
constexpr bool WOODY_ADAPTER = true; 

//#define ENABLE_I2C_GIMBAL
//#define ENABLE_PWM_GIMBAL
// -----------------------------------------------------------------




// Hardware quad encoder library
// https://github.com/mjs513/Teensy-4.x-Quad-Encoder-Library
//#include "QuadEncoder.h"

// For reading calibration button well
#include <Bounce2.h>

// Was needed somehow for something :)
unsigned __exidx_start;
unsigned __exidx_end;




// -----------------------------------------------------------------
// I2C Encoder readings for encoder 3,4,5
// -----------------------------------------------------------------
#ifdef ENABLE_I2C_GIMBAL
#include <Wire.h>

uint8_t gimbalSensorChipAddress[] = {0x41, 0x40, 0x43};
uint8_t gimbalSensorErrorCode[] = {1, 2, 4};
constexpr uint16_t SENSOR_READ_ERROR = 10000;
int i2cErrorCount{0};
#define AS5048B_ANGLMSB_REG 0xFE

uint16_t readGimbalSensor(uint8_t chipAddress) {
    //16 bit value got from 2 8bits registers (7..0 MSB + 5..0 LSB) => 14 bits value

    uint8_t address = AS5048B_ANGLMSB_REG;
    uint8_t nbByte2Read = 2;
    byte requestResult;
    byte readArray[2];
    uint16_t readValue = 0;
     
    Wire.beginTransmission(chipAddress);
    Wire.write(address);
    requestResult = Wire.endTransmission(false);  // Richard email 2023-01-04
    if (requestResult) {
      i2cErrorCount++;
      return SENSOR_READ_ERROR;
        //Serial.print("I2C error: ");
        //Serial.println(requestResult);
    }

    Wire.requestFrom(chipAddress, nbByte2Read, true); // Richard email 2023-01-04
    
    for (byte i=0; i < nbByte2Read; i++) {
      readArray[i] = Wire.read();
    }
  
    readValue = (((uint16_t) readArray[0]) << 6);
    readValue += (readArray[1] & 0x3F);
    return readValue;
}

#endif
// -----------------------------------------------------------------









// -----------------------------------------------------------------
// COMPUTER MESSAGE INTERFACE 
// -----------------------------------------------------------------
constexpr int model_polhem_2022_raw = 1;
constexpr int model_vintage_raw = 2;
constexpr int model_woody_raw = 3;
struct device_to_pc_message {
  int model{WOODY_ADAPTER? model_woody_raw : (VINTAGE? model_vintage_raw : model_polhem_2022_raw)};
  int enc[6];
  int error_code{0};

  // Returns number of characters, also writes a trailing \0
  int toChars(char *c){
    return sprintf(c, "[%d,%d,%d,%d,%d,%d,%d,%d]\n", 
                        model, enc[0], enc[1], enc[2], enc[3], enc[4], enc[5],  error_code);
  }

  // Returns 1 if success, 0 if fail
  int fromChars(const char *c){
      return 8 == sscanf(c, "[%d,%d,%d,%d,%d,%d,%d,%d]", 
                              &model, &enc[0], &enc[1], &enc[2], &enc[3], &enc[4], &enc[5], &error_code);
  }
};

struct pc_to_device_message {
  int ma[3];          // milliamps per motor

  // Returns number of characters, also writes a trailing \0
  int toChars(char *c){
    return sprintf(c, "[%d,%d,%d]\n", ma[0], ma[1], ma[2]);
  }

  // Returns 1 if success, 0 if fail
  int fromChars(const char *c){
      return 3 == sscanf(c, "[%d,%d,%d]", &ma[0], &ma[1], &ma[2]);
  }
};
// -----------------------------------------------------------------




// -----------------------------------------------------------------
// Pinout configuration & setup
// -----------------------------------------------------------------
// Check if we are in emulator mode
constexpr int emulator_pin =  6; // GND on emulator, NC on Polhem
/*
bool checkEmulatorMode(){
  pinMode(emulator_pin, INPUT_PULLUP);
  delay(100);
  return !digitalRead(emulator_pin);
}
const bool emulator_mode = checkEmulatorMode();
*/

constexpr int ledPin = 13;
bool ledState=false;
constexpr int pwmPinA = 10;
constexpr int pwmPinB =  VINTAGE? 8: 9;
constexpr int pwmPinC =  VINTAGE? 9: 8;
constexpr int pwmResolutionBits = 13;
constexpr int pwmMax = pow(2, pwmResolutionBits) - 1;
constexpr int calib_pin =  WOODY_ADAPTER ? 6 : 23; // aka BUTTON_A
constexpr int enable_pin = WOODY_ADAPTER ? 1: 5;
constexpr int power_enable_switch_pin = WOODY_ADAPTER ? 7: 17;
constexpr int enc0_a_pin = 21;//WOODY_ADAPTER? 21 : (VINTAGE? 0: 0);
constexpr int enc0_b_pin = 23;//WOODY_ADAPTER? 23 : (VINTAGE? 1: 1);
constexpr int enc1_a_pin = WOODY_ADAPTER? 20 : (VINTAGE? 4: 2);
constexpr int enc1_b_pin = WOODY_ADAPTER? 18 : (VINTAGE? 7: 3);
constexpr int enc2_a_pin = WOODY_ADAPTER? 17 : (VINTAGE? 3: 4);
constexpr int enc2_b_pin = WOODY_ADAPTER? 15 : (VINTAGE? 2: 7);
constexpr int debug_pin0 = 11;
constexpr int gimbal_pwm_1_pin = 21; // third pin on enc intermediate pcb (from top left, with motor enc connectors at bottom)
constexpr int gimbal_pwm_3_pin = 18; // "sda"
constexpr int gimbal_pwm_2_pin = 19; // "scl"
//constexpr int enc_3_4_5_scl0 = 19; // For reference, we use first wire interface
//constexpr int enc_3_4_5_sda0 = 18;

/* VINTAGE Gimbal
constexpr int enc3_a_pin = 16;  // actually b on device
constexpr int enc3_b_pin = 19;  // actually a on device
constexpr int enc4_a_pin = 15; 
constexpr int enc4_b_pin = 20;
constexpr int enc5_a_pin = 21;  // actually b on device
constexpr int enc5_b_pin = 22;  // actually a on device
constexpr int stylus_switch = 14;
*/
// -----------------------------------------------------------------




// -----------------------------------------------------------------
// Globals
// -----------------------------------------------------------------
volatile int gimbal_offsets[]={0,0,0};
volatile int gimbal_in10bit[]={0,0,0};
volatile int counter[] = {0,0,0,0,0,0};
volatile int milliamps[]={0,0,0};
// -----------------------------------------------------------------


// -------------------------------------------------
// Enconder logic (motor encoders)
// -------------------------------------------------
Encoder quadEnc0(enc0_b_pin,enc0_a_pin);
Encoder quadEnc1(enc1_b_pin,enc1_a_pin);
Encoder quadEnc2(enc2_b_pin,enc2_a_pin);
// -------------------------------------------------



#ifdef ENABLE_PWM_GIMBAL
unsigned long last_rise[3]={0,0,0};
unsigned long last_fall[3]{0,0,0};
unsigned long period[3]={0,0,0};
double duty[3]={0,0,0};
FASTRUN void callback_gimbal_pwm_1(){
  if(!digitalRead(gimbal_pwm_1_pin)) {
    period[0] = ARM_DWT_CYCCNT - last_fall[0];
    if(period[0] != 0 && last_rise[0]>last_fall[0])
      duty[0] = double((last_rise[0] - last_fall[0]))/double(period[0]);
    last_fall[0] = ARM_DWT_CYCCNT;
  } else {
    last_rise[0] = ARM_DWT_CYCCNT;    
  }
}

FASTRUN void callback_gimbal_pwm_2(){
  if(!digitalRead(gimbal_pwm_2_pin)) {
    period[1] = ARM_DWT_CYCCNT - last_fall[1];
    if(period[1] != 0 && last_rise[1]>last_fall[1])
      duty[1] = double((last_rise[1] - last_fall[1]))/double(period[1]);
    last_fall[1] = ARM_DWT_CYCCNT;
  } else {
    last_rise[1] = ARM_DWT_CYCCNT;    
  }
}

FASTRUN void callback_gimbal_pwm_3(){
  if(!digitalRead(gimbal_pwm_3_pin)) {
    period[2] = ARM_DWT_CYCCNT - last_fall[2];
    if(period[2] != 0 && last_rise[2]>last_fall[2])
      duty[2] = double((last_rise[2] - last_fall[2]))/double(period[2]);
    last_fall[2] = ARM_DWT_CYCCNT;
  } else {
    last_rise[2] = ARM_DWT_CYCCNT;    
  }
}
#endif




// -------------------------------------------------
// Push button
// -------------------------------------------------
volatile unsigned long lastDebounceTime = 0; 
Bounce pushbutton = Bounce(); 
void calib_button(void) {
  if (pushbutton.update()) {
    if (pushbutton.fell()) {
      calibrate_device();
    }
  }
}
IntervalTimer btnTimer;
// -------------------------------------------------





// -------------------------------------------------
// Message watchdog
// if no message has been received in a short while,
// disable the motors
// -------------------------------------------------
IntervalTimer wdTimer;
void wdCallback() {
  wdTimer.end();
  digitalWrite(enable_pin, 0);
  analogWrite(pwmPinA, pwmMax / 2);
  analogWrite(pwmPinB, pwmMax / 2);
  analogWrite(pwmPinC, pwmMax / 2);  
}

// Woody adapter: no power-enable switch in use; amps are enabled whenever
// servo messages flow (enable_pin high in loop(), dropped by the watchdog).
volatile bool power_enable_switch_mode = true;
void callback_power_enable_switch(void) {
  //power_enable_switch_mode = !digitalRead(power_enable_switch_pin);
}
// -------------------------------------------------




// -------------------------------------------------
// Calibration
// Set all encoders to 0 when pressing button in home position
// -------------------------------------------------
float precalibration_count = 0;  // Used to "fake" motion before first calibration
bool calibration_needed{false}; // User has to calibrate before using device
void calibrate_device(){

    quadEnc0.write(0); // Set hw counter
    quadEnc1.write(0);
    quadEnc2.write(0);

    for(int i=0;i<6;++i) counter[i]=0;
    
#ifdef ENABLE_I2C_GIMBAL
  for(int r=0;r<3;++r) {
    int gimbal_value = readGimbalSensor(gimbalSensorChipAddress[r]);
    if(gimbal_value != SENSOR_READ_ERROR) gimbal_offsets[r] = -gimbal_value/16;
  }    
#endif

#ifdef ENABLE_PWM_GIMBAL
    for(int r=0;r<3;++r) {
      gimbal_offsets[r] = -gimbal_in10bit[r];   
    }
#endif

    calibration_needed = false;
}
// -------------------------------------------------






// -------------------------------------------------
// Setup
// -------------------------------------------------
void setup() {

  /*
  // Enable led in emulator mode
  if(emulator_mode){
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, 0);
  }
  */

  pinMode(debug_pin0, OUTPUT);
  digitalWrite(debug_pin0, 0);
  
  pinMode(pwmPinA, OUTPUT);
  pinMode(pwmPinB, OUTPUT);
  pinMode(pwmPinC, OUTPUT);
  analogWriteResolution(pwmResolutionBits); // 0-4095
  analogWriteFrequency(pwmPinA, 4000);
  analogWriteFrequency(pwmPinB, 4000);
  analogWriteFrequency(pwmPinC, 4000);

  analogWrite(pwmPinA, pwmMax / 2);
  analogWrite(pwmPinB, pwmMax / 2);
  analogWrite(pwmPinC, pwmMax / 2);

  pinMode(calib_pin, INPUT_PULLUP); // pull-up by default
  pinMode(enable_pin, OUTPUT);
  
  digitalWrite(enable_pin, 0);

  pinMode(power_enable_switch_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(power_enable_switch_pin), callback_power_enable_switch, CHANGE);

#ifdef ENABLE_PWM_GIMBAL
  pinMode(gimbal_pwm_1_pin, INPUT_PULLUP);
  pinMode(gimbal_pwm_2_pin, INPUT_PULLUP);
  pinMode(gimbal_pwm_3_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(gimbal_pwm_1_pin), callback_gimbal_pwm_1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(gimbal_pwm_2_pin), callback_gimbal_pwm_2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(gimbal_pwm_3_pin), callback_gimbal_pwm_3, CHANGE);

  NVIC_SET_PRIORITY(IRQ_GPIO6789, 64);
  ARM_DEMCR |= ARM_DEMCR_TRCENA;
  ARM_DWT_CTRL |= ARM_DWT_CTRL_CYCCNTENA; // enable tick count
#endif
  
  //quadEnc0.setInitConfig();
  //quadEnc0.init();
  quadEnc0.write(0);
  //quadEnc1.setInitConfig();
  //quadEnc1.init();
  quadEnc1.write(0);
  //quadEnc2.setInitConfig();
  //quadEnc2.init();
  quadEnc2.write(0);

  Serial.begin(115200); // speed is ignored on teensy
  Serial.flush();

  delay(400);

   
  // Set up reading of button with debounce
  pushbutton.interval(10);
  pushbutton.attach(calib_pin);
  btnTimer.begin(calib_button,2); // Check satus every 2ms   
  
#ifdef ENABLE_I2C_GIMBAL
  Wire.begin();
#endif
}
// ----------------------------------------------------------





// ----------------------------------------------------------
// Main Loop
// ----------------------------------------------------------
constexpr size_t buf_len = 127;
char in_buf[buf_len+1];
char out_buf[buf_len+1];

// returns 0 if success, error code else
// start_pos is where [ is located
int await_message(size_t& start_pos){
  bool start_found = false;
  int comma_count = 0;
  
  for (size_t buf_pos = 0; buf_pos < buf_len; buf_pos++) {
    while (Serial.available() <= 0){
      // wait
    }

    char c = Serial.read();
    in_buf[buf_pos] = c;
    
    if (c == ']'){
      if(!start_found)
        return 256; // no start character before end character
      if(comma_count != 2)
        return 512; // wrong legth of array, should be [0,0,0]

      // Do we still have waiting messages? Discard and report error
      while(Serial.available()){
        c = Serial.read();
        if(c!='\n' && c!='\r' && c!='\0') 
          return 8; // trailing characters in message
      }

      return 0;
    }
    if (c == '['){
      start_found=true;
      start_pos = buf_pos;
    }
    if (c == ','){
      comma_count++;
    }    
  }
  return 128; // buffer full
}

void return_last_values(int error_code=0){
    device_to_pc_message out_msg;
    for(int i=0;i<6;++i)
      out_msg.enc[i] = counter[i];
    out_msg.error_code |= error_code;
    
    int len = out_msg.toChars(out_buf);
    Serial.write(out_buf, len);
    Serial.send_now();  
}

int loop_count = 0;

void loop() {
  loop_count++;
  memset(in_buf,0,buf_len+1);
  memset(out_buf,0,buf_len+1);

  // receive from serial
  size_t start_pos;
  int receive_error = await_message(start_pos);

  if(receive_error){
    return_last_values(receive_error);
    return;
  }

  // parse
  pc_to_device_message pc_msg;
  if(pc_msg.fromChars(in_buf+start_pos)==0) {
    return_last_values(1024); // Parse error
    return;
  }
  

  // Successfully got message, so reset msg timeout watchdog 
  wdTimer.end();
  wdTimer.begin(wdCallback,100000); // 100ms timeout for next message

  // Set motor amps unless we have not yet calibrated.
  if(calibration_needed) {
    milliamps[0]=0;
    milliamps[1]=0;
    milliamps[2]=0;
    wdCallback(); // Disable escons and reset timer

  } else {
    digitalWrite(enable_pin, 1); // enable follows data flow; see wdCallback()

    // Set motor amps (0 by default, 0 if over max is provided)      
    constexpr int chan[] = {pwmPinA, pwmPinB, pwmPinC};
    for(int i=0;i<3;++i){
      milliamps[i] = pc_msg.ma[i];
      milliamps[i] = (milliamps[i]<=escon_max_milliamps && milliamps[i]>=-escon_max_milliamps)? milliamps[i] : 0;  // extra safety check
      analogWrite(chan[i], int(pwmMax * (0.4 * (milliamps[i] / float(escon_max_milliamps)) + 0.5)));
    }        
  }

  // Prepare return message
  device_to_pc_message out_msg;

  // Read encoders from hardware counter
  counter[0] = quadEnc0.read();
  counter[1] = quadEnc1.read();
  counter[2] = quadEnc2.read();
  
#ifdef ENABLE_I2C_GIMBAL
  // Only read one sensor per loop
  int gimbal_index = loop_count % 3;
  int counter_index = gimbal_index + 3;
  int counter_value = readGimbalSensor(gimbalSensorChipAddress[gimbal_index]);

  if(counter_value == SENSOR_READ_ERROR) out_msg.error_code|=gimbalSensorErrorCode[gimbal_index];
  else  counter[counter_index] = (1024 + counter_value/16 + gimbal_offsets[gimbal_index])%1024;
#endif

#ifdef ENABLE_PWM_GIMBAL
  gimbal_in10bit[0] = int( 1023*(4119.0/4095.0)*(duty[1]-8.0/4119.0) ) % 1024; // store for calibration
  gimbal_in10bit[1] = int( 1023*(4119.0/4095.0)*(duty[0]-8.0/4119.0) ) % 1024; // store for calibration
  gimbal_in10bit[2] = int( 1023*(4119.0/4095.0)*(duty[2]-8.0/4119.0) ) % 1024; // store for calibration

  counter[3] = (1024 + gimbal_in10bit[0] + gimbal_offsets[0])%1024;
  counter[4] = (1024 + gimbal_in10bit[1] + gimbal_offsets[1])%1024;
  counter[5] = 1023-(1024 + gimbal_in10bit[2] + gimbal_offsets[2])%1024;
#endif

  // Return message
  for(int i=0;i<6;++i)
    out_msg.enc[i] = counter[i];
  if(calibration_needed){
    out_msg.enc[0] = 8000+4000*sin(0.0002*precalibration_count++);
    out_msg.enc[1] = pc_msg.ma[0];
  }
  
  int len = out_msg.toChars(out_buf);
  Serial.write(out_buf, len);
  Serial.send_now();
}