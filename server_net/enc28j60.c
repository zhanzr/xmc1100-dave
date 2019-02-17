#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include <xmc1100.h>
#include <xmc_spi.h>
#include <xmc_gpio.h>

#include "enc28j60.h"

static uint8_t g_current_enc_bank;
static uint32_t NextPacketPtr;

extern void HAL_Delay(uint32_t d);

#if(0 != SOFT_SPI)
#warning use soft spi

void SCLK_H(void){
	XMC_GPIO_SetOutputHigh(XMC_GPIO_PORT0, 8);
}

void SCLK_L(void){
	XMC_GPIO_SetOutputLow(XMC_GPIO_PORT0, 8);
}

void MOSI_H(void){
	XMC_GPIO_SetOutputHigh(XMC_GPIO_PORT1, 0);
}

void MOSI_L(void){
	XMC_GPIO_SetOutputLow(XMC_GPIO_PORT1, 0);
}

uint32_t MISO_STAT(void){
	return XMC_GPIO_GetInput(XMC_GPIO_PORT1, 1);
}

void ENC28J60_CSH(void){
	XMC_GPIO_SetOutputHigh(XMC_GPIO_PORT0, 9);
}

void ENC28J60_CSL(void){
	XMC_GPIO_SetOutputLow(XMC_GPIO_PORT0, 9);
}

#define	BIT_DELAY	1
static inline void bit_delay(uint32_t d){
	for(uint32_t i=0; i<d; ++i){
		__NOP();
	}
}

static inline void spi_xfer(uint8_t *dout, uint8_t *din){
	uint8_t tmpdin  = 0;
	uint8_t tmpdout = *dout;

	for(uint8_t j = 0; j < 8; j++) {
		SCLK_L();
		(tmpdout & 0x80)?MOSI_H():MOSI_L();
		bit_delay(BIT_DELAY);
		SCLK_H();
		bit_delay(BIT_DELAY);
		tmpdin  <<= 1;
		tmpdin   |= MISO_STAT();
		tmpdout <<= 1;
	}

	*din = tmpdin;

	//SPI wants the clock left low for idle
	SCLK_L();

	return;
}

void spi_write(uint8_t* p_raw, uint16_t len){
	uint8_t dummy;
	for(uint16_t n=0; n<len; ++n){
		spi_xfer(p_raw+n, &dummy);
	}
	return;
}

void spi_read(uint8_t* p_buf, uint16_t len){
	uint8_t dummy = 0xff;

	for(uint16_t n=0; n<len; ++n){
		spi_xfer(&dummy, p_buf+n);
	}
	return;
}

void spi_init(void){
	//CS -> P0.9
	XMC_GPIO_SetMode(XMC_GPIO_PORT0, 9, XMC_GPIO_MODE_OUTPUT_PUSH_PULL);
	//SCLK -> P0.8
	XMC_GPIO_SetMode(XMC_GPIO_PORT0, 8, XMC_GPIO_MODE_OUTPUT_PUSH_PULL);

	//MOSI -> P1.0
	XMC_GPIO_SetMode(XMC_GPIO_PORT1, 0, XMC_GPIO_MODE_OUTPUT_PUSH_PULL);

	//MISO -> P1.1
	XMC_GPIO_SetMode(XMC_GPIO_PORT1, 1, XMC_GPIO_MODE_INPUT_TRISTATE);
	//	XMC_GPIO_SetMode(XMC_GPIO_PORT1, 1, XMC_GPIO_MODE_OUTPUT_PUSH_PULL);

	ENC28J60_CSH();
	SCLK_L();
	MOSI_H();
}

#else
void ENC28J60_CSL(void){
	XMC_SPI_CH_EnableSlaveSelect(XMC_SPI0_CH0, XMC_SPI_CH_SLAVE_SELECT_0);
}

void ENC28J60_CSH(void){
	XMC_SPI_CH_DisableSlaveSelect(XMC_SPI0_CH0);
}

void spi_write(uint8_t raw){
	/*Sending a byte*/
	XMC_SPI_CH_Transmit(XMC_SPI0_CH0, raw, XMC_SPI_CH_MODE_STANDARD);
	//uint16_t XMC_SPI_CH_GetReceivedData(XMC_USIC_CH_t *const channel);
	/*Wait till the byte has been transmitted*/
	while((XMC_SPI_CH_GetStatusFlag(XMC_SPI0_CH0) & XMC_SPI_CH_STATUS_FLAG_TRANSMIT_SHIFT_INDICATION) == 0U);
	XMC_SPI_CH_ClearStatusFlag(XMC_SPI0_CH0, XMC_SPI_CH_STATUS_FLAG_TRANSMIT_SHIFT_INDICATION);
}

uint8_t spi_read(uint8_t dummy){
	XMC_SPI_CH_Receive(XMC_SPI0_CH0, XMC_SPI_CH_MODE_STANDARD);
	/*Wait till the byte has been transmitted*/
	while((XMC_SPI_CH_GetStatusFlag(XMC_SPI0_CH0) & XMC_SPI_CH_STATUS_FLAG_TRANSMIT_SHIFT_INDICATION) == 0U);
	XMC_SPI_CH_ClearStatusFlag(XMC_SPI0_CH0, XMC_SPI_CH_STATUS_FLAG_TRANSMIT_SHIFT_INDICATION);
	//	while((XMC_SPI_CH_GetStatusFlag(XMC_SPI0_CH0) & XMC_SPI_CH_STATUS_FLAG_ALTERNATIVE_RECEIVE_INDICATION) == 0U);
	//	XMC_SPI_CH_ClearStatusFlag(XMC_SPI0_CH0, XMC_SPI_CH_STATUS_FLAG_ALTERNATIVE_RECEIVE_INDICATION);
	//	while((XMC_SPI_CH_GetStatusFlag(XMC_SPI0_CH0) & XMC_SPI_CH_STATUS_FLAG_RECEIVE_INDICATION) == 0U);
	//	XMC_SPI_CH_ClearStatusFlag(XMC_SPI0_CH0, XMC_SPI_CH_STATUS_FLAG_RECEIVE_INDICATION);

	uint16_t ret16 = XMC_SPI_CH_GetReceivedData(XMC_SPI0_CH0);
	if(ret16 != (ret16&0x00ff)){
		printf("%04X ", ret16);
	}
	return (uint8_t)ret16;
}

XMC_GPIO_CONFIG_t rx_pin_config;
XMC_GPIO_CONFIG_t tx_pin_config;
XMC_GPIO_CONFIG_t selo_pin_config;
XMC_GPIO_CONFIG_t clk_pin_config;

/**
 * @brief SPI configuration structure
 */
XMC_SPI_CH_CONFIG_t spi_config =
{
		.baudrate = 5000,
		.bus_mode = XMC_SPI_CH_BUS_MODE_MASTER,
		.selo_inversion = XMC_SPI_CH_SLAVE_SEL_INV_TO_MSLS,
		.parity_mode = XMC_USIC_CH_PARITY_MODE_NONE
};

void spi_init(void){
	//	//CS -> P0.9
	//	XMC_GPIO_SetMode(XMC_GPIO_PORT0, 9, XMC_GPIO_MODE_OUTPUT_PUSH_PULL);
	//	//SCLK -> P0.8
	//	XMC_GPIO_SetMode(XMC_GPIO_PORT0, 8, XMC_GPIO_MODE_OUTPUT_PUSH_PULL);
	//
	//	//MOSI -> P1.0
	//	XMC_GPIO_SetMode(XMC_GPIO_PORT1, 0, XMC_GPIO_MODE_OUTPUT_PUSH_PULL);

	//	//MISO -> P1.1
	////	XMC_GPIO_SetMode(XMC_GPIO_PORT1, 1, XMC_GPIO_MODE_INPUT_TRISTATE);
	//	XMC_GPIO_SetMode(XMC_GPIO_PORT1, 1, XMC_GPIO_MODE_OUTPUT_PUSH_PULL);

	/* Initialize and Start SPI */
	XMC_SPI_CH_Init(XMC_SPI0_CH0, &spi_config);
	XMC_SPI_CH_SetInputSource(XMC_SPI0_CH0, XMC_SPI_CH_INPUT_DIN0, USIC0_C0_DX0_P1_1);
	XMC_SPI_CH_SetBitOrderMsbFirst(XMC_SPI0_CH0);
	XMC_SPI_CH_SetSlaveSelectDelay(XMC_SPI0_CH0, 1);
	XMC_SPI_CH_EnableInterwordDelay(XMC_SPI0_CH0);
	XMC_SPI_CH_SetInterwordDelaySCLK(XMC_SPI0_CH0, 1);

	/* GPIO Input pin configuration */
	rx_pin_config.mode = XMC_GPIO_MODE_INPUT_TRISTATE;
	XMC_GPIO_Init(XMC_GPIO_PORT1,1, &rx_pin_config);

	/* GPIO Output pin configuration */
	tx_pin_config.mode = XMC_GPIO_MODE_OUTPUT_PUSH_PULL_ALT7;
	XMC_GPIO_Init(XMC_GPIO_PORT1,0, &tx_pin_config);

	/* GPIO Slave Select line pin configuration */
	selo_pin_config.mode = XMC_GPIO_MODE_OUTPUT_PUSH_PULL_ALT6;
	XMC_GPIO_Init(XMC_GPIO_PORT0,9, &selo_pin_config);

	/* GPIO Clock pin configuration */
	clk_pin_config.mode = XMC_GPIO_MODE_OUTPUT_PUSH_PULL_ALT6;
	XMC_GPIO_Init(XMC_GPIO_PORT0,8, &clk_pin_config);

	XMC_SPI_CH_Start(XMC_SPI0_CH0);
}

#endif	//#if(0 != SOFT_SPI)

static uint8_t enc28j60ReadOp(uint8_t op, uint8_t address){
	uint8_t dat;

	ENC28J60_CSL();

	dat = op | (address & ADDR_MASK);
	spi_write(&dat, 1);

	dat = 0xff;
	spi_read(&dat, 1);

	// skip dummy read if needed (for mac and mii, see datasheet page 29)
	if(address & SPRD_MASK){
		spi_read(&dat, 1);
	}

	ENC28J60_CSH();
	return dat;
}

static void enc28j60WriteOp(uint8_t op, uint8_t address, uint8_t data){
	uint8_t dat;

	ENC28J60_CSL();

	dat = op | (address & ADDR_MASK);
	spi_write(&dat, 1);

	dat = data;
	spi_write(&dat, 1);

	ENC28J60_CSH();
}

static void enc28j60ReadBuffer(uint32_t len, uint8_t* buf){
	uint8_t dat;

	ENC28J60_CSL();

	dat = ENC28J60_READ_BUF_MEM;
	spi_write(&dat, 1);

	spi_read(buf, len);

	*(buf+len)=0;

	ENC28J60_CSH();
}

static void enc28j60WriteBuffer(uint32_t len, uint8_t* buf){
	uint8_t dat;

	ENC28J60_CSL();

	dat = ENC28J60_WRITE_BUF_MEM;
	spi_write(&dat, 1);

	spi_write(buf, len);

	ENC28J60_CSH();
}

static void enc28j60SetBank(uint8_t address){
	// set the bank (if needed)
	if((address & BANK_MASK) != g_current_enc_bank){
		enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1|ECON1_BSEL0));
		enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, (address & BANK_MASK)>>5);
		g_current_enc_bank = (address & BANK_MASK);
	}
}

static uint8_t enc28j60Read(uint8_t address){
	// set the bank
	enc28j60SetBank(address);
	// do the read
	return enc28j60ReadOp(ENC28J60_READ_CTRL_REG, address);
}

static void enc28j60Write(uint8_t address, uint8_t data){
	// set the bank
	enc28j60SetBank(address);
	// do the write
	enc28j60WriteOp(ENC28J60_WRITE_CTRL_REG, address, data);
}

static void enc28j60PhyWrite(uint8_t address, uint32_t data){
	// set the PHY register address
	enc28j60Write(MIREGADR, address);
	// write the PHY data
	enc28j60Write(MIWRL, data);
	enc28j60Write(MIWRH, data>>8);
	// wait until the PHY write completes
	while(enc28j60Read(MISTAT) & MISTAT_BUSY){
		__NOP();
	}
}

uint16_t enc28j60_phy_read(uint8_t address){
	// Set the right address and start the register read operation
	enc28j60Write(MIREGADR, address);
	enc28j60Write(MICMD, MICMD_MIIRD);

	// wait until the PHY read completes
	while(enc28j60Read(MISTAT) & MISTAT_BUSY){
		__NOP();
	}

	// reset reading bit
	enc28j60Write(MICMD, 0x00);

	uint16_t dat16 = enc28j60Read(MIRDH);
	uint8_t dat8 = enc28j60Read(MIRDL);
	return (dat16<<8)|dat8;
}

static void enc28j60clkout(uint8_t clk){
	//111 = Reserved for factory test. Do not use. Glitch prevention not assured.
	//110 = Reserved for factory test. Do not use. Glitch prevention not assured.
	//101 = CLKOUT outputs main clock divided by 8 (3.125 MHz)
	//100 = CLKOUT outputs main clock divided by 4 (6.25 MHz)
	//011 = CLKOUT outputs main clock divided by 3 (8.333333 MHz)
	//010 = CLKOUT outputs main clock divided by 2 (12.5 MHz)
	//001 = CLKOUT outputs main clock divided by 1 (25 MHz)
	//000 = CLKOUT is disabled. The pin is driven low.	
	enc28j60Write(ECOCON, clk & 0x7);
}

void enc28j60Init(const uint8_t* macaddr){
	ENC28J60_CSH();	    

	//Soft Reset of the MAC
	enc28j60WriteOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
	HAL_Delay(100);

	NextPacketPtr = RXSTART_INIT;

	enc28j60Write(ERXSTL, RXSTART_INIT&0xFF);	 
	enc28j60Write(ERXSTH, RXSTART_INIT>>8);

	// set receive pointer address
	enc28j60Write(ERXRDPTL, RXSTART_INIT&0xFF);
	enc28j60Write(ERXRDPTH, RXSTART_INIT>>8);
	// RX end
	enc28j60Write(ERXNDL, RXSTOP_INIT&0xFF);
	enc28j60Write(ERXNDH, RXSTOP_INIT>>8);
	// TX start	  1500
	enc28j60Write(ETXSTL, TXSTART_INIT&0xFF);
	enc28j60Write(ETXSTH, TXSTART_INIT>>8);
	// TX end
	enc28j60Write(ETXNDL, TXSTOP_INIT&0xFF);
	enc28j60Write(ETXNDH, TXSTOP_INIT>>8);
	// do bank 1 stuff, packet filter:
	// For broadcast packets we allow only ARP packtets
	// All other packets should be unicast only for our mac (MAADR)
	//
	// The pattern to match on is therefore
	// Type     ETH.DST
	// ARP      BROADCAST
	// 06 08 -- ff ff ff ff ff ff -> ip checksum for theses bytes=f7f9
	// in binary these poitions are:11 0000 0011 1111
	// This is hex 303F->EPMM0=0x3f,EPMM1=0x30
	enc28j60Write(ERXFCON, ERXFCON_UCEN|ERXFCON_CRCEN|ERXFCON_PMEN);
	enc28j60Write(EPMM0, 0x3f);
	enc28j60Write(EPMM1, 0x30);
	enc28j60Write(EPMCSL, 0xf9);
	enc28j60Write(EPMCSH, 0xf7);
	// do bank 2 stuff
	// enable MAC receive
	enc28j60Write(MACON1, MACON1_MARXEN|MACON1_TXPAUS|MACON1_RXPAUS);
	// bring MAC out of reset
	enc28j60Write(MACON2, 0x00);
	// enable automatic padding to 60bytes and CRC operations
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN|MACON3_FULDPX);
	// set inter-frame gap (non-back-to-back)
	enc28j60Write(MAIPGL, 0x12);
	enc28j60Write(MAIPGH, 0x0C);
	// set inter-frame gap (back-to-back)
	enc28j60Write(MABBIPG, 0x15);

	// Set the maximum packet size which the controller will accept
	// Do not send packets longer than MAX_FRAMELEN:
	enc28j60Write(MAMXFLL, MAX_FRAMELEN&0xFF);	
	enc28j60Write(MAMXFLH, MAX_FRAMELEN>>8);

	// write MAC address
	// NOTE: MAC address in ENC28J60 is byte-backward
	enc28j60Write(MAADR5, macaddr[0]);	
	enc28j60Write(MAADR4, macaddr[1]);
	enc28j60Write(MAADR3, macaddr[2]);
	enc28j60Write(MAADR2, macaddr[3]);
	enc28j60Write(MAADR1, macaddr[4]);
	enc28j60Write(MAADR0, macaddr[5]);

	enc28j60PhyWrite(PHCON1, PHCON1_PDPXMD);

	// no loopback of transmitted frames
	enc28j60PhyWrite(PHCON2, PHCON2_HDLDIS);
	// switch to bank 0
	enc28j60SetBank(ECON1);
	// enable interrutps
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE);
	// enable packet reception
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);

	//enc28j60PhyWrite(PHLCON,0x7a4);
	enc28j60PhyWrite(PHLCON,0x0476);	

	enc28j60clkout(2); 

	printf("MAC Rev: 0x%02X\n", enc28j60getrev());
	printf("MAADR0: %02X\n", enc28j60Read(MAADR0));
	printf("MAADR1: %02X\n", enc28j60Read(MAADR1));
	printf("MAADR2: %02X\n", enc28j60Read(MAADR2));
	printf("MAADR3: %02X\n", enc28j60Read(MAADR3));
	printf("MAADR4: %02X\n", enc28j60Read(MAADR4));
	printf("MAADR5: %02X\n", enc28j60Read(MAADR5));
	printf("PHHID1: %04X\n", enc28j60_phy_read(PHHID1));
	printf("PHHID2: %04X\n", enc28j60_phy_read(PHHID2));
}

// read the revision of the chip:
uint8_t enc28j60getrev(void){
	return(enc28j60Read(EREVID));
}

void enc28j60PacketSend(uint32_t len, uint8_t* packet){
	// Set the write pointer to start of transmit buffer area
	enc28j60Write(EWRPTL, TXSTART_INIT&0xFF);
	enc28j60Write(EWRPTH, TXSTART_INIT>>8);

	// Set the TXND pointer to correspond to the packet size given
	enc28j60Write(ETXNDL, (TXSTART_INIT+len)&0xFF);
	enc28j60Write(ETXNDH, (TXSTART_INIT+len)>>8);

	// write per-packet control byte (0x00 means use macon3 settings)
	enc28j60WriteOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);

	// copy the packet into the transmit buffer
	enc28j60WriteBuffer(len, packet);

	// send the contents of the transmit buffer onto the network
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);

	// Reset the transmit logic problem. See Rev. B4 Silicon Errata point 12.
	if( (enc28j60Read(EIR) & EIR_TXERIF) )
	{
		enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRTS);
	}
}

// Gets a packet from the network receive buffer, if one is available.
// The packet will by headed by an ethernet header.
//      maxlen  The maximum acceptable length of a retrieved packet.
//      packet  Pointer where packet data should be stored.
// Returns: Packet length in bytes if a packet was retrieved, zero otherwise.
uint32_t enc28j60PacketReceive(uint32_t maxlen, uint8_t* packet){
	uint32_t rxstat;
	uint32_t len;

	// check if a packet has been received and buffered
	// The above does not work. See Rev. B4 Silicon Errata point 6.
	if( enc28j60Read(EPKTCNT) ==0 ) {
		return(0);
	}

	// Set the read pointer to the start of the received packet	
	enc28j60Write(ERDPTL, (NextPacketPtr));
	enc28j60Write(ERDPTH, (NextPacketPtr)>>8);

	// read the next packet pointer
	NextPacketPtr  = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
	NextPacketPtr |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0)<<8;

	// read the packet length (see datasheet page 43)
	len  = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
	len |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0)<<8;

	len-=4; //remove the CRC count

	// read the receive status (see datasheet page 43)
	rxstat  = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
	rxstat |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0)<<8;

	// limit retrieve length
	if (len>maxlen-1)
	{
		len=maxlen-1;
	}

	// check CRC and symbol errors (see datasheet page 44, table 7-3):
	// The ERXFCON.CRCEN is set by default. Normally we should not
	// need to check this.
	if ((rxstat & 0x80)==0)
	{
		// invalid
		len=0;
	}
	else
	{
		// copy the packet from the receive buffer
		enc28j60ReadBuffer(len, packet);
	}

	// Move the RX read pointer to the start of the next received packet
	// This frees the memory we just read out
	enc28j60Write(ERXRDPTL, (NextPacketPtr));
	enc28j60Write(ERXRDPTH, (NextPacketPtr)>>8);

	// decrement the packet counter indicate we are done with this packet
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);

	return(len);
}
