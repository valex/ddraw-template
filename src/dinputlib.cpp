#define WIN32_LEAN_AND_MEAN

// INCLUDES ///////////////////////////////////////////////
#include <windows.h>   // include all the windows headers
#include <windowsx.h>  // include useful macros
#include <stdio.h>

#include <dinput.h>
#include "dinputlib.h"

// EXTERNALS /////////////////////////////////////////////

extern HWND main_window_handle;     // access to main window handle in main module
extern HINSTANCE main_instance; // save the instance

// GLOBALS ////////////////////////////////////////////////

// directinput globals
LPDIRECTINPUT8       lpdi      = NULL;    // dinput object
LPDIRECTINPUTDEVICE8 lpdikey   = NULL;    // dinput keyboard
LPDIRECTINPUTDEVICE8 lpdimouse = NULL;    // dinput mouse


// these contain the target records for all di input packets
UCHAR keyboard_state[256]; // contains keyboard state table
DIMOUSESTATE mouse_state;  // contains state of mouse


// FUNCTIONS //////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

int DInput_Init(void)
{
	// this function initializes directinput
	if (FAILED(DirectInput8Create(main_instance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&lpdi,NULL)))
		return(0);

	// return success
	return(1);
} // end DInput_Init

///////////////////////////////////////////////////////////

void DInput_Shutdown(void)
{
	// this function shuts down directinput
	if (lpdi)
		lpdi->Release();

} // end DInput_Shutdown

///////////////////////////////////////////////////////////

int DInput_Init_Mouse(void)
{
// this function intializes the mouse

// create a mouse device
if (lpdi->CreateDevice(GUID_SysMouse, &lpdimouse, NULL)!=DI_OK)
   return(0);

// set cooperation level
// change to EXCLUSIVE FORGROUND for better control
if (lpdimouse->SetCooperativeLevel(main_window_handle,
                       DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)!=DI_OK)
   return(0);

// set data format
if (lpdimouse->SetDataFormat(&c_dfDIMouse)!=DI_OK)
   return(0);

// acquire the mouse
if (lpdimouse->Acquire()!=DI_OK)
   return(0);

// return success
return(1);

} // end DInput_Init_Mouse

///////////////////////////////////////////////////////////

int DInput_Init_Keyboard(void)
{
	// this function initializes the keyboard device

	// create the keyboard device
	if (lpdi->CreateDevice(GUID_SysKeyboard, &lpdikey, NULL)!=DI_OK)
		return(0);

	// set cooperation level
	if (lpdikey->SetCooperativeLevel(main_window_handle,
                 DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)!=DI_OK)
		return(0);

	// set data format
	if (lpdikey->SetDataFormat(&c_dfDIKeyboard)!=DI_OK)
		return(0);

	// acquire the keyboard
	if (FAILED(lpdikey->Acquire()))
		return(0);

	// return success
	return(1);
} // end DInput_Init_Keyboard

///////////////////////////////////////////////////////////

int DInput_Read_Mouse(void)
{
// this function reads  the mouse state

if (lpdimouse)
    {
    if (lpdimouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&mouse_state)!=DI_OK)
        return(0);
    }
else
    {
    // mouse isn't plugged in, zero out state
    memset(&mouse_state,0,sizeof(mouse_state));

    // return error
    return(0);
    } // end else

// return sucess
return(1);

} // end DInput_Read_Mouse

///////////////////////////////////////////////////////////

int DInput_Read_Keyboard(void)
{
	// this function reads the state of the keyboard

	if (lpdikey)
    {
		if (lpdikey->GetDeviceState(256, (LPVOID)keyboard_state)!=DI_OK)
			return(0);
    }else
    {
    	// keyboard isn't plugged in, zero out state
    	memset(keyboard_state,0,sizeof(keyboard_state));

    	// return error
    	return(0);
    } // end else

// return sucess
return(1);

} // end DInput_Read_Keyboard

///////////////////////////////////////////////////////////

void DInput_Release_Mouse(void)
{
// this function unacquires and releases the mouse

if (lpdimouse)
    {
    lpdimouse->Unacquire();
    lpdimouse->Release();
    } // end if

} // end DInput_Release_Mouse

///////////////////////////////////////////////////////////

void DInput_Release_Keyboard(void)
{
	// this function unacquires and releases the keyboard

	if (lpdikey)
    {
		lpdikey->Unacquire();
		lpdikey->Release();
    } // end if
} // end DInput_Release_Keyboard


