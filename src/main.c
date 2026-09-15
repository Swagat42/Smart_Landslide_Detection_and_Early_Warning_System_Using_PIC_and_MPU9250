#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

// Configuration settings
#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config BOREN = ON
#pragma config LVP = OFF
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

#define _XTAL_FREQ 20000000UL

// MPU9250 register addresses
#define MPU_ADDR  0x68
#define MPU_WHOAMI 0x75
#define MPU_PWR 0x6B
#define MPU_ACCEL_CFG 0x1C
#define MPU_AX_H 0x3B
#define MPU_AY_H 0x3D
#define MPU_AZ_H  0x3F
#define ACCEL_SENS 16384.0f

// LCD I2C address
#define LCD_ADDR 0x4E  

// Tilt threshold used for detecting ground movement
#define THRESHOLD_TILT 0.40f

// Buzzer connected to RB0
#define BUZZER RB0

// UART setup
void UART_Init(void){
    TRISC6=0; TRISC7=1;
    SPBRG=129; TXSTA=0x24; RCSTA=0x90;
}

void UART_Char(char c){while(!TXIF); TXREG=c; }

void UART_Print(const char *s){while(*s) UART_Char(*s++); }

void UART_Println(const char *s){ UART_Print(s); UART_Char('\r'); UART_Char('\n'); }

void UART_Int(int16_t v){
    uint8_t buf[8], i=0;
    if(v==0){UART_Char('0');return;}
    if(v<0){UART_Char('-');v=-v;}
    while(v>0){buf[i++]='0'+(v%10);v/=10;}
    while(i--) UART_Char(buf[i]);
}

void UART_UInt(uint16_t v){
    uint8_t buf[8], i=0;
    if(v==0){UART_Char('0');return;}
    while(v>0){buf[i++]='0'+(v%10);v/=10;}
    while(i--) UART_Char(buf[i]);
}

void UART_Hex(uint8_t v){
    const char h[]="0123456789ABCDEF";
    UART_Print("0x"); UART_Char(h[v>>4]); UART_Char(h[v&0x0F]);
}

void UART_Float(float v){
    int16_t w,f;
    if(v<0){UART_Char('-');v=-v;}
    w=(int16_t)v; f=(int16_t)((v-w)*100);
    UART_Int(w); UART_Char('.');
    if(f<10) UART_Char('0');
    UART_Int(f);
}

// Soil moisture sensor is connected to AN0
void ADC_Init(void){
    ADCON0=0x41; ADCON1=0xCE; TRISA0=1;
}

uint16_t ADC_Read(uint8_t channel){
    ADCON0 &= 0xC5;
    ADCON0 |= (channel<<3);
    __delay_ms(2);
    GO_nDONE=1;
    while(GO_nDONE);
    return ((uint16_t)(ADRESH<<8)|ADRESL);
}

uint16_t GetMoisturePercent(uint16_t adcVal){
    unsigned long m=(unsigned long)(1023-adcVal)*100;
    return (uint16_t)(m/1023);
}

// I2C setup
void I2C_Init(void){
    TRISC3=1; TRISC4=1;
    SSPCON=0x28; SSPCON2=0x00;
    SSPADD=49; SSPSTAT=0x80;
    __delay_ms(200);
}

void I2C_Wait(void){
    uint16_t t=10000;
    while(((SSPSTAT&0x04)||(SSPCON2&0x1F))&&t--);
    __delay_us(20);
}

void I2C_Start(void){I2C_Wait();SEN =1;I2C_Wait();__delay_us(30);}
void I2C_Stop(void){I2C_Wait();PEN =1;I2C_Wait();__delay_us(30);}
void I2C_RS(void) {I2C_Wait();RSEN=1;I2C_Wait();__delay_us(30);}
void I2C_Nack(void){ACKDT=1;I2C_Wait();ACKEN=1;I2C_Wait();__delay_us(20);}

uint8_t I2C_Write(uint8_t d){
    I2C_Wait(); SSPBUF=d; I2C_Wait(); __delay_us(30); return ACKSTAT;
}

uint8_t I2C_Read(void){
    uint8_t d; I2C_Wait(); RCEN=1; I2C_Wait(); __delay_us(30); d=SSPBUF; return d;
}

// LCD functions
void LCD_Write_Nibble(uint8_t nibble, uint8_t rs) {
    uint8_t data = nibble & 0xF0;
    data |= rs;
    data |= 0x08; // Backlight ON
    
    I2C_Start();
    I2C_Write(LCD_ADDR);
    I2C_Write(data | 0x04); // EN high
    __delay_us(50);
    I2C_Write(data & ~0x04); // EN low
    __delay_us(50);
    I2C_Stop();
}

void LCD_Cmd(uint8_t cmd) {
    LCD_Write_Nibble(cmd & 0xF0, 0);
    LCD_Write_Nibble((cmd << 4) & 0xF0, 0);
    __delay_ms(2);
}

void LCD_Char(char data) {
    LCD_Write_Nibble(data & 0xF0, 1);
    LCD_Write_Nibble((data << 4) & 0xF0, 1);
    __delay_us(50);
}

void LCD_Init() {
    __delay_ms(50);
    LCD_Write_Nibble(0x30, 0); __delay_ms(5);
    LCD_Write_Nibble(0x30, 0); __delay_us(150);
    LCD_Write_Nibble(0x30, 0); 
    LCD_Write_Nibble(0x20, 0); // Set 4-bit mode
    LCD_Cmd(0x28); 
    LCD_Cmd(0x0C); 
    LCD_Cmd(0x06); 
    LCD_Cmd(0x01); 
    __delay_ms(2);
}

void LCD_String(const char *str) {
    while(*str) LCD_Char(*str++);
}

void LCD_SetCursor(uint8_t row, uint8_t col) {
    uint8_t addr = (row == 0) ? 0x80 : 0xC0;
    addr += col;
    LCD_Cmd(addr);
}

void LCD_Clear() {
    LCD_Cmd(0x01);
    __delay_ms(2);
}

// MPU9250 functions
void MPU_Write(uint8_t reg, uint8_t val){
    I2C_Start(); I2C_Write((MPU_ADDR<<1)|0); I2C_Write(reg); I2C_Write(val); I2C_Stop(); __delay_ms(10);
}

uint8_t MPU_Read(uint8_t reg){
    uint8_t d;
    I2C_Start(); I2C_Write((MPU_ADDR<<1)|0); I2C_Write(reg);
    I2C_RS(); I2C_Write((MPU_ADDR<<1)|1); d=I2C_Read();
    I2C_Nack(); I2C_Stop(); __delay_us(100); return d;
}

int16_t MPU_ReadWord(uint8_t rH){
    uint8_t h=MPU_Read(rH); uint8_t l=MPU_Read(rH+1);
    return (int16_t)((h<<8)|l);
}

uint8_t MPU_Init(void){
    uint8_t id, tr=10;

    // Check if the MPU9250 is responding
    while(tr--){
        id=MPU_Read(MPU_WHOAMI);
        if(id==0x71||id==0x73||id==0x70) break;
        __delay_ms(100);
    }

    if(id!=0x71&&id!=0x73&&id!=0x70) return 0;

    // Reset and wake up the sensor
    MPU_Write(MPU_PWR,0x80); __delay_ms(200);
    MPU_Write(MPU_PWR,0x00); __delay_ms(150);

    // Use the ±2g accelerometer range
    MPU_Write(MPU_ACCEL_CFG,0x00); __delay_ms(50);

    return id;
}

// Main program
void main(void){
    int16_t rx, ry, rz;
    float   ax, ay, az;
    uint8_t id;
    uint16_t adcValue, moisturePct;

    // Set RB0 as the buzzer output
    TRISB0 = 0; 
    BUZZER = 0; 

    UART_Init();
    ADC_Init();
    I2C_Init();
    
    __delay_ms(500);
    
    // Beep once during startup to check the buzzer
    BUZZER = 1; __delay_ms(100); BUZZER = 0; __delay_ms(100);
    BUZZER = 1; __delay_ms(100); BUZZER = 0;

    // Start the LCD
    LCD_Init();
    LCD_SetCursor(0, 0);
    LCD_String("Landslide System");
    LCD_SetCursor(1, 0);
    LCD_String("Initializing... ");
    __delay_ms(1500);

    // Start the MPU9250
    id = MPU_Init();
    if(!id){
        UART_Println("ERROR: MPU9250 not found!");
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_String("SENSOR ERROR!");
        while(1);
    }

    LCD_Clear();
    
    while(1){
        // Read soil moisture
        adcValue    = ADC_Read(0);
        moisturePct = GetMoisturePercent(adcValue);

        // Read acceleration values
        rx=MPU_ReadWord(MPU_AX_H);
        ry=MPU_ReadWord(MPU_AY_H);
        rz=MPU_ReadWord(MPU_AZ_H);

        ax=(float)rx/ACCEL_SENS;
        ay=(float)ry/ACCEL_SENS;
        az=(float)rz/ACCEL_SENS;

        UART_Println("--- LANDSLIDE MONITORING ---");
        UART_Print("Moisture : "); UART_UInt(moisturePct); UART_Println("%");
        UART_Print("Tilt X   : "); UART_Float(ax); UART_Println("g");
        UART_Print("Tilt Y   : "); UART_Float(ay); UART_Println("g");

        // Check the two conditions used for risk detection
        bool isMoistureHigh = (moisturePct > 75);
        bool isTilted = (ax > THRESHOLD_TILT || ax < -THRESHOLD_TILT || 
                         ay > THRESHOLD_TILT || ay < -THRESHOLD_TILT);

        LCD_SetCursor(0, 0);
        LCD_String("M:"); 
        LCD_Char((moisturePct/100)%10 + '0'); 
        LCD_Char((moisturePct/10)%10 + '0'); 
        LCD_Char((moisturePct)%10 + '0'); 
        LCD_String("%  ");

        // Check the risk level
        if(isMoistureHigh && isTilted) {
            // Both moisture and ground movement are high
            UART_Println(">> ALERT: LANDSLIDE DETECTED! <<");
            LCD_SetCursor(0, 8); LCD_String("DANGER!!");
            LCD_SetCursor(1, 0); LCD_String("LANDSLIDE ALERT ");
            BUZZER = 1; // Buzzer ON
        } 
        else if (isMoistureHigh) {
            // Soil is wet, but no ground movement is detected
            UART_Println(">> STATUS: HIGH RISK (WET SOIL)");
            LCD_SetCursor(0, 8); LCD_String("RISK:WET");
            LCD_SetCursor(1, 0); LCD_String("SOIL UNSTABLE   ");
            
            // Beep for wet soil warning
            BUZZER = 1; 
            __delay_ms(500); 
            BUZZER = 0; 
        } 
        else if (isTilted) {
            // Ground movement has been detected
            UART_Println(">> STATUS: GROUND SHIFT DETECTED");
            LCD_SetCursor(0, 8); LCD_String("RISK:TLT");
            LCD_SetCursor(1, 0); LCD_String("GROUND SHIFTING ");
            
            // Beep for ground movement warning
            BUZZER = 1; 
            __delay_ms(500); 
            BUZZER = 0; 
        } 
        else {
            // Everything is within the normal range
            UART_Println(">> STATUS: SAFE");
            LCD_SetCursor(0, 8); LCD_String("SAFE    ");
            LCD_SetCursor(1, 0); LCD_String("AREA SECURE     ");
            BUZZER = 0; 
        }

        UART_Println("----------------------------\r\n");
        __delay_ms(500); // Wait before taking the next reading
    }
}
