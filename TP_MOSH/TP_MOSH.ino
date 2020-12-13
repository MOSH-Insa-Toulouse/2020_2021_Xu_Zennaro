#include <rn2xx3.h>
#include <SoftwareSerial.h>
#include <avr/sleep.h>

#define UART_rxPin 5
#define UART_txPin 6

#define button1 8 //display
#define button2 9 //transmission
#define bip 10
#define led1 13

#define interruptPin 2

#define gasSensorPin A0

SoftwareSerial mySerial(UART_rxPin, UART_txPin); // RX, TX
rn2xx3 myLora(mySerial);

float prev_value;

void setup()
{
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(led1, OUTPUT);
  pinMode(bip, OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);
  
 // Open serial communications and wait for port to open:
  Serial.begin(57600); //serial port to computer
  mySerial.begin(9600); //serial port to radio
  
  initialize_radio();
  myLora.tx("TTN Mapper on TTN Enschede node");

  delay(2000);
  Serial.println("Ready to serve");

  prev_value = 0;
}

////////////////////////////////
void loop() // run over and over
{  
  TX_RETURN_TYPE ack;
  
  float value = analogRead(gasSensorPin);
  Serial.println(value);

  if(abs(prev_value - value) > 50){
    value = value/1023 * 5; //plage de tensions de l'Arduino [0V ; 5V]
    value *= 100;
    int int_value = (int)value;
    String str_value = (String)int_value;
    Serial.print("TXing: ");
    Serial.println(str_value);
    ack = myLora.tx(str_value);
    prev_value = value;
  }

/*
 * Interruption:
 * Setup le reading du capteur sur une pin
 * Setup en hardware une valeur de référence sur une autre pin
 * Setup en setup() une comparaison hardware des deux pins
 * Si la différence dépasse un certain seuil, interruption
 */

  delay(2000);
  
  if (digitalRead(button1)){
    Serial.print("Last received message: ");
    Serial.println(myLora.getRx());
    delay(200);
      for (int i=0;i<1000;i++){
        digitalWrite(bip,HIGH);
        delayMicroseconds(2272);
        digitalWrite(bip,LOW);
        delayMicroseconds(2272);
      }
  }

  if (ack){
    digitalWrite(led1,HIGH);
    delay(1000);
    digitalWrite(led1,LOW);
  }else{
    for (int i=0;i<10;i++){
      digitalWrite(led1,HIGH);
      delay(100);
      digitalWrite(led1,LOW);
      delay(100);
    }
  }
    
  eco_mode_board();
  delay(1000);

}


/**************************************************************************************************/
/**************************************************************************************************/
void initialize_radio()
{
  //reset rn2483
  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);
  delay(500);
  digitalWrite(12, HIGH);

  delay(100); //wait for the RN2xx3's startup message
  mySerial.flush();

  //Autobaud the rn2483 module to 9600. The default would otherwise be 57600.
  myLora.autobaud();

  //check communication with radio
  String hweui = myLora.hweui();
  while(hweui.length() != 16)
  {
    Serial.println("Communication with RN2xx3 unsuccessful. Power cycle the board.");
    Serial.println(hweui);
    delay(10000);
    hweui = myLora.hweui();
  }

  //print out the HWEUI so that we can register it via ttnctl
  Serial.println("When using OTAA, register this DevEUI: ");
  Serial.println(myLora.hweui());
  Serial.println("RN2xx3 firmware version:");
  Serial.println(myLora.sysver());

  //configure your keys and join the network
  Serial.println("Trying to join TTN");
  bool join_result = false;


  /*
   * ABP: initABP(String addr, String AppSKey, String NwkSKey);
   * Paste the example code from the TTN console here:
   */
  const char *devAddr = "260137D5";
  const char *nwkSKey = "E15DC306273AEA30CD8BE65D584D78EF";
  const char *appSKey = "573E09DB4F96FD3DA3BCC74FD52B58EF";

  join_result = myLora.initABP(devAddr, appSKey, nwkSKey);
  

  while(!join_result)
  {
    Serial.println("Unable to join. Are your keys correct, and do you have TTN coverage?");
    delay(60000); //delay a minute before retry
    join_result = myLora.init();
  }
  Serial.println("Successfully joined TTN");

}

/*Procedure to put the board to sleep*/
void eco_mode_board(){
  sleep_enable();
  attachInterrupt(0, wakeUp_ISR, CHANGE);  //Attaching an interrupt to pin D2
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  //Setting the sleep mode (here, full sleep)
  sleep_cpu();
}


/*ISR of the interrupt to wake up the Arduino*/
void wakeUp_ISR(){
  Serial.println("Interrupt fired");
  sleep_disable();
  detachInterrupt(0); //from Pin D2
}
