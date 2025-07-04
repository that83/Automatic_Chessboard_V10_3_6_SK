/*
================ HƯỚNG DẪN NỐI DÂY ESP32 - HC4067 - CẢM BIẾN HALL ================

- S0 (HC4067)  <--- GPIO15 (ESP32)
- S1 (HC4067)  <--- GPIO2  (ESP32)
- S2 (HC4067)  <--- GPIO4  (ESP32)
- S3 (HC4067)  <--- GPIO16 (ESP32)
- SIG (Z) (HC4067) ---> GPIO36 (VP, ADC1_CH0, ESP32)  // Đọc analog
- VCC (HC4067)  <--- 3.3V (ESP32)
- GND (HC4067)  <--- GND (ESP32)
- C0 (HC4067)   <--- Chân tín hiệu cảm biến Hall (1 cảm biến nối vào đây)
- 2 chân còn lại của cảm biến Hall nối 3.3V và GND (tùy loại cảm biến)

Lưu ý:
- Nếu dùng Arduino, thay các chân GPIO bằng A0, A1, ... như hướng dẫn trước.
- Nếu dùng kênh khác, đổi số trong setChannel().
- Đảm bảo cảm biến và IC dùng chung mức điện áp (3.3V hoặc 5V).
===============================================================================
*/
#include "HC4067.h"

const byte S0 = 15;
const byte S1 = 2;
const byte S2 = 4;
const byte S3 = 16;
const int Mux_Out = 36; // GPIO36 = VP = ADC1_CH0

HC4067 mux1(S0, S1, S2, S3, 22); // Chân 22 có thể dùng làm SIG nếu bạn cần, hoặc bỏ qua nếu không dùng

void setup() {
  Serial.begin(115200); // ESP32 thường dùng baud cao hơn
}

void loop() {
  mux1.enable();
  mux1.setChannel(0);
  delay(5);
  int hallMeasure = analogRead(Mux_Out);
  mux1.disable();

  Serial.print("Hall sensor value (mux1, channel 0): ");
  Serial.println(hallMeasure);

  delay(500);
}
