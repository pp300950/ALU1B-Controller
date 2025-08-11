// ===================================================================================
// Arduino 1-Bit ALU Controller (แก้ไขเพื่อส่งค่ากลับให้โปรแกรม C)
// ===================================================================================

// --- ขาควบคุมและอินพุต ---
const int MUX_PIN_0 = 2;       // ขาควบคุม MUX บิตที่ 0
const int MUX_PIN_1 = 3;       // ขาควบคุม MUX บิตที่ 1
const int MUX_PIN_2 = 4;       // ขาควบคุม MUX บิตที่ 2
const int SUB_ADD_PIN = 5;     // ขาควบคุมโหมดบวก/ลบ (0=บวก, 1=ลบ)
const int A_INPUT_PIN = 6;       // อินพุต A
const int B_INPUT_PIN = 7;       // อินพุต B

// --- ขาเอาต์พุต ---
const int RESULT_OUTPUT_PIN = 9;   // ขาเอาต์พุตแสดงผลลัพธ์ทั่วไปจาก MUX
const int CARRY_OUT_PIN = 11;    // ขาเอาต์พุตสำหรับค่าทด (Carry)

void setup() {
  // ตั้งค่าขาควบคุมและอินพุตของ ALU เป็น OUTPUT
  pinMode(MUX_PIN_0, OUTPUT);
  pinMode(MUX_PIN_1, OUTPUT);
  pinMode(MUX_PIN_2, OUTPUT);
  pinMode(SUB_ADD_PIN, OUTPUT);
  pinMode(A_INPUT_PIN, OUTPUT);
  pinMode(B_INPUT_PIN, OUTPUT);

  // ตั้งค่าขาเอาต์พุตของ ALU เป็น INPUT
  pinMode(RESULT_OUTPUT_PIN, INPUT);
  pinMode(CARRY_OUT_PIN, INPUT);
  
  // เริ่มต้นการสื่อสาร Serial
  Serial.begin(9600);
}

void loop() {
  // ตรวจสอบว่ามีข้อมูลเข้ามาใน Serial หรือไม่
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n'); // อ่านข้อมูลเข้ามาทีละบรรทัด
    input.trim(); // ลบช่องว่างหน้า-หลัง

    // แยกข้อมูลด้วยช่องว่าง: MuxCode Sub/Add A B
    char muxCodeStr[4];
    int subAddPin, aInput, bInput;

    // ใช้ sscanf เพื่อแยกข้อมูลอย่างปลอดภัย
    int items = sscanf(input.c_str(), "%3s %d %d %d", muxCodeStr, &subAddPin, &aInput, &bInput);

    if (items == 4) {
      // แปลงค่า MuxCode string เป็น int สำหรับแต่ละบิต
      int muxBit2 = muxCodeStr[0] - '0';
      int muxBit1 = muxCodeStr[1] - '0';
      int muxBit0 = muxCodeStr[2] - '0';

      // --- ควบคุม ALU ---
      digitalWrite(MUX_PIN_2, muxBit2);
      digitalWrite(MUX_PIN_1, muxBit1);
      digitalWrite(MUX_PIN_0, muxBit0);
      digitalWrite(SUB_ADD_PIN, subAddPin);
      digitalWrite(A_INPUT_PIN, aInput);
      digitalWrite(B_INPUT_PIN, bInput);

      // หน่วงเวลาเล็กน้อยเพื่อให้วงจรทำงาน
      delay(10); 

      // --- อ่านผลลัพธ์ ---
      int resultOutput = digitalRead(RESULT_OUTPUT_PIN);
      int carryOutput = digitalRead(CARRY_OUT_PIN);
      
      // --- ส่งผลลัพธ์กลับไปให้โปรแกรม C ---
      // รูปแบบ: "Result_Bit Carry_Bit\n" เช่น "1 0\n"
      Serial.print(resultOutput);
      Serial.print(" ");
      Serial.println(carryOutput);

    }
  }
}