#include <conf_usart_serial.h>


// Se incluyen las bibliotecas necesarias
#include <asf.h>
#include <twi_megarf.h>
#include <ioport.h>
#include <avr/io.h>
#include <util/delay.h>


const uint8_t conf_data[] = {0x60};

typedef struct temp_ctx_t{
	uint8_t readData[2];
} TEMP_CTX_T;


void sensor_conf (void);
uint8_t* read_temperature (void);
void twi_init (void);

TEMP_CTX_T temp_ctx;

// Definimos la variable de la temperatura sacada del sensor externo
uint8_t *pData;


#define TWI_MASTER           &TWBR
#define TWI_SPEED            125000
#define TWI_SLAVE_ADDR       0x96
#define SLAVE_MEM_ADDR      0x01
#define SLAVE_MEM_ADDR_LENGTH   TWI_SLAVE_ONE_BYTE_SIZE
#define DATA_LENGTH  sizeof(conf_data)



void sensor_conf (void){
	/* configures the TWI configuration packet*/
	twi_package_t packet = {
		.addr[0] = (uint8_t) SLAVE_MEM_ADDR,
		.addr_length = (uint8_t)SLAVE_MEM_ADDR_LENGTH,
		.chip = TWI_SLAVE_ADDR,
		.buffer = (void *)conf_data,
		.length = DATA_LENGTH
	};
	/* Perform a multi-byte write access */
	while (twi_master_write(TWI_MASTER,&packet) != TWI_SUCCESS) {
	}
	/* waits for write completion*/
	delay_ms(5);
}


// FUNCIÓN PARA LEER LA TEMPERATURA DEL SENSOR EXTERNO
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
	temp_ctx.readData[0] = received_data[0];
	temp_ctx.readData[1] = received_data[1];
	
	// SE PUEDE IMPRIMIR POR EL HIPERTERMINAL AQUÍ, PERO LO HAREMOS EN EL MAIN
	return temp_ctx.readData;
}



void twi_init (void){
	/* TWI master initialization options. */
	twi_master_options_t m_options = {
		.speed      = TWI_SPEED,
		.chip  = TWI_SLAVE_ADDR,
	};
	m_options.baud_reg = TWI_CLOCK_RATE(sysclk_get_cpu_hz(), m_options.speed);
	/* Enable the peripheral clock for TWI module */
	sysclk_enable_peripheral_clock(TWI_MASTER);
	/* Initialize the TWI master driver. */
	twi_master_init(TWI_MASTER,&m_options);
}


int main(void){
	const usart_serial_options_t usart_serial_options = {
		.baudrate   = USART_SERIAL_BAUDRATE,
		.charlength = USART_SERIAL_CHAR_LENGTH,
		.paritytype = USART_SERIAL_PARITY,
		.stopbits   = USART_SERIAL_STOP_BIT,
	};
	board_init();
	
	sysclk_init();
	
	ioport_init();
	
	stdio_serial_init(USART_SERIAL, &usart_serial_options);
	twi_init();
	sensor_conf();
	
	
	while(1) {
	
		// LEEMOS LA TEMPERATURA DEL SENSOR EXTERNO
		pData = read_temperature();
	
		// LA IMPRIMIMOS
		printf("\n La temperatura es: %d,%d C \n \r",
		pData[0],pData[1]);
	
		printf ("\n\r");
	
		delay_ms(1000); // Esperamos 1 segundo entre cada impresión
	
	} // FIN WHILE
	
	return 0;

}
