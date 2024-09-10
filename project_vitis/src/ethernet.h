/*
 * ethernet.h
 *
 *  Created on: 13 mars 2023
 *      Author: s.marcuzzi
 */

#ifndef SRC_ETHERNET_H_
#define SRC_ETHERNET_H_

#include "xstatus.h"
#include "lwip/tcp.h"


XStatus ethernetInit();
XStatus start_application();
err_t recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
err_t accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err);
void ethernet_mainloop();
int transfer_data();


#endif /* SRC_ETHERNET_H_ */
