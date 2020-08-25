//#include <SoftwareSerial.h> //Used for transmitting to the device
//SoftwareSerial softSerial(2, 3); //RX, TX

#include "SparkFun_UHF_RFID_Reader.h" //Library for controlling the M6E Nano module
RFID nano; //Create instance

int switchPin = 8;
int switchPin2 = 9;

boolean setupNano(long baudRate);
void detectbottles();

char wineTags2[25][25] = {};
char CurrentTags[10][25] = {};
char TemporaryTags[10][25] = {};
char DifferenceTags[10][25] = {};

int k = 1;
int jj = 0;

#define SERIAL1 SerialUSB
#define SERIAL2 Serial1

//デバイス起動時の処理を考える。
//入っているワインを全てpostする処理が必要そう
//それとも初期コマンドっぽいものが必要かもしれぬ
//デバイス起動時にCurrentTagsに追加、APIにポストする（初期設定）（ワインが設置されているとする。）
//内容(data)も一気に取得できるようにしておくべき
void setup()
{
  SERIAL1.begin(115200);

  pinMode(switchPin, INPUT);
  pinMode(switchPin2, INPUT);
  while (!SERIAL1); //Wait for the serial port to come online

  if (setupNano(38400) == false) //Configure nano to run at 38400bps
  {
    SERIAL1.println(F("Module failed to respond. Please check wiring."));
    while (1); //Freeze!
  }

  nano.setRegion(REGION_NORTHAMERICA); //Set to North America

  //ここの大きさでどれくらいの距離を通信可能か調べておこう。
  nano.setReadPower(1500); //5.00 dBm. Higher values may caues USB port to brown out
  //Max Read TX Power is 27.00 dBm and may cause temperature-limit throttling

  SERIAL1.println(F("Press a key to begin scanning for tags."));
  while (!SERIAL1.available()); //Wait for user to send a character
  SERIAL1.read(); //Throw away the user's character

  nano.startReading(); //Begin scanning for tags

  //割り込み処理
  //割り込みの必要が今のところなさそうだ
  //attachInterrupt(0, detectbottles, FALLING);

  //初期設定
  //全てスキャンするまでにはどれくらかかるのか？
  //常時スキャンしててスイッチがLOWになったときだけ違う処理を走らせるとか？
  //一定時間スキャン時間をとるか
  //参考
  //https://detail.chiebukuro.yahoo.co.jp/qa/question_detail/q10180394955
  //unsigned long t0 = millis()/1000; 
  //1分60秒
  //変にdelayを挟むと検出精度がさがるイメージ。
  while (millis()/1000<60){
    detectbottles();
  }
  memcpy(CurrentTags, TemporaryTags, sizeof(CurrentTags));
  SERIAL1.println("Initial setting is done...");
  

}

void loop()
{
  //1:open 0:close
  int buttonState = digitalRead(switchPin);
  //Serial.println(buttonState);
  int arrayNumber = sizeof(CurrentTags) / sizeof(CurrentTags[0]);
  if(buttonState == LOW){
    //何度か取得を繰り返す
    //短期間で開け閉めされた場合はワインタグチェック処理実行しない、機能を実装する必要がある。
    //detectbottles();
    //格納されているタグを表示する部分
      SERIAL1.println("All tags is : ");
      for(int j = 0; j <arrayNumber; j++){
        SERIAL1.println(CurrentTags[j]);
      }
  }else{
    //Serial.println("BUTTON LOW");
  }

  delay(100);
}


void detectbottles(){
  //検出されたタグのEPCを一時的に格納する変数
  String uniqueEPC;
  
  if (nano.check() == true) //Check to see if any new data has come in from module
  {
    byte responseType = nano.parseResponse(); //Break response into tag ID, RSSI, frequency, and timestamp

    if (responseType == RESPONSE_IS_KEEPALIVE)
    {
      SERIAL1.println(F("Scanning"));
    }
    else if (responseType == RESPONSE_IS_TAGFOUND)
    {
      
      //If we have a full record we can pull out the fun bits
      int rssi = nano.getTagRSSI(); //Get the RSSI for this tag read

      long freq = nano.getTagFreq(); //Get the frequency this tag was detected at

      long timeStamp = nano.getTagTimestamp(); //Get the time this was read, (ms) since last keep-alive message

      byte tagEPCBytes = nano.getTagEPCBytes(); //Get the number of bytes of EPC from response

      SERIAL1.print(F(" rssi["));
      SERIAL1.print(rssi);
      SERIAL1.print(F("]"));

      SERIAL1.print(F(" freq["));
      SERIAL1.print(freq);
      SERIAL1.print(F("]"));

      SERIAL1.print(F(" time["));
      SERIAL1.print(timeStamp);
      SERIAL1.print(F("]"));

      //Print EPC bytes, this is a subsection of bytes from the response/msg array
      SERIAL1.print(F(" epc["));
      for (byte x = 0 ; x < tagEPCBytes ; x++)
      {
        if (nano.msg[31 + x] < 0x10) SERIAL1.print(F("0")); //Pretty print
        //Serial.print(nano.msg[31 + x], HEX);
        //Serial.print(F(" "));
        String epc = String(nano.msg[31 + x], HEX);
        uniqueEPC.concat(epc);
        
      }
      SERIAL1.print(uniqueEPC);
      SERIAL1.print(F("]"));
      SERIAL1.println();

      if(jj == 0){
        SERIAL1.println("uniqueEPC is : ");
        SERIAL1.println(uniqueEPC.c_str());
        strcpy(TemporaryTags[0], uniqueEPC.c_str());
        //wineTags2[0][30] = "HELLO";
        jj += 1;
      }

      int arrayNumber = sizeof(TemporaryTags) / sizeof(TemporaryTags[0]);
      SERIAL1.println("wineTags2 is : ");
      SERIAL1.println(sizeof(TemporaryTags));
      SERIAL1.println("wineTags2[0] is : ");
      SERIAL1.println(sizeof(TemporaryTags[0]));
      SERIAL1.println(TemporaryTags[0]);
      SERIAL1.println("ArrayNumber is : ");
      SERIAL1.println(arrayNumber);
      int count = 0;
      if (arrayNumber != 0){
        for (int i = 0; i<=arrayNumber; i++){
          String s_temp = TemporaryTags[i];
          if(s_temp == uniqueEPC){
            count += 1;
          }   
        }
        
        if (count == 0){
          strcpy(TemporaryTags[k], uniqueEPC.c_str());
          k += 1;
        }
      }

      SERIAL1.println("All tags is : ");
      for(int j = 0; j <arrayNumber; j++){
        SERIAL1.println(TemporaryTags[j]);
      }
      
    }

    else if (responseType == ERROR_CORRUPT_RESPONSE)
    {
      SERIAL1.println("Bad CRC");
    }
    else
    {
      //Unknown response
      SERIAL1.print("Unknown error");
    }
  }
}

//下記はあまり触らない方が良さそう。
//Gracefully handles a reader that is already configured and already reading continuously
//Because Stream does not have a .begin() we have to do this outside the library
boolean setupNano(long baudRate)
{
  nano.begin(SERIAL2); //Tell the library to communicate over software serial port

  //Test to see if we are already connected to a module
  //This would be the case if the Arduino has been reprogrammed and the module has stayed powered
  SERIAL2.begin(baudRate); //For this test, assume module is already at our desired baud rate
  //ここコメントアウトOK?
  //while (SERIAL2.isListening() == false); //Wait for port to open
  while (!SERIAL2);
  //About 200ms from power on the module will send its firmware version at 115200. We need to ignore this.
  while (SERIAL2.available()) SERIAL2.read();

  nano.getVersion();

  if (nano.msg[0] == ERROR_WRONG_OPCODE_RESPONSE)
  {
    //This happens if the baud rate is correct but the module is doing a ccontinuous read
    nano.stopReading();

    SERIAL1.println(F("Module continuously reading. Asking it to stop..."));

    delay(1500);
  }
  else
  {
    //The module did not respond so assume it's just been powered on and communicating at 115200bps
    SERIAL2.begin(115200); //Start software serial at 115200

    nano.setBaud(baudRate); //Tell the module to go to the chosen baud rate. Ignore the response msg

    SERIAL2.begin(baudRate); //Start the software serial port, this time at user's chosen baud rate

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
