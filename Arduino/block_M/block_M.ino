//RFIDを使ったブロック玩具のテスト

#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <MQTTClient.h>

#define RST_PIN 16 // MFRC522用リセットピンGPIO NO
#define SS_PIN  5  // MFRC522用SS(SDA)ピンGPIO NO

MFRC522 mfrc522(SS_PIN, RST_PIN); // MFRC522のインスタンスを作成

#define CARD_PRESENT_PIN 4 // MFRC522にカードが検知されたら光らせるGPIO NO
bool cardset;     // MFRC522にカードが検知されているかどうか
int timeoutcount; // MFRC522からカードが連続で離れた回数を記録

const char* ssid = "SPWH_H32_D15FE1";
const char* password = "5b2dhen4ag37q75";
//const char* ssid = "W01_B8BC1BEED935";
//const char* password = "6hd63qer60qen4h";

WiFiClient net;
MQTTClient client;
unsigned long lastMillis = 0;
void connect();  // <- predefine connect() for setup()

#define START_R "04 29 9C D2 C7 55 80"
#define END_R "04 88 94 D2 C7 55 81"
#define UDID0 "04 79 94 D2 C7 55 81"
#define UDID1 "04 6A 94 D2 C7 55 81"
#define UDID2 "04 4C 94 D2 C7 55 81"
#define UDID3 "04 5B 94 D2 C7 55 81"

String udid[4] = {UDID0,UDID1,UDID2,UDID3};
String lastChar;
String moji[] = {"り","ん","ご","に"};
int countChar= 0;
bool readFlag;
String subString;

void setup() {
  // UART接続初期化
  Serial.begin(9600); // UARTの通信速度を変更したい場合はこちらを変更
  delay(10);

  // カード検出用のGPIO初期化
  pinMode(CARD_PRESENT_PIN, OUTPUT);
  digitalWrite(CARD_PRESENT_PIN, LOW);

  // MFRC522用変数初期化
  cardset = false;
  timeoutcount = 0;

  SPI.begin();          // SPI初期化
  mfrc522.PCD_Init();   // MFRC522初期化
  ShowReaderDetails();  // MFRC522 Card Readerのバージョンを返す。00かFFなら配線間違いの可能性

  // Conect Wifi.
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Print the IP address
  Serial.println(WiFi.localIP());

  client.begin("broker.shiftr.io", net);
  connect();
 
}

void loop() {
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  String strBuf[mfrc522.uid.size];
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      strBuf[i] =  String(mfrc522.uid.uidByte[i], HEX);  // (E)using a constant integer
      if(strBuf[i].length() == 1){  // 1桁の場合は先頭に0を追加
        strBuf[i] = "0" + strBuf[i];
      }
    }

    String strUID = strBuf[0] + " " + strBuf[1] + " " + strBuf[2] + " " + strBuf[3] + " " + strBuf[4] + " " + strBuf[5] + " " + strBuf[6];

    if ( strUID.equalsIgnoreCase(START_R) && readFlag == false){
      Serial.println("Start");
      readFlag = true;
    }
    
    
    if ( strUID.equalsIgnoreCase(END_R) && readFlag == true ){
      Serial.println(subString);
      if(!client.connect("arduino", "0cce1b17", "43365f8d45ed2a89")){
        connect();
        }
      
      if(client.publish("/word", subString) == true){
        Serial.println("Published");
        }
      
      Serial.println("End");
      countChar = 0;
      subString = "";
      lastChar = "";
      readFlag = false;
    }

    if(readFlag == true){
        for(int j=0; j<4 ; j++){
          if(strUID.equalsIgnoreCase(udid[j])){
              if(strUID != lastChar){
                subString = subString + moji[j];
                countChar ++;
                lastChar = strUID;
              }
            }
          }
      }
   
  delay(100);
  
}

void ShowReaderDetails() {
  // Get the MFRC522 software version
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print(F("MFRC522 Software Version: 0x"));
  Serial.print(v, HEX);
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else
    Serial.print(F(" (unknown)"));
  Serial.println("");
  // When 0x00 or 0xFF is returned, communication probably failed
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
  }
}


