#include <SoftwareSerial.h> // ver1.0.0
#include <TimeLib.h>

#define FSID            0x02 // FieldServer ID
#define PIN_RESET       13
#define LORA_RESET_WAIT 100
#define LORA_INIT_WAIT  500
#define LORA_SEND_BUF   70 // 64
#define LORA_RECV_BUF   70 // 64

#define GATE_PIN 10 // センサ回路へ電源供給するFETのゲートをこのポートに接続　
#define START_MINUTE 0 // 開始する分
int work = 0; // 処理実行フラグ
time_t start_m; // 処理開始分
TimeElements te;            // holds the time elements for setting the time

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
  pinMode(GATE_PIN, OUTPUT);
  digitalWrite(GATE_PIN, LOW);

  initLoRa();
  // readLoRa();
}

void loop() {
  //ADCSRA &= ~(1<<ADEN); //アナログ変換回路OFF？　未検証
  Serial.println(" in main loop");
  char data[LORA_RECV_BUF];
  Packet *packet = NULL;

  /* ************************************************** */
  /* 管理者からのパワーオン指令を受け取る受信処理 */
  /* ************************************************** */
  recvLoRa(data, 10000); readLoRa();
  // パケット解析
  packet = parseLoRa(data);
  Serial.print("after if(packet == NULL)");
  Serial.println(packet->payload);

  if (packet == NULL) return;

  Serial.println("--------- Packet Info ---------");
  Serial.print("["); Serial.print(packet->type); Serial.print("] "); Serial.print(packet->dst); Serial.print(" <- "); Serial.println(packet->src);
  Serial.print("# "); Serial.println(packet->payload);
  if (packet->dst != FSID) {
    free(packet);
    return;
  }
  work = detect_poweron(packet->src); //　管理者からのパワーオン指令をチェックする関数だが今のところ引数が適当 戻り値は指令があれば1、なければ0
  free(packet);



  breakTime(now(), te);
  if (te.Second == 0 &&  te.Minute == START_MINUTE || work == 1) { // 現時刻の秒が0sかつ開始する分の場合
    start_m = now();
    work = 0;
  }


  Serial.println(te.Minute);

  /* ************************************************** */
  /* 十分間パワーオン時処理 */
  /* パワーオン時に管理者からのパワーオン指令が届いても無視するものとする*/

  while ( now() - start_m <= 600) { // とりま動作時間 60s × 10
    if (now() - start_m < 1) { // LoRa busy防止のためパワーオンの最初の瞬間だけ初期化
      initLoRa();
      readLoRa();
    }

    /* ************************************************** */
    /* とりま受信処理 */
    /* ************************************************** */
    recvLoRa(data, 0); readLoRa();
    // パケット解析
    packet = parseLoRa(data);
    if (packet == NULL) return;

    Serial.println("--------- Packet Info ---------");
    Serial.print("["); Serial.print(packet->type); Serial.print("] "); Serial.print(packet->dst); Serial.print(" <- "); Serial.println(packet->src);
    Serial.print("# "); Serial.println(packet->payload);
    if (packet->dst != FSID) {
      free(packet);
      return;
    }
    free(packet);

    /* ************************************************** */
    /* 送信? */
    /* ************************************************** */
    //  Serial.print("Send:"); Serial.println(send_data);
    //  sendLoRa(send_data); readLoRa();


    digitalWrite(GATE_PIN, HIGH); // センサ回路へ電源供給するFETのスイッチング動作ON!
  }

  /* 十分間パワーオン時処理 終了 */
  /* ************************************************** */

  digitalWrite(GATE_PIN, LOW); // センサ回路へ電源供給するFETのスイッチング動作OFF
}
