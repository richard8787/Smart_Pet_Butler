#include "setting_hardaware/setting.h"
#include <stdlib.h>
#include "stdio.h"
#include "string.h"
#include <math.h>
#define _XTAL_FREQ 8000000
char str[20];
int time = 0;
int delay_cycle;
int count=0;
int in = 0;

void delay(unsigned int val)
{
     unsigned int i,j;
        for(i=0;i<val;i++)
            for(j=0;j<165;j++);
}

void __interrupt (high_priority) my_isr_routine (void) {//時間到開始控制馬達轉動開關掉落飼料
    T1CONbits.TMR1ON = 0;
    CCPR1L = 37;
    CCP1CONbits.DC1B = 0b11;
    int counter=0;
    if(in == 1) {
        if(count==delay_cycle){
            for(int i=0;i<4;i++){//開關四次，每次 -90度 ~ +90度
                while(CCPR2L >= 7){

                    if(CCP2CONbits.DC2B==0){
                        CCP2CONbits.DC2B = 0b11;
                        CCPR2L--;
                    }
                    else{
                        CCP2CONbits.DC2B --; 
                    }
                    counter=0;
                    while(counter<500)
                        counter++;
                }
                counter = 0;
                CCPR2L = 37;
            }
            count = 0;
        }
        else{
            count++;
        }
    }
    T1CONbits.TMR1ON = 1;
    PIR1bits.TMR1IF = 0;
    return;
}

void set_time(void)//利用UART給使用者設定間隔多久掉落一次飼料(單位:秒)
{
    ClearBuffer();
    int b = 0;
    while(1){
        strcpy(str, GetString());
        for(int i=19; i>=0; --i) {
            if(str[i] == 'o') {
                for(int j=i-1; j>=0; --j) {//轉換char to int
                    time = (str[j] - '0') * pow(10, (i-j-1));
                }
                T1CONbits.TMR1ON = 1;
                delay_cycle = time/2;
                in = 1;
                b = 1;
                break;
            }
        }
        if(b == 1)
            break;
    }
    return;
}

void main(void) 
{
	//初始設定
    SYSTEM_Initialize() ;
    T2CON = 0b01111111;
    CCP1CONbits.CCP1M = 0b1100;
    TRISC = 0;
    LATC = 0;
    PR2 = 0xff;
    CCPR1L = 37;
    CCP1CONbits.DC1B = 0b11;
    
    T1CON = 0x30;
    TMR1H = 0;
    TMR1L = 0;
    T1CONbits.TMR1ON = 0;
    PIE1bits.TMR1IE = 1;
    PIR1bits.TMR1IF = 0;
    CCP2CONbits.CCP2M = 0b1100;
    CCPR2L = 37;
    CCP2CONbits.DC2B = 3;
    
    while(1) {
        strcpy(str, GetString());
        if(str[0]=='s' && str[1]=='e' && str[2]=='t' && str[3]==' ' && str[4]=='t' && str[5]=='i' && str[6]=='m' && str[7]=='e') {
            set_time();
            ClearBuffer();
        }
        
        float n = ADC_Read(7);//接收壓力感測器訊號

        if(n < 1 ) {
            while(CCPR1L >= 7) {//如果小於一定值就會啟動開關加水，否則馬達不動
            if(CCP1CONbits.DC1B == 0b00) {
                CCPR1L -= 0b01;
                CCP1CONbits.DC1B = 0b11;
            }
            else
                CCP1CONbits.DC1B -= 0b01;
            delay(5);
            }
        CCPR1L = 37;
        CCP1CONbits.DC1B = 0b11;
        }
        
    }
    return;
}