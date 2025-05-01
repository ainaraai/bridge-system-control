

//PROXIMDIAD

#define PROX  0x02  /* LSE as RTC clock */
//#define GO 	0x04

/* LSE as RTC clock */


//PROBLEMAS

#define TEMP  0x08
//#define PRES  0x07
#define GAS 	0x10
#define PIEZO 	0x800
//ESTADOS PRINCPAL

#define COMPRUEBA 	0x100
#define FUERA		0x40
#define ALARMA	0x80
#define ALERTA	0x04
#define CONSUMO 0x20

//PARADO
#define ZSTOP 	0x400

//TELCADO
#define TEC 0x20
//SALIR
//#define SALIR 	0x800


//out FLAGS (PRINCIPAL)
//#define READY											0x1000				//flag to indicate that flash is ready for operation
#define READY									0x200			//flag to indicate the final action is completed (you can only use READY)

