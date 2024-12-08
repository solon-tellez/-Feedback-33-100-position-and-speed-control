#include <16F886.h>
#device *=16
#device ADC=10

#FUSES NOWDT                    //No Watch Dog Timer

#FUSES INTRC                    //Internal RC Osc

#FUSES NOPUT                    //No Power Up Timer
#FUSES NOBROWNOUT               //No brownout reset
#FUSES NOLVP                    //No low voltage prgming, B3(PIC16) or B5(PIC18) used for I/O


#use delay(internal=8000000)
#use FIXED_IO( B_outputs=PIN_B0 )


#use rs232(baud=9600,parity=N,xmit=PIN_C6,rcv=PIN_C7,bits=8)


#define TAMANO_CMD 20

#define TAMANO_TABLA 4

#define LED1 PIN_C2
#define LED2 PIN_C3
#define LED3 PIN_C4
#define LED4 PIN_C5

void ActualizarLED();
void AplicarTension(float v);
void ComandosDeControl();
void DatosDeControl_LCD();
void LeerPos();
void LeerPot();
void LeerVel();
void ModoDeControl_LCD();
float RelLineal(float x1, float y1, float x2, float y2, float X);


   
struct comandos
{
    float POS;     // -180<Grados<180
    float VEL;     // -40<RPM<40
    short CONTROL; // 0=POS, 1=VEL
    short CONSIGNA;    // 0=POT, 1=Usuario
    char datos[TAMANO_CMD];
    int n_datos;
};

struct Rz
{
    float a[2];
    float b[3];
    int n;
    int m;
};