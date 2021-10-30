#include <conf_usart_serial.h>
#include <asf.h>
#include <twi_megarf.h>
#include <ioport.h>
#include <avr/io.h>
#include <util/delay.h>


const uint8_t config_data[] = {0x60};

typedef struct temp_ctx_t{
	uint8_t read_data[2];
} TEMP_CTX_T;


void sensor_config (void);
uint8_t* read_temperature (void);
void twi_init (void);

TEMP_CTX_T temp_ctx;

// Puntero para guardar la temperatura medida
uint8_t *pData;


#define TWI_MASTER           &TWBR
#define TWI_SPEED            125000
#define TWI_SLAVE_ADDR       0x96
#define SLAVE_MEM_ADDR      0x01
#define SLAVE_MEM_ADDR_LENGTH   TWI_SLAVE_ONE_BYTE_SIZE
#define DATA_LENGTH  sizeof(config_data)



void sensor_config (void){
	//Paquete para la configuración del sensor
	twi_package_t packet = {
		.addr[0] = (uint8_t) SLAVE_MEM_ADDR,
		.addr_length = (uint8_t)SLAVE_MEM_ADDR_LENGTH,
		.chip = TWI_SLAVE_ADDR,
		.buffer = (void *)config_data,
		.length = DATA_LENGTH
	};
	//Escribe la configuración en el sensor de temperatura
	while (twi_master_write(TWI_MASTER,&packet) != TWI_SUCCESS) {
	}
	//Pequeño delay para que el sensor aploque la configuración
	delay_ms(5);
}


// Lectura de temperatura
uint8_t* read_temperature (void){
	uint8_t received_data[2] = {0, 0};
		
		
	/* configures the TWI read packet*/
	twi_package_t packet_received = {
		.addr[0] = 0x00,
		.addr_length = (uint8_t)SLAVE_MEM_ADDR_LENGTH,
		.chip = TWI_SLAVE_ADDR,
		.buffer = received_data,
		.length = 2,
	};
	/* Perform a multi-byte read access*/
	while (twi_master_read(TWI_MASTER,&packet_received) != TWI_SUCCESS) {
	}
	temp_ctx.read_data[0] = received_data[0];
	temp_ctx.read_data[1] = received_data[1];
	
	return temp_ctx.read_data;
}



void twi_init (void){
	//Configuración del TWI: Velocidad y direcciçon
	twi_master_options_t m_options = {
		.speed      = TWI_SPEED,
		.chip  = TWI_SLAVE_ADDR,
	};
	m_options.baud_reg = TWI_CLOCK_RATE(sysclk_get_cpu_hz(), m_options.speed);
	//Habilita el reloj del periférico TWI
	sysclk_enable_peripheral_clock(TWI_MASTER);
	//Inicializa el TWI con la biblioteca
	twi_master_init(TWI_MASTER,&m_options);
}


int main(void){
	//Configuración de la USART como serie
	const usart_serial_options_t usart_serial_options = {
		.baudrate   = USART_SERIAL_BAUDRATE,
		.charlength = USART_SERIAL_CHAR_LENGTH,
		.paritytype = USART_SERIAL_PARITY,
		.stopbits   = USART_SERIAL_STOP_BIT,
	};
	board_init();
	
	sysclk_init();
	
	ioport_init();
	//Asifnaión de la USART para las funciones de stdio como printf
	stdio_serial_init(USART_SERIAL, &usart_serial_options);
	twi_init();
	sensor_config();
	
	
	while(1) {
	
		// Lectura de temperatura
		pData = read_temperature();
	
		// LA IMPRIMIMOS
		printf("\n La temperatura es de: %d,%d ºC \n \r",pData[0],pData[1]);
		printf ("\n\r");
	
		delay_ms(1000); // Delay de 1 segundo entre cada lectura
	
	} 	
	return 0;

}
