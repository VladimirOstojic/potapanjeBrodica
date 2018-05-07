/*
 * Copyright (c) 2009-2012 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xparameters.h"

#include "xiic.h"
#include "xintc.h"
#include "xil_exception.h"


/////////////////////////////////////
// Master

#define SLAVE_ADDRESS		0x70	/* 0xE0 as an 8 bit number. */
#define RECEIVE_COUNT		25
#define SEND_COUNT		25

#define PAGE_SIZE   16

typedef u8 AddressType;


#define IIC_DEVICE_ID		XPAR_IIC_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define IIC_INTR_ID		XPAR_INTC_0_IIC_0_VEC_ID

/*
 * The following structure contains fields that are used with the callbacks
 * (handlers) of the IIC driver. The driver asynchronously calls handlers
 * when abnormal events occur or when data has been sent or received. This
 * structure must be volatile to work when the code is optimized.
 */
volatile struct {
	int  EventStatus;
	int  RemainingRecvBytes;
	int  RemainingSendBytes;
	int EventStatusUpdated;
	int RecvBytesUpdated;
	int SendBytesUpdated;
} HandlerInfo;

XIic IicInstance;		/* The instance of the IIC device. */
XIntc InterruptController;	/* The instance of the Interrupt Controller */


/*
 * Write buffer for writing a page.
 */
u8 WriteBuffer[sizeof(AddressType) + PAGE_SIZE];

volatile u8 TransmitComplete;
volatile u8 ReceiveComplete;
volatile u8 BusNotBusy;

static void SendHandler(void *CallbackRef, int ByteCount){
	HandlerInfo.RemainingSendBytes = ByteCount;
	HandlerInfo.SendBytesUpdated = TRUE;
}

static void RecvHandler(void *CallbackRef, int ByteCount)
{
	HandlerInfo.RemainingRecvBytes = ByteCount;
	HandlerInfo.RecvBytesUpdated = TRUE;
}

static void StatusHandler(void *CallbackRef, int Status)
{
	HandlerInfo.EventStatus |= Status;
	HandlerInfo.EventStatusUpdated = TRUE;
}

static int SetupInterruptSystem(XIic * IicInstPtr)
{
	int Status;

	if (InterruptController.IsStarted == XIL_COMPONENT_IS_STARTED) {
		return XST_SUCCESS;
	}

	/*
	 * Initialize the interrupt controller driver so that it's ready to use.
	 */
	Status = XIntc_Initialize(&InterruptController, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above
	 * performs the specific interrupt processing for the device.
	 */
	Status = XIntc_Connect(&InterruptController, IIC_INTR_ID,
				   (XInterruptHandler) XIic_InterruptHandler,
				   IicInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller so interrupts are enabled for all
	 * devices that cause interrupts.
	 */
	Status = XIntc_Start(&InterruptController, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupts for the IIC device.
	 */
	XIntc_Enable(&InterruptController, IIC_INTR_ID);

	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				 (Xil_ExceptionHandler) XIntc_InterruptHandler,
				 &InterruptController);

	/*
	 * Enable non-critical exceptions.
	 */
	Xil_ExceptionEnable();


	return XST_SUCCESS;
}


int initIICMaster(u16 IicDeviceId, u8 slaveAddress) {
	int Status;
	XIic_Config *ConfigPtr;	/* Pointer to configuration data */

	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
	ConfigPtr = XIic_LookupConfig(IicDeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XIic_CfgInitialize(&IicInstance, ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Setup handler to process the asynchronous events which occur,
	 * the driver is only interrupt driven such that this must be
	 * done prior to starting the device.
	 */
	XIic_SetRecvHandler(&IicInstance, (void *)&HandlerInfo, RecvHandler);
	XIic_SetSendHandler(&IicInstance, (void *)&HandlerInfo, SendHandler);
	XIic_SetStatusHandler(&IicInstance, (void *)&HandlerInfo,
					StatusHandler);

	/*
	 * Connect the ISR to the interrupt and enable interrupts.
	 */
	Status = SetupInterruptSystem(&IicInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the IIC driver such that it is ready to send and
	 * receive messages on the IIC interface, set the address
	 * to send to which is the temperature sensor address
	 */
	XIic_Start(&IicInstance);
	XIic_SetAddress(&IicInstance, XII_ADDR_TO_SEND_TYPE, slaveAddress);

	return Status;
}

int sendToSlave(void) {
	u8 Index;
	AddressType Address = SLAVE_ADDRESS;

	/*
	 * Initialize the data to write and the read buffer.
	 */
	if (sizeof(Address) == 1) {
		WriteBuffer[0] = (u8) (Address);
	}
	else {
		WriteBuffer[0] = (u8) (Address >> 8);
		WriteBuffer[1] = (u8) (Address);
	}
	for (Index = 0; Index < PAGE_SIZE; Index++) {
		WriteBuffer[sizeof(Address) + Index] = 0xFF;
	}

	IicInstance.Stats.TxErrors = 0;
	/*
	 * Set the defaults.
	 */
	TransmitComplete = 1;

	XIic_MasterSend(&IicInstance, WriteBuffer, PAGE_SIZE);
/*
	while (1) {
		XIic_MasterSend(&IicInstance, WriteBuffer, PAGE_SIZE);

		if ((!TransmitComplete) &&
			(XIic_IsIicBusy(&IicInstance) == FALSE))
			break;
	}
	*/

	return XST_SUCCESS;
}

/////////////////////////////
// Slave.


u8 ReadBuffer[RECEIVE_COUNT];	/* Read buffer for reading a page. */
int initIICSlave(u16 IicDeviceId, u8 slaveAddress){

	int Status;
	XIic_Config *ConfigPtr;	/* Pointer to configuration data */

	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
	ConfigPtr = XIic_LookupConfig(IIC_DEVICE_ID);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XIic_CfgInitialize(&IicInstance, ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the Interrupt System.
	 */
	Status = SetupInterruptSystem(&IicInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Include the Slave functions.
	 */
	XIic_SlaveInclude();

	/*
	 * Set the Transmit, Receive and Status Handlers.
	 */
	XIic_SetStatusHandler(&IicInstance, &IicInstance,
				  (XIic_StatusHandler) StatusHandler);
	XIic_SetSendHandler(&IicInstance, &IicInstance,
				(XIic_Handler) SendHandler);
	XIic_SetRecvHandler(&IicInstance, &IicInstance,
				(XIic_Handler) RecvHandler);

	/*
	 * Start the IIC device.
	 */
	Status = XIic_Start(&IicInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the Address as a RESPOND type.
	 */
	Status = XIic_SetAddress(&IicInstance, XII_ADDR_TO_RESPOND_TYPE,
				 SLAVE_ADDRESS);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}

int SlaveReadData(u8 *BufferPtr, u16 ByteCount)
{
	int Status;

	/*
	 * Set the defaults.
	 */
	ReceiveComplete = 1;

	/*
	 * Set the Global Interrupt Enable.
	 */
	XIic_IntrGlobalEnable(IicInstance.BaseAddress);

	/*
	 * Wait for AAS interrupt and completion of data reception.
	 */
	/*
	while ((ReceiveComplete) || (XIic_IsIicBusy(&IicInstance) == TRUE)) {
		if (SlaveRead) {
			XIic_SlaveRecv(&IicInstance, ReadBuffer, RECEIVE_COUNT);
			SlaveRead = 0;
		}
	}*/
	while (XIic_IsIicBusy(&IicInstance) == TRUE) {
		XIic_SlaveRecv(&IicInstance, ReadBuffer, RECEIVE_COUNT);
	}

	/*
	 * Disable the Global Interrupt Enable.
	 */
	XIic_IntrGlobalDisable(IicInstance.BaseAddress);

	/*
	 * Stop the IIC device.
	 */
	Status = XIic_Stop(&IicInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

////////////////////////////////////


int main()
{
    init_platform();

    // Init.



    return 0;
}
