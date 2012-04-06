/*
 * dinputlib.h
 *
 *  Created on: 07.03.2012
 *      Author: alex
 */

#ifndef DINPUTLIB_H_
#define DINPUTLIB_H_

// PROTOTYPES /////////////////////////////////////////////

// input
int DInput_Init(void);
void DInput_Shutdown(void);

int DInput_Init_Mouse(void);
int DInput_Init_Keyboard(void);
int DInput_Read_Mouse(void);
int DInput_Read_Keyboard(void);
void DInput_Release_Mouse(void);
void DInput_Release_Keyboard(void);

// EXTERNALS //////////////////////////////////////////////

extern HWND main_window_handle; // save the window handle
extern HINSTANCE main_instance; // save the instance

// directinput globals
extern LPDIRECTINPUT8       lpdi;         // dinput object
extern LPDIRECTINPUTDEVICE8 lpdikey;      // dinput keyboard
extern LPDIRECTINPUTDEVICE8 lpdimouse;    // dinput mouse


// these contain the target records for all di input packets
extern UCHAR keyboard_state[256]; // contains keyboard state table
extern DIMOUSESTATE mouse_state;  // contains state of mouse

#endif /* DINPUTLIB_H_ */
