///////////////////////////////////////////////////////////////////////////
////                             LCD_PICDEM2 plus v2006.c                         ////
////                 Driver for common LCD modules 
//// Nota: Modificado por F.F.Linera (05-12-08)                    ////
//// Nota: Modificado por Manuel Rico Secades para la placa            ////
//// añadido la función lcd_set_cgram que coloca el puntero         ////
//// de la CGRAM que permite definir caracteres nuevos            ////
////   PICDEM 2 PLUS versión 2006                                       ////
////  Apoyada en la rutina original LDC.C y la modificada por          ////
////  Fernando Nuño    LDC_nuno.C                                      ////
////  Abril de 2008                                                    ////
///////////////////////////////////////////////////////////////////////////

// En la PICDEM 2 PLUS v2006 la conexión del LCD es la siguiente:
//
//     RD7  es Vcc
//     RD6  es E (enable)
//     RD5  es RW
//     RD4  es RS
//     RD3  es DB7
//     RD2  es DB6
//     RD1  es DB5
//     RD0  es DB4
//   


struct lcd_pin_map {  
         int       data : 4;  // dato(0) es el de menor peso             
           BOOLEAN rs;           
           BOOLEAN rw;               
           BOOLEAN enable;              
           BOOLEAN vcc;       // vcc es el de mas peso     
        } lcd;

// Asigno el lcd al puerto D para esta placa, que es la dirección 0x08. 
#locate lcd = 0x05
#define set_tris_lcd(x) set_tris_a(x)


#define lcd_type 2           // 0=5x7, 1=5x10, 2=2 lines
#define lcd_line_two 0x40    // LCD RAM address for the second line

/////////////////////////////////////////////////////////////////////////////////////////////
// La constante LCD_INIT_STRING[4] recoge los 4 códigos de inicialización que se envian
// al LCD y son muy importantes. Cambiarlos para adaptar a otras necesidades.
/////////////////////////////////////////////////////////////////////////////////////////////

BYTE const LCD_INIT_STRING[4] = {0x28,0x0C,0x01, 0x06};
//
// El significado de los códigos es el siguiente:
// 0x28 = 0010 1000 = 001 DL  N F **
// entonces DL=0 control a 4 bits (DL=0 sería control con 8 bits)
// N =1 significa 2 lineas  (N=0 sería 1 línea)
// F=0 significa, caracteres 5x8  (F=1 sería 5x10)
//
// 0x0C = 0000 1100 = 0000 1 D C B
// es el control de display ON/OFF
// D=1 significa diaplay ON
// C=0 significa cursor OFF (puede interesar poner C=1 para ver donde esta el cursor)
// B=0 sigifica parpadeo del cursor OFF
//
// 0x01 = 0000 0001 significa borrar display
//
// 0x06 = 0000 0110 = 0000 01 I/D S
// selecciona el modo de funcionamiento (Entry mode set)
// I/D = 1 significa incremento automatico del cursor
// S = 0 sifnifica sin desplazamiento del display
//

struct lcd_pin_map const LCD_WRITE = {0,0,0,0,0}; // Para escribir, todo salidas
struct lcd_pin_map const LCD_READ = {15,0,0,0,0}; // Para leer, los 4 bajos son entradas


/////////////////////////////////////////////////////////////////////////////////////////////
// Lee el byte señalado por el puntero, 1º parte baja, 2º parte alta
// Si al llamar a esta función rs=0, devuelve busy flag (BF) (+signif.) y dirección actual
/////////////////////////////////////////////////////////////////////////////////////////////

BYTE lcd_read_byte() {
      BYTE high,low;
      set_tris_lcd(LCD_READ);
      lcd.rw = 1;
      delay_cycles(1);
      lcd.enable = 1;
      delay_cycles(1);
      high = lcd.data; // Se lee primero la parte baja (ES AL REVÉS QUE EN OTROS LCDs)
      lcd.enable = 0;
      delay_cycles(1);
      lcd.enable = 1;
      delay_us(1);
      low = lcd.data; // Se lee despues la parte alta (AL REVÉS QUE EN OTROS LCDs)
      lcd.enable = 0;
      set_tris_lcd(LCD_WRITE);
      return( (high<<4) | low);
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Envía medio byte, los 4 bits más bajos de n
// Necesario poner rs y rw de modo adecuado y entrar con enable=0
/////////////////////////////////////////////////////////////////////////////////////////////

void lcd_send_nibble( BYTE n ) {
      lcd.data = n;
      delay_cycles(1);
      lcd.enable = 1;
      delay_us(2);
      lcd.enable = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Envía un byte (n) al registro de instrucciones (si address=0) o reg. de datos (address=1)
// Utiliza lcd_send_nibble(n) enviando primero nibble alto del byte
/////////////////////////////////////////////////////////////////////////////////////////////

void lcd_send_byte( BYTE address, BYTE n ) {

      lcd.rs = 0;
      while ( bit_test(lcd_read_byte(),7) ) ;
     //delay_us(100);
      lcd.rs = address;
      delay_cycles(1);
      lcd.rw = 0;
      delay_cycles(1);
      lcd.enable = 0;
      lcd_send_nibble(n >> 4);
      lcd_send_nibble(n & 0xf);
 
}

///////////////////////////////////////////////////////////////////////////////////////////
/// Función que inicializa el LCD, se deberían cambiar bits para cambiar configuracion
///////////////////////////////////////////////////////////////////////////////////////////
void lcd_init() {
    BYTE i;
    set_tris_lcd(LCD_WRITE);
    lcd.rs = 0;
    lcd.rw = 0;
    lcd.enable = 0;
   lcd.vcc = 1;
    delay_ms(15);
    for(i=1;i<=3;++i) {
       lcd_send_nibble(3);
       delay_ms(5);
    }
    lcd_send_nibble(2);
    for(i=0;i<=3;++i){
       lcd_send_byte(0,LCD_INIT_STRING[i]);
      delay_ms(5);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Sitúa el contador de direcciones en la DDRAM (para lectura o escritura posterior)
// x puede ir de 1 a 40, posición dentro de una línea (16 visibles)
// y puede ser 1 (línea 1) o 2 (línea 2)
/////////////////////////////////////////////////////////////////////////////////////////////

void lcd_gotoxy( BYTE x, BYTE y) {
   BYTE address;

   if(y!=1)
     address=lcd_line_two;
   else
     address=0;
   address+=x-1;
   lcd_send_byte(0,0x80|address);
}

////////////////////////////////////////////////////////////////////////////////////////////
// Envía un carácter c a la DDRAM del LCD, también algunos caracteres de control
////////////////////////////////////////////////////////////////////////////////////////////

void lcd_putc( char c) 
{
   switch (c) {
     case '\f'   : lcd_send_byte(0,1);    //Limpia la pantalla
                   delay_ms(2);
                   break;
     case '\n'   : lcd_gotoxy(1,2);       //Coloca puntero en 1ª posicion de la 2ª línea
                   break;
     case '\b'   : lcd_send_byte(0,0x10); //Retrocede una posición el cursor
                   break;
     case '\t'   : lcd_send_byte(0,0x14); //Avanza una posición el cursor
                   break;
     case '\r'   : lcd_send_byte(0,0x18); //Retrocede una posición la pantalla visible
                   break;
     case '\v'   : lcd_send_byte(0,0x1C); //Avanza una posición la pantalla visible
                   break;
     default     : lcd_send_byte(1,c);    //Envía caracter a DDRAM,
                   break;                 //Si es una tira, los envía todos uno a uno
             }
}

///////////////////////////////////////////////////////////////////////////////////////////
// Devuelve el carácter situado en la posición x,y de la DDRAM
///////////////////////////////////////////////////////////////////////////////////////////

char lcd_getc( BYTE x, BYTE y) {
   char value;

    lcd_gotoxy(x,y);
    while ( bit_test(lcd_read_byte(),7) ); // wait until busy flag is low
    lcd.rs=1;
    value = lcd_read_byte();
    lcd.rs=0;
    return(value);
}

////////////////////////////////////////////////////////////////////////////////////
// Limpia la linea correspondiente y se situa al principio de la misma            //
////////////////////////////////////////////////////////////////////////////////////

void lcd_clr_line(char fila)
{
   int j;

   lcd_gotoxy(1,fila);
    for (j=0;j<40;j++) lcd_putc(' ');

    lcd_gotoxy(1,fila);
}
////////////////////////////////////////////////////////////////////////////////////
// Función que coloca el puntero de la CGRAM para definir nuevos caracteres       //
////////////////////////////////////////////////////////////////////////////////////
void lcd_set_cgram(char cgram_p)
{
   lcd_send_byte(0,0x40|cgram_p);
}