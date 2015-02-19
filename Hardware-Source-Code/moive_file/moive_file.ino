#include <SPI.h>
#include <WiFi.h>
char tag1[10] = "AD357C24";  
char tag2[10] = "9E4E7C24";
char tag3[10] = "117D7C24";
int brightCheckPin = A0;
int bright = 0;
int led1 = 4;
int led2 = 5;
int led3 = 3;

byte sensorPin = 2;
byte indicator = 13;
byte state;
char ssid[] = "AndroidHotspot A840S(6411)";    //  your network SSID (name) 
char pass[] = "64116411";   // your network password
int keyIndex = 0;              
int status = WL_IDLE_STATUS;
WiFiServer server(80);
//WiFiClient client;// = server.available();
void setup() {
  pinMode(led1, OUTPUT); // 리셋 핀을 설정
  pinMode(led2, OUTPUT); // 리셋 핀을 설정  
  pinMode(led3, OUTPUT); // 리셋 핀을 설정  
  pinMode(indicator,OUTPUT);
  pinMode(sensorPin,INPUT);
  //Initialize serial and wait for port to open:
  Serial.begin(9600); 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present"); 
    // don't continue:
    while(true);
  } 
  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) { 
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:    
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  } 
  server.begin();
  // you're connected now, so print out the status:
  printWifiStatus();
  //check connection
  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);
  digitalWrite(led3, HIGH);
  delay(2000);
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
}

void loop() {
  char tagString[8]; //  tagString를 선언합니다
  int index =0 ;
  bright = analogRead(brightCheckPin);
  
  while(Serial.available()) { // 태그 리더가 태그를 가져다 대면, 읽기 시작합니다. 
    int readByte = Serial.read(); // read next available byte
    tagString[index] = readByte;
    index++;
    Serial.println(tagString);
  }

  digitalWrite(indicator,state);
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
      
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 0.5");  // refresh the page automatically every 0.5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");          
          checkTag(tagString, client); // 일치하는지 확인     
          //client.println("<html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

void checkTag(char tag[], WiFiClient client){
  state = digitalRead(sensorPin);
  if (strlen(tag) == 0 ){ // 문자열이 0이란 소리는 아무것도 읽지 않았다는 겁니다~
    return; // empty no ndeed to continue
  }
  if(compareTag(tag, tag1)){ 
    Serial.println("LED1 on");
    lightLED(led1);  
    client.print("* GoOutMode *");
  }
  if(compareTag(tag, tag2)){
    lightLED(led2);
    Serial.println("LED2 on");
    client.print("* SleepMode *");
  }
  if(compareTag(tag, tag3)){  
    digitalWrite(4,LOW);
    digitalWrite(5,LOW);
    lightLED(led3);
    
    //////////////////////////////////////////////
    
    //morning
    if(bright>30){
      digitalWrite(led1, LOW);
      digitalWrite(led2, LOW);
//      digitalWrite(led3, LOW);
    }
    //night
    if(bright<30){
      digitalWrite(led1, HIGH);
      digitalWrite(led2, HIGH);
      digitalWrite(led3, HIGH);
    }
    
    if(state == 1){  
      client.print("<tr><td>* Precent State  ");
      client.print("* Somebody is in this area! *");
    }
    if(state == 0){ 
      client.print("<tr><td>* Precent State  ");
      client.print("* No one is in this area! *");
    }
 }
 else { // 읽은 태그와 일치하는 저장 태그 문자열이 없습니다. !! 새로운 태그입니다. 태그 이름을 확인해 봅니다. 
   Serial.println("No matching Tag, New tag name is display");
   Serial.println(tag); // read out any unknown tag
 }
}

void lightLED(int pin){ // led를 깜빡입니다. 
  Serial.println(pin);
  if(pin == 3)
  {
    digitalWrite(5,LOW);
    digitalWrite(4,LOW);
    digitalWrite(3,HIGH);
  }
  if(pin == 4)
  {
    digitalWrite(3,LOW);
    digitalWrite(5,LOW);
    digitalWrite(4,HIGH);
  }
  if(pin == 5)
  {
    digitalWrite(3,LOW);    
    digitalWrite(4,LOW);
    digitalWrite(5,HIGH);
  }
}

boolean compareTag(char one[], char two[]){ // 입력 태그와 기설정된 태그 이름을 비교합니다. 
 // compare two value to see if same strcmp not wirking 100% so we do this
 Serial.println("in the compareTag");
 if(strlen(one) == 0 ){ // 스트링이 0 이네요 에러입니다 처음으로 돌아갑니다. 
 Serial.println("empty string");
  return false; // empty
  }
  for (int i=0; i<strlen(two); i++){  // 1대1로 문자를 비교합니다. 태그 이름은 12자리니까 0 부터 11 까지 12번 확인해야합니다 
    if(one[i]!=two[i]) {
    Serial.println("one of the string mismatches");
    return false; // 만약 하나라도 다르면, 일치하지 않는 것으로 간주하고 처음으로 돌아갑니다. 
    }
  }
  Serial.println("All string matches");
  return true;
}

