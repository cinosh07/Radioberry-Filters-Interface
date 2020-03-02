
#include <Wire.h>
//const uint8_t I2C_ADDRESS = 0x21; //Alex Board Protocol
const uint8_t I2C_ADDRESS = 0x20; //Generic Board Protocol
uint8_t received;
uint8_t message;
const uint8_t COMMAND_ON_OFF = 0x01;
const uint8_t COMMAND_SPEED = 0x02;

const int FILTER_160M = 8;
const int FILTER_80m = 4;
const int FILTER_60m = 2;
const int FILTER_40m = 802;
const int FILTER_30m = 401;
const int FILTER_20m = 101;
const int FILTER_17m = 164;
const int FILTER_15m = 264;
const int FILTER_12m = 232;
const int FILTER_10m = 232;
const int FILTER_6m = 232;

boolean genericMode = false;

#define debug

//Pin definitions
#define lpf_board_xf-lpf-hf
//https://www.ebay.ca/itm/12v-100W-3-5Mhz-30Mhz-HF-power-amplifier-low-pass-filter-Diy-kit/113761898910?ssPageName=STRK%3AMEBIDX%3AIT&_trksid=p2057872.m2749.l2649
#if defined lpf_board_xf-lpf-hf
int lpf_15_10m = 5;
int lpf_20_17m = 4;
int lpf_40m = 3;
int lpf_80m = 2;
#endif

#define bpf_board_russian
//https://www.ebay.ca/itm/HF-BPF-band-pass-filter-RTL-SDR-HERMES-ODYSSEY-HiQSDR-Red-Pitaya-transceiver/332074659425?ssPageName=STRK%3AMEBIDX%3AIT&_trksid=p2057872.m2749.l2649
//BPF pins logic definition russian board
//160m 1000
//80m 0100
//40m 1100
//30m 0010
//20m 1010
//17m 0110
//15m 1110
//12m 0001
//10m 1001
#if defined bpf_board_russian
int bpf_pin1 = 9;
int bpf_pin2 = 10;
int bpf_pin3 = 11;
int bpf_pin4 = 12;
#endif



int ptt_pin = 8;
int tx_pin = 7;
int pa_pin = 6;

int currentBand = 0;
boolean transmit = false;
boolean transmitAllowed = true;

void setup() {
#if defined debug
  Serial.begin(115200);
#endif
#if defined lpf_board_xf-lpf-hf
  pinMode(lpf_15_10m, OUTPUT);
  pinMode(lpf_20_17m, OUTPUT);
  pinMode(lpf_40m, OUTPUT);
  pinMode(lpf_80m, OUTPUT);
  digitalWrite(lpf_15_10m, LOW);
  digitalWrite(lpf_20_17m, LOW);
  digitalWrite(lpf_40m, LOW);
  digitalWrite(lpf_80m, LOW);
#endif

#if defined bpf_board_russian
  pinMode(bpf_pin1, OUTPUT);
  pinMode(bpf_pin2, OUTPUT);
  pinMode(bpf_pin3, OUTPUT);
  pinMode(bpf_pin4, OUTPUT);
  digitalWrite(bpf_pin1, LOW);
  digitalWrite(bpf_pin2, LOW);
  digitalWrite(bpf_pin3, LOW);
  digitalWrite(bpf_pin4, LOW);
#endif

  pinMode(ptt_pin, INPUT);
  pinMode(tx_pin, OUTPUT);
  digitalWrite(tx_pin, LOW);
  pinMode(pa_pin, OUTPUT);
  digitalWrite(pa_pin, LOW);

  //Start i2c as slave
  Wire.begin(I2C_ADDRESS);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);

}
void requestEvent()
{

  Serial.print("Connect Request: ");
  if (received == COMMAND_ON_OFF) {
    //Wire.write((uint8_t *)&speed, sizeof(speed));
  } else {
    Wire.write(0);
  }
}
void loop() {


  int ptt = digitalRead(ptt_pin);
  if (ptt != transmit) {
    toggleTxmit(ptt);
  }
  //  Serial.print("PTT Status: ");
  //  Serial.println(ptt);
  //  digitalWrite(tx_pin, ptt);
  delay(50);
}
void toggleTxmit(int ptt) {
  if (transmitAllowed == true) {
    transmit = ptt;
    if (ptt == 1) {
      digitalWrite(tx_pin, HIGH);
      digitalWrite(pa_pin, HIGH);
    } else {
      digitalWrite(tx_pin, LOW);
      digitalWrite(pa_pin, LOW);
    }
  }
}
void receiveEvent(int bytes) {

  int byteCount = 0;
  int command = 0;
  uint8_t byte1 = 0;
  uint8_t byte2 = 0;
  uint8_t byte3 = 0;
  int freqHigh [8] = {0, 0, 0, 0, 0, 0, 0, 0};
  int freqLow [7] = {0, 0, 0, 0, 0, 0, 0};
  while (0 < Wire.available()) {
    byte x = Wire.read();
    if (genericMode == false) {
      if (byteCount == 0) {
        byte1 = x;
      }
      if (byteCount == 1) {
        byte2 = x;
        command = command + (byte2 * 100);
      }
      if (byteCount == 2) {
        byte3 = x;
        command = command + byte3;
      }
      if (byte1 == 2 && byte2 == 2 && byte3 == 3) {
        genericMode = true;
#if defined debug
        Serial.println("Generic Mode set to true");
#endif
      }
      byteCount = byteCount + 1;
    } else {



      freqHigh[byteCount] = x;




      byteCount = byteCount + 1;
#if defined debug
      Serial.println("Generic Mode Command Received");
#endif
    }
  }
#if defined debug
  if (genericMode == false) {
    Serial.print("Bytes: ");
    Serial.println(bytes);
    Serial.print("byte1: ");
    Serial.println(byte1, HEX);
    Serial.print("byte2: ");
    Serial.println(byte2, HEX);
    Serial.print("byte3: ");
    Serial.println(byte3, HEX);
    Serial.print("Command: ");
    Serial.println(command);
  } else {

    Serial.print("freq Received: ");
    Serial.print(freqHigh[0]);
    Serial.print(freqHigh[1]);
    Serial.print(freqHigh[2]);
    Serial.print(freqHigh[3]);
    Serial.print(freqHigh[4]);
    Serial.print(freqHigh[5]);
    Serial.print(freqHigh[6]);
    Serial.println(freqHigh[7]);
  }
#endif
  processCommand(command);

}
void processCommand(int command) {

  if (currentBand != command) {
    currentBand = command;
    switch (command) {
      case FILTER_160M:
#if defined bpf_board_russian
        digitalWrite(bpf_pin1, HIGH);
        digitalWrite(bpf_pin2, LOW);
        digitalWrite(bpf_pin3, LOW);
        digitalWrite(bpf_pin4, LOW);
#endif
#if defined lpf_board_xf-lpf-hf
        //Not supported
        digitalWrite(lpf_15_10m, LOW);
        digitalWrite(lpf_20_17m, LOW);
        digitalWrite(lpf_40m, LOW);
        digitalWrite(lpf_80m, LOW);
#endif
        break;
      case FILTER_80m:
#if defined bpf_board_russian
        digitalWrite(bpf_pin1, LOW);
        digitalWrite(bpf_pin2, HIGH);
        digitalWrite(bpf_pin3, LOW);
        digitalWrite(bpf_pin4, LOW);
#endif
#if defined lpf_board_xf-lpf-hf
        digitalWrite(lpf_15_10m, LOW);
        digitalWrite(lpf_20_17m, LOW);
        digitalWrite(lpf_40m, LOW);
        digitalWrite(lpf_80m, HIGH);
#endif
        break;
      case FILTER_60m:
#if defined bpf_board_russian
        //Not supported
        digitalWrite(bpf_pin1, LOW);
        digitalWrite(bpf_pin2, LOW);
        digitalWrite(bpf_pin3, LOW);
        digitalWrite(bpf_pin4, LOW);
#endif
#if defined lpf_board_xf-lpf-hf
        //Not supported
        digitalWrite(lpf_15_10m, LOW);
        digitalWrite(lpf_20_17m, LOW);
        digitalWrite(lpf_40m, LOW);
        digitalWrite(lpf_80m, LOW);
#endif
        break;
      case FILTER_40m:
#if defined bpf_board_russian
        digitalWrite(bpf_pin1, HIGH);
        digitalWrite(bpf_pin2, HIGH);
        digitalWrite(bpf_pin3, LOW);
        digitalWrite(bpf_pin4, LOW);
#endif
#if defined lpf_board_xf-lpf-hf
        digitalWrite(lpf_15_10m, LOW);
        digitalWrite(lpf_20_17m, LOW);
        digitalWrite(lpf_40m, HIGH);
        digitalWrite(lpf_80m, LOW);
#endif
        break;
      case FILTER_30m:
#if defined bpf_board_russian
        digitalWrite(bpf_pin1, LOW);
        digitalWrite(bpf_pin2, LOW);
        digitalWrite(bpf_pin3, HIGH);
        digitalWrite(bpf_pin4, LOW);
#endif
#if defined lpf_board_xf-lpf-hf
        //Not supported
        digitalWrite(lpf_15_10m, LOW);
        digitalWrite(lpf_20_17m, LOW);
        digitalWrite(lpf_40m, LOW);
        digitalWrite(lpf_80m, LOW);
#endif
        break;
      case FILTER_20m:
#if defined bpf_board_russian
        digitalWrite(bpf_pin1, HIGH);
        digitalWrite(bpf_pin2, LOW);
        digitalWrite(bpf_pin3, HIGH);
        digitalWrite(bpf_pin4, LOW);
#endif
#if defined lpf_board_xf-lpf-hf
        digitalWrite(lpf_15_10m, LOW);
        digitalWrite(lpf_20_17m, HIGH);
        digitalWrite(lpf_40m, LOW);
        digitalWrite(lpf_80m, LOW);
#endif
        break;
      case FILTER_17m:
#if defined bpf_board_russian
        digitalWrite(bpf_pin1, LOW);
        digitalWrite(bpf_pin2, HIGH);
        digitalWrite(bpf_pin3, HIGH);
        digitalWrite(bpf_pin4, LOW);
#endif
#if defined lpf_board_xf-lpf-hf
        digitalWrite(lpf_15_10m, LOW);
        digitalWrite(lpf_20_17m, HIGH);
        digitalWrite(lpf_40m, LOW);
        digitalWrite(lpf_80m, LOW);
#endif
        break;
      case FILTER_15m:
#if defined bpf_board_russian
        digitalWrite(bpf_pin1, HIGH);
        digitalWrite(bpf_pin2, HIGH);
        digitalWrite(bpf_pin3, HIGH);
        digitalWrite(bpf_pin4, LOW);
#endif
#if defined lpf_board_xf-lpf-hf
        digitalWrite(lpf_15_10m, HIGH);
        digitalWrite(lpf_20_17m, LOW);
        digitalWrite(lpf_40m, LOW);
        digitalWrite(lpf_80m, LOW);
#endif
        break;
      case FILTER_12m:
#if defined bpf_board_russian
        digitalWrite(bpf_pin1, LOW);
        digitalWrite(bpf_pin2, LOW);
        digitalWrite(bpf_pin3, LOW);
        digitalWrite(bpf_pin4, HIGH);
#endif
#if defined lpf_board_xf-lpf-hf
        digitalWrite(lpf_15_10m, HIGH);
        digitalWrite(lpf_20_17m, LOW);
        digitalWrite(lpf_40m, LOW);
        digitalWrite(lpf_80m, LOW);
#endif
        break;
      //                case FILTER_10m:
      //                        #if defined bpf_board_russian
      //                        digitalWrite(bpf_pin1, HIGH);
      //                        digitalWrite(bpf_pin2, LOW);
      //                        digitalWrite(bpf_pin3, LOW);
      //                        digitalWrite(bpf_pin4, HIGH);
      //                        #endif
      //                        #if defined lpf_board_xf-lpf-hf
      //                        digitalWrite(lpf_15_10m, HIGH);
      //                        digitalWrite(lpf_20_17m, LOW);
      //                        digitalWrite(lpf_40m, LOW);
      //                        digitalWrite(lpf_80m, LOW);
      //                        #endif
      //                        break;
      //                case FILTER_6m:
      //                        #if defined bpf_board_russian
      //                        digitalWrite(bpf_pin1, LOW);
      //                        digitalWrite(bpf_pin2, LOW);
      //                        digitalWrite(bpf_pin3, LOW);
      //                        digitalWrite(bpf_pin4, LOW);
      //                        #endif
      //                        #if defined lpf_board_xf-lpf-hf
      //                        digitalWrite(lpf_15_10m, LOW);
      //                        digitalWrite(lpf_20_17m, LOW);
      //                        digitalWrite(lpf_40m, LOW);
      //                        digitalWrite(lpf_80m, LOW);
      //                        #endif
      //                        break;
      default:
        //TODO Disable TX
        //Wire.write(0);
#if defined bpf_board_russian
        digitalWrite(bpf_pin1, LOW);
        digitalWrite(bpf_pin2, LOW);
        digitalWrite(bpf_pin3, LOW);
        digitalWrite(bpf_pin4, LOW);
#endif
#if defined lpf_board_xf-lpf-hf
        digitalWrite(lpf_15_10m, LOW);
        digitalWrite(lpf_20_17m, LOW);
        digitalWrite(lpf_40m, LOW);
        digitalWrite(lpf_80m, HIGH);
#endif
        break;
    }
  }
}
