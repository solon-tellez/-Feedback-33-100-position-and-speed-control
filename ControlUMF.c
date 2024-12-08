#include <ControlUMF.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "LCD_886_PORTA.c"

#BYTE PORTC=0x07
#BYTE PORTB=0x06
#BYTE TRISC=0x87
#BYTE TRISB=0x86

#BIT PULSADOR=PORTB.0


float Vel=0,Pos=0,Consigna=0;

float um=0;
float ek=0;
float ekp=0;


float Err_tabla[TAMANO_TABLA];
float Um_tabla[TAMANO_TABLA];
float Pos_1=0;



short ActualizarDatos=0;
int t_ActualizarDatos=0;
short ActualizarModoDeControl_LCD=0;


short error=0;
float valor=0;
float dec=1;
int i;

int Pos_temp=0;

char clave[6][4]={"POS","VEL","Kp","Va1","Vb0","Vb1"};

   
struct comandos cmd;
struct comandos cmd_temp;

float Rzp=0.6;
//struct Rz R_Pos;
struct Rz R_Vel;


   
#int_TIMER1
void TIMER1_isr() 
{
    //set_timer1(15536);
    set_timer1(25536);
    
    //output_high(LED4);
    LeerPos();
    LeerVel();
    
    //if((Vel<1.5)&(Vel>-1.5))
               //Vel=0;
    

    
        if(cmd.CONTROL==0) // 0=POS, 1=VEL
        {    
            //LeerPos();
            //LeerVel();
            Pos_temp=Pos;
            
            if((Pos-Pos_1)<-180)
                Pos=Pos+360;

            if((Pos-Pos_1)>180)
                Pos=Pos-360;
            

            if(cmd.CONSIGNA==0)// 0=POT, 1=Usuario
            {
                LeerPot();                             
                ek=Consigna-Pos; 
            }
            else
            {
                ek=(-1*(cmd.POS)-Pos);                
            }

            
            if(ek<-180)
                ek=ek+360;

            if(ek>180)
                ek=ek-360;
                       
            //um=(0.6*ek);    
            
            ekp=(Rzp*ek);
            
            //if((ekp<2)&(ekp>-2))
              // ekp=0;
            
            ek=ekp-Vel;
            
            um=(R_Vel.a[1]*Um_tabla[1])+(R_Vel.b[0]*ek)-(R_Vel.b[1]*Err_tabla[1]);
            
            
                        
        } 
        
        else
        {
            
            //LeerVel();
            

            if(cmd.CONSIGNA==0)// 0=POT, 1=Usuario
            {
                LeerPot();
                Consigna=RelLineal(-180,-50,180,50,Consigna);                
                ek=Consigna-Vel;                    
            }
            else
            {
                ek=(cmd.VEL)-Vel;                                                                   
            }
            
            um=(R_Vel.a[1]*Um_tabla[1])+(R_Vel.b[0]*ek)-(R_Vel.b[1]*Err_tabla[1]); 
            
            
            
            
            
        }
            

       if(um>13.6)
            um=13.6;
        if(um<-13.3)
            um=-13.3;

        AplicarTension(um);
  
        
       //Um_tabla[3]=Um_tabla[2];
       Um_tabla[2]=Um_tabla[1];
       Um_tabla[1]=um;

       Err_tabla[2]=Err_tabla[1];
       Err_tabla[1]=ek;
       
       Pos_1=Pos;
       //output_low(LED4);
      

} 




#int_ext    //Activo interrupcion externa por flanco en RB0
void ext_isr()
{
    cmd.CONSIGNA=cmd_temp.CONSIGNA;
    cmd.CONTROL=cmd_temp.CONTROL;
    cmd.POS=cmd_temp.POS;
    cmd.VEL=cmd_temp.VEL;    
    ActualizarModoDeControl_LCD=1;
    //cmd.POS=-100;
}

 
#INT_RTCC 
void rtcc_isr()
{
    set_timer0(61);
    //output_high(LED4);
    t_ActualizarDatos++;
            
    if(t_ActualizarDatos==10)//10 20ms
    {
        ActualizarDatos=1;
        t_ActualizarDatos=0;
    }
    
    
    //output_low(LED4);
    
}

void main ()
{

    //setup_timer_1 (T1_INTERNAL|T1_DIV_BY_4);
    setup_timer_1 (T1_INTERNAL|T1_DIV_BY_1); //25mS
    setup_timer_0(RTCC_INTERNAL|RTCC_DIV_256);//50ms
  


    setup_ccp2(CCP_PWM_PLUS_2);                   // Configure CCP2 as a PWM
    delay_ms(100);                         // Wait 100ms
   
    setup_timer_2(T2_DIV_BY_16, 249, 1); //500Hz 9.96-10bits
    

    setup_adc_ports(sAN8|sAN9|sAN10);
    setup_adc(ADC_CLOCK_INTERNAL);
    set_adc_channel(8);
    
    enable_interrupts(INT_TIMER1); 
    enable_interrupts(INT_RTCC);
    
    port_B_pullups(0x01);
    ext_int_edge(H_TO_L);
    enable_interrupts(INT_EXT); 
    

    enable_interrupts(GLOBAL);
    set_timer0(61);
    //set_timer1(15536);
    set_timer1(25536);

   
    lcd_init();
    delay_ms(500);
    
    

   
 
    
   // R_Pos.a[0]=1;
    //R_Pos.a[1]=0.1609;
    //R_Pos.a[2]=0.0067;
    
   // R_Pos.b[0]=1;
    //R_Pos.b[1]=1.159;
    //R_Pos.b[2]=0.1615;
    
   // R_Pos.n=1;
   // R_Pos.m=1;
    
    R_Vel.a[1]=1;
    
    
    //100mS
    //R_Vel.b[0]=0.091;
    //R_Vel.b[1]=0.049;
    
    //20mS
    R_Vel.b[0]=0.0742;
    R_Vel.b[1]=0.0658;
    
    //R_Vel.b[0]=0.07;
    //R_Vel.b[1]=0.028;
    R_Vel.b[2]=1;
    
    R_Vel.n=3;
    R_Vel.m=2;
    

    Um_tabla[3]=0;
    Um_tabla[2]=0;
    Um_tabla[1]=0;
    Um_tabla[0]=0;
    
    Err_tabla[3]=0;
    Err_tabla[2]=0;
    Err_tabla[1]=0;
    Err_tabla[0]=0;
    
        
    cmd.n_datos=TAMANO_CMD;
    cmd.POS=0;
    cmd.VEL=0; 
    cmd.CONTROL=0;
    cmd.CONSIGNA=0;
    
        
    cmd_temp.n_datos=TAMANO_CMD;
    cmd_temp.POS=0;
    cmd_temp.VEL=0; 
    cmd_temp.CONTROL=0;
    cmd_temp.CONSIGNA=0;
    
  
   
   lcd_gotoxy(1,1); 
   printf(lcd_putc,"MODO:POS|IN:POT ");
   delay_ms(200);
   
   lcd_gotoxy(1,2); 
   printf(lcd_putc,"REF:    |MT:   ");
   delay_ms(200);
   
 
   
   output_low(LED1);
   output_low(LED2);
   output_low(LED3);
   output_low(LED4);
   
   printf(">>");
   
   while(TRUE)
   {

      if(kbhit()!=0)
      {
        gets(cmd.datos); 

        ComandosDeControl();
    
        printf("\n\rIntroduzca comandos en esta consola\n\r");
        printf(">>");
        
    
        while(kbhit( )) getc( );
    
      } 
      
      if(ActualizarDatos==1)
      {
          ActualizarDatos=0;
          
          if(ActualizarModoDeControl_LCD==1)
          {
              ActualizarModoDeControl_LCD=0;
              ModoDeControl_LCD();
          }
          
          ActualizarLED();
          DatosDeControl_LCD();
          
         // printf("%.2f\r\n",Pos);
          
          
              
      }        
      

   }


}




void LeerVel()
{
    float Uad0=0,Utaco=0;
    
    set_adc_channel(10);
    Vel=read_adc();
   
    Uad0=RelLineal(0,0,1023,5,Vel);
    Utaco=RelLineal(0,-15,5,15,Uad0);
    Vel=RelLineal(-15,-50,15,50,Utaco);
    
    
}

void LeerPot()
{
    float Uad=0,potenciometro=0;
    
    set_adc_channel(9);
    Consigna=read_adc();
    potenciometro=Consigna;
    
    Uad=RelLineal(0,0,1023,5.13,Consigna);
    Consigna=RelLineal(0,-180,5.13,180,Uad);

}


void LeerPos()
{
    float Uad1=0;     
    
    set_adc_channel(8);
    Pos=read_adc();
    
    Uad1=RelLineal(0,0,1023,5.11,Pos);
    
    Pos=RelLineal(0,-180,5.11,180,Uad1);        
    
}

void AplicarTension(float v)
{
    set_pwm2_duty((int16)RelLineal(-13.3,0,13.6,1023,v));
}

void DatosDeControl_LCD()
{
    lcd_gotoxy(8,2);
    printf(lcd_putc," ");  
    
    
    if(cmd.CONTROL==0)
    {   
        if(cmd.CONSIGNA==0)// 0=POT, 1=Usuario
        {               
            lcd_gotoxy(8,2);
            printf(lcd_putc," ");  
            lcd_gotoxy(5,2); 
            printf(lcd_putc,"%3.0f",Consigna); 
        }
        
        else
        {
            lcd_gotoxy(8,2);
            printf(lcd_putc," ");  
            lcd_gotoxy(5,2); 
            printf(lcd_putc,"%3.0f",cmd.POS);  
        }        
        
        
        lcd_gotoxy(16,2);
        printf(lcd_putc," ");  
        lcd_gotoxy(13,2); 
        printf(lcd_putc,"%3.0f",Pos);
        
        
    }
    else if(cmd.CONTROL==1)
    {
        
        if(cmd.CONSIGNA==0)// 0=POT, 1=Usuario
        {               
            lcd_gotoxy(8,2);
            printf(lcd_putc," ");  
            lcd_gotoxy(5,2); 
            printf(lcd_putc,"%3.0f",Consigna); 
        }
        else
        {
            lcd_gotoxy(8,2);
            printf(lcd_putc," ");  
            lcd_gotoxy(5,2); 
            printf(lcd_putc,"%3.0f",cmd.VEL);            
        }
        
        
        lcd_gotoxy(16,2);
        printf(lcd_putc," ");  
        lcd_gotoxy(13,2); 
        printf(lcd_putc,"%3.0f",Vel);
        
    }
}

void ModoDeControl_LCD()
{
    if(cmd.CONSIGNA==0)
            {
                output_high(LED2);
                lcd_gotoxy(13,1); 
                printf(lcd_putc,"POT");

            }
            else
            {
                output_low(LED2);
                lcd_gotoxy(13,1); 
                printf(lcd_putc,"USR");    

            }
    
            if(cmd.CONTROL==0)
            {
                output_high(LED1);
                lcd_gotoxy(6,1); 
                printf(lcd_putc,"POS");        
            }
            else
            {
                output_low(LED1);
                lcd_gotoxy(6,1); 
                printf(lcd_putc,"VEL");        
            }
}

void ActualizarLED()
{
    if(Vel>1.5)
    {
        output_high(LED3);
        output_low(LED4);
    }
    else if(Vel<-1.5)
    {
        output_high(LED4);
        output_low(LED3);
    }
    else
    {
        output_low(LED4);
        output_low(LED3);
    }
}

void ComandosDeControl()
{

      
    error=0;
    valor=0;
    dec=1;
    i=0;
    char *cmd_f;
    
    
    char pot[]="POT";
    
    

    for ( i=0; i<7; i++)
    {   
     
   
        cmd_f=strstr(cmd.datos,clave[i]);
        
        if (cmd_f!=NULL)
        {

            for (cmd_f=cmd_f+strlen(clave[i]);*cmd_f==' ';cmd_f++);
            if (*cmd_f=='=')
            {
                cmd_f=cmd_f+1;

                if(*cmd_f==' ')
                {
                    for (cmd_f=cmd_f+1;*cmd_f==' ';cmd_f++);

                }

                if(((48<=*(cmd_f))&&(*(cmd_f)<=57))||(*(cmd_f)==45))
                {
                    valor=atof(cmd_f);           
                }
                    else if(strstr(cmd.datos,pot)!=NULL)
                    {
                        strcpy(cmd.datos,"POT");
                    }
                        else
                        {
                          error=1;
                          printf("\n\rERROR: SE ESPERA UN NUMERO o 'POT' !\n\r");
                        }
            }
                else
                {
                    error=1;
                    printf("\n\rERROR: SE ESPERA '=' DESPUES DEL COMANDO!\n\r");
                }

            break;
        }

    }

    if(i==7)
    {
        printf("\n\rERROR: COMANDO INVALIDO!\n\r");
    }
    else if(i==2)
    {
        Rzp=valor;
        printf("\n\rKp de Controlador de posicion actualizado a : %f\n\r",Rzp);
    }
    else if(i==3)
    {
         R_Vel.a[1]=valor;
         printf("\n\ra[1] de Controlador de velocidad actualizado a : %f\n\r",R_Vel.a[1]);
    }
    else if(i==4)
    {
         R_Vel.b[0]=valor;
         printf("\n\rb[0] de Controlador de velocidad actualizado a : %f\n\r",R_Vel.b[0]);
    }
    else if(i==5)
    {
         R_Vel.b[1]=valor;
         printf("\n\rb[1] de Controlador de velocidad actualizado a : %f\n\r",R_Vel.b[1]);
    }
    else if(error==0)
    {
        if(strcmp(cmd.datos,pot)!=0)
        {
            *(&cmd_temp.POS+i)=valor;
            cmd_temp.CONSIGNA=1;
        }
        else
        {
            cmd_temp.CONSIGNA=0;
        }


        if(i==0)
        {
            cmd_temp.CONTROL=0;;
        }
            else if(i==1)
            {
                cmd_temp.CONTROL=1;
            }

    }


}

float RelLineal(float x1, float y1, float x2, float y2, float X)
{
    float y=0;

    y=((y2-y1)/(x2-x1))*(X-x1)+y1;

    return y;
}

