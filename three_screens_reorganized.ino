#include <WiFi.h>
#include <SoftwareSerial.h>

//lcd variables
SoftwareSerial lcd1(2, 4);
SoftwareSerial lcd2(2, 3);
SoftwareSerial lcd3(2, 5);

//internet variables
char ssid[] = "PumpingStationOne";     //  your network SSID (name) 
char pass[] = "ps1frocks";    // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status
char server[] = "bry.arbitrarion.com";
WiFiClient client1;
WiFiClient client2;
WiFiClient client3;
char response[] = "";
IPAddress dns(8, 8, 8, 8);  //Google dns  
IPAddress ip;

//input button variables
const int button1Pin = 6;
const int button2Pin = 8;
const int button3Pin = 9;
int button1State = 0;
int button2State = 0;
int button3State = 0;

//logic variables
const int millsToWaitToCheckButtons = 4000;
int timeToCheckButton1 = 0;
int timeToCheckButton2 = 0;
int timeToCheckButton3 = 0;
boolean isShowingAnswer1 = false;
boolean isShowingAnswer2 = false;
boolean isShowingAnswer3 = false;
boolean readingForQuestion1 = true;
boolean readingForQuestion2 = false;
boolean readingForQuestion3 = false;
boolean hasConnectedToClient1 = false;
boolean hasConnectedToClient2 = false;
boolean hasConnectedToClient3 = false;
boolean dummyVariable = true;
String display1Question;
String display2Question;
String display3Question;
String display1Answer;
String display2Answer;
String display3Answer;
String display1Buffer;
String fromClient;
String message = "";
String answer = "";
int currentTime = 0;

void setup() {
  lcd1.begin(9600);
  lcd2.begin(9600);
  lcd3.begin(9600);
  Serial.begin(9600);
  
  ConnectToWifi();
  
  setBacklight(lcd1, 255);
  setBacklight(lcd2, 255);
  setBacklight(lcd3, 255);
  
  //display1Question = "Question 1";
  //display2Question = "Question 2";
  //display3Question = "Question 3";
  
  //display1Answer = "Answer 1";
  //display2Answer = "Answer 2";
  //display3Answer = "Answer 3";
  
  setAsDisplay(lcd1, display1Question);
  setAsDisplay(lcd2, display2Question);
  setAsDisplay(lcd3, display3Question);
  
  pinMode(button1Pin, INPUT);
  pinMode(button2Pin, INPUT);
  pinMode(button3Pin, INPUT);
  
  currentTime = millis();
  timeToCheckButton1 = currentTime;
  timeToCheckButton2 = currentTime;
  timeToCheckButton3 = currentTime;
}

void loop() {
  currentTime = millis();
  
  RunButtonLogic(button1Pin, button1State, lcd1, timeToCheckButton1, isShowingAnswer1, currentTime, display1Question, display1Answer);  
  RunButtonLogic(button2Pin, button2State, lcd2, timeToCheckButton2, isShowingAnswer2, currentTime, display2Question, display2Answer);
  RunButtonLogic(button3Pin, button3State, lcd3, timeToCheckButton3, isShowingAnswer3, currentTime, display3Question, display3Answer);
  RunWifiUpdateLogic(readingForQuestion1, client1, display1Question, display1Answer, lcd1, readingForQuestion2, "/message1", hasConnectedToClient1);
  RunWifiUpdateLogic(readingForQuestion2, client2, display2Question, display2Answer, lcd2, readingForQuestion3, "/message2", hasConnectedToClient2);
  RunWifiUpdateLogic(readingForQuestion3, client3, display3Question, display3Answer, lcd3, dummyVariable, "/message3", hasConnectedToClient3);
}

void RunWifiUpdateLogic(boolean& isReadingForQuestion, WiFiClient theClient, String& displayQuestion, String& displayAnswer, SoftwareSerial theLCD, boolean& isReadingForNextQuestion, String pageToConnectTo, boolean& hasConnectedToClient) {
  if( isReadingForQuestion) {
      boolean angleBracketHasBeenSited = false;
      
      if(hasConnectedToClient == false) {
        Serial.print("Attempting to connect to client through ");
        Serial.println(pageToConnectTo);
        ConnectToServer(theClient, pageToConnectTo);
        hasConnectedToClient = true;
        delay(5000);
        fromClient = "";
      }
      
      while (theClient.available()) {
        char c = theClient.read();
        if(c == '<') {
          angleBracketHasBeenSited = true;
        }
        
        if(angleBracketHasBeenSited) {
          fromClient += c;
        }
        Serial.write(c);
        //Serial.print("from client so far: ");
        //Serial.println(fromClient);
      }
        // if the server's disconnected, stop the client:
      if (!theClient.connected()) {
        Serial.println();
        Serial.print("disconnecting from server with connection ");
        Serial.println(pageToConnectTo);
        theClient.stop();
        
        String beginningAnswerTag = "<answer>";
        String beginningMessageTag = "<message>";
        //Serial.print("the raw message is: ");
        //Serial.println(fromClient);
        //Serial.print("beginning message tag length: ");
        //Serial.println(beginningMessageTag.length());
        //Serial.print("beginning answer tag length: ");
        //Serial.println(beginningAnswerTag.length());
        //Serial.print("from client index of beginning message tag");
        //Serial.println(fromClient.indexOf(beginningMessageTag));
        //Serial.print("from client index of end message tag");
        //Serial.println(fromClient.indexOf("</message>"));
        message = fromClient.substring(fromClient.indexOf(beginningMessageTag) + beginningMessageTag.length(), fromClient.indexOf("</message>"));
        answer = fromClient.substring(fromClient.indexOf(beginningAnswerTag) + beginningAnswerTag.length(), fromClient.indexOf("</answer>"));
        displayQuestion = message;
        displayAnswer = answer;
        
        Serial.print("The message is: ");
        Serial.println(message);
        Serial.print("The answer is: ");
        Serial.println(answer);
        isReadingForQuestion = false;
        isReadingForNextQuestion = true;
        hasConnectedToClient = false;
        setAsDisplay(theLCD, message); 
      }
    }    
}

void RunButtonLogic(int buttonPin, int& buttonState, SoftwareSerial theLCD, 
    int& timeToCheckButton, boolean& isShowingAnswer, int currentTime,
    String question, String answer) {
      
  buttonState = digitalRead(buttonPin);
  
  if(buttonState == LOW)
  {
     setAsDisplay(theLCD, answer); 
     timeToCheckButton = millis() + millsToWaitToCheckButtons;
     isShowingAnswer = true;
  }
  
  if(isShowingAnswer && currentTime >= timeToCheckButton)
  {
      setAsDisplay(theLCD, question);  
      isShowingAnswer = false;
  }
}

void ConnectToWifi() {
  // attempt to connect using WPA2 encryption:
  Serial.println("Trying to connect to WPA network");
  status = WiFi.begin(ssid, pass);
  
  // if you're not connected, stop here:
  if ( status != WL_CONNECTED) { 
    Serial.println("Couldn't get a wifi connection");
    while(true);
  } 
  // if you are connected, print out info about the connection:
  else {
    Serial.println("Connected to network");
    
    //ip = WiFi.localIP();
    //setAsDisplay(theLCD, "IP address is: ");
    //delay(2000);
    //clearDisplay(theLCD);
    //setLCDCursor(theLCD, 0);
    //theLCD.print(ip);
    //delay(2000);
    
    Serial.println("Starting connection...");
    
    //ConnectToServer(client1, "/message1");
    //ConnectToServer(client2, "/message2");
    //ConnectToServer(client3, "/message3");
  }
}

void ConnectToServer(WiFiClient& theClient, String theMessage) {
  if(theClient.connect(server, 80)) {
      Serial.println("connected");     
      
      String getMessage = "GET " + theMessage + " HTTP/1.0";
      theClient.println(getMessage);
      theClient.println("Host:bry.arbitrarion.com");
      theClient.println("Connection: close");
      theClient.println();
    }
    else {
      Serial.println("not connected.");
    }  
}

void setAsDisplay(SoftwareSerial theLCD, char stuff[])
{
   clearDisplay(theLCD);
   setLCDCursor(theLCD, 0);
   theLCD.print(stuff);
}

void setAsDisplay(SoftwareSerial theLCD, String stuff)
{
   clearDisplay(theLCD);
   setLCDCursor(theLCD, 0);
   theLCD.print(stuff);
}

void setBaudRate(SoftwareSerial theLCD, byte baud_rate) {
  theLCD.write(0x81);
  theLCD.write(baud_rate);  
}

void clearDisplay(SoftwareSerial theLCD) {
  theLCD.write(0xFE);  // send the special command
  theLCD.write(0x01);  // send the clear screen command
}

void setLCDCursor(SoftwareSerial theLCD, byte cursor_position)
{
  theLCD.write(0xFE);  // send the special command
  theLCD.write(0x80);  // send the set cursor command
  theLCD.write(cursor_position);  // send the cursor position
}

void setBacklight(SoftwareSerial theLCD, byte brightness)
{
  theLCD.write(0x80);  // send the backlight command
  theLCD.write(brightness);  // send the brightness value
}
