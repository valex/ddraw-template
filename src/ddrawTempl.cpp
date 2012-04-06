#define INITGUID       // make sure all the COM interfaces are available
                       // instead of this you can include the .LIB file
                       // DXGUID.LIB
#define WIN32_LEAN_AND_MEAN  // just say no to MFC

// INCLUDES ///////////////////////////////////////////////
#include <windows.h>   // include all the windows headers
#include <windowsx.h>  // include useful macros
#include <stdio.h>

#include <ddraw.h>    // directX includes
#include <dinput.h>
#include "ddrawlib.h"
#include "dinputlib.h"

// DEFINES ////////////////////////////////////////////////
#define WINDOW_CLASS_NAME "WINCLASS1"	// class name

#define WINDOW_TITLE      "DDRAW Template"
#define WINDOW_WIDTH      1024   // size of window
#define WINDOW_HEIGHT     768
#define MILSEC_IN_SEC	  1000

// PROTOTYPES /////////////////////////////////////////////

// game console
int Game_Init(void *parms=NULL);
int Game_Shutdown(void *parms=NULL);
int Game_Main(void *parms=NULL);

int Get_Execute_Folder(char *);
// GLOBALS ////////////////////////////////////////////////

HWND main_window_handle           = NULL; // save the window handle
HINSTANCE main_instance           = NULL; // save the instance
char buffer[256];                          // used to print text
char exe_folder[MAX_PATH];                 // путь к папке с исполняемым файлом
int start_frame_time = 0;
int start_fps_time = 0;
float fps = 0.0;
float temp_fps = 0.0;

// FUNCTIONS //////////////////////////////////////////////
LRESULT CALLBACK WindowProc(HWND hwnd,
						    UINT msg,
                            WPARAM wparam,
                            LPARAM lparam)
{
	// this is the main message handler of the system
	PAINTSTRUCT		ps;		// used in WM_PAINT
	HDC				hdc;	// handle to a device context

	// what is the message
	switch(msg)
	{
	case WM_CREATE:
	{
		// do initialization stuff here

		// return success
		return(0);
		} break;

	case WM_PAINT:
		{
		// simply validate the window
		hdc = BeginPaint(hwnd,&ps);
		// you would do all your painting here
        EndPaint(hwnd,&ps);

        // return success
		return(0);
   		} break;

	case WM_DESTROY:
		{
		// kill the application, this sends a WM_QUIT message
		PostQuitMessage(0);

        // return success
		return(0);
		} break;

	default:break;

	} // end switch

	// process any messages that we didn't take care of
	return (DefWindowProc(hwnd, msg, wparam, lparam));
} // end WinProc

// WINMAIN ////////////////////////////////////////////////
int WINAPI WinMain(	HINSTANCE hinstance,
					HINSTANCE hprevinstance,
					LPSTR lpcmdline,
					int ncmdshow)
{
	WNDCLASSEX winclass; // this will hold the class we create
	HWND	   hwnd;	 // generic window handle
	MSG		   msg;		 // generic message

	// first fill in the window class stucture
	winclass.cbSize         = sizeof(WNDCLASSEX);
	winclass.style			= CS_DBLCLKS | CS_OWNDC |
	                          CS_HREDRAW | CS_VREDRAW;
	winclass.lpfnWndProc	= WindowProc;
	winclass.cbClsExtra		= 0;
	winclass.cbWndExtra		= 0;
	winclass.hInstance		= hinstance;
	winclass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	winclass.hCursor		= LoadCursor(NULL, IDC_ARROW);
	winclass.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	winclass.lpszMenuName	= NULL;
	winclass.lpszClassName	= WINDOW_CLASS_NAME;
	winclass.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);

	// register the window class
	if (!RegisterClassEx(&winclass))
		return(0);

	// create the window
	if (!(hwnd = CreateWindowEx(NULL,                  // extended style
	                            WINDOW_CLASS_NAME,     // class
	                            WINDOW_TITLE, // title
	                            WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION,
						 	    //0,0,	  // initial x,y
						 	    CW_USEDEFAULT,CW_USEDEFAULT,	    // initial x,y
							    WINDOW_WIDTH,WINDOW_HEIGHT,  // initial width, height
						 	    //CW_USEDEFAULT,CW_USEDEFAULT,  // initial width, height
							    NULL,	    // handle to parent
							    NULL,	    // handle to menu
							    hinstance,// instance of this application
							    NULL)))	// extra creation parms
	return(0);

	// save the window handle and instance in a global
	main_window_handle = hwnd;
	main_instance      = hinstance;

	// now resize the window, so the client area is the actual size requested
	// since there may be borders and controls if this is going to be a windowed app
	// if the app is not windowed then it won't matter
	RECT window_rect = {0,0,WINDOW_WIDTH-1,WINDOW_HEIGHT-1};

	// make the call to adjust window_rect
	AdjustWindowRectEx(&window_rect,
	     GetWindowStyle(main_window_handle),
	     GetMenu(main_window_handle) != NULL,
	     GetWindowExStyle(main_window_handle));

	// save the global client offsets, they are needed in DDraw_Flip()
	window_client_x0 = -window_rect.left;
	window_client_y0 = -window_rect.top;


	//данные для центрирования окна
	int window_width = window_rect.right - window_rect.left;
	int window_height = window_rect.bottom - window_rect.top;
	int screen_width = GetSystemMetrics(SM_CXSCREEN);
	int screen_height = GetSystemMetrics(SM_CYSCREEN);

	// now resize the window with a call to MoveWindow()
	MoveWindow(main_window_handle,
				(screen_width>>1)-(window_width>>1), // x position
				(screen_height>>1)-(window_height>>1) - (screen_height>>4), // y position
				window_width, // width
				window_height, // height
				FALSE);

	ShowWindow(main_window_handle, SW_SHOW);

	// perform all game console specific initialization
	Game_Init();

	// enter main event loop, but this time we use PeekMessage()
	// instead of GetMessage() to retrieve messages
	while(TRUE)
	{
	    // test if there is a message in queue, if so get it
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
		   // test if this is a quit
	       if (msg.message == WM_QUIT)
	           break;

		   // translate any accelerator keys
		   TranslateMessage(&msg);

		   // send the message to the window proc
		   DispatchMessage(&msg);
		} // end if

	    // main game processing goes here
	    Game_Main();

	} // end while

	// shutdown game and release all resources
	Game_Shutdown();

	// return to Windows like this
	return(msg.wParam);

} // end WinMain

///////////////////////////////////////////////////////////
int Game_Init(void *parms)
{
	// start up DirectDraw (replace the parms as you desire)
	DDraw_Init(WINDOW_WIDTH, WINDOW_HEIGHT);

	// initialize directinput
	DInput_Init();

	// acquire the keyboard
	DInput_Init_Keyboard();
	DInput_Init_Mouse();

	// start the timing clock
	start_frame_time = Start_Clock();
	start_fps_time = start_frame_time;


	return(1);

} // end Game_Init

///////////////////////////////////////////////////////////
int Game_Main(void *parms)
{


	// clear the drawing surface
	DDraw_Fill_Surface(lpddsback, 0);

	// чтение состояния клавиатуры и мыши
	DInput_Read_Keyboard();
	DInput_Read_Mouse();


	//расчет FPS
	temp_fps += 1.0;
	int elapsed_fps_time = Get_Clock()-start_fps_time;
		if(elapsed_fps_time >= 1000){
			fps = temp_fps/(float)elapsed_fps_time * 1000.;
			temp_fps = 0.0;
			start_fps_time += elapsed_fps_time;
	}

	//время прошедшее с предыдущего кадра
	int elapsed_time = Get_Clock()-start_frame_time;
	if(elapsed_time > 0){
		//выполнить все необходимые расчеты с учетом прошедшего времени


		DDraw_Lock_Back_Surface();


		//Draw_Pixel(100,100,0x00FFFFFF,back_buffer,back_lpitch);
		//*((DWORD*)back_buffer) = 0x00FFFFFF;


		sprintf(buffer, "fps: %.1f", fps);
		Draw_Text_GDI(buffer, 10,150,_RGB32BIT(0, 0, 255,0), lpddsback);
		sprintf(buffer, "start_frame_time: %u", start_frame_time);
		Draw_Text_GDI(buffer, 10,170,_RGB32BIT(0, 0, 255,0), lpddsback);
		//Draw_Circle(450,450,77,_RGB32BIT(0, 255, 255,255),back_buffer,back_lpitch);

		DDraw_Unlock_Back_Surface();

		// flip the surfaces
		DDraw_Flip();

		start_frame_time += elapsed_time;
	}




	// Выходим?
	if (KEY_DOWN(VK_ESCAPE) || keyboard_state[DIK_ESCAPE]){
		PostMessage(main_window_handle, WM_DESTROY,0,0);
	}

	// return success
	return(1);

} // end Game_Main

///////////////////////////////////////////////////////////
int Game_Shutdown(void *parms)
{
	DInput_Release_Mouse();
	DInput_Release_Keyboard();
	DInput_Shutdown();

	// shutdown directdraw last
	DDraw_Shutdown();



	// return success
	return(1);
} // end Game_Shutdown

int Get_Execute_Folder(char * fexe_folder){
	//получить путь к исполняемому файлу
	GetModuleFileName(NULL, fexe_folder, MAX_PATH );
	//удалить название исполняемого файла
	//т.о. остается только путь в папку в исполняемым файлом
	*strrchr(fexe_folder, '\\') = '\0';

	return 0;
}
