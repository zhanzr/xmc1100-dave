#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include <XMC1100.h>
#include <xmc_spi.h>
#include <xmc_gpio.h>

#include "GPIO.h"

#include "enc28j60.h"
#include "ip_arp_udp_tcp.h"
#include "net.h"

extern volatile uint16_t g_adc_buf[2];

extern const uint8_t g_ip_addr[4];
extern const uint8_t g_mac_addr[6];

#define TEST_UDP_PORT	((uint16_t)1200)
#define BUFFER_SIZE 1500

static uint8_t buf[BUFFER_SIZE+1];

int32_t g_tmpK;

const char ON_STR[] = "On";
const char OFF_STR[] = "Off";

void LD_05_ON(void){
	XMC_GPIO_SetOutputLow(XMC_GPIO_PORT0, 5);
}

void LD_05_OFF(void){
	XMC_GPIO_SetOutputHigh(XMC_GPIO_PORT0, 5);
}

#define LED_ON_STAT		0
#define LED_OFF_STAT	1
uint32_t LD_05_Stat(void) {
	return XMC_GPIO_GetInput(XMC_GPIO_PORT0, 5);
}

int8_t analyse_get_url(char *str) {
	char* p_str = str;

	if (0==strncmp(p_str, ON_STR, strlen(ON_STR))) {
		return 1;
	} else if (0==strncmp(p_str, OFF_STR, strlen(OFF_STR))) {
		return 0;
	} else {
		return(-1);
	}
}

// prepare the webpage by writing the data to the tcp send buffer
uint32_t print_webpage(uint8_t *buf){
	uint32_t plen;
	uint8_t tmp_compose_buf[64];
			
	plen=fill_tcp_data_p(buf,0,("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nPragma: no-cache\r\n\r\n"));
	
	plen=fill_tcp_data_p(buf,plen,("<!DOCTYPE html>\r\n<html lang=\"en\">\r\n"));
	
	sprintf((char*)tmp_compose_buf, "<title>Server[%s]</title>\r\n", __TIME__);
	plen=fill_tcp_data(buf,plen,(const char*)tmp_compose_buf);		
	
	plen=fill_tcp_data_p(buf,plen,("<body>\r\n"));
	plen=fill_tcp_data_p(buf,plen,("<center>\r\n"));
	
	plen=fill_tcp_data_p(buf,plen,("<p>Tempeature: "));
	sprintf((char*)tmp_compose_buf, "%i 'C\r\n", g_tmpK);
	plen=fill_tcp_data(buf,plen,(const char*)tmp_compose_buf);
	plen=fill_tcp_data_p(buf,plen,("<p>Provided by Cortex M board\r\n"));

	sprintf((char*)tmp_compose_buf, "<p>system Ticks:%u, soft_spi:%u\r\n",
	SysTick->VAL,
	SOFT_SPI);
	plen=fill_tcp_data(buf,plen,(const char*)tmp_compose_buf);		
	
	sprintf((char*)tmp_compose_buf, "<p>MAC Rev: 0x%02X\r\n", enc28j60getrev());
	plen=fill_tcp_data(buf,plen,(const char*)tmp_compose_buf);				
	
	plen=fill_tcp_data_p(buf,plen,("\r\n<p>LED Status:"));
	if ((LED_ON_STAT == LD_05_Stat())){
		plen=fill_tcp_data_p(buf,plen,("<font color=\"#00FF00\">"));
		plen=fill_tcp_data_p(buf,plen,(ON_STR));
		plen=fill_tcp_data_p(buf,plen,("</font>"));
	} else {
		plen=fill_tcp_data_p(buf,plen,("<font color=\"#FF0000\">"));
		plen=fill_tcp_data_p(buf,plen,(OFF_STR));
		plen=fill_tcp_data_p(buf,plen,("</font>"));
	}

	plen=fill_tcp_data_p(buf,plen,("<a href=\""));
	sprintf((char*)tmp_compose_buf, "http://%u.%u.%u.%u/", 
	g_ip_addr[0],g_ip_addr[1],g_ip_addr[2],g_ip_addr[3]);
	plen=fill_tcp_data(buf,plen, (const char*)tmp_compose_buf);
	plen=fill_tcp_data_p(buf,plen,("\">[Refresh]</a>\r\n<p><a href=\""));

	plen=fill_tcp_data(buf,plen, (const char*)tmp_compose_buf);
	if ((LED_ON_STAT == LD_05_Stat())){
		plen=fill_tcp_data_p(buf,plen,(OFF_STR));
	}else{
		plen=fill_tcp_data_p(buf,plen,(ON_STR));
	}
	plen=fill_tcp_data_p(buf,plen,("\">LED Switch</a><p>"));

	plen=fill_tcp_data_p(buf,plen,("<hr><p>XMC1 Board Simple Server\r\n"));

	plen=fill_tcp_data_p(buf,plen,("</center>\r\n"));
	plen=fill_tcp_data_p(buf,plen,("</body>\r\n"));
	plen=fill_tcp_data_p(buf,plen,("</html>\r\n"));	
	
	return plen;
}

void protocol_init(void){
	//using static configuration now
}

void server_loop(void){  
	uint32_t plen,dat_p,i1=0,payloadlen=0;
	char *buf1 = 0;
	signed char cmd;

	printf("MAC:%02X,%02X,%02X,%02X,%02X,%02X\n",
	g_mac_addr[0],g_mac_addr[1],g_mac_addr[2],g_mac_addr[3],g_mac_addr[4],g_mac_addr[5]);
	printf("IP:%d.%d.%d.%d\n",
	g_ip_addr[0],g_ip_addr[1],g_ip_addr[2],g_ip_addr[3]);
	printf("Port:%d\n",HTTP_PORT);

	//init the ethernet/ip layer:
	while(true){
		g_tmpK = XMC1000_CalcTemperature()-273;

		plen = enc28j60PacketReceive(BUFFER_SIZE, buf);

		if(plen==0){
			continue; 
		}

		//Process ARP Request
		if(eth_type_is_arp_and_my_ip(buf,plen)){
			make_arp_answer_from_request(buf);
			continue;
		}

		//Only Process IP Packet destinated at me
		if(eth_type_is_ip_and_my_ip(buf,plen)==0) {
			printf("$");
			continue;
		}
		
		//Process ICMP packet
		if(buf[IP_PROTO_P]==IP_PROTO_ICMP_V && buf[ICMP_TYPE_P]==ICMP_TYPE_ECHOREQUEST_V){

			printf("Rcvd ICMP from [%d.%d.%d.%d]\n",buf[ETH_ARP_SRC_IP_P],buf[ETH_ARP_SRC_IP_P+1],
					buf[ETH_ARP_SRC_IP_P+2],buf[ETH_ARP_SRC_IP_P+3]);
			make_echo_reply_from_request(buf, plen);
			continue;
		}

		//Process TCP packet with port 80
		if (buf[IP_PROTO_P]==IP_PROTO_TCP_V&&buf[TCP_DST_PORT_H_P]==0&&buf[TCP_DST_PORT_L_P]==HTTP_PORT) {
			printf("XMC1 Rcvd TCP[80] pkt\n");
			if (buf[TCP_FLAGS_P] & TCP_FLAGS_SYN_V) {
				printf("Type SYN\n");
				make_tcp_synack_from_syn(buf);
				continue;
			}
			if (buf[TCP_FLAGS_P] & TCP_FLAGS_ACK_V) {
				printf("Type ACK\n");
				init_len_info(buf); // init some data structures
				dat_p=get_tcp_data_pointer();
				if (dat_p==0) {
					if (buf[TCP_FLAGS_P] & TCP_FLAGS_FIN_V) {
						make_tcp_ack_from_any(buf);
					}
					continue;
				}
					// Process Telnet request
				if (strncmp("GET ",(char *)&(buf[dat_p]),4)!=0){
					plen=fill_tcp_data_p(buf,0,("XMC1\r\n\n\rHTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>200 OK</h1>"));
					goto SENDTCP;
				}
					//Process HTTP Request
				if (strncmp("/ ",(char *)&(buf[dat_p+4]),2)==0) {
					//Update Web Page Content
					plen=print_webpage(buf);

					goto SENDTCP;
				}
				
				//Analysis the command in the URL
				cmd=analyse_get_url((char *)&(buf[dat_p+5]));
				if (cmd < 0) {
					printf("*");
					plen=fill_tcp_data_p(buf,0,("HTTP/1.0 401 Unauthorized\r\nContent-Type: text/html\r\n\r\n<h1>401 Unauthorized</h1>"));
					goto SENDTCP;
				}
				if (cmd==1)	{
					if(LED_ON_STAT != LD_05_Stat()) {
						LD_05_ON();
					}
				} else if (cmd==0) {
					if(LED_OFF_STAT != LD_05_Stat()) {
						LD_05_OFF();
					}
				}
				//Update Web Page Content
				plen=print_webpage(buf);

				SENDTCP:
				// send ack for http get
				make_tcp_ack_from_any(buf); 
				// send data
				make_tcp_ack_with_data(buf,plen); 
				continue;
			}
		}

		//Process UDP Packet with port TEST_UDP_PORT
		if ( (buf[IP_PROTO_P]==IP_PROTO_UDP_V)&&
			(buf[UDP_DST_PORT_H_P]==(TEST_UDP_PORT>>8))&&
		(buf[UDP_DST_PORT_L_P]==(uint8_t)TEST_UDP_PORT) ) {
			payloadlen=	  buf[UDP_LEN_H_P];
			payloadlen=payloadlen<<8;
			payloadlen=(payloadlen+buf[UDP_LEN_L_P])-UDP_HEADER_LEN;

			for(i1=0; i1<payloadlen; i1++){         
				buf1[i1]=buf[UDP_DATA_P+i1];
			}

			make_udp_reply_from_request(buf,buf1,payloadlen, TEST_UDP_PORT);
		}
	}
}
