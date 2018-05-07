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
#include "vga_periph_mem.h"
#include "xparameters.h"

#include "xiic.h"
#include "xintc.h"
#include "xil_exception.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define IIC_DEVICE_ID		XPAR_IIC_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define IIC_INTR_ID		XPAR_INTC_0_IIC_0_VEC_ID

/*
 * The following constant defines the address of the IIC device on the IIC bus.
 * Since the address is only 7 bits, this constant is the address divided by 2.
 */
#define SLAVE_ADDRESS		0x70	/* 0xE0 as an 8 bit number. */

#define RECEIVE_COUNT		25
//#define SEND_COUNT		25
#define SEND_COUNT		2

#define RECV_BUFF_SIZE 2

#define UP 15
#define DOWN 0b00000001
#define LEFT 0b00001000
#define RIGHT 0b00000010
#define CENTER 0b00000100

/************************** Variable Definitions *****************************/

XIic IicInstance;		/* The instance of the IIC device. */
XIntc InterruptController;	/* The instance of the Interrupt Controller */


u8 WriteBuffer[SEND_COUNT];	/* Write buffer for writing a page. */
u8 ReadBuffer[RECEIVE_COUNT];	/* Read buffer for reading a page. */


u8 buf[25];

volatile u8 TransmitComplete;
volatile u8 ReceiveComplete;

volatile u8 SlaveRead;
volatile u8 SlaveWrite;

/************************** Function Prototypes ******************************/

int SlaveWriteData(u8 simbol);
int SlaveReadData(u8 *BufferPtr, u16 ByteCount);
int initIICSlave(u16 IicDeviceId, u8 slaveAddress);
static int SetupInterruptSystem(XIic * IicInstPtr);
static void StatusHandler(XIic *InstancePtr, int Event);
static void SendHandler(XIic *InstancePtr);
static void ReceiveHandler(XIic *InstancePtr);

int main()
{
	u8 buf[RECV_BUFF_SIZE];
	u8 win[1];
	typedef enum {
		IDLE,
		LEFT_PRESSED,
		RIGHT_PRESSED,
		CENTER_PRESSED,
		DOWN_PRESSED,
		UP_PRESSED
	} state_t;

	unsigned char string_s[] = "ODABERITE SIMBOL\n";
	unsigned char string_odabrali[] = "ODABRALI STE SIMBOL";
	unsigned char string_igrac[] = "IGRAC 2";
	unsigned char string_kamen[] = "KAMEN";
	unsigned char string_makaze[] = "MAKAZE";
	unsigned char string_papir[] = "PAPIR";
	unsigned char string_cekam[] = "CEKAM";
	unsigned char string_nereseno[] = "NERESENO";
	unsigned char string_pobeda_1[] = "IGRAC 1 POBEDJUJE";
	unsigned char string_pobeda_2[] = "IGRAC 2 POBEDJUJE";

	init_platform();
	initIICSlave(XPAR_AXI_IIC_0_DEVICE_ID, SLAVE_ADDRESS);
	u8 simbol;
	state_t state = IDLE;
	state_t p_state = IDLE;
	int button;
	int i;
	int j;

	while(1){

		state = IDLE;
		p_state = IDLE;

		VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x00, 0x0);// direct mode   0
		VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x04, 0x3);// display_mode  1
		VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x08, 0x1);// show frame      2
		VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x0C, 0x1);// font size       3
		VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x10, 0xFFFFFF);// foreground 4
		VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x14, 0x0000FF);// background color 5
		VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x18, 0xFF0000);// frame color      6

		clear_text_screen(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR);
		clear_graphics_screen(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR);

		set_cursor(395);
		print_string(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, string_cekam, 5);

		set_cursor(4228);
		print_string(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, string_igrac, 7);

		SlaveReadData(buf, sizeof(buf));

		set_cursor(373);
		print_string(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, string_s, 16);


		while(1){
			button = Xil_In32LE(XPAR_MY_PERIPHERAL_0_BASEADDR);

			if(button == UP) {
				state = UP_PRESSED;
			}else if ((button & DOWN) == 0) {
				state = DOWN_PRESSED;
			}else if ((button & RIGHT) == 0) {
				state = RIGHT_PRESSED;
			}else if ((button & LEFT) == 0) {
				state = LEFT_PRESSED;
			}else if ((button & CENTER) == 0) {
				state = CENTER_PRESSED;
			}else {
				state = IDLE;
			}

			if(p_state != state){
				break;
			}
		}

		clear_text_screen(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR);
		clear_graphics_screen(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR);

		set_cursor(4228);
		print_string(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, string_igrac, 7);
		set_cursor(368);
		print_string(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, string_odabrali, 19);


		switch(state){
			case UP_PRESSED :
				set_cursor(1030);
				print_string(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, string_makaze, 6);
				simbol = 'M';
				break;
			case LEFT_PRESSED :
				set_cursor(1033);
				print_string(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, string_papir, 5);
				simbol = 'P';
				break;
			case RIGHT_PRESSED :
				set_cursor(1033);
				print_string(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, string_kamen, 5);
				simbol = 'K';
				break;
			case DOWN_PRESSED :
				break;
			case CENTER_PRESSED :
				break;
			case IDLE :
				break;
		}

		SlaveWriteData(simbol);
		SlaveReadData(win, sizeof(win));

		int winner = win[0];

		switch(winner){
			case 0 :
				set_cursor(1668);
				print_string(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, string_nereseno, 8);
				break;
			case 1 :
				set_cursor(1652);
				print_string(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, string_pobeda_1, 17);
				break;
			case 2 :
				set_cursor(1652);
				print_string(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, string_pobeda_2, 17);
				break;
		}

		if(winner == 2) {
			for(i = 0; i < 4; i++) {
				if(i % 2 == 0) {
					VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x10, 0x0000FF);// foreground 4
					VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x14, 0xFFFFFF);// background color 5
					for(j = -2500000; j < 2500000; j++);
				}
				else {
					VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x10, 0xFFFFFF);// foreground 4
					VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x14, 0x0000FF);// background color 5
					for(j = -2500000; j < 2500000; j++);
				}
			}
		}
		else {
			for (i = -10000000; i < 10000000; i++){
			}
		}

		VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x10, 0xFFFFFF);// foreground 4
		VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x14, 0x0000FF);// background color 5
		i = 0;
	}

	return 0;
}

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
				(XIic_Handler) ReceiveHandler);

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

/*****************************************************************************/
/**
* This function reads a buffer of bytes  when the IIC Master on the bus writes
* data to the slave device.
*
* @param	BufferPtr contains the address of the data buffer to be filled.
* @param	ByteCount contains the number of bytes in the buffer to be read.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int SlaveReadData(u8 *BufferPtr, u16 ByteCount)
{
	int Status;
	int i;

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
	while ((XIic_IsIicBusy(&IicInstance) == TRUE) || ReceiveComplete) {
		if (SlaveRead) {
			XIic_SlaveRecv(&IicInstance, ReadBuffer, RECEIVE_COUNT);
			SlaveRead = 0;
		}
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

	for (i = 0; i < RECV_BUFF_SIZE; i++)
		BufferPtr[i] = ReadBuffer[i];

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function writes a buffer of bytes to the IIC bus when the IIC master
* initiates a read operation.
*
* @param	ByteCount contains the number of bytes in the buffer to be
*		written.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int SlaveWriteData(u8 simbol)
{
	int Status;

	/*
	 * Set the defaults.
	 */
	TransmitComplete = 1;

	/*
	 * Set the Global Interrupt Enable.
	 */
	XIic_IntrGlobalEnable(IicInstance.BaseAddress);


	WriteBuffer[0] = simbol;


	/*
	 * Wait for AAS interrupt and transmission to complete.
	 */
	while ((XIic_IsIicBusy(&IicInstance) == TRUE) || TransmitComplete) {
		if (SlaveWrite) {
			XIic_SlaveSend(&IicInstance, WriteBuffer, SEND_COUNT);
			SlaveWrite = 0;
		}
	}

	/*
	 * Disable the Global Interrupt Enable bit.
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

/****************************************************************************/
/**
* This Status handler is called asynchronously from an interrupt context and
* indicates the events that have occurred.
*
* @param	InstancePtr is not used, but contains a pointer to the IIC
* 		device driver instance which the handler is being called for.
* @param	Event indicates whether it is a request for a write or read.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
static void StatusHandler(XIic *InstancePtr, int Event)
{
	/*
	 * Check whether the Event is to write or read the data from the slave.
	 */
	if (Event == XII_MASTER_WRITE_EVENT) {
		/*
		 * Its a Write request from Master.
		 */
		SlaveRead = 1;
	} else {
		/*
		 * Its a Read request from the master.
		 */
		SlaveWrite = 1;
	}
}

/****************************************************************************/
/**
* This Send handler is called asynchronously from an interrupt
* context and indicates that data in the specified buffer has been sent.
*
* @param	InstancePtr is a pointer to the IIC driver instance for which
*		the handler is being called for.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
static void SendHandler(XIic *InstancePtr)
{
	TransmitComplete = 0;
}

/****************************************************************************/
/**
* This Receive handler is called asynchronously from an interrupt
* context and indicates that data in the specified buffer has been Received.
*
* @param	InstancePtr is a pointer to the IIC driver instance for which
* 		the handler is being called for.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
static void ReceiveHandler(XIic *InstancePtr)
{
	ReceiveComplete = 0;
}

/****************************************************************************/
/**
* This function setups the interrupt system so interrupts can occur for the
* IIC. The function is application-specific since the actual system may or
* may not have an interrupt controller. The IIC device could be directly
* connected to a processor without an interrupt controller. The user should
* modify this function to fit the application.
*
* @param	IicInstPtr contains a pointer to the instance of the IIC  which
*		is going to be connected to the interrupt controller.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
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

