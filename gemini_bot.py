import os
import subprocess
import google.generativeai as genai
import time
import re

C_EXECUTABLE_PATH = r"C:\Users\Administrator\Desktop\ALU4B-Controller\my_alu_simulator.exe" 

TEMP_CODE_FILE = "temp_code.txt"

# --- คำสั่งสำหรับสอน Gemini ---
SYSTEM_PROMPT = """
You are an expert programmer writing code for a custom-built C-like language simulator. Your goal is to translate user requests into functional code for this environment.

**--- Performance & Complexity Limits (EXTREMELY IMPORTANT) ---**
- The simulator can handle a generous number of assembly instructions, up to around **2048** in total. You can now generate more complex logic, including nested loops.
- Always prefer the simplest, most direct logic that fulfills the user's request. Simplicity and correctness are key.

**--- Supported Language Features ---**
- **Variable Declaration:** `int variable_name;` (Only `int` type is supported)
- **Assignment:** `variable = another_variable + 5 * 2;` (Supports `+`, `-`, `*`, `/`) 
- **Don't support:**  `%`, `++`, `--`
- **Printing:** `print("Message: ", variable, "\\n");` (Can take multiple arguments)
- **For Loops:** `for(i = 1; i <= 10; i = i + 1) { ... }` (The update statement is flexible)
- **If Statements:** `if (a > b) { ... }` (Supports `==`, `!=`, `<`, `<=`, `>`, `>=`)
- **Code Blocks:** `{` and `}` for `for` and `if`.
- **Statements:** All statements must end with a semicolon `;`.

**--- Unsupported Features (NEVER USE) ---**
- **DO NOT USE:** `while` loops, functions, arrays, `switch`, `break`, `continue`, `else if`, `else`.

**--- CRITICAL RULES & GUIDELINES ---**
1.  **SEPARATE DECLARATION AND ASSIGNMENT:** You MUST declare a variable on its own line *before* assigning a value.
    - **Correct:** `int x; x = 10;`
    - **INCORRECT:** `int x = 10;`
2.  **ONLY output the raw code.** Do not include any explanations, comments, or markdown formatting like ```.

**--- Examples ---**

**User:** "แสดงสูตรคูณแม่ 7"
**You:**
int i;
int result;
for(i = 1; i <= 12; i = i + 1) {
    result = 7 * i;
    print("7 x ", i, " = ", result, "\\n");
}

**User:** "8+5"
**You:**
int a;
int b;
int result;
a = 5;
b = 5;
result = a + b;
print("Result: ", result, "\\n");

**User:** "หาจำนวนเฉพาะตั้งแต่ 2 ถึง 30"
**You:**
int i;
int j;
int k;
int is_prime;
for (i = 2; i <= 30; i = i + 1) {
is_prime = 1;
for (j = 2; j < i; j = j + 1) {
k = i;
for( ; k >= j; ) {
k = k - j;
}
if (k == 0) {
is_prime = 0;
}
}
if (is_prime == 1) {
print(i, " is a prime number.\n");
}
}

**User:** "หา หรม. ของ 24กับ36"
**You:**
int A = 48;
int B = 18;
int i;
int gcd = 1;

int minVal;
if (A < B) {
    minVal = A;
} else {
    minVal = B;
}

for(i = minVal; i >= 1; i--) {
    int q1 = A / i;
    int r1 = A - q1 * i;

    int q2 = B / i;
    int r2 = B - q2 * i;

    if (r1 == 0 && r2 == 0) {
        gcd = i;
        break;
    }
}

print(gcd);



**User:** "5-2+1"
**You:**
int a;
int b;
int c;
int result;
a = 5;
b = 2;
c = 1;
result = a - b;
result = result + c;
print("Result: ", result, "\n");

"""

def get_gemini_code(user_prompt):
   
    api_key = os.getenv("GOOGLE_API_KEY")
    if not api_key:
        print("!!! ERROR: ไม่พบ GOOGLE_API_KEY")
        return None
    
    genai.configure(api_key=api_key)
    
    # เลือกโมเดล
    #model = genai.GenerativeModel('gemini-1.5-flash-8b')
    model = genai.GenerativeModel('gemini-1.5-flash-latest')
    
    full_prompt = f"{SYSTEM_PROMPT}\nUser: \"{user_prompt}\"\nYou:"
    
    max_retries = 5
    retry_count = 0
    base_wait_time = 5
    
    while retry_count < max_retries:
        try:
            print("... Gemini กำลังคิด ...")
            response = model.generate_content(full_prompt)
            generated_code = response.text.strip()
            return generated_code
        
        except Exception as e:
            error_message = str(e)
            
            if "429 You exceeded your current quota" in error_message:
                print(f"เกิดข้อผิดพลาดในการเรียก Gemini API: {error_message}")
                
                match = re.search(r"seconds: (\d+)", error_message)
                if match:
                    wait_time = int(match.group(1))
                else:
                    wait_time = base_wait_time * (2 ** retry_count)
                
                print(f"โควต้าเกิน! กำลังรอ {wait_time} วินาทีก่อนลองใหม่อีกครั้ง ({retry_count + 1}/{max_retries})")
                time.sleep(wait_time)
                retry_count += 1
            else:
                print(f"เกิดข้อผิดพลาดในการเรียก Gemini API: {error_message}")
                return None

    print(f"!!! ERROR: ลองส่งคำขอ {max_retries} ครั้งแล้วแต่ยังไม่สำเร็จ โปรดลองอีกครั้งในภายหลัง")
    return None

def run_c_program(code_filepath):
    """รันโปรแกรม C โดยส่ง path ของไฟล์โค้ดไปเป็น argument"""
    try:
        print(f"\n--- กำลังรันโปรแกรม C ({C_EXECUTABLE_PATH}) ---")
        print(f"คำสั่งที่จะรัน: {C_EXECUTABLE_PATH} {code_filepath}")

        # ใช้ subprocess.run ซึ่งจัดการ argument ได้ดีกว่า
        subprocess.run([C_EXECUTABLE_PATH, code_filepath], check=True)

    except FileNotFoundError:
        print(f"!!! ERROR: ไม่พบไฟล์ '{C_EXECUTABLE_PATH}'. กรุณาตรวจสอบว่าคอมไพล์โค้ด C แล้ว")
    except subprocess.CalledProcessError as e:
        print(f"!!! ERROr: ฝั่ง c โง่ๆเเจ้งerr : {e}")
    except Exception as e:
        print(f"เกิดข้อผิดพลาดที่ไม่คาดคิดขณะรันโปรแกรม C: {e}")

def main():
    """ฟังก์ชันหลักของบอท"""
    print("--- Gemini ALU Bot ---")
    print("ป้อนคำสั่งของคุณ (หรือพิมพ์ 'exit' เพื่อออก)")
    
    while True:
        user_input = input("\nคุณ: ")
        if user_input.lower() == 'exit':
            print("กำลังออกจากโปรแกรม...")
            break
            
        code = get_gemini_code(user_input)
        
        if code:
            print("\n--- โค้ดที่ Gemini สร้าง ---")
            print(code)
            print("--------------------------")
            
            try:
                with open(TEMP_CODE_FILE, "w", encoding="utf-8") as f:
                    f.write(code)
            except Exception as e:
                print(f"!!! ERROR: ไม่สามารถเขียนไฟล์ชั่วคราวได้: {e}")
                continue

            run_c_program(TEMP_CODE_FILE)
            
if __name__ == "__main__":
    main()

