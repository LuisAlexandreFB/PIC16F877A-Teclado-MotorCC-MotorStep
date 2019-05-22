#include <16F877A.h>
#device adc=10

#FUSES NOWDT                    //No Watch Dog Timer
#FUSES HS                       //High speed Osc (> 4mhz for PCM/PCH) (>10mhz for PCD)
#FUSES NOPUT                    //No Power Up Timer
#FUSES NOBROWNOUT               //No brownout reset
#FUSES NOLVP                    //No low voltage prgming, B3(PIC16) or B5(PIC18) used for I/O
#FUSES NOCPD                    //No EE protection
#FUSES NOWRT                    //Program memory not write protected
#FUSES NODEBUG                  //No Debug mode for ICD
#FUSES NOPROTECT                //Code not protected from reading

#use delay(clock=20000000)

#define use_portb_kbd TRUE
#include "key_awm4x4.c"

#define LCD_ENABLE_PIN  PIN_E0                                    ////
#define LCD_RS_PIN      PIN_E1                                    ////
#define LCD_RW_PIN      PIN_E2                                    ////
#define LCD_DATA4       PIN_A1                                    ////
#define LCD_DATA5       PIN_A2                                   ////
#define LCD_DATA6       PIN_A3                                    ////
#define LCD_DATA7       PIN_A4 

#include <lcd.c>

// Variáveis do motor de passo

int1 sentido = 0, sentido_cc = 0;
int8 tempo=0, cont = 0;
int8 passos[4] = {0b000001010, 0b00001001, 0b00000101, 0b00000110};
signed int i = 0;
int16 velocidade_cc = 0;

char c='0', aux='0', novoChar;

#INT_RTCC
void  RTCC_isr(void) {

// MOTOR DE PASSO   
   if(tempo > 0){ 
      cont++;
      if (sentido == 1){
         if (i > 3) i = 0; 
         if(cont >= tempo){
            output_d(passos[i]);
            i++;
            cont = 0;
         }
      }else{
         if (i < 0) i = 3;
         if(cont >= tempo){
            output_d(passos[i]);
            i--;    
            cont = 0;
         }
      }
   }

// MOTOR DE CORRENTE CONTINUA  
   if(sentido_cc){
      set_pwm2_duty((int16)0);
      delay_us(50);
      set_pwm1_duty((int16)velocidade_cc); // liga CCP1
   } else{
      set_pwm1_duty((int16)0);
      delay_us(50);
      set_pwm2_duty((int16)velocidade_cc); // liga CCP2
   }
}


void main()
{
   setup_adc_ports(AN0);
   setup_adc(ADC_CLOCK_INTERNAL);
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_32|RTCC_8_BIT);      //1,6 ms overflow
   setup_timer_2(T2_DIV_BY_16,255,1);      //819 us overflow, 819 us interrupt
   
   setup_ccp1(CCP_PWM);
   setup_ccp2(CCP_PWM);
   set_pwm1_duty((int16)0);
   set_pwm2_duty((int16)0);
   
   enable_interrupts(INT_RTCC);
   enable_interrupts(GLOBAL);
   
   kbd_init();
   lcd_init();
   
   char c;  
   int16 vel_cc, novoVel;
   while(true){
      
      velocidade_cc = read_adc(); // LÊ VALOR DO POTENCIOMETRO
      delay_us(30);
           
      c = kbd_getc(); // LÊ CARACTERE
      delay_ms(1);
                        
      if(input(PIN_C0)) // FUNCIONAMENTO DO BOTAO
         sentido_cc = 0;
      else
         sentido_cc = 1;
      
      if(c != 0){  // 
         aux = c;
         
         switch(c){
            case '1':
               tempo = 62;
               break;
            case '2': 
               tempo = 47; 
               break;
            case '3': 
               tempo = 32; 
               break;
            case '4': 
               tempo = 17; 
               break;
            case '5': 
               tempo = 2; 
               break;
            case '0': 
               tempo = 0; 
               break;
            case '*':   // SENTIDO ANTI-HORARIO
               sentido = 0;    
               break;
            case '#':   // SENTIDO HORARIO
               sentido = 1; 
               break;
            default: break;
         }   
      }
      vel_cc = (int16)(velocidade_cc*(0.01))/2;
      
      if((vel_cc != novoVel || aux != novoChar) && aux>=48 && aux<=53){
         novoChar = aux;
         novoVel = vel_cc;
         printf(lcd_putc, "\fVel MP = %c \n", novoChar);
         printf(lcd_putc, "Vel MCC = %lu", vel_cc);         
      }
   }
}
