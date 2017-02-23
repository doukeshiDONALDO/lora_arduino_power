#include <SoftwareSerial.h> // ver1.0.0
#include <TimeLib.h>

#define FSID            0x02 // FieldServer ID
#define PIN_RESET       13
#define LORA_RESET_WAIT 100
#define LORA_INIT_WAIT  200
#define LORA_SEND_BUF   70 // 64
#define LORA_RECV_BUF   70 // 64

#define GATE_PIN 10 // センサ回路へ電源供給するFETのゲートをこのポートに接続　
#define START_MINUTE 0 // 開始する分
int work = 1; // 処理実行フラグ

SoftwareSerial loraSerial(11, 12); // LoRa通信用

typedef struct _Packet {
  byte dst;
  byte src;
  byte type;
  char payload[64];
} Packet;

void setup() {
  Serial.begin(9600);
  Serial.println("---------- Arduino start ----------");

  setTime(0, 0, 0, 1, 1, 2017); // hours, minutes, seconds, days, months, years
  pinMode(GATE_PIN,OUTPUT);
  digitalWrite(GATE_PIN, LOW);

  initLoRa();
  readLoRa();
}

void loop() {
  //ADCSRA &= ~(1<<ADEN); //アナログ変換回路OFF？　未検証

  char data[LORA_RECV_BUF];
  char send_data[LORA_SEND_BUF] = "010220000000000000000000000000";
  Packet *packet = NULL;

  while (minute() >= START_MINUTE && minute() <= START_MINUTE + 10 && work) {
    wakeupLoRa();
    Serial.println("Wakeup LoRa");
    
    /* ************************************************** */
    /* 受信 */
    /* ************************************************** */
     recvLoRa(data, 0); readLoRa();
    // パケット解析
      packet = parseLoRa(data);
      if(packet == NULL) return;

      Serial.println("--------- Packet Info ---------");
      Serial.print("["); Serial.print(packet->type); Serial.print("] "); Serial.print(packet->dst); Serial.print(" <- "); Serial.println(packet->src);
      Serial.print("# "); Serial.println(packet->payload);
      if(packet->dst != FSID) {
        free(packet);
        return;
      }
      free(packet);

    /* ************************************************** */
    /* 送信 */
    /* ************************************************** */
    //  Serial.print("Send:"); Serial.println(send_data);
    //  sendLoRa(send_data); readLoRa();

    digitalWrite(GATE_PIN, LOW);

    work = 0;
  }

  if(minute() > START_MINUTE)
}
