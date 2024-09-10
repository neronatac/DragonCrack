/*
 * platform.c
 *
 *  Created on: 13 mars 2023
 *      Author: s.marcuzzi
 */

#include "platform.h"
#include "xil_cache.h"
#include "xstatus.h"
#include "xscutimer.h"
#include "xscugic.h"
#include "netif/xadapter.h"


#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define TIMER_DEVICE_ID		XPAR_SCUTIMER_DEVICE_ID
#define INTC_BASE_ADDR		XPAR_SCUGIC_0_CPU_BASEADDR
#define INTC_DIST_BASE_ADDR	XPAR_SCUGIC_0_DIST_BASEADDR
#define TIMER_IRPT_INTR		XPAR_SCUTIMER_INTR

#define RESET_RX_CNTR_LIMIT	400


extern struct netif* p_server_netif;


static XScuTimer TimerInstance;
volatile int TcpFastTmrFlag = 0;
volatile int TcpSlowTmrFlag = 0;
static int ResetRxCntr = 0;


/**
 * Initializes the platform.
 * Must be done at the beginning of the program.
 */
void init_platform()
{
	platform_setup_timer();
	platform_setup_interrupts();

	return;
}


/**
 * Cleanups the platform.
 * Must be done at the end of the program.
 */
void cleanup_platform()
{
	Xil_ICacheDisable();
	Xil_DCacheDisable();
	return;
}


/**
 * Setups the timers for Ethernet handling.
 */
void platform_setup_timer()
{
	int Status = XST_SUCCESS;
	XScuTimer_Config *ConfigPtr;
	int TimerLoadValue = 0;

	ConfigPtr = XScuTimer_LookupConfig(TIMER_DEVICE_ID);
	Status = XScuTimer_CfgInitialize(&TimerInstance, ConfigPtr,
			ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {

		xil_printf("In %s: Scutimer Cfg initialization failed...\r\n",
		__func__);
		return;
	}

	Status = XScuTimer_SelfTest(&TimerInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("In %s: Scutimer Self test failed...\r\n",
		__func__);
		return;

	}

	XScuTimer_EnableAutoReload(&TimerInstance);
	/*
	 * Set for 250 milli seconds timeout.
	 */
	TimerLoadValue = XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ / 8;

	XScuTimer_LoadTimer(&TimerInstance, TimerLoadValue);
	return;
}


/**
 * Setups the interrupts for Ethernet handling.
 */
void platform_setup_interrupts()
{
	Xil_ExceptionInit();

	XScuGic_DeviceInitialize(INTC_DEVICE_ID);

	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
			(Xil_ExceptionHandler)XScuGic_DeviceInterruptHandler,
			(void *)INTC_DEVICE_ID);
	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	XScuGic_RegisterHandler(INTC_BASE_ADDR, TIMER_IRPT_INTR,
					(Xil_ExceptionHandler)timer_callback,
					(void *)&TimerInstance);
	/*
	 * Enable the interrupt for scu timer.
	 */
	XScuGic_EnableIntr(INTC_DIST_BASE_ADDR, TIMER_IRPT_INTR);

	return;
}


/**
 * Enable some interrupts.
 */
void platform_enable_interrupts()
{
	Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);
	XScuTimer_EnableInterrupt(&TimerInstance);
	XScuTimer_Start(&TimerInstance);
}


/**
 * Callback used by timer in TCP stack.
 */
void timer_callback(XScuTimer * TimerInstance)
{
	static int DetectEthLinkStatus = 0;
	/* we need to call tcp_fasttmr & tcp_slowtmr at intervals specified
	 * by lwIP. It is not important that the timing is absolutely accurate.
	 */
	static int odd = 1;

	DetectEthLinkStatus++;
	TcpFastTmrFlag = 1;

	odd = !odd;
	ResetRxCntr++;

	if (odd) {
		TcpSlowTmrFlag = 1;
	}

	/* For providing an SW alternative for the SI #692601. Under heavy
	 * Rx traffic if at some point the Rx path becomes unresponsive, the
	 * following API call will ensures a SW reset of the Rx path. The
	 * API xemacpsif_resetrx_on_no_rxdata is called every 100 milliseconds.
	 * This ensures that if the above HW bug is hit, in the worst case,
	 * the Rx path cannot become unresponsive for more than 100
	 * milliseconds.
	 */
	if (ResetRxCntr >= RESET_RX_CNTR_LIMIT) {
		xemacpsif_resetrx_on_no_rxdata(p_server_netif);
		ResetRxCntr = 0;
	}
	/* For detecting Ethernet phy link status periodically */
	if (DetectEthLinkStatus == ETH_LINK_DETECT_INTERVAL) {
		eth_link_detect(p_server_netif);
		DetectEthLinkStatus = 0;
	}

	XScuTimer_ClearInterruptStatus(TimerInstance);
}
