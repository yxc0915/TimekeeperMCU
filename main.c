#include <REG51.H>

#define LED_PORT P2
#define SHUMAGUAN_PORT P0
sbit LSB = P2 ^ 2;
sbit MID = P2 ^ 3;
sbit MSB = P2 ^ 4;
sbit KEY1 = P3 ^ 1; // 开始/暂停
sbit KEY2 = P3 ^ 0; // 清零

// 全局变量
unsigned char hours = 0;
unsigned char minutes = 0;
unsigned char seconds = 0;
unsigned char centiseconds = 0;
bit isRunning = 0;
unsigned int timerCount = 0;

// 数码管数字码表
unsigned char code SEG_CODE[] = { 0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F };

void delay(unsigned int t)
{
    while (t--);
}

// 数码管位选
void setSHUMAGUAN(unsigned char position) {
    LSB = position & 0x01;
    MID = (position >> 1) & 0x01;
    MSB = (position >> 2) & 0x01;
}

// 显示时间
void displayTime() {
    unsigned char digits[8];
    unsigned char i;
    
    digits[0] = centiseconds % 10;
    digits[1] = centiseconds / 10;
    digits[2] = seconds % 10;
    digits[3] = seconds / 10;
    digits[4] = minutes % 10;
    digits[5] = minutes / 10;
    digits[6] = hours % 10;
    digits[7] = hours / 10;

    for (i = 0; i < 8; i++) {
        setSHUMAGUAN(i);
        SHUMAGUAN_PORT = SEG_CODE[digits[i]];
        delay(100); // 短暂延时
        SHUMAGUAN_PORT = 0x00; // 消隐
    }
}

// 定时器0初始化
void initTimer0() {
    TMOD &= 0xF0;
    TMOD |= 0x01;    // 设置定时器0为模式1（16位定时器）
    TH0 = 0xFC;      // 设置定时器初值为0xFC66，对应1ms (11.0592MHz)
    TL0 = 0x66;
    ET0 = 1;         // 开启定时器0中断
    EA = 1;          // 开启总中断
    TR0 = 1;         // 启动定时器0
}

// 定时器0中断服务程序
void timer0_isr() interrupt 1
{
    TH0 = 0xFC;      // 重新加载定时器初值
    TL0 = 0x66;
    
    if (isRunning) {
        timerCount++;
        if (timerCount >= 10) {  // 每10ms更新一次
            timerCount = 0;
            if (++centiseconds >= 100) {
                centiseconds = 0;
                if (++seconds >= 60) {
                    seconds = 0;
                    if (++minutes >= 60) {
                        minutes = 0;
                        if (++hours >= 24) {
                            hours = 0;
                        }
                    }
                }
            }
        }
    }
}

// 按键扫描函数
void keyScan() {
    if (KEY1 == 0) {
        delay(10);  // 消抖
        if (KEY1 == 0) {
            isRunning = !isRunning;
            while (KEY1 == 0);  // 等待按键释放
        }
    }
    if (KEY2 == 0) {
        delay(10);  // 消抖
        if (KEY2 == 0) {
            isRunning = 0;
            hours = 0;
            minutes = 0;
            seconds = 0;
            centiseconds = 0;
            while (KEY2 == 0);  // 等待按键释放
        }
    }
}

// 主函数
void main(void) {
    initTimer0();
    
    while(1) {
        keyScan();
        displayTime();
    }
}
