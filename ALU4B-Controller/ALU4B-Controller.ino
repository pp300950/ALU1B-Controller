
// --- ขาควบคุมและอินพุต ---
const int MUX_PIN_0 = 2;
const int MUX_PIN_1 = 3;
const int MUX_PIN_2 = 4;
const int SUB_ADD_PIN = 5;
const int A_INPUT_PIN = 6;
const int B_INPUT_PIN = 7;

// --- ขาเอาต์พุต ---
const int RESULT_OUTPUT_PIN = 9;
const int INVERT_OUTPUT_PIN = 10;
const int CARRY_OUT_PIN = 11;

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
    int subAddPin, aInput, bInput, carryIn;

    int items = sscanf(input.c_str(), "%3s %d %d %d %d", muxCodeStr, &subAddPin, &aInput, &bInput, &carryIn);

    if (items == 5) {
      int muxBit2 = muxCodeStr[0] - '0';
      int muxBit1 = muxCodeStr[1] - '0';
      int muxBit0 = muxCodeStr[2] - '0';
      
      int resultOutput = 0;
      int carryOutput = 0;

      // ----------------------------------------------------
      // ขั้นตอนที่ 1: ตั้งค่า A, B, และ SUB/ADD
      // ----------------------------------------------------
      digitalWrite(A_INPUT_PIN, aInput);
      digitalWrite(B_INPUT_PIN, bInput);
      digitalWrite(SUB_ADD_PIN, subAddPin);

      // ----------------------------------------------------
      // ขั้นตอนที่ 2: อ่านผลลัพธ์ตาม MUX code
      // ----------------------------------------------------
      digitalWrite(MUX_PIN_2, muxBit2);
      digitalWrite(MUX_PIN_1, muxBit1);
      digitalWrite(MUX_PIN_0, muxBit0);
      delay(10);
      
      // ถ้าเป็น ADD/SUB operation (MUX code 001)
      if (muxBit2 == 0 && muxBit1 == 0 && muxBit0 == 1) {
        // อ่านค่า A XOR B จาก Half Adder
        int halfAdderSum = digitalRead(RESULT_OUTPUT_PIN);
        
        // ✅ คำนวณ Full Adder Sum = (A XOR B) XOR CarryIn
        resultOutput = halfAdderSum ^ carryIn;
        
        // อ่านค่า Carry จาก Half Adder (A AND B)
        digitalWrite(MUX_PIN_2, 0);  // Set MUX to 010 for Carry
        digitalWrite(MUX_PIN_1, 1);
        digitalWrite(MUX_PIN_0, 0);
        delay(10);
        
        int halfAdderCarry = digitalRead(CARRY_OUT_PIN);
        
        // ✅ คำนวณ Full Adder Carry = (A AND B) OR (CarryIn AND (A XOR B))
        carryOutput = halfAdderCarry | (carryIn & halfAdderSum);
      }
      // ถ้าเป็น NOT operation (MUX code 111)
      else if (muxBit2 == 1 && muxBit1 == 1 && muxBit0 == 1) {
        resultOutput = digitalRead(INVERT_OUTPUT_PIN);
        carryOutput = 0; // NOT operation ไม่มี carry
      }
      // ถ้าเป็น Logic operations อื่นๆ (AND, OR, XOR)
      else {
        resultOutput = digitalRead(RESULT_OUTPUT_PIN);
        carryOutput = 0; // Logic operations ไม่มี carry
      }
      
      // ----------------------------------------------------
      // ขั้นตอนที่ 3: ส่งผลลัพธ์กลับ
      // ----------------------------------------------------
      Serial.print(resultOutput);
      Serial.print(" ");
      Serial.println(carryOutput);
    }
  }
}