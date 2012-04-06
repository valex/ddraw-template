// INCLUDES ///////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN

#include <windows.h>   // include important windows stuff
#include <windowsx.h>
#include <math.h>

#include <ddraw.h>    // directX includes
#include "ddrawlib.h"
// EXTERNALS /////////////////////////////////////////////

extern HWND main_window_handle; // save the window handle
extern HINSTANCE main_instance; // save the instance

// GLOBALS ////////////////////////////////////////////////
DWORD                start_clock_count = 0;     // used for timing

// notice that interface 7.0 is used on a number of interfaces
LPDIRECTDRAW7        lpdd         = NULL;  // dd object
LPDIRECTDRAWSURFACE7 lpddsprimary = NULL;  // dd primary surface
LPDIRECTDRAWSURFACE7 lpddsback    = NULL;  // dd back surface
LPDIRECTDRAWSURFACE7 lpddsball    = NULL;  // общая внеэкр поверхность
LPDIRECTDRAWCLIPPER  lpddclipper  = NULL;   // dd clipper for back surface
LPDIRECTDRAWCLIPPER  lpddclipperwin = NULL; // dd clipper for window

DDSURFACEDESC2       ddsd;                 // a direct draw surface description struct
UCHAR                *back_buffer    = NULL; // secondary back buffer
int                  back_lpitch     = 0;    // memory line pitch for back buffer

int dd_pixel_format;  // default pixel format

// these are overwritten globally by DDraw_Init()
int screen_width,            // width of screen
    screen_height;           // height of screen

int window_client_x0   = 0;   // used to track the starting (x,y) client area for
int window_client_y0   = 0;   // for windowed mode directdraw operations

//////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////

UCHAR *DDraw_Lock_Surface(LPDIRECTDRAWSURFACE7 lpdds, int *lpitch)
{
	// this function locks the sent surface and returns a pointer to it

	// is this surface valid
	if (!lpdds)
		return(NULL);

	// lock the surface
	DDRAW_INIT_STRUCT(ddsd);
	lpdds->Lock(NULL,&ddsd,DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR,NULL);

	// set the memory pitch
	if (lpitch)
		*lpitch = ddsd.lPitch;

	// return pointer to surface
	return((UCHAR *)ddsd.lpSurface);
} // end DDraw_Lock_Surface

///////////////////////////////////////////////////////////

int DDraw_Unlock_Surface(LPDIRECTDRAWSURFACE7 lpdds)
{
// this unlocks a general surface

// is this surface valid
if (!lpdds)
   return(0);

// unlock the surface memory
lpdds->Unlock(NULL);

// return success
return(1);
} // end DDraw_Unlock_Surface

///////////////////////////////////////////////////////////

int Draw_Text_GDI(char *text, int x,int y,COLORREF color, LPDIRECTDRAWSURFACE7 lpdds)
{
	// this function draws the sent text on the sent surface
	// using color index as the color in the palette

	HDC xdc; // the working dc

	// get the dc from surface
	if (FAILED(lpdds->GetDC(&xdc)))
		return(0);

	// set the colors for the text up
	SetTextColor(xdc,color);

	// set background mode to transparent so black isn't copied
	SetBkMode(xdc, TRANSPARENT);

	// draw the text a
	TextOut(xdc,x,y,text,strlen(text));

	// release the dc
	lpdds->ReleaseDC(xdc);

	// return success
	return(1);
} // end Draw_Text_GDI

///////////////////////////////////////////////////////////

int DDraw_Flip(void)
{
// this function flip the primary surface with the secondary surface

// test if either of the buffers are locked
//if (primary_buffer || back_buffer)
	if (back_buffer)
		return(0);

	// flip pages
	RECT    dest_rect;    // used to compute destination rectangle

	// get the window's rectangle in screen coordinates
	GetWindowRect(main_window_handle, &dest_rect);

	// compute the destination rectangle
	dest_rect.left   +=window_client_x0;
	dest_rect.top    +=window_client_y0;

	dest_rect.right  =dest_rect.left+screen_width;
	dest_rect.bottom =dest_rect.top +screen_height;

   // clip the screen coords

	// blit the entire back surface to the primary
	if (FAILED(lpddsprimary->Blt(&dest_rect, lpddsback,NULL,DDBLT_WAIT,NULL)))
		return(0);


	// return success
	return(1);

} // end DDraw_Flip

///////////////////////////////////////////////////////////

UCHAR *DDraw_Lock_Back_Surface(void)
{
// this function locks the secondary back surface and returns a pointer to it
// and updates the global variables secondary buffer, and back_lpitch

	// is this surface already locked
	if (back_buffer)
	{
		// return to current lock
		return(back_buffer);
	} // end if

	// lock the primary surface
	DDRAW_INIT_STRUCT(ddsd);
	lpddsback->Lock(NULL,&ddsd,DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR,NULL);

	// set globals
	back_buffer = (UCHAR *)ddsd.lpSurface;
	back_lpitch = ddsd.lPitch;

	// return pointer to surface
	return(back_buffer);

} // end DDraw_Lock_Back_Surface

///////////////////////////////////////////////////////////

int DDraw_Unlock_Back_Surface(void)
{
// this unlocks the secondary

	// is this surface valid
	if (!back_buffer)
	return(0);

	// unlock the secondary surface
	lpddsback->Unlock(NULL);

	// reset the secondary surface
	back_buffer = NULL;
	back_lpitch = 0;

	// return success
	return(1);
} // end DDraw_Unlock_Back_Surface

///////////////////////////////////////////////////////////

LPDIRECTDRAWCLIPPER DDraw_Attach_Clipper(LPDIRECTDRAWSURFACE7 lpdds,
                                         int num_rects,
                                         LPRECT clip_list)

{
	// this function creates a clipper from the sent clip list and attaches
	// it to the sent surface

	int index;                         // looping var
	LPDIRECTDRAWCLIPPER lpddclipper2;   // pointer to the newly created dd clipper
	LPRGNDATA region_data;             // pointer to the region data that contains
		// the header and clip list

	// first create the direct draw clipper
	if (FAILED(lpdd->CreateClipper(0,&lpddclipper2,NULL)))
		return(NULL);

	// now create the clip list from the sent data

	// first allocate memory for region data
	region_data = (LPRGNDATA)malloc(sizeof(RGNDATAHEADER)+num_rects*sizeof(RECT));

	// now copy the rects into region data
	memcpy(region_data->Buffer, clip_list, sizeof(RECT)*num_rects);

	// set up fields of header
	region_data->rdh.dwSize          = sizeof(RGNDATAHEADER);
	region_data->rdh.iType           = RDH_RECTANGLES;
	region_data->rdh.nCount          = num_rects;
	region_data->rdh.nRgnSize        = num_rects*sizeof(RECT);

	region_data->rdh.rcBound.left    =  64000;
	region_data->rdh.rcBound.top     =  64000;
	region_data->rdh.rcBound.right   = -64000;
	region_data->rdh.rcBound.bottom  = -64000;

	// find bounds of all clipping regions
	for (index=0; index<num_rects; index++)
    {
		// test if the next rectangle unioned with the current bound is larger
		if (clip_list[index].left < region_data->rdh.rcBound.left)
			region_data->rdh.rcBound.left = clip_list[index].left;

		if (clip_list[index].right > region_data->rdh.rcBound.right)
			region_data->rdh.rcBound.right = clip_list[index].right;

		if (clip_list[index].top < region_data->rdh.rcBound.top)
			region_data->rdh.rcBound.top = clip_list[index].top;

		if (clip_list[index].bottom > region_data->rdh.rcBound.bottom)
			region_data->rdh.rcBound.bottom = clip_list[index].bottom;

    } // end for index

	// now we have computed the bounding rectangle region and set up the data
	// now let's set the clipping list

	if (FAILED(lpddclipper2->SetClipList(region_data, 0)))
	{
		// release memory and return error
		free(region_data);
		return(NULL);
	} // end if

	// now attach the clipper to the surface
	if (FAILED(lpdds->SetClipper(lpddclipper2)))
	{
		// release memory and return error
		free(region_data);
		return(NULL);
	} // end if

	// all is well, so release memory and send back the pointer to the new clipper
	free(region_data);

	//рекомендуют в Microsoft
	if(lpddclipper2)
		lpddclipper2->Release();

	return(lpddclipper2);

} // end DDraw_Attach_Clipper

///////////////////////////////////////////////////////////
int DDraw_Fill_Surface(LPDIRECTDRAWSURFACE7 lpdds, DWORD color, RECT *client)
{
	DDBLTFX ddbltfx; // this contains the DDBLTFX structure

	// clear out the structure and set the size field
	DDRAW_INIT_STRUCT(ddbltfx);

	// set the dwfillcolor field to the desired color
	ddbltfx.dwFillColor = color;

	// ready to blt to surface
	lpdds->Blt(client,     // ptr to dest rectangle
           NULL,       // ptr to source surface, NA
           NULL,       // ptr to source rectangle, NA
           DDBLT_COLORFILL | DDBLT_WAIT,   // fill and wait
           &ddbltfx);  // ptr to DDBLTFX structure

	// return success
	return(1);
} // end DDraw_Fill_Surface

//////////////////////////////////////////////////////

LPDIRECTDRAWSURFACE7 DDraw_Create_Surface(int width,
                                          int height,
                                          int mem_flags,
                                          DWORD color_key_value)
{
	// this function creates an offscreen plain surface

	LPDIRECTDRAWSURFACE7 lpdds;  // temporary surface

	// set to access caps, width, and height
	DDRAW_INIT_STRUCT(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;

	// set dimensions of the new bitmap surface
	ddsd.dwWidth  =  width;
	ddsd.dwHeight =  height;

	// set surface to offscreen plain
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | mem_flags;

	// create the surface
	if (FAILED(lpdd->CreateSurface(&ddsd,&lpdds,NULL)))
		return(NULL);

	// set color key to default color 000
	// note that if this is a 8bit bob then palette index 0 will be
	// transparent by default
	// note that if this is a 16bit bob then RGB value 000 will be
	// transparent
	DDCOLORKEY color_key; // used to set color key
	color_key.dwColorSpaceLowValue  = color_key_value;
	color_key.dwColorSpaceHighValue = color_key_value;

	// now set the color key for source blitting
	lpdds->SetColorKey(DDCKEY_SRCBLT, &color_key);

	// return surface
	return(lpdds);
} // end DDraw_Create_Surface


//////////////////////////////////////////////////////////

int DDraw_Init(int width, int height)
{
	// this function initializes directdraw
	//int index; // looping variable

	// create IDirectDraw interface 7.0 object and test for error
	if (FAILED(DirectDrawCreateEx(NULL, (void **)&lpdd, IID_IDirectDraw7, NULL)))
	return(0);

	// set cooperation level to windowed mode
	if (FAILED(lpdd->SetCooperativeLevel(main_window_handle,DDSCL_NORMAL)))
		return(0);

	// Create the primary surface
	//memset(&ddsd,0,sizeof(ddsd));
	//ddsd.dwSize = sizeof(ddsd);
	DDRAW_INIT_STRUCT(ddsd);

	// windowed mode
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	// set the backbuffer count to 0 for windowed mode
	// 1 for fullscreen mode, 2 for triple buffering
	//ddsd.dwBackBufferCount = 0;

	// create the primary surface
	lpdd->CreateSurface(&ddsd,&lpddsprimary,NULL);



	// get the pixel format of the primary surface
	DDPIXELFORMAT ddpf; // used to get pixel format

	// initialize structure
	DDRAW_INIT_STRUCT(ddpf);

	// query the format from primary surface
	lpddsprimary->GetPixelFormat(&ddpf);

	// use number of bits, better method
	dd_pixel_format = ddpf.dwRGBBitCount;




	// create a double buffer that will be blitted
	// rather than flipped as in full screen mode
	lpddsback = DDraw_Create_Surface(width, height, DDSCAPS_SYSTEMMEMORY); // int mem_flags, USHORT color_key_flag);

	// only clear backbuffer
	DDraw_Fill_Surface(lpddsback,0);


	// set globals
	screen_height   = height;
	screen_width    = width;
/*
	// set software algorithmic clipping region
	min_clip_x = 0;
	max_clip_x = screen_width - 1;
	min_clip_y = 0;
	max_clip_y = screen_height - 1;
*/

	// setup backbuffer clipper always
	RECT screen_rect = {0,0,screen_width,screen_height};
	lpddclipper = DDraw_Attach_Clipper(lpddsback,1,&screen_rect);



	// set windowed clipper
	if (FAILED(lpdd->CreateClipper(0,&lpddclipperwin,NULL)))
		return(0);

	if (FAILED(lpddclipperwin->SetHWnd(0, main_window_handle)))
		return(0);

	if (FAILED(lpddsprimary->SetClipper(lpddclipperwin)))
		return(0);

	//Рекомендуют в Microsoft
	if(lpddclipperwin)
		lpddclipperwin->Release();

	// return success
	return(1);

} // end DDraw_Init

////////////////////////////////////////////////////

int DDraw_Shutdown(void)
{
// this function release all the resources directdraw
// allocated, mainly to com objects

	if (lpddclipperwin)
		lpddclipperwin->Release();

	// release the clippers first
	if (lpddclipper)
		lpddclipper->Release();

	// release the secondary surface
	if (lpddsback)
		lpddsback->Release();

	// release the primary surface
	if (lpddsprimary)
		lpddsprimary->Release();

	// finally, the main dd object
	if (lpdd)
		lpdd->Release();

	// return success
	return(1);
} // end DDraw_Shutdown

///////////////////////////////////////////////////////////

DWORD Get_Clock(void){
	// this function returns the current tick count

	return(GetTickCount());

} // end Get_Clock

///////////////////////////////////////////////////////////

DWORD Start_Clock(void){
	// this function starts the clock, that is, saves the current
	// count, use in conjunction with Wait_Clock()

	return(start_clock_count = Get_Clock());
} // end Start_Clock

///////////////////////////////////////////////////////////
