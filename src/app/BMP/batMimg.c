/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
*                           www.segger.com                           *
**********************************************************************
*                                                                    *
* C-file generated by                                                *
*                                                                    *
*        Bitmap Converter for emWin V5.22.                           *
*        Compiled Jul  4 2013, 12:18:24                              *
*        (c) 1998 - 2013 Segger Microcontroller GmbH && Co. KG       *
*                                                                    *
**********************************************************************
*                                                                    *
* Source file: batMimg                                               *
* Dimensions:  8 * 4                                                 *
* NumColors:   2                                                     *
*                                                                    *
**********************************************************************
*/

#include <stdlib.h>

#include "GUI.h"

#ifndef GUI_CONST_STORAGE
  #define GUI_CONST_STORAGE const
#endif

extern GUI_CONST_STORAGE GUI_BITMAP bmbatMimg;

#if 0


/*********************************************************************
*
*  This palette is included for reference only
*  As it is saved as device dependent bitmap without color info.
*  Please note that this bitmap requires the physical palette to be
*  identical to the palette of the display.
*  If this does not work out, please convert your bitmap into a DIB 
*/
/*********************************************************************
*
*       Palette
*
*  Description
*    The following are the entries of the palette table.
*    The entries are stored as a 32-bit values of which 24 bits are
*    actually used according to the following bit mask: 0xBBGGRR
*
*    The lower   8 bits represent the Red   component.
*    The middle  8 bits represent the Green component.
*    The highest 8 bits represent the Blue  component.
*/
static GUI_CONST_STORAGE GUI_COLOR _ColorsbatMimg[] = {
  0x000000, 0xFFFFFF
};

#endif

static GUI_CONST_STORAGE GUI_LOGPALETTE _PalbatMimg = {
  2,  // Number of entries
  0,  // No transparency
  NULL
};

static GUI_CONST_STORAGE unsigned char _acbatMimg[] = {
  XXXXXXX_,
  XXXX__XX,
  XXXX__XX,
  XXXXXXX_
};

GUI_CONST_STORAGE GUI_BITMAP bmbatMimg = {
  8, // xSize
  4, // ySize
  1, // BytesPerLine
  1, // BitsPerPixel
  _acbatMimg,  // Pointer to picture data (indices)
  &_PalbatMimg   // Pointer to palette
};

/*************************** End of file ****************************/
