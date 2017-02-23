/*void sleepLoRa(void){ // LoRaを一定時間省電力モードに移行させる
  
}

void wakeupLoRa(void){ // LoRaが省電力モードから復帰しているか確認
  initLoRa();
}
*/
int detect_poweron(char *data){ //　管理者からのパワーオン指令をチェックする関数だが今のところ未実装 戻り値は指令があれば1、なければ0

  return 0;
}
void _resetLoRa(void) {
  pinMode(PIN_RESET, OUTPUT);
  digitalWrite(PIN_RESET, LOW);
  delay(LORA_RESET_WAIT);
  digitalWrite(PIN_RESET, HIGH);
  delay(LORA_RESET_WAIT);
}

void initLoRa(void) {
  _resetLoRa();
  loraSerial.begin(57600); delay(LORA_INIT_WAIT);
  loraSerial.write("radio set pwr 15\r\n"); delay(LORA_INIT_WAIT); // RN2903:pwr 20
  loraSerial.write("radio set bw 125\r\n"); delay(LORA_INIT_WAIT);
  loraSerial.write("radio set rxbw 250\r\n"); delay(LORA_INIT_WAIT);
  loraSerial.write("radio set afcbw 250\r\n"); delay(LORA_INIT_WAIT);
  loraSerial.write("radio set freq 863000000\r\n"); delay(LORA_INIT_WAIT); // RN2903:923300000
  loraSerial.write("radio set fdev 25000\r\n"); delay(LORA_INIT_WAIT);
  loraSerial.write("radio set bitrate 50000\r\n"); delay(LORA_INIT_WAIT);
  loraSerial.write("radio set prlen 8\r\n"); delay(LORA_INIT_WAIT);
  loraSerial.write("radio set bt 0.5\r\n"); delay(LORA_INIT_WAIT);
  loraSerial.write("radio set crc on\r\n"); delay(LORA_INIT_WAIT);
  loraSerial.write("radio set cr 4/5\r\n"); delay(LORA_INIT_WAIT);
  loraSerial.write("radio set mod lora\r\n"); delay(LORA_INIT_WAIT);
  loraSerial.write("radio set iqi off\r\n"); delay(LORA_INIT_WAIT);
  loraSerial.write("radio set wdt 0\r\n"); delay(LORA_INIT_WAIT); // Default:15000
  loraSerial.write("radio set sf sf12\r\n"); delay(LORA_INIT_WAIT);
  loraSerial.write("radio set sync 34\r\n"); delay(LORA_INIT_WAIT);
}

void readLoRa(void) {
  while(loraSerial.available()) Serial.write(loraSerial.read());
}

void flushLoRa(void) {
  while(loraSerial.available()) loraSerial.read();
}

void sendLoRa(char *data) {
  char buf[LORA_SEND_BUF];
  sprintf(buf, "radio tx %s\r\n", data);
  loraSerial.write("mac pause\r\n"); delay(200);
  loraSerial.write(buf); delay(200);
  delay(3000);
}



void recvLoRa(char *data, unsigned long timeout) {
  char buf[LORA_RECV_BUF];
  char *p = NULL;
  unsigned char i, n = 0;
  unsigned long start = millis();

  loraSerial.write("mac pause\r\n"); delay(200);
  loraSerial.write("radio rx 0\r\n"); delay(200);
  
  while(true) {
    // タイムアウト機構 (1000ms以上で作動)
    if(timeout > 1000 && millis() - start > timeout) {
      strcpy(data, "ERR");
      return;
    }

    // 受信待機
    if(loraSerial.available() < 1) continue;

    // データ読み出し
    buf[n] = loraSerial.read();
    if(++n > LORA_RECV_BUF) n = 0; // スタックオーバーフロー対策

    // 1行を読み出し終えた場合
    if(buf[n-1] == '\n') {
      buf[n] = '\0';
      if(strstr(buf, "radio_rx") > 0) {
        p = strtok(buf, " ");
        p = strtok(NULL, " ");
        strcpy(data, p);
        return;
      }
      if(strstr(buf, "radio_err") > 0 || strstr(buf, "busy") > 0) {
        loraSerial.write("mac pause\r\n"); delay(200);
        loraSerial.write("radio rx 0\r\n"); delay(200);
      }
      n = 0;
    }
  }
  return;
}

Packet *parseLoRa(char *data) {
  Packet *p = NULL;
  if(strlen(data) < 6) return p;
  p = (Packet *)malloc(sizeof(Packet));
  p->dst = 0;
  p->dst |= (htoi(data[0]) << 4) & 0xF0;
  p->dst |= (htoi(data[1])) & 0x0F;
  p->src = 0;
  p->src |= (htoi(data[2]) << 4) & 0xF0;
  p->src |= (htoi(data[3])) & 0x0F;
  p->type = 0;
  p->type |= (htoi(data[4])) & 0x0F;
  strcpy(p->payload, &data[6]);
  return p;
}
