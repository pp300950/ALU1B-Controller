// ===================================================================================
// Arduino 1-Bit ALU Controller (แก้ไขให้ 111 อ่านจากพิน 10 และ invert B ผ่าน A)
// ===================================================================================

// --- ขาควบคุมและอินพุต ---
const int MUX_PIN_0 = 2;       // ขาควบคุม MUX บิตที่ 0
const int MUX_PIN_1 = 3;       // ขาควบคุม MUX บิตที่ 1
const int MUX_PIN_2 = 4;       // ขาควบคุม MUX บิตที่ 2
const int SUB_ADD_PIN = 5;     // ขาควบคุมโหมดบวก/ลบ (0=บวก, 1=ลบ)
const int A_INPUT_PIN = 6;     // อินพุต A
const int B_INPUT_PIN = 7;     // อินพุต B

// --- ขาเอาต์พุต ---
const int RESULT_OUTPUT_PIN = 9;    // ขาเอาต์พุตหลักจาก MUX
const int INVERT_OUTPUT_PIN = 10;   // ขาเอาต์พุตเฉพาะโหมด invert (111)
const int CARRY_OUT_PIN = 11;       // ขาเอาต์พุตค่าทด (Carry)

void setup() {
  pinMode(MUX_PIN_0, OUTPUT);
  pinMode(MUX_PIN_1, OUTPUT);
  pinMode(MUX_PIN_2, OUTPUT);
  pinMode(SUB_ADD_PIN, OUTPUT);
  pinMode(A_INPUT_PIN, OUTPUT);
  pinMode(B_INPUT_PIN, OUTPUT);

  pinMode(RESULT_OUTPUT_PIN, INPUT);
  pinMode(INVERT_OUTPUT_PIN, INPUT);
  pinMode(CARRY_OUT_PIN, INPUT);
  
  Serial.begin(9600);
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    char muxCodeStr[4];
    int subAddPin, aInput, bInput;

    int items = sscanf(input.c_str(), "%3s %d %d %d", muxCodeStr, &subAddPin, &aInput, &bInput);

    if (items == 4) {
      int muxBit2 = muxCodeStr[0] - '0';
      int muxBit1 = muxCodeStr[1] - '0';
      int muxBit0 = muxCodeStr[2] - '0';

      // ปรับให้โหมด invert (111) ใช้ A เป็น B แทน
      if (muxBit2 == 1 && muxBit1 == 1 && muxBit0 == 1) {
        // ย้ายค่า bInput มาใส่ A และ A เดิมไม่ใช้
        digitalWrite(A_INPUT_PIN, bInput);
        digitalWrite(B_INPUT_PIN, 0); // ค่า B ไม่สำคัญ
      } else {
        digitalWrite(A_INPUT_PIN, aInput);
        digitalWrite(B_INPUT_PIN, bInput);
      }

      digitalWrite(MUX_PIN_2, muxBit2);
      digitalWrite(MUX_PIN_1, muxBit1);
      digitalWrite(MUX_PIN_0, muxBit0);
      digitalWrite(SUB_ADD_PIN, subAddPin);

      delay(10);

      int resultOutput;
      if (muxBit2 == 1 && muxBit1 == 1 && muxBit0 == 1) {
        // อ่าน invert จากพิน 10
        resultOutput = digitalRead(INVERT_OUTPUT_PIN);
      } else {
        resultOutput = digitalRead(RESULT_OUTPUT_PIN);
      }

      int carryOutput = digitalRead(CARRY_OUT_PIN);

      Serial.print(resultOutput);
      Serial.print(" ");
      Serial.println(carryOutput);
    }
  }
}
