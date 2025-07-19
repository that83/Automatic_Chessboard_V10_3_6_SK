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

// Đo ngưỡng: 947 - nguong 1850 -  [1930 - nguong 2050 - 3070

#include "HC4067.h"

const byte S0 = 19;
const byte S1 = 18;
const byte S2 = 5;
const byte S3 = 17;
const int Mux_Out = 34; // GPIO36 = VP = ADC1_CH0
const byte EN = 22;     // Chân EN của MUX
// Ngưỡng cho từng cảm biến Hall (từ phải sang trái - kênh 7 đến 0)
const int hall_thresholds[8][2] = {
  {1950, 1930},  // Kênh 7: 1930-1950 (giá trị trung bình ~1940)
  {1940, 1920},  // Kênh 6: 1920-1940 (giá trị trung bình ~1935)
  {2040, 2020},  // Kênh 5: 2020-2040 (giá trị trung bình ~2030)
  {1890, 1870},  // Kênh 4: 1870-1890 (giá trị trung bình ~1880)
  {1990, 1970},  // Kênh 3: 1970-1990 (giá trị trung bình ~1980)
  {1980, 1960},  // Kênh 2: 1960-1980 (giá trị trung bình ~1970)
  {1950, 1930},  // Kênh 1: 1930-1950 (giá trị trung bình ~1940)
  {1960, 1940}   // Kênh 0: 1940-1960 (giá trị trung bình ~1950)
};
HC4067 mux1(S0, S1, S2, S3, EN); // Truyền EN vào hàm khởi tạo

void setup() {
  Serial.begin(115200);
}

void loop() {
  print_result();
  delay(1000);

}

int read_hall_sensor(int column) {
  mux1.enable();           // Bật MUX
  mux1.setChannel(column);
  delay(5);
  int hallMeasure = analogRead(Mux_Out);
  mux1.disable();          // Tắt MUX
  delay(100);
  //Serial.print(hallMeasure);
  //Serial.print("|");
  
  // Sử dụng ngưỡng riêng cho từng cảm biến
  int upper_threshold = hall_thresholds[column][0];
  int lower_threshold = hall_thresholds[column][1];
  
  if (hallMeasure > upper_threshold || hallMeasure < lower_threshold) {
    return 1;  // Có quân cờ
  } else {
    return 0;  // Không có quân cờ
  }
}

void print_result() {
  for (int i = 7; i >= 0; i--) {
    int result = read_hall_sensor(i);
    if (result == 1) {
      Serial.print("O|");
    } else {
      Serial.print(" |");
    }

  }
  Serial.println();
}