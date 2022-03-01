#include <M5StickC.h>
#include <Wire.h>
#include "Ambient.h"

#define uS_TO_S_FACTOR 1000000    // マイクロ秒から秒への変換係数
#define TIME_TO_SLEEP 600        // ESP32がスリープ状態になる時間（秒単位

//関数プロトタイプ宣言
void wifi_conect(void);
void ambient_access(void);
void lcd_display(void);

WiFiClient client;
Ambient ambient;

const char* ssid = "HUAWEI-F4E1";               //wifiのID
const char* password = "64141340";              //wifiのパスワード

unsigned int channelId = 38700;             //AmbientのチャネルID
const char* writeKey = "a35923f4e0ec9168";  // ライトキー

int sensorPin = 33;
uint16_t soilm = 0;       //土壌水分
uint16_t readData = 0;
RTC_DATA_ATTR uint16_t hAveSoilm = 0;                //1時間平均気温変数
RTC_DATA_ATTR char flg = 5;     //1時間経過フラグ

void setup() 
{
    M5.begin();
    M5.Axp.ScreenBreath(10);                        //画面の輝度を少し下げる
    M5.Lcd.setRotation(3);                          //画面を("x*90°")回転させる
    Wire.begin();                                   //I2Cを初期化する
}


void loop() 
{
    readData = analogRead(sensorPin);
    soilm = readData;
    
    //1時間平均土壌水分計算
    if(hAveSoilm != 0)
    {
      hAveSoilm = (hAveSoilm+soilm)/ 2;
    }
    else
    {
      hAveSoilm = soilm;
    }
    
    if(flg == 5)
    {
        wifi_conect();      //wifi接続関数
        ambientup();        //アンビエントアクセス関数
        WiFi.disconnect();  //wifiから切断
        hAveSoilm = 0; 
        flg = 0;   
    }
    else
    {
      flg++;
    }
    
    esp_deep_sleep(TIME_TO_SLEEP * uS_TO_S_FACTOR ); //deepsleepに移行し、引数μ秒後に復帰する
}

//wifi接続関数
void wifi_conect(void)
{
    WiFi.begin(ssid, password);               //  Wi-Fi APに接続 
    while (WiFi.status() != WL_CONNECTED)     //  Wi-Fi AP接続待ち
    {
        delay(500);
        Serial.print(".");
    }
    Serial.print("WiFi connected\r\nIP address: ");
    Serial.println(WiFi.localIP());
}

 //ambientにデータアップロード
void ambientup()
{
    ambient.begin(channelId, writeKey, &client); // チャネルIDとライトキーを指定してAmbientの初期化

    //ambientに送るデータをセット
    ambient.set(1, soilm);
    ambient.set(1, hAveSoilm);

    //ambientにデータを送信
    ambient.send(); 

}
