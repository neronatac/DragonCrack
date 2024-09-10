/*
 * ethernet.c
 *
 *  Created on: 13 mars 2023
 *      Author: s.marcuzzi
 */

#include "ethernet.h"
#include "lwip/tcp.h"
#include "xstatus.h"
#include "xil_types.h"
#include "platform.h"
#include "netif/xadapter.h"
#include "process.h"

/* missing declaration in lwIP */
void lwip_init();
err_t tcp_send_empty_ack(struct tcp_pcb *pcb);

void tcp_fasttmr(void);
void tcp_slowtmr(void);

extern volatile int TcpFastTmrFlag;
extern volatile int TcpSlowTmrFlag;

#define PORT 5017

static struct netif server_netif;
struct netif* p_server_netif;

extern u8 response_buffer[];


/**
 * Initializes the Ethernet peripheral
 * @return
 * 		- XST_SUCCESS if successful
 * 		- another Xstatus if not
 */
XStatus ethernetInit(){
	ip_addr_t ipaddr, netmask, gw;
	/* the mac address of the board. this should be unique per board */
	// TODO: change it for each device
	unsigned char mac_ethernet_address[] = { 0x00, 0x26, 0x32, 0xF0, 0xAD, 0xEC };

	p_server_netif = &server_netif;

	IP4_ADDR(&ipaddr,  192, 168,  20, 42);
	IP4_ADDR(&netmask, 255, 255, 255,  0);
	IP4_ADDR(&gw,      192, 168,  20,  1);

	lwip_init();

	/* Add network interface to the netif_list, and set it as default */
	if (!xemac_add(&server_netif, &ipaddr, &netmask,
						&gw, mac_ethernet_address,
						PLATFORM_EMAC_BASEADDR)) {
		xil_printf("Error adding N/W interface\n\r");
		return XST_FAILURE;
	}
	netif_set_default(p_server_netif);

	/* now enable interrupts */
	platform_enable_interrupts();

	/* specify that the network if is up */
	netif_set_up(p_server_netif);

	/* start the application (web server, rxtest, txtest, etc..) */
	return start_application();
}



/**
 * Setups TCP application.
 * @return 0 in case of SUCESS
 */
XStatus start_application()
{
	struct tcp_pcb *pcb;
	err_t err;

	/* create new TCP PCB structure */
	pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
	if (!pcb) {
		xil_printf("Error creating PCB. Out of Memory\n\r");
		return XST_FAILURE;
	}

	/* bind to specified @port */
	err = tcp_bind(pcb, IP_ANY_TYPE, PORT);
	if (err != ERR_OK) {
		xil_printf("Unable to bind to port %d: err = %d\n\r", PORT, err);
		return XST_FAILURE;
	}

	/* we do not need any arguments to callback functions */
	tcp_arg(pcb, NULL);

	/* listen for connections */
	pcb = tcp_listen(pcb);
	if (!pcb) {
		xil_printf("Out of memory while tcp_listen\n\r");
		return XST_FAILURE;
	}

	/* specify callback to use for incoming connections */
	tcp_accept(pcb, accept_callback);

	xil_printf("TCP server started @ port %d\n\r", PORT);

	return XST_SUCCESS;
}


/**
 * Called when some data is received over TCP
 * @param arg
 * @param tpcb TCP Control Block
 * @param p Packet
 * @param err
 * @return
 */
err_t recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	/* do not read the packet if we are not in ESTABLISHED state */
	if (!p) {
		tcp_close(tpcb);
		tcp_recv(tpcb, NULL);
		return ERR_OK;
	}

	/* indicate that the packet has been received */
	tcp_recved(tpcb, p->len);

	/* call the process() function */
	process(p->payload);

	/* send response */
	if (tcp_sndbuf(tpcb) > response_buffer[0]+1) {
		err = tcp_write(tpcb, response_buffer, response_buffer[0]+1, 1);
		tcp_output(tpcb);  // send the previous data immediately
	} else
		xil_printf("no space in tcp_sndbuf\n\r");

	/* free the received pbuf */
	pbuf_free(p);

	return ERR_OK;
}

/**
 * Called when an incoming connection arrives.
 * @param arg
 * @param newpcb
 * @param err
 * @return
 */
err_t accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
	static int connection = 1;

	/* set the receive callback for this connection */
	tcp_recv(newpcb, recv_callback);

	/* just use an integer number indicating the connection id as the
	   callback argument */
	tcp_arg(newpcb, (void*)(UINTPTR)connection);

	/* increment for subsequent accepted connections */
	connection++;

	xil_printf("New connection accepted! Total=%d\n\r", connection-1);

	return ERR_OK;
}


/**
 * receives and process packets
 * NEVER ENDS
 */
void ethernet_mainloop()
{
	while (1) {
		if (TcpFastTmrFlag) {
			tcp_fasttmr();
			TcpFastTmrFlag = 0;
		}
		if (TcpSlowTmrFlag) {
			tcp_slowtmr();
			TcpSlowTmrFlag = 0;
		}
		xemacif_input(p_server_netif);
		transfer_data();
	}
}


/**
 * Copied as-is from example.
 * Don't know its role.
 * @return
 */
int transfer_data()
{
	return 0;
}
