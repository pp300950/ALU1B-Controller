import os
import subprocess
import google.generativeai as genai
import time
import re

C_EXECUTABLE_PATH = r"C:\Users\Administrator\Desktop\ALU4B-Controller\my_alu_simulator.exe" 

TEMP_CODE_FILE = "temp_code.txt"

# --- คำสั่งสำหรับสอน Gemini ---
SYSTEM_PROMPT = """
You are an expert programmer. Your task is to convert a user's natural language request into a simple high-level C-like language that my custom C program can parse and execute.

My program's language has the following features and strict rules:

**Language Features:**
- **Variable Declaration:** `int variable_name;`
  - All variables must be of type `int`.
- **Assignment:** `variable_name = another_variable + 5;`
  - Standard arithmetic operators `+`, `-`, `*`, `/` are supported.
  - The right side can be a number, another variable, or a simple expression of two operands.
- **For Loops:** `for(int i = 1; i <= 10; i++) { ... }`
  - The loop structure is standard.
- **Printing:** `print("any message", variable, "\\n");`
  - The `print` function can take multiple arguments, including string literals (in double quotes) and variables.
  - Use `\\n` for a new line.
- **Code Blocks:** `{` and `}` are used for code blocks inside loops.
- **Statements:** All statements must end with a semicolon `;`.

**--- CRITICAL RULES ---**
1.  **SEPARATE DECLARATION AND ASSIGNMENT:** You MUST declare a variable on its own line *before* assigning a value to it.
2.  NEVER combine declaration with an assignment or calculation.
    - **Correct:**
      ```
      int result;
      result = 5 * 8;
      ```
    - **INCORRECT AND INVALID:** `int result = 5 * 8;`
3.  ONLY output the raw code. Do not include any explanations, comments, or markdown formatting like ```.

**--- Examples ---**

**User:** "คำนวณ 5 บวก 15 แล้วแสดงผล"
**You:**
int a = 5;
int b = 15;
int sum;
sum = a + b;
print("The sum is: ", sum, "\\n");

**User:** "แสดงตัวเลขตั้งแต่ 1 ถึง 5"
**You:**
int i;
for(i = 1; i <= 5; i++) {
    print(i, "\\n");
}

**User:** "แสดงสูตรคูณแม่ 9 ตั้งแต่ 9*1 ถึง 9*5"
**You:**
int i;
int result;
for(i=1; i<=5; i++) {
    result = 9 * i;
    print("9 x ", i, " = ", result, "\\n");
}
"""

def get_gemini_code(user_prompt):
   
    api_key = os.getenv("GOOGLE_API_KEY")
    if not api_key:
        print("!!! ERROR: ไม่พบ GOOGLE_API_KEY")
        return None
    
    genai.configure(api_key=api_key)
    
    # เลือกโมเดล
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
        print(f"!!! ERROR: โปรแกรม C ทำงานผิดพลาดและส่งคืนค่า error: {e}")
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

