import os
import google.generativeai as genai

try:
    api_key = os.getenv("GOOGLE_API_KEY")
    if not api_key:
        print("!!! ERROR: ไม่พบ GOOGLE_API_KEY กรุณาตั้งค่า Environment Variable ก่อน")
    else:
        genai.configure(api_key=api_key)
        
        print("--- รายชื่อโมเดลที่ใช้งานได้ ---")
        for m in genai.list_models():
            # กรองเฉพาะโมเดลที่รองรับการสร้างเนื้อหา (generateContent)
            if 'generateContent' in m.supported_generation_methods:
                print(f"ชื่อโมเดล: {m.name}")
                print(f"  คำอธิบาย: {m.description}")
                print(f"  รองรับการสร้างเนื้อหา: ใช่")
                print("-" * 20)

except Exception as e:
    print(f"เกิดข้อผิดพลาด: {e}")