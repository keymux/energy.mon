#include "EmonLib.h"

EnergyMonitor ct1;
EnergyMonitor ct2;
EnergyMonitor ct3;
EnergyMonitor ct4;

const int LEDpin = 9;

int ct1Enabled = false;
int ct2Enabled = false;
int ct3Enabled = false;
int ct4Enabled = false;

void setup()
{
  Serial.begin(9600);

  Serial.println("{\"message\": \"Welcome to Miner Mon\"}");
  Serial.println("{\"instructions\": \"To begin, send me a protocol string to turn on a current transformer.  Try sending me '|1|T' to enable CT1\"}");
  Serial.println("{\"disableInstructions\": \"You can disable CT1 by sending me '|1|F'.  The same commands work for CT 2, 3, and 4.\"}");
  
  ct1.current(1, 60.606);
  ct2.current(2, 60.606);
  ct3.current(3, 60.606);
  ct4.current(4, 60.606);

  float calibrationConstant = 115 * 11 / (9 * 1.075);

  ct1.voltage(0, calibrationConstant, 1.7);
  ct2.voltage(0, calibrationConstant, 1.7);
  ct3.voltage(0, calibrationConstant, 1.7);
  ct4.voltage(0, calibrationConstant, 1.7);
}

enum State {
  emptyState,
  startBar,
  ctNum,
  midBar,
  enabledState
};

State protocolState = emptyState;
int* ctSwitch;
int* ctSwitches[] = {&ct1Enabled, &ct2Enabled, &ct3Enabled, &ct4Enabled};

// Serial protocol: |CTNUM|T/F
//  e.g. Enable ct1: |1|T
//  e.g. Disable ct4: |4|F
void readProtocol() {
  while (Serial.available() > 0) {
    byte incomingByte = Serial.read();

    /*
    Serial.print("{\"protocolState\": ");
    Serial.print(protocolState);
    Serial.print(", \"incomingByte\":");
    Serial.print(incomingByte);
    Serial.println("}");
    */

    switch (protocolState) {
      case emptyState:
        if (incomingByte == '|') {
          protocolState = startBar;
        }
        
        break;
      case startBar:
        if (incomingByte > 48 && incomingByte < 53) {
          ctSwitch = ctSwitches[((int) incomingByte) - 49];
          protocolState = ctNum;
        } else {
          protocolState = emptyState;
        }
        
        break;
      case ctNum:
        if (incomingByte == '|') {
          protocolState = midBar;
        } else {
          protocolState = emptyState;
        }
        
        break;
      case midBar:
        if (incomingByte == 'T') {
          *ctSwitch = 1;
        } else if (incomingByte == 'F') {
          *ctSwitch = 0;
        }

        //printStates();

        ctSwitch = 0;
        protocolState = emptyState;

        break;
      default:
        break;
    }
  }
}

void printStates() {
  Serial.print("{\"ct1Enabled\": ");
  Serial.print(ct1Enabled);
  Serial.print(", \"ct2Enabled\": ");
  Serial.print(ct2Enabled);
  Serial.print(", \"ct3Enabled\": ");
  Serial.print(ct3Enabled);
  Serial.print(", \"ct4Enabled\": ");
  Serial.print(ct4Enabled);
  Serial.println("}");
}

void loop() 
{

  readProtocol();

  if (!ct1Enabled && !ct2Enabled && !ct3Enabled && !ct4Enabled) {
    return;
  }
  
  Serial.print("{");
  
  if (ct1Enabled) {
    ct1.calcVI(20,2000);
    Serial.print("\"ct1\":{\"realPower\":");
    Serial.print(ct1.realPower);
    Serial.print(",\"Irms\":");
    Serial.print(ct1.Irms);
    Serial.print(",\"Vrms\":");
    Serial.print(ct1.Vrms);
    Serial.print("}"); 
  }
  
  if (ct2Enabled) {
    ct2.calcVI(20,2000);
    if (ct1Enabled) {
      Serial.print(",");
    }

    Serial.print("\"ct2\":{\"realPower\":");
    Serial.print(ct2.realPower);
    Serial.print(",\"Irms\":");
    Serial.print(ct2.Irms);
    Serial.print(",\"Vrms\":");
    Serial.print(ct2.Vrms);
    Serial.print("}"); 
  }
  
  if (ct3Enabled) {
    ct3.calcVI(20,2000);
    if (ct1Enabled || ct2Enabled) {
      Serial.print(",");
    }

    Serial.print("\"ct3\":{\"realPower\":");
    Serial.print(ct3.realPower);
    Serial.print(",\"Irms\":");
    Serial.print(ct3.Irms);
    Serial.print(",\"Vrms\":");
    Serial.print(ct3.Vrms);
    Serial.print("}"); 
  }
  
  if (ct4Enabled) {
    ct4.calcVI(20,2000);
    if (ct1Enabled || ct2Enabled || ct3Enabled) {
      Serial.print(",");
    }

    Serial.print("\"ct4\":{\"realPower\":");
    Serial.print(ct4.realPower);
    Serial.print(",\"Irms\":");
    Serial.print(ct4.Irms);
    Serial.print(",\"Vrms\":");
    Serial.print(ct4.Vrms);
    Serial.print("}"); 
  }
  
  Serial.println("}");
}
