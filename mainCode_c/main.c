#include "serial_comm.h"
#include "alu_control.h"
#include <stdio.h>


int main() {
    if (!serial_init("COM3")) {
        printf("Failed to open serial port\n");
        return 1;
    }

    // ตัวอย่างส่งคำสั่งผ่าน ALU control
    if (!alu_send_command("0110 1 0\n")) {
        printf("Failed to send command to ALU\n");
    }

    serial_close();
    return 0;
}
