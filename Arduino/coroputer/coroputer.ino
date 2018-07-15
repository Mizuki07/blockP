//Coroputer
//一文字毎、MQTTにUIDを送信

#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <MQTTClient.h>

#define RST_PIN 16 // MFRC522用リセットピンGPIO NO
#define SS_PIN  5  // MFRC522用SS(SDA)ピンGPIO NO

MFRC522 mfrc522(SS_PIN, RST_PIN); // MFRC522のインスタンスを作成

bool cardset;     // MFRC522にカードが検知されているかどうか
int timeoutcount; // MFRC522からカードが連続で離れた回数を記録

//const char* ssid = "SPWH_H32_D15FE1";
//const char* password = "5b2dhen4ag37q75";

const char* ssid = "SPWN_H36_809AD8";
const char* password = "2aaj4eb805539tj";

WiFiClient net;
MQTTClient client;
unsigned long lastMillis = 0;
void connect();  // <- predefine connect() for setup()

String lastChar;
int countChar= 0;
bool readFlag = true;
String subString;

void setup() {
  // UART接続初期化
  Serial.begin(9600); // UARTの通信速度を変更したい場合はこちらを変更
  delay(10);

  // MFRC522用変数初期化
  cardset = false;
  timeoutcount = 0;

  SPI.begin();          // SPI初期化
  mfrc522.PCD_Init();   // MFRC522初期化
  ShowReaderDetails();  // MFRC522 Card Readerのバージョンを返す。00かFFなら配線間違いの可能性

  // Conect Wifi.
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  client.begin("broker.shiftr.io", net);
  client.onMessage(messageReceived);
  
  connect();
 
}

void loop() {
  client.loop();
  delay(10);
  
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
    Serial.println(strUID);
    Serial.println(lastChar);
    
    if (!client.connected()) {
      connect();  
    }

    if(lastChar != strUID){
      if(client.publish("/word", strUID) == true){
      Serial.println("Published");
      lastChar = strUID;
      }
    }

    if (millis() - lastMillis > 1000) {
      lastMillis = millis();
      client.publish("/hello", "world");
    }
}
