// ขาควบคุม
const int MUX_PIN_0 = 2; // ขาควบคุม MUX บิตที่ 0
const int MUX_PIN_1 = 3; // ขาควบคุม MUX บิตที่ 1
const int MUX_PIN_2 = 4; // ขาควบคุม MUX บิตที่ 2
const int SUB_ADD_PIN = 5; // ขาควบคุมโหมดบวก/ลบ (0=บวก, 1=ลบ)

// ขาอินพุตของ ALU
const int A_INPUT_PIN = 6; // อินพุต A
const int B_INPUT_PIN = 7; // อินพุต B

// ขาเอาต์พุตของ ALU
const int NEGATIVE_OUTPUT_PIN = 8; // ขาเอาต์พุตแสดงค่าติดลบ (1=ติดลบ, 0=ไม่ติดลบ)
const int RESULT_OUTPUT_PIN = 9;   // ขาเอาต์พุตแสดงผลลัพธ์จาก MUX

// --- Setup Function ---
void setup() {
  // ตั้งค่าขาควบคุมและอินพุตของ ALU เป็น OUTPUT
  pinMode(MUX_PIN_0, OUTPUT);
  pinMode(MUX_PIN_1, OUTPUT);
  pinMode(MUX_PIN_2, OUTPUT);
  pinMode(SUB_ADD_PIN, OUTPUT);
  pinMode(A_INPUT_PIN, OUTPUT);
  pinMode(B_INPUT_PIN, OUTPUT);

  // ตั้งค่าขาเอาต์พุตของ ALU เป็น INPUT
  pinMode(NEGATIVE_OUTPUT_PIN, INPUT);
  pinMode(RESULT_OUTPUT_PIN, INPUT);

  // เริ่มต้นการสื่อสาร Serial
  Serial.begin(9600);
  Serial.println("ALU 1-Bit Controller is ready.");
}

// --- Test Function ---
void runTest(const char* testName, int muxCode, int subAddPin, int aInput, int bInput, int expectedResult, int expectedNegative) {
  // ตั้งค่าขาควบคุม MUX
  digitalWrite(MUX_PIN_0, (muxCode & 0b001));
  digitalWrite(MUX_PIN_1, (muxCode & 0b010) >> 1);
  digitalWrite(MUX_PIN_2, (muxCode & 0b100) >> 2);

  // ตั้งค่าขาบวก/ลบ
  digitalWrite(SUB_ADD_PIN, subAddPin);

  // ตั้งค่าขาอินพุต A และ B
  digitalWrite(A_INPUT_PIN, aInput);
  digitalWrite(B_INPUT_PIN, bInput);
  
  // หน่วงเวลาเล็กน้อยเพื่อให้ ALU ประมวลผล
  delay(10);
  
  // อ่านค่าเอาต์พุต
  int resultOutput = digitalRead(RESULT_OUTPUT_PIN);
  int negativeOutput = digitalRead(NEGATIVE_OUTPUT_PIN);
  
  // ตรวจสอบผลลัพธ์และแสดงผลใน Serial Monitor
  Serial.print("Testing ");
  Serial.print(testName);
  Serial.print(" (A="); Serial.print(aInput);
  Serial.print(", B="); Serial.print(bInput);
  Serial.print(")... ");

  bool testPassed = false;
  // เงื่อนไขการตรวจสอบ: ถ้าเป็นโหมด 'ลบ' (subAddPin=1) ให้ตรวจสอบทั้ง Result และ Negative
  if (subAddPin == 1) {
    if (resultOutput == expectedResult && negativeOutput == expectedNegative) {
      testPassed = true;
    }
  } 
  // ถ้าเป็นโหมดอื่น ให้ตรวจสอบแค่ Result เท่านั้น
  else {
    if (resultOutput == expectedResult) {
      testPassed = true;
    }
  }

  if (testPassed) {
    Serial.print("PASS | Got: Result=");
    Serial.print(resultOutput);
    
    if (subAddPin == 1) {
      Serial.print(", Negative=");
      Serial.println(negativeOutput);
    } else {
      Serial.println();
    }
    
  } else {
    Serial.print("FAIL | Got: Result=");
    Serial.print(resultOutput);
    
    // แสดงค่า Negative ที่คาดหวังและที่ได้ เฉพาะในโหมด 'ลบ' เท่านั้น
    if (subAddPin == 1) {
      Serial.print(", Negative=");
      Serial.println(negativeOutput);
    } else {
      Serial.println();
    }
  }
}

// --- Loop Function ---
void loop() {
  Serial.println("--- Starting Automated Test Cases ---");

  // Test Cases for ADD/SUB function
  runTest("ADD (0+0=0)", 0b001, 0, 0, 0, 0, 0); 
  runTest("ADD (0+1=1)", 0b001, 0, 0, 1, 1, 0); 
  runTest("ADD (1+0=1)", 0b001, 0, 1, 0, 1, 0);
  runTest("ADD (1+1=0)", 0b001, 0, 1, 1, 0, 0);

  runTest("SUB (0-0=0)", 0b001, 1, 0, 0, 0, 0);
  runTest("SUB (0-1=1, Negative)", 0b001, 1, 0, 1, 1, 1);
  runTest("SUB (1-0=1)", 0b001, 1, 1, 0, 1, 0);
  runTest("SUB (1-1=0)", 0b001, 1, 1, 1, 0, 0);

  // Test Cases for NOT function
  runTest("NOT (A=0)", 0b011, 0, 0, 0, 1, 0);
  runTest("NOT (A=1)", 0b011, 0, 1, 0, 0, 0);
  
  // Test Cases for AND function
  runTest("AND (0&0=0)", 0b100, 0, 0, 0, 0, 0);
  runTest("AND (0&1=0)", 0b100, 0, 0, 1, 0, 0);
  runTest("AND (1&0=0)", 0b100, 0, 1, 0, 0, 0);
  runTest("AND (1&1=1)", 0b100, 0, 1, 1, 1, 0);

  // Test Cases for XOR function
  runTest("XOR (0^0=0)", 0b101, 0, 0, 0, 0, 0);
  runTest("XOR (0^1=1)", 0b101, 0, 0, 1, 1, 0);
  runTest("XOR (1^0=1)", 0b101, 0, 1, 0, 1, 0);
  runTest("XOR (1^1=0)", 0b101, 0, 1, 1, 0, 0);

  // Test Cases for OR function
  runTest("OR (0|0=0)", 0b000, 0, 0, 0, 0, 0);
  runTest("OR (0|1=1)", 0b000, 0, 0, 1, 1, 0);
  runTest("OR (1|0=1)", 0b000, 0, 1, 0, 1, 0);
  runTest("OR (1|1=1)", 0b000, 0, 1, 1, 1, 0);

  Serial.println("--- All test cases have been completed. ---");
  while(true); // หยุดการทำงาน
}