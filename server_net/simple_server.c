#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include <XMC1100.h>

#include "enc28j60.h"
#include "ip_arp_udp_tcp.h"
#include "net.h"

extern void LD2_ON(void);
extern void LD2_OFF(void);
extern volatile uint16_t g_adc_buf[2];

extern const uint8_t g_ip_addr[4];
extern const uint8_t g_mac_addr[6];

#define TEST_UDP_PORT	((uint16_t)1200)
#define BUFFER_SIZE 1500

static uint8_t buf[BUFFER_SIZE+1];

static uint8_t g_led_state;

// takes a string of the form password/commandNumber and analyse it
// return values: -1 invalid password, otherwise command number
//                -2 no command given but password valid
signed char analyse_get_url(char *str){
	uint8_t i=0;

	// find first "/"
	// passw not longer than 9 char:
	while(*str && i<10 && *str >',' && *str<'{')
	{
		if (*str=='/')
		{
			str++;
			break;
		}
		i++;
		str++;
	}
	if (*str < 0x3a && *str > 0x2f)
	{
		// is a ASCII number, return it
		return(*str-0x30);
	}
	return(-2);
}

// prepare the webpage by writing the data to the tcp send buffer
uint32_t print_webpage(uint8_t *buf,uint8_t on_off)
{
	uint32_t plen;
	uint8_t tmp_compose_buf[64];
			
	plen=fill_tcp_data_p(buf,0,("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nPragma: no-cache\r\n\r\n"));
	
	plen=fill_tcp_data_p(buf,plen,("<!DOCTYPE html>\r\n<html lang=\"en\">\r\n"));
	
	sprintf((char*)tmp_compose_buf, "<title>Server[%s]</title>\r\n", __TIME__);
	plen=fill_tcp_data(buf,plen,(const char*)tmp_compose_buf);		
	
	plen=fill_tcp_data_p(buf,plen,("<body>\r\n"));
	plen=fill_tcp_data_p(buf,plen,("<center>\r\n"));
	
	plen=fill_tcp_data_p(buf,plen,("<p>Tempeature and IntRef: "));
//	sprintf((char*)tmp_compose_buf, "%u %u mV\r\n", g_adc_buf[0], (g_adc_buf[1]*VDD_VALUE)/4095);
//	plen=fill_tcp_data(buf,plen,(const char*)tmp_compose_buf);
	plen=fill_tcp_data_p(buf,plen,("<p>Provided by Cortex M board\r\n"));

	sprintf((char*)tmp_compose_buf, "<p>system Ticks:%u LED State:%u\r\n",					
	SysTick->VAL,
	g_led_state);
	plen=fill_tcp_data(buf,plen,(const char*)tmp_compose_buf);		
	
	sprintf((char*)tmp_compose_buf, "<p>MAC Rev: 0x%02X\r\n", enc28j60getrev());
	plen=fill_tcp_data(buf,plen,(const char*)tmp_compose_buf);				
	
	plen=fill_tcp_data_p(buf,plen,("\r\n<p>LED Status:"));
	if (on_off){
		plen=fill_tcp_data_p(buf,plen,("<font color=\"#00FF00\"> On</font>"));
	}else{
		plen=fill_tcp_data_p(buf,plen,("Off"));
	}

	plen=fill_tcp_data_p(buf,plen,("<a href=\""));
	sprintf((char*)tmp_compose_buf, "http://%u.%u.%u.%u/", 
	g_ip_addr[0],g_ip_addr[1],g_ip_addr[2],g_ip_addr[3]);
	plen=fill_tcp_data(buf,plen, (const char*)tmp_compose_buf);
	plen=fill_tcp_data_p(buf,plen,("\">[Refresh]</a>\r\n<p><a href=\""));

	plen=fill_tcp_data(buf,plen, (const char*)tmp_compose_buf);
	if (on_off){
		plen=fill_tcp_data_p(buf,plen,("0\">LED Off</a><p>"));
	}else{
		plen=fill_tcp_data_p(buf,plen,("1\">LED On</a><p>"));
	}

	plen=fill_tcp_data_p(buf,plen,("<hr><p>F767 Board Simple Server\r\n"));

	plen=fill_tcp_data_p(buf,plen,("</center>\r\n"));
	plen=fill_tcp_data_p(buf,plen,("</body>\r\n"));
	plen=fill_tcp_data_p(buf,plen,("</html>\r\n"));	
	
	return(plen);
}

void protocol_init(void){
	//using static configuration now
}

void server_loop(void){  
	uint32_t plen,dat_p,i1=0,payloadlen=0;
	char *buf1 = 0;
	signed char cmd;

	//printf("MAC:%02X,%02X,%02X,%02X,%02X,%02X\n",
//	g_mac_addr[0],g_mac_addr[1],g_mac_addr[2],g_mac_addr[3],g_mac_addr[4],g_mac_addr[5]);
	//printf("IP:%d.%d.%d.%d\n",
//	g_ip_addr[0],g_ip_addr[1],g_ip_addr[2],g_ip_addr[3]);
	//printf("Port:%d\n",HTTP_PORT);

	//init the ethernet/ip layer:
	while(true){
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
			//printf("$");
			continue;
		}
		
		//Process ICMP packet
		if(buf[IP_PROTO_P]==IP_PROTO_ICMP_V && buf[ICMP_TYPE_P]==ICMP_TYPE_ECHOREQUEST_V){

			//printf("Rcvd ICMP from [%d.%d.%d.%d]",buf[ETH_ARP_SRC_IP_P],buf[ETH_ARP_SRC_IP_P+1],
//					buf[ETH_ARP_SRC_IP_P+2],buf[ETH_ARP_SRC_IP_P+3]);
			make_echo_reply_from_request(buf, plen);
			continue;
		}

		//Process TCP packet with port 80
		if (buf[IP_PROTO_P]==IP_PROTO_TCP_V&&buf[TCP_DST_PORT_H_P]==0&&buf[TCP_DST_PORT_L_P]==HTTP_PORT){
			//printf("\n\rF767 Rcvd TCP[80] packet");
			if (buf[TCP_FLAGS_P] & TCP_FLAGS_SYN_V)
			{
				//printf("Type SYN");
				make_tcp_synack_from_syn(buf);
				continue;
			}
			if (buf[TCP_FLAGS_P] & TCP_FLAGS_ACK_V)
			{
				//printf("Type ACK");
				init_len_info(buf); // init some data structures
				dat_p=get_tcp_data_pointer();
				if (dat_p==0)
				{
					if (buf[TCP_FLAGS_P] & TCP_FLAGS_FIN_V)
					{
						make_tcp_ack_from_any(buf);
					}
					continue;
				}
					// Process Telnet request
				if (strncmp("GET ",(char *)&(buf[dat_p]),4)!=0){
					plen=fill_tcp_data_p(buf,0,("F767\r\n\n\rHTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>200 OK</h1>"));
					goto SENDTCP;
				}
					//Process HTTP Request
				if (strncmp("/ ",(char *)&(buf[dat_p+4]),2)==0){
					
					//Update Web Page Content
					plen=print_webpage(buf, g_led_state);

					goto SENDTCP;
				}
				
				//Analysis the command in the URL
				cmd=analyse_get_url((char *)&(buf[dat_p+5]));
				if (cmd==-1){
					plen=fill_tcp_data_p(buf,0,("HTTP/1.0 401 Unauthorized\r\nContent-Type: text/html\r\n\r\n<h1>401 Unauthorized</h1>"));
					goto SENDTCP;
				}
				if (cmd==1)	{
					LD2_ON();
					g_led_state=1;
				}

				if (cmd==0){
					LD2_OFF();
					g_led_state=0;
				}

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
