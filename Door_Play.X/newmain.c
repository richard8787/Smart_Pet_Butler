#include <xc.h>
#include <pic18f4520.h>
#include <stdlib.h>
#include <time.h>


#pragma config OSC = INTIO67 // Oscillator Selection bits
#pragma config WDT = OFF     // Watchdog Timer Enable bit 
#pragma config PWRT = OFF    // Power-up Enable bit
#pragma config BOREN = ON   // Brown-out Reset Enable bit
#pragma config PBADEN = OFF     // Watchdog Timer Enable bit 
#pragma config LVP = OFF     // Low Voltage (single -supply) In-Circute Serial Pragramming Enable bit
#pragma config CPD = OFF     // Data EEPROM　Memory Code Protection bit (Data EEPROM code protection off)
int count_dou=0;//逗貓次數
int count_time=0;//time counting
int count_door=0;
unsigned int dou_array[10] = {13,11,5,18,10,17,3,15,0,11};

unsigned int x = 0;
void delay()
{
    for(int i = 0; i < 1500; i++)
            asm("nop");
}
void delayq()
{
    for(int i = 0; i < 1000; i++)
            asm("nop");
}
void dou_meow()
{
    srand(time(0));
    
    while(count_dou <10)
    {
        //逗貓的角度 隨機設定
        x = (unsigned int)rand()%6;
        if(count_dou%2)
        {
            //%2原因是讓角度可以比較大幅度改變
            CCPR1L = (x)*3/4;
            CCP1CONbits.DC1B = (x)*3%4;
        }
        else
        {
            CCPR1L = (x+14)*3/4;
            CCP1CONbits.DC1B = (x+14)*3%4;
        }
        delayq();
        count_dou ++;
        
        /*
        if(CCPR1L != 0b00010010 && CCP1CONbits.DC1B != 0b11)
        {
            CCPR1L = 0b00010010 ;
            CCP1CONbits.DC1B = 0b11;
            delayq();
        }

        if(CCPR1L == 0b00010010 && CCP1CONbits.DC1B == 0b11)
        {
            CCPR1L = 0x04;
            CCP1CONbits.DC1B = 0b00;
            delayq();
            count_dou++;
        }
         */
    }
    count_dou = 0;
}
void door()
{
    //開門鈴聲
    PORTD = 3;
    for(int i = 0; i<10; i++)
        delay();
    PORTD = 0;
    //開門
    if(CCPR2L != 0b00001011 && CCP2CONbits.DC2B != 0b01)
    {
        CCPR2L = 0b00001011 ;
        CCP2CONbits.DC2B = 0b01;
    }
    //開門的時間delay
    for(int i =0; i < 10; i++)
        delay();
    //關門的鈴聲
    PORTD = 3;
    for(int i = 0; i < 10; i++)
        delay();
    PORTD = 0;
    //關門
    if(CCPR2L != 0x04 && CCP2CONbits.DC2B != 0b00)
    {
        CCPR2L = 0x04;
        CCP2CONbits.DC2B = 0b00;
    }
}


void __interrupt () time_interrupt(void)
{  
    //PORTD = 3;
    //重新設定timer interrupt
    TMR0IF=0;
    TMR0H=0xFB;
    TMR0L=0xFF;
    count_time++;
    //500時開始逗貓
    if(count_time == 500)
    {
        dou_meow();
    }
    //700時開關門
    if(count_time == 700)
        door();
    //到1500重新開始
    if(count_time >= 1500)
    {
        //PORTD ^= 3;
        count_time = 0;
    }
    
      
    
}



void main(void) {
    // 設定TIMER2, prescaler為4
    T2CON = 0b01111101;
    // 設定OSC頻率，Fosc為125k = Tosc為8µs
    OSCCONbits.IRCF = 0b001;
    // 設定CCP1
    CCP1CONbits.CCP1M = 0b1100;
    CCP2CONbits.CCP2M = 0b1100;
    // 將RC2設定為輸出，並初始化(因為CCP1和RC2是同一個port)
    TRISC = 0;
    TRISB = 1;
    LATC = 0;
    // 設定pr2和ccp的數字來調整輸出的PWM週期和Duty Cycle
    // period = (PR2+1)*4*Tosc*(TMR2 prescaler) = (0x9b + 1) * 4 * 8µs * 4 = 0.019968s ~= 20ms
    PR2 = 0x9b;
    // duty cycle = (CCPR1L:CCP1CON<5:4>)*Tosc*(TMR2 prescaler) = (0x0b*4 + 0b01) * 8µs * 4 = 0.00144s ~= 1450µs
    CCPR1L = 0x04;
    CCP1CONbits.DC1B = 0b00;
            
    CCPR2L = 0x04;
    CCP2CONbits.DC2B = 0b00;
    // Demo時需要向TAs解釋如何設定Tosc和PR2以及CCPR1L和CCP1CON，並且計算出如何得到週期20ms以及duty cycle的500µs和2400µs。
    TRISD=0;
    LATD=0;
    
    //timer0
    INTCONbits.TMR0IF=0;
    INTCONbits.PEIE=1;
    INTCONbits.TMR0IE=1;
    INTCONbits.GIE=1;
    T0PS2=1;
    T0PS1=1;
    T0PS0=0;
    //timer0 initialize
    TMR0H=0xFB;
    TMR0L=0xFF;
    
    count_dou=0;
    //開始計時
    T0CONbits.TMR0ON=1;
    while(1)
    {
        ;
    }
    return;
}