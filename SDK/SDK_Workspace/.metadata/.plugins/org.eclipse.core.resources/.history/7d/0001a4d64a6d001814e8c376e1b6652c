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
#include <stdlib.h>
#include <stdbool.h>
#include "platform.h"
#include "vga_periph_mem.h"
#include "xparameters.h"
#include "maps.h"
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
#define IIC_INTR_ID			XPAR_INTC_0_IIC_0_VEC_ID

/*
 * The following constant defines the address of the IIC device on the IIC bus.
 * Since the address is only 7 bits, this constant is the address divided by 2.
 */
#define SLAVE_ADDRESS		0x70	/* 0xE0 as an 8 bit number. */
#define RECEIVE_COUNT		25
#define SEND_COUNT			2
#define RECV_BUFF_SIZE 		2

#define UP 					0b00010000
#define DOWN 				0b00000001
#define LEFT 				0b00001000
#define RIGHT 				0b00000010
#define CENTER 				0b00000100
//starting positions cursor for both of matrices
#define START_POSITION 		1540
#define START_POSITION_LEFT 1460
/************************** Variable Definitions *****************************/

XIic IicInstance; /* The instance of the IIC device. */
XIntc InterruptController; /* The instance of the Interrupt Controller */

u8 WriteBuffer[SEND_COUNT]; /* Write buffer for writing a page. */
u8 ReadBuffer[RECEIVE_COUNT]; /* Read buffer for reading a page. */

u8 buf[25];

volatile u8 TransmitComplete;
volatile u8 ReceiveComplete;
volatile u8 SlaveRead;
volatile u8 SlaveWrite;

typedef enum {
	IDLE,
	LEFT_PRESSED,
	RIGHT_PRESSED,
	CENTER_PRESSED,
	DOWN_PRESSED,
	UP_PRESSED
} state_t;

/************************** Function Prototypes ******************************/

int SlaveWriteData(u8 simbol);
int SlaveReadData(u8 *BufferPtr, u16 ByteCount);
int initIICSlave(u16 IicDeviceId, u8 slaveAddress);
static int SetupInterruptSystem(XIic * IicInstPtr);
static void StatusHandler(XIic *InstancePtr, int Event);
static void SendHandler(XIic *InstancePtr);
static void ReceiveHandler(XIic *InstancePtr);

void printMatrix(int cursorPos, char c[]);
void print_char(Xuint32 BaseAddress, char c);
state_t detectKeyPress();
int moveCursor(int cursor, state_t keyPressed);
void removeEdges(char* mask, char* map, int p);
int getCursorFromMemory(int memoryLocation);
int getMemoryLocationFromCursor(int cursorPosition);

/*************************** Strings ***************************************/
unsigned char stringGameName[] = "POTAPANJE BRODICA\n";
unsigned char stringLoser[] = "LOSER";
unsigned char stringPlayer[] = "IGRAC 2";
unsigned char stringWinner[] = "WINNER";

int main() {

	int completedShips = 0;
	int setCursorHere = START_POSITION;
	u8 buf[RECV_BUFF_SIZE];
	state_t state = IDLE;

	init_platform();
	initIICSlave(XPAR_AXI_IIC_0_DEVICE_ID, SLAVE_ADDRESS);

	int i;
	int j;

	while (1) {

		state = IDLE;

		VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x00, 0x0); 		// direct mode   0
		VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x04, 0x3); 		// display_mode  1
		VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x08, 0x1); 		// show frame      2
		VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x0C, 0x1); 		// font size       3
		VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x10, 0xFFFFFF); 	// foreground 4
		VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x14, 0x0000FF); 	// background color 5
		VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x18, 0xFF0000); 	// frame color      6

		clear_text_screen(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR);
		clear_graphics_screen(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR);

		set_cursor(4228);
		print_string(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, stringPlayer, 7);

		SlaveReadData(buf, sizeof(buf));
		int mapNumber = (int) (buf[0]);

		char* masterMap = allMaps[mapNumber];
		printMatrix(START_POSITION, masterMap);

		int random = rand() % 10;
		while (random == mapNumber) {
			random = rand() % 10;
		}

		char* slaveMap = allMaps[random];
		printMatrix(START_POSITION_LEFT, slaveMap);
		SlaveWriteData(random);

		set_cursor(373);
		print_string(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, stringGameName, 16);
		char whiteSpace = ' ';
		int frameCounter = 0;
		bool slaveTurn = false;
		int x;
		while (completedShips < 20) {
			while (slaveTurn) {
				state_t key = IDLE;
				while (key != CENTER_PRESSED) {
					key = detectKeyPress();
					int j = 0;
					for (j = 0; j < 1000000; j++) {}
					frameCounter++;
					setCursorHere = moveCursor(setCursorHere, key);
					set_cursor(setCursorHere);
					if (frameCounter % 10 < 5) {
						print_char(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, whiteSpace);
					} else {
						printMatrix(START_POSITION, mask);
					}
				}

				x = getMemoryLocationFromCursor(setCursorHere);

				if (masterMap[x] == '0') {
					mask[x] = 'O';
					masterMap[x] = 'O';
					slaveTurn = false;
					SlaveWriteData('+');
				} else if (masterMap[x] == '1') {
					mask[x] = 'X';
					masterMap[x] = 'X';
					printMatrix(START_POSITION, mask);
					removeEdges(mask, masterMap, x);
					completedShips++;
					if (completedShips == 20) {
						while (detectKeyPress()!=IDLE) {
							set_cursor(555);
							clear_text_screen(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR);
							for (i = 0; i < 10; i++) {
								if (i % 2 == 0) {
									print_string(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, stringWinner, 6);
									VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x10, 0x0000FF);	// foreground 4
									VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x14, 0xFFFFFF);	// background color 5
									for (j = -2500000; j < 2500000; j++);
								} else {
									print_string(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, stringWinner, 6);
									VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x10, 0xFFFFFF);	// foreground 4
									VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x14, 0x0000FF);	// background color 5
									for (j = -2500000; j < 2500000; j++) {}
								}
							}
						}
					}
				}
			}
			slaveTurn = true;
			clear_text_screen(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR);
			printMatrix(START_POSITION, mask);
			printMatrix(START_POSITION_LEFT, slaveMap);
			set_cursor(368);
			print_string(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, stringGameName, 17);
			set_cursor(4228);
			print_string(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, stringPlayer, 7);
			u8 flag = 0;
			while (flag != '+') {
				SlaveReadData(&flag, 8);
				if (flag == 'W') {
					clear_text_screen(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR);
					set_cursor(556);
					while (1) {
						print_string(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, stringLoser, sizeof(stringLoser) / sizeof(char) - 1);
					}
				}
			}
		}
	}
	return 0;
}

int initIICSlave(u16 IicDeviceId, u8 slaveAddress) {

	int Status;
	XIic_Config *ConfigPtr; /* Pointer to configuration data */

	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
	ConfigPtr = XIic_LookupConfig(IIC_DEVICE_ID);
	if (ConfigPtr == NULL ) {
		return XST_FAILURE;
	}

	Status = XIic_CfgInitialize(&IicInstance, ConfigPtr, ConfigPtr->BaseAddress);
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
	XIic_SetStatusHandler(&IicInstance, &IicInstance, (XIic_StatusHandler) StatusHandler);
	XIic_SetSendHandler(&IicInstance, &IicInstance, (XIic_Handler) SendHandler);
	XIic_SetRecvHandler(&IicInstance, &IicInstance,	(XIic_Handler) ReceiveHandler);

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
	Status = XIic_SetAddress(&IicInstance, XII_ADDR_TO_RESPOND_TYPE, SLAVE_ADDRESS);
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
int SlaveReadData(u8 *BufferPtr, u16 ByteCount) {
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
int SlaveWriteData(u8 simbol) {
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
static void StatusHandler(XIic *InstancePtr, int Event) {
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
static void SendHandler(XIic *InstancePtr) {
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
static void ReceiveHandler(XIic *InstancePtr) {
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
static int SetupInterruptSystem(XIic * IicInstPtr) {
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
			(XInterruptHandler) XIic_InterruptHandler, IicInstPtr);
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
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler) XIntc_InterruptHandler, &InterruptController);

	/*
	 * Enable non-critical exceptions.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

/* 
 * Calculate position of cursor from mem location 
 */
int getCursorFromMemory(int memoryLocation) {
	int cursorX, cursorY;
	cursorY = memoryLocation / 10;
	cursorX = memoryLocation % 10;
	return START_POSITION + cursorX * 4 + cursorY * 160;
}

/* 
 * Calculate position in array (matrix) when you know where is cursor pressed 
 */
int getMemoryLocationFromCursor(int cursorPosition) {
	int memX, memY;
	cursorPosition -= START_POSITION;
	memY = cursorPosition / 160;
	memX = cursorPosition % 40 / 4;
	return memY * 10 + memX;
}

/* 
 * 100 elements long array prints as matrix 10x10 with given start position of first element
 */
void printMatrix(int cursorPos, char c[]) {

	int i = 0;
	for (i = 0; i < 100; i++) {
		if ((4 * (i % 10) == 0) && (i != 0)) {
			cursorPos += 160;
			set_cursor(cursorPos + 4 * (i % 10));
			print_char(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, c[i]);

		} else {
			set_cursor(cursorPos + 4 * (i % 10));
			print_char(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, c[i]);
		}
	}
}
state_t detectKeyPress() {
	state_t state = IDLE;
	int button = Xil_In32LE(XPAR_MY_PERIPHERAL_0_BASEADDR);
	if ((button & UP) == 0) {
		state = UP_PRESSED;
	} else if ((button & DOWN) == 0) {
		state = DOWN_PRESSED;
	} else if ((button & RIGHT) == 0) {
		state = RIGHT_PRESSED;
	} else if ((button & LEFT) == 0) {
		state = LEFT_PRESSED;
	} else if ((button & CENTER) == 0) {
		state = CENTER_PRESSED;
	} else {
		state = IDLE;
	}

	return state;
}

/* 
 * Move selection cursor through matrix 10x10 on screen 
 */
int moveCursor(int cursor, state_t keyPressed) {
	bool rightEdge, leftEdge, upEdge, downEdge;

	int pos = getMemoryLocationFromCursor(cursor);

	leftEdge = (pos % 10 == 0) ? true : false;
	rightEdge = (pos % 10 == 9) ? true : false;
	downEdge = (pos >= 90) ? true : false;
	upEdge = (pos <= 9) ? true : false;

	if (keyPressed == LEFT_PRESSED)
		pos = (leftEdge) ? pos : pos - 1;
	else if (keyPressed == RIGHT_PRESSED)
		pos = (rightEdge) ? pos : pos + 1;
	else if (keyPressed == UP_PRESSED)
		pos = (upEdge) ? pos : pos - 10;
	else if (keyPressed == DOWN_PRESSED)
		pos = (downEdge) ? pos : pos + 10;
	else if (keyPressed == CENTER_PRESSED)
		pos = pos;

	return getCursorFromMemory(pos);
}
/*
 * Function that detects when one ship is completely destroyed.
 * When ship is destroyed this functions discovers all fields in matrix next to ship
 */
void removeEdges(char* mask, char* map, int p) {

	bool horizontal, vertical, single;
	bool rightEdge, leftEdge, upEdge, downEdge;
	bool leftUp, leftDown, rightDown, rightUp;
	int upper, down, left, right;

	leftEdge = (p % 10 == 0) ? true : false;
	rightEdge = (p % 10 == 9) ? true : false;
	downEdge = (p > 90) ? true : false;
	upEdge = (p <= 9) ? true : false;

	leftUp = leftEdge && upEdge;
	leftDown = leftEdge && downEdge;
	rightDown = rightEdge && downEdge;
	rightUp = rightEdge && upEdge;

	if (upEdge) {
		vertical =((map[p] == '1' || map[p] == 'X') && (map[p + 10] == '1' || map[p + 10] == 'X')) ? true : false;
	} else if (downEdge) {
		vertical =((map[p] == '1' || map[p] == 'X')	&& (map[p - 10] == '1' || map[p - 10] == 'X')) ? true : false;
	} else if (!(upEdge || downEdge)) {
		vertical =((map[p] == '1' || map[p] == 'X')	&& ((map[p + 10] == '1' || map[p + 10] == 'X') || (map[p - 10] == '1' || map[p - 10] == 'X'))) ? true : false;
	}

	if (leftEdge) {
		horizontal =((map[p] == '1' || map[p] == 'X') && (map[p + 1] == '1' || map[p + 1] == 'X')) ? true : false;
	} else if (rightEdge) {
		horizontal =((map[p] == '1' || map[p] == 'X') && (map[p - 1] == '1' || map[p - 1] == 'X')) ? true : false;
	} else if (!(leftEdge || rightEdge)) {
		horizontal =((map[p] == '1' || map[p] == 'X') && ((map[p + 1] == '1' || map[p + 1] == 'X') || (map[p - 1] == '1' || map[p - 1] == 'X'))) ? true : false;
	}

	single = (!horizontal) && (!vertical);
	int temp = p;

	if (horizontal || single) {
		if (!rightEdge) {
			while (map[temp] == '1' || map[temp] == 'X') {
				right = temp;
				temp += 1;
			}
		} else {
			right = temp;
		}
		temp = p;
		if (!leftEdge) {
			while (map[temp] == '1' || map[temp] == 'X') {
				left = temp;
				temp -= 1;
			}
		} else {
			left = temp;
		}
	}

	temp = p;
	if (vertical) {
		if (!downEdge) {
			while (map[temp] == '1' || map[temp] == 'X') {
				upper = temp;
				temp -= 10;
			}
		} else {
			down = temp;
		}

		temp = p;
		if (!upEdge) {
			while (map[temp] == '1' || map[temp] == 'X') {
				down = temp;
				temp += 10;
			}
		} else {
			upper = temp;
		}
	}

	temp = p;

	bool completed = true;
	if (vertical) {
		completed = true;
		int t = upper;
		for (t = upper; t <= down; t += 10) {
			if (map[t] == '1') {
				completed = false;
			}
		}

		leftEdge = (upper % 10 == 0) ? true : false;
		rightEdge = (upper % 10 == 9) ? true : false;
		downEdge = (down > 90) ? true : false;
		upEdge = (upper <= 9) ? true : false;

		leftUp = leftEdge && upEdge;
		leftDown = leftEdge && downEdge;
		rightDown = rightEdge && downEdge;
		rightUp = rightEdge && upEdge;

		if (completed) {
			if (!upEdge && !leftEdge && !rightEdge && !downEdge) {
				map[upper - 1] = 'O';
				map[upper - 11] = 'O';
				map[upper - 10] = 'O';
				map[upper - 9] = 'O';
				map[upper + 1] = 'O';
				map[down + 10] = 'O';
				map[down + 11] = 'O';
				map[down + 9] = 'O';
				map[down + 1] = 'O';
				map[down - 1] = 'O';

				mask[upper - 1] = 'O';
				mask[upper - 11] = 'O';
				mask[upper - 10] = 'O';
				mask[upper - 9] = 'O';
				mask[upper + 1] = 'O';
				mask[down + 10] = 'O';
				mask[down + 11] = 'O';
				mask[down + 9] = 'O';
				mask[down + 1] = 'O';
				mask[down - 1] = 'O';

				for (t = upper; t < down; t += 10) {
					mask[t + 1] = 'O';
					mask[t - 1] = 'O';
					map[t + 1] = 'O';
					map[t - 1] = 'O';
				}
			}
			if (leftUp) {
				map[upper + 1] = 'O';
				map[down + 1] = 'O';
				map[down + 10] = 'O';
				map[down + 11] = 'O';

				mask[upper + 1] = 'O';
				mask[down + 1] = 'O';
				mask[down + 10] = 'O';
				mask[down + 11] = 'O';

				for (t = upper; t < down; t += 10) {
					mask[t + 1] = 'O';
					map[t + 1] = 'O';
				}
			}
			if (rightUp) {
				map[upper - 1] = 'O';
				map[down + 10] = 'O';
				map[down + 9] = 'O';
				map[down - 1] = 'O';

				mask[upper - 1] = 'O';
				mask[down + 10] = 'O';
				mask[down + 9] = 'O';
				mask[down - 1] = 'O';

				for (t = upper; t < down; t += 10) {
					mask[t - 1] = 'O';
					map[t - 1] = 'O';
				}
			}
			if (leftDown) {
				map[upper - 10] = 'O';
				map[upper - 9] = 'O';
				map[upper + 1] = 'O';
				map[down + 1] = 'O';

				mask[upper - 10] = 'O';
				mask[upper - 9] = 'O';
				mask[upper + 1] = 'O';
				mask[down + 1] = 'O';

				for (t = upper; t < down; t += 10) {
					mask[t + 1] = 'O';
					map[t + 1] = 'O';
				}
			}
			if (rightDown) {
				mask[upper - 11] = 'O';
				mask[upper - 10] = 'O';
				mask[upper - 1] = 'O';
				mask[down - 1] = 'O';

				map[upper - 11] = 'O';
				map[upper - 10] = 'O';
				map[upper - 1] = 'O';
				map[down - 1] = 'O';

				for (t = upper; t < down; t += 10) {
					mask[t - 1] = 'O';
					map[t - 1] = 'O';
				}
			}
			if (leftEdge && !upEdge && !downEdge) {
				map[upper + 1] = 'O';
				map[upper - 10] = 'O';
				map[upper - 9] = 'O';
				map[down + 10] = 'O';
				map[down + 11] = 'O';
				map[down + 1] = 'O';

				mask[upper + 1] = 'O';
				mask[upper - 10] = 'O';
				mask[upper - 9] = 'O';
				mask[down + 10] = 'O';
				mask[down + 11] = 'O';
				mask[down + 1] = 'O';

				for (t = upper; t < down; t += 10) {
					map[t + 1] = 'O';
					mask[t + 1] = 'O';
				}
			}
			if (rightEdge && !upEdge && !downEdge) {
				map[upper - 1] = 'O';
				map[upper - 11] = 'O';
				map[upper - 10] = 'O';
				map[down - 1] = 'O';
				map[down + 10] = 'O';
				map[down + 9] = 'O';

				mask[upper - 1] = 'O';
				mask[upper - 11] = 'O';
				mask[upper - 10] = 'O';
				mask[down - 1] = 'O';
				mask[down + 10] = 'O';
				mask[down + 9] = 'O';

				for (t = upper; t < down; t += 10) {
					mask[t - 1] = 'O';
					map[t - 1] = 'O';
				}
			}
			if (upEdge && !rightEdge && !leftEdge) {
				map[upper + 1] = 'O';
				map[upper - 1] = 'O';
				map[down + 1] = 'O';
				map[down - 1] = 'O';
				map[down + 10] = 'O';
				map[down + 11] = 'O';
				map[down + 9] = 'O';

				mask[upper + 1] = 'O';
				mask[upper - 1] = 'O';
				mask[down + 1] = 'O';
				mask[down - 1] = 'O';
				mask[down + 10] = 'O';
				mask[down + 11] = 'O';
				mask[down + 9] = 'O';

				for (t = upper; t < down; t += 10) {
					map[t + 1] = 'O';
					map[t - 1] = 'O';

					mask[t + 1] = 'O';
					mask[t - 1] = 'O';
				}
			}
			if (downEdge && !leftEdge && !rightEdge) {
				map[upper + 1] = 'O';
				map[upper - 1] = 'O';
				map[upper - 9] = 'O';
				map[upper - 11] = 'O';
				map[upper - 10] = 'O';
				map[down + 1] = 'O';
				map[down - 1] = 'O';

				mask[upper + 1] = 'O';
				mask[upper - 1] = 'O';
				mask[upper - 9] = 'O';
				mask[upper - 11] = 'O';
				mask[upper - 10] = 'O';
				mask[down + 1] = 'O';
				mask[down - 1] = 'O';

				for (t = upper; t < down; t += 10) {
					map[t + 1] = 'O';
					map[t - 1] = 'O';

					mask[t + 1] = 'O';
					mask[t - 1] = 'O';
				}
			}
		}
	}
	if (horizontal || single) {
		completed = true;
		int t = left;
		for (t = left; t <= right; t++) {
			if (map[t] == '1') {
				completed = false;
			}
		}

		if (completed) {
			leftEdge = (left % 10 == 0) ? true : false;
			rightEdge = (right % 10 == 9) ? true : false;
			downEdge = (left > 90) ? true : false;
			upEdge = (left <= 9) ? true : false;

			leftUp = leftEdge && upEdge;
			leftDown = leftEdge && downEdge;
			rightDown = rightEdge && downEdge;
			rightUp = rightEdge && upEdge;

			if (!upEdge && !leftEdge && !rightEdge && !downEdge) {
				map[left - 1] = 'O';
				map[left - 11] = 'O';
				map[left - 10] = 'O';
				map[left + 9] = 'O';
				map[left + 10] = 'O';
				map[right - 10] = 'O';
				map[right - 9] = 'O';
				map[right + 1] = 'O';
				map[right + 10] = 'O';
				map[right + 11] = 'O';

				mask[left - 1] = 'O';
				mask[left - 11] = 'O';
				mask[left - 10] = 'O';
				mask[left + 9] = 'O';
				mask[left + 10] = 'O';
				mask[right - 10] = 'O';
				mask[right - 9] = 'O';
				mask[right + 1] = 'O';
				mask[right + 10] = 'O';
				mask[right + 11] = 'O';

				for (t = left + 1; t < right; t++) {
					map[t + 10] = 'O';
					map[t - 10] = 'O';

					mask[t + 10] = 'O';
					mask[t - 10] = 'O';
				}
			}
			if (leftUp) {
				map[right + 1] = 'O';
				map[right + 10] = 'O';
				map[right + 11] = 'O';
				map[left + 10] = 'O';

				mask[right + 1] = 'O';
				mask[right + 10] = 'O';
				mask[right + 11] = 'O';
				mask[left + 10] = 'O';

				for (t = left + 1; t < right; t++) {
					map[t + 10] = 'O';
					mask[t + 10] = 'O';
				}
			}

			if (rightUp) {
				map[left - 1] = 'O';
				map[left + 10] = 'O';
				map[left + 9] = 'O';
				map[right + 10] = 'O';

				mask[left - 1] = 'O';
				mask[left + 10] = 'O';
				mask[left + 9] = 'O';
				mask[right + 10] = 'O';

				for (t = left + 1; t < right; t++) {
					mask[t + 10] = 'O';
					map[t + 10] = 'O';
				}
			}

			if (leftDown) {
				map[right - 10] = 'O';
				map[right - 9] = 'O';
				map[right + 1] = 'O';
				map[left - 10] = 'O';

				mask[right - 10] = 'O';
				mask[right - 9] = 'O';
				mask[right + 1] = 'O';
				mask[left - 10] = 'O';

				for (t = left + 1; t < right; t++) {
					map[t - 10] = 'O';
					mask[t - 10] = 'O';
				}
			}
			if (rightDown) {
				map[left - 11] = 'O';
				map[left - 10] = 'O';
				map[left - 1] = 'O';
				map[right - 10] = 'O';

				mask[left - 11] = 'O';
				mask[left - 10] = 'O';
				mask[left - 1] = 'O';
				mask[right - 10] = 'O';

				for (t = left + 1; t < right; t++) {
					mask[t - 10] = 'O';
					map[t - 10] = 'O';
				}

			}
			if (leftEdge && !upEdge && !downEdge) {
				map[left - 10] = 'O';
				map[left + 10] = 'O';
				map[right - 10] = 'O';
				map[right + 10] = 'O';
				map[right + 1] = 'O';
				map[right + 11] = 'O';
				map[right - 9] = 'O';

				mask[left - 10] = 'O';
				mask[left + 10] = 'O';
				mask[right - 10] = 'O';
				mask[right + 10] = 'O';
				mask[right + 1] = 'O';
				mask[right + 11] = 'O';
				mask[right - 9] = 'O';

				for (t = left + 1; t < right; t++) {
					mask[t + 10] = 'O';
					mask[t - 10] = 'O';

					map[t + 10] = 'O';
					map[t - 10] = 'O';
				}
			}
			if (rightEdge && !upEdge && !downEdge) {
				map[left - 1] = 'O';
				map[left + 9] = 'O';
				map[left - 11] = 'O';
				map[left - 10] = 'O';
				map[left + 10] = 'O';
				map[right - 10] = 'O';
				map[right + 10] = 'O';

				mask[left - 1] = 'O';
				mask[left + 9] = 'O';
				mask[left - 11] = 'O';
				mask[left - 10] = 'O';
				mask[left + 10] = 'O';
				mask[right - 10] = 'O';
				mask[right + 10] = 'O';

				for (t = left + 1; t < right; t++) {
					map[t + 10] = 'O';
					map[t - 10] = 'O';

					mask[t + 10] = 'O';
					mask[t - 10] = 'O';
				}
			}
			if (upEdge && !rightEdge && !leftEdge) {
				map[left - 1] = 'O';
				map[left + 9] = 'O';
				map[left + 10] = 'O';
				map[right + 10] = 'O';
				map[right + 1] = 'O';
				map[right + 11] = 'O';

				mask[left - 1] = 'O';
				mask[left + 9] = 'O';
				mask[left + 10] = 'O';
				mask[right + 10] = 'O';
				mask[right + 1] = 'O';
				mask[right + 11] = 'O';

				for (t = left; t < right; t++) {
					mask[t + 10] = 'O';
					map[t + 10] = 'O';
				}
			}
			if (downEdge && !leftEdge && !rightEdge) {
				mask[left - 1] = 'O';
				mask[left - 11] = 'O';
				mask[left - 10] = 'O';
				mask[right + 1] = 'O';
				mask[right - 9] = 'O';
				mask[right - 10] = 'O';

				map[left - 1] = 'O';
				map[left - 11] = 'O';
				map[left - 10] = 'O';
				map[right + 1] = 'O';
				map[right - 9] = 'O';
				map[right - 10] = 'O';

				for (t = left + 1; t < right; t++) {
					map[t - 10] = 'O';
					mask[t - 10] = 'O';
				}
			}
		}
	}
	printMatrix(START_POSITION, mask);
}
