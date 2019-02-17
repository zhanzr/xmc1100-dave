#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include <xmc1100.h>
#include <xmc_spi.h>

#include "enc28j60.h"

static uint8_t Enc28j60Bank;
static uint32_t NextPacketPtr;

extern void HAL_Delay(uint32_t d);

void spi_write_byte(uint8_t* pbuf, uint32_t len){
	ENC28J60_CSL();

	for(uint32_t i=0; i<len; ++i){
		/*Sending a byte*/
		XMC_SPI_CH_Transmit(XMC_SPI0_CH0, *(pbuf+i), XMC_SPI_CH_MODE_STANDARD);
		/*Wait till the byte has been transmitted*/
		while((XMC_SPI_CH_GetStatusFlag(XMC_SPI0_CH0) & XMC_SPI_CH_STATUS_FLAG_TRANSMIT_SHIFT_INDICATION) == 0U);
		XMC_SPI_CH_ClearStatusFlag(XMC_SPI0_CH0, XMC_SPI_CH_STATUS_FLAG_TRANSMIT_SHIFT_INDICATION);
	}

	ENC28J60_CSH();
}

void spi_read_byte(uint8_t* pbuf, uint32_t len){
	ENC28J60_CSL();

	for(uint32_t i=0; i<len; ++i){
		/*Rcving a byte*/
		*(pbuf+i) = (uint8_t)XMC_SPI_CH_GetReceivedData(XMC_SPI0_CH0);
//		/*Wait till the byte has been rcving*/
//		while((XMC_SPI_CH_GetStatusFlag(XMC_SPI0_CH0) & XMC_SPI_CH_STATUS_FLAG_RECEIVE_INDICATION) == 0U);
		XMC_SPI_CH_ClearStatusFlag(XMC_SPI0_CH0, XMC_SPI_CH_STATUS_FLAG_RECEIVE_INDICATION);
	}

	ENC28J60_CSH();
}

static uint8_t enc28j60ReadOp(uint8_t op, uint8_t address){
	uint8_t dat;

	ENC28J60_CSL();

	dat = op | (address & ADDR_MASK);

	spi_write_byte(&dat, 1);
	spi_read_byte(&dat, 1);

	// do dummy read if needed (for mac and mii, see datasheet page 29)
	if(address & 0x80){
		spi_read_byte(&dat, 1);
	}
	// release CS
	ENC28J60_CSH();
	return dat;
}

static void enc28j60WriteOp(uint8_t op, uint8_t address, uint8_t data){
	uint8_t dat;

	ENC28J60_CSL();
	// issue write command
	dat = op | (address & ADDR_MASK);
	spi_write_byte(&dat, 1);
	// write data
	dat = data;
	spi_write_byte(&dat, 1);
	ENC28J60_CSH();
}

static void enc28j60ReadBuffer(uint32_t len, uint8_t* buf){
	uint8_t dat;

	ENC28J60_CSL();
	// issue read command
	dat = ENC28J60_READ_BUF_MEM;
	spi_write_byte(&dat, 1);

	spi_read_byte(buf, len);

	*(buf+len)=0;

	ENC28J60_CSH();
}

static void enc28j60WriteBuffer(uint32_t len, uint8_t* buf){
	uint8_t dat;

	ENC28J60_CSL();
	// issue write command
	dat = ENC28J60_WRITE_BUF_MEM;
	spi_write_byte(&dat, 1);

	spi_read_byte(buf, len);

	ENC28J60_CSH();
}

static void enc28j60SetBank(uint8_t address){
	// set the bank (if needed)
	if((address & BANK_MASK) != Enc28j60Bank){
		// set the bank
		enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1|ECON1_BSEL0));
		enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, (address & BANK_MASK)>>5);
		Enc28j60Bank = (address & BANK_MASK);
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
	HAL_Delay(200);

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
