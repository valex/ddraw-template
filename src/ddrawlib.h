#ifndef DDRAWLIB_H_
#define DDRAWLIB_H_

// MACROS /////////////////////////////////////////////////
// these read the keyboard asynchronously
#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEY_UP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

// this builds a 32 bit color value in A.8.8.8 format (8-bit alpha mode)
#define _RGB32BIT(a,r,g,b) ((b) + ((g) << 8) + ((r) << 16) + ((a) << 24))

// initializes a direct draw struct, basically zeros it and sets the dwSize field
#define DDRAW_INIT_STRUCT(ddstruct) { memset(&ddstruct,0,sizeof(ddstruct)); ddstruct.dwSize=sizeof(ddstruct); }

// TYPES //////////////////////////////////////////////////

// PROTOTYPES /////////////////////////////////////////////


// DirectDraw functions
int DDraw_Init(int width, int height);
int DDraw_Shutdown(void);

LPDIRECTDRAWSURFACE7 DDraw_Create_Surface(int width, int height, int mem_flags=0, DWORD color_key_value=0);
int DDraw_Fill_Surface(LPDIRECTDRAWSURFACE7 lpdds, DWORD color, RECT *client=NULL);
LPDIRECTDRAWCLIPPER DDraw_Attach_Clipper(LPDIRECTDRAWSURFACE7 lpdds, int num_rects, LPRECT clip_list);

UCHAR *DDraw_Lock_Back_Surface(void);
int DDraw_Unlock_Back_Surface(void);
UCHAR *DDraw_Lock_Surface(LPDIRECTDRAWSURFACE7 lpdds,int *lpitch);
int DDraw_Unlock_Surface(LPDIRECTDRAWSURFACE7 lpdds);

int DDraw_Flip(void);

// gdi functions
int Draw_Text_GDI(char *text, int x,int y,COLORREF color, LPDIRECTDRAWSURFACE7 lpdds);

// graphics functions

// general utility functions
DWORD Get_Clock(void);
DWORD Start_Clock(void);

// GLOBALS ////////////////////////////////////////////////

// notice that interface 4.0 is used on a number of interfaces
extern LPDIRECTDRAW7        lpdd;
extern LPDIRECTDRAWSURFACE7 lpddsback;            // dd back surface
extern LPDIRECTDRAWSURFACE7 lpddsball;
extern DDSURFACEDESC2       ddsd;

extern UCHAR                *back_buffer;         // secondary back buffer
extern int                  back_lpitch;          // memory line pitch

extern int defBckColor;

extern int window_client_x0;   // used to track the starting (x,y) client area for
extern int window_client_y0;   // for windowed mode directdraw operations

#endif /* DDRAWLIB_H_ */
