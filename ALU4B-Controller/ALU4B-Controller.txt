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

// Setup function
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
  Serial.println("Enter command in format: [MuxCode][SubAddPin] [A] [B]");
  Serial.println("Example: 0010 1 0");
  Serial.println("MuxCode: 000=ว่าง, 001=S1(บวก), 010=C1(ค่าทด), 011=NOT(A), 100=AND, 101=XOR, 110=OR");
  Serial.println("SubAddPin: 0=บวก, 1=ลบ");
  Serial.println("A, B: 0 or 1");
}

// Loop function
void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim(); // ลบช่องว่างหน้าหลัง
    
    // แยกคำสั่ง
    String muxCodeStr = input.substring(0, 3);
    String subAddPinStr = input.substring(3, 4);
    String aInputStr = input.substring(5, 6);
    String bInputStr = input.substring(7, 8);
    
    // แปลงค่าจาก String เป็น Integer
    int muxCode = strtol(muxCodeStr.c_str(), NULL, 2);
    int subAddPin = subAddPinStr.toInt();
    int aInput = aInputStr.toInt();
    int bInput = bInputStr.toInt();

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
    
    // แสดงผลลัพธ์ใน Serial Monitor
    Serial.println("----------------------------------------");
    Serial.print("Input MuxCode: "); Serial.println(muxCodeStr);
    Serial.print("Input Sub/Add Pin: "); Serial.println(subAddPinStr);
    Serial.print("Input A: "); Serial.println(aInput);
    Serial.print("Input B: "); Serial.println(bInput);
    Serial.println("---");
    Serial.print("Digital Output (Result): "); Serial.println(resultOutput);
    Serial.print("Digital Output (Negative Flag): "); Serial.println(negativeOutput);
    Serial.println("----------------------------------------");
  }
}