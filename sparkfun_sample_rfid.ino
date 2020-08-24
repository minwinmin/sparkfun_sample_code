#include <SoftwareSerial.h> //Used for transmitting to the device
SoftwareSerial softSerial(2, 3); //RX, TX

#include "SparkFun_UHF_RFID_Reader.h" //Library for controlling the M6E Nano module
RFID nano; //Create instance

int switchPin = 8;
int switchPin2 = 9;
//関数定義
boolean setupNano(long baudRate);
void detectWinebottles();

char Tags[25][25] = {};
char CurrentTags[10][25] = {};
char TemporaryTags[10][25] = {};
char DifferenceTags[10][25] = {};

int k = 1;
int jj = 0;

void setup()
{
  Serial.begin(115200);

  pinMode(switchPin, INPUT);
  pinMode(switchPin2, INPUT);
  
  while (!Serial); //Wait for the serial port to come online

  if (setupNano(38400) == false) //Configure nano to run at 38400bps
  {
    Serial.println(F("Module failed to respond. Please check wiring."));
    while (1); //Freeze!
  }

  nano.setRegion(REGION_NORTHAMERICA); //Set to North America

  nano.setReadPower(2000); //5.00 dBm. Higher values may caues USB port to brown out
  //Max Read TX Power is 27.00 dBm and may cause temperature-limit throttling

  Serial.println(F("Press a key to begin scanning for tags."));
  while (!Serial.available()); //Wait for user to send a character
  Serial.read(); //Throw away the user's character

  nano.startReading(); //Begin scanning for tags

  //initial detection
  float now_time = millis() / 1000;
  while ((millis()/1000 - now_time) < 30){
    detectWinebottles();
  }
  memcpy(CurrentTags, TemporaryTags, sizeof(CurrentTags));

  //initialize TemporaryTags
  memset(TemporaryTags, 0 , sizeof(TemporaryTags));
  Serial.println("Initial setting is done...");
  
}

void loop()
{
  int arrayNumber = sizeof(CurrentTags) / sizeof(CurrentTags[0]);
  /*
  //1:open 0:close
  int buttonState = digitalRead(switchPin);
  //Serial.println(buttonState);
  if(buttonState == LOW){
    //何度か取得を繰り返す
    //短期間で開け閉めされた場合はワインタグチェック処理実行しない、機能を実装する必要がある。
    //detectWinebottles();
    //格納されているタグを表示する部分
      Serial.println("All tags is : ");
      for(int j = 0; j <arrayNumber; j++){
        Serial.println(CurrentTags[j]);
      }
  }else{
    //Serial.println("BUTTON LOW");
  }
  */

  //detect increase or decrease of the number of tags
  //int buttonState2 = digitalRead(switchPin2);
  int buttonState = digitalRead(switchPin);
  //if(buttonState2 == LOW){
  if(buttonState == LOW){
    
    int dz = 0;
    
    Serial.println("Detecting...");
    
    float now_time = millis() / 1000;
    while ((millis()/1000 - now_time) < 30){
      detectWinebottles();
    }
    for( int di = 0; di<arrayNumber; di++){
      int dk = 0;
      for( int dj = 0; dj<arrayNumber; dj++){
        String temp_CurrentWineTag = CurrentTags[di];
        String temp_TemporaryWineTag = TemporaryTags[dj];
        if(temp_CurrentWineTag == temp_TemporaryWineTag){
          dk = dk + 1;
          Serial.println("dk is :");
          Serial.println(dk);
        }
      }
      //DifferenceTagsに追加
      if(dk == 0){
        strcpy(DifferenceTags[dz], CurrentTags[di]); 
        dz += 1;       
      }
    }
    Serial.println("Removed tags is : ");
    for(int j = 0; j <arrayNumber; j++){
        Serial.println(DifferenceTags[j]);
    }
    //TemporaryTagsは使用後、初期化しておく（初期状態として0を代入する）。
    memset(TemporaryTags, 0 , sizeof(TemporaryTags));
    Serial.println("Done...");

    }
  
  delay(100);
}


void detectWinebottles(){
  //検出されたタグのEPCを一時的に格納する変数
  String uniqueEPC;
  
  if (nano.check() == true) //Check to see if any new data has come in from module
  {
    byte responseType = nano.parseResponse(); //Break response into tag ID, RSSI, frequency, and timestamp

    if (responseType == RESPONSE_IS_KEEPALIVE)
    {
      Serial.println(F("Scanning"));
    }
    else if (responseType == RESPONSE_IS_TAGFOUND)
    {
      
      //If we have a full record we can pull out the fun bits
      int rssi = nano.getTagRSSI(); //Get the RSSI for this tag read

      long freq = nano.getTagFreq(); //Get the frequency this tag was detected at

      long timeStamp = nano.getTagTimestamp(); //Get the time this was read, (ms) since last keep-alive message

      byte tagEPCBytes = nano.getTagEPCBytes(); //Get the number of bytes of EPC from response

      // Serial.print(F(" rssi["));
      // Serial.print(rssi);
      // Serial.print(F("]"));

      // Serial.print(F(" freq["));
      // Serial.print(freq);
      // Serial.print(F("]"));

      // Serial.print(F(" time["));
      // Serial.print(timeStamp);
      // Serial.print(F("]"));

      //Print EPC bytes, this is a subsection of bytes from the response/msg array
      Serial.print(F(" epc["));
      for (byte x = 0 ; x < tagEPCBytes ; x++)
      {
        if (nano.msg[31 + x] < 0x10) Serial.print(F("0")); //Pretty print
        //Serial.print(nano.msg[31 + x], HEX);
        //Serial.print(F(" "));
        String epc = String(nano.msg[31 + x], HEX);
        uniqueEPC.concat(epc);
        
      }
      Serial.print(uniqueEPC);
      Serial.print(F("]"));
      Serial.println();


      if(jj == 0){
        Serial.println("uniqueEPC is : ");
        Serial.println(uniqueEPC.c_str());
        strcpy(TemporaryTags[0], uniqueEPC.c_str());
        //Tags[0][30] = "HELLO";
        jj += 1;
      }
      
      int arrayNumber = sizeof(TemporaryTags) / sizeof(TemporaryTags[0]);
      // Serial.println("Tags is : ");
      // Serial.println(sizeof(TemporaryTags));
      // Serial.println("Tags[0] is : ");
      // Serial.println(sizeof(TemporaryTags[0]));
      // Serial.println(TemporaryTags[0]);
      // Serial.println("ArrayNumber is : ");
      // Serial.println(arrayNumber);
      int count = 0;
      if (arrayNumber != 0){
        for (int i = 0; i<=arrayNumber; i++){

          String s_temp = TemporaryTags[i];
          //重複がみつかったらカウントする
          if(s_temp == uniqueEPC){
            count += 1;
          }   
        }

        if (count == 0){
          strcpy(TemporaryTags[k], uniqueEPC.c_str());
          k += 1;
        }
      }

      Serial.println("All tags is : ");
      for(int j = 0; j <arrayNumber; j++){
        Serial.println(TemporaryTags[j]);
      }
      
    }
    //複数タグのレスポンスが重なると下記が表示されるようだ。
    else if (responseType == ERROR_CORRUPT_RESPONSE)
    {
      Serial.println("Bad CRC");
    }
    else
    {
      //Unknown response
      Serial.print("Unknown error");
    }
  }
}

//Gracefully handles a reader that is already configured and already reading continuously
//Because Stream does not have a .begin() we have to do this outside the library
boolean setupNano(long baudRate)
{
  nano.begin(softSerial); //Tell the library to communicate over software serial port

  //Test to see if we are already connected to a module
  //This would be the case if the Arduino has been reprogrammed and the module has stayed powered
  softSerial.begin(baudRate); //For this test, assume module is already at our desired baud rate
  while (softSerial.isListening() == false); //Wait for port to open

  //About 200ms from power on the module will send its firmware version at 115200. We need to ignore this.
  while (softSerial.available()) softSerial.read();

  nano.getVersion();

  if (nano.msg[0] == ERROR_WRONG_OPCODE_RESPONSE)
  {
    //This happens if the baud rate is correct but the module is doing a ccontinuous read
    nano.stopReading();

    Serial.println(F("Module continuously reading. Asking it to stop..."));

    delay(1500);
  }
  else
  {
    //The module did not respond so assume it's just been powered on and communicating at 115200bps
    softSerial.begin(115200); //Start software serial at 115200

    nano.setBaud(baudRate); //Tell the module to go to the chosen baud rate. Ignore the response msg

    softSerial.begin(baudRate); //Start the software serial port, this time at user's chosen baud rate

    delay(250);
  }

  //Test the connection
  nano.getVersion();
  if (nano.msg[0] != ALL_GOOD) return (false); //Something is not right

  //The M6E has these settings no matter what
  nano.setTagProtocol(); //Set protocol to GEN2

  nano.setAntennaPort(); //Set TX/RX antenna ports to 1

  return (true); //We are ready to rock
}
