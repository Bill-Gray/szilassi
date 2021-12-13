// sp/graphics.cc  Tom Ace   crux@qnet.com

#define IN_GRAPHICS in_fact
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
// #include <new.h>
#include <math.h>
#include "sp1.h"
#include "spprocs.h"
#include "graphics.h"
#include "polyicon.h"

#define tile1_width 8
#define tile1_height 8
static unsigned char tile1_bits[] = {
 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA};

#define tile2_width 8
#define tile2_height 8
static unsigned char tile2_bits[] = {
 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55};

#define XULC 0
#define YULC 0

#define WHEIGHT 700
#define WWIDTH  900

static unsigned long  blackpixel;
static unsigned long  codeColors[7];
static Colormap       colormap;
static int            depth;
static Display        *display;
static const char           *fontName[] = {
       "-b&h-lucida-bold-r-normal-sans-24-240-75-75-p-152-iso8859-1",
       "-adobe-courier-bold-r-normal--24-240-75-75-m-150-iso8859-1",
       "-adobe-helvetica-medium-r-normal--24-240-75-75-p-130-iso8859-1",
       "-b&h-lucida-bold-r-normal-sans-14-140-75-75-p-92-iso8859-1",
       "-adobe-courier-bold-r-normal--14-140-75-75-m-90-iso8859-1",
       "-adobe-courier-medium-r-normal--14-140-75-75-m-90-iso8859-1",
       ""  /* end marker (necessary) */  };
static XFontStruct    *fontStruct;
static GC             gc;
static GC             gcs[3];
static XWindowAttributes  lastWinAtts;
static int            screen;
static unsigned long  whitepixel;
static Window         window;

#define IFLOOR(x) (((x)<0 && (x)!=(int)(x))?(int)x-1:(int)x)

static void hsv_to_rgb(float hsv[3], float rgb[3])
{
 /*   hue/saturation/value --> RGB
      (from Rick Ace, who got this code for me from a colleague at Pixar)  */

 float h, f;
 float m, n, k;
 int i;

 h = hsv[0];
 h = h - (int)h + (h < 0.0 ? 1.0 : 0.0); /* h mod 1.0 */
 h = h * 6;
 i = IFLOOR(h);   /* integer part */
 f = h - i;   /* fractional part */
 m = hsv[2] * (1 - hsv[1]);
 n = hsv[2] * (1 - (hsv[1] * f));
 k = hsv[2] * (1 - (hsv[1] * (1 - f)));

 switch (i) { /* around the color wheel */
 case 0:
  rgb[0] = hsv[2];
  rgb[1] = k;
  rgb[2] = m;
  break;
 case 1:
  rgb[0] = n;
  rgb[1] = hsv[2];
  rgb[2] = m;
  break;
 case 2:
  rgb[0] = m;
  rgb[1] = hsv[2];
  rgb[2] = k;
  break;
 case 3:
  rgb[0] = m;
  rgb[1] = n;
  rgb[2] = hsv[2];
  break;
 case 4:
  rgb[0] = k;
  rgb[1] = m;
  rgb[2] = hsv[2];
  break;
 case 5:
  rgb[0] = hsv[2];
  rgb[1] = m;
  rgb[2] = n;
 }
}

static void initFaceDispInfo()
{
   SPfaceDispInfo[0].initStd(1.,400.,414.,798.);
   SPfaceDispInfo[1].initStd(1.,1.,414.,399.);
   SPfaceDispInfo[2].initStd(415.,457.,649.,798.);
   SPfaceDispInfo[3].initStd(415.,115.,649.,456.);
   SPfaceDispInfo[4].initStd(650.,457.,1022.,798.);
   SPfaceDispInfo[5].initStd(650.,115.,1022.,456.);
   SPfaceDispInfo[6].initStd(415.,1.,1022.,114.);
}

Boolean SPgetWindowSize()
{
   Boolean            changed;
   XWindowAttributes  winAtts;

   /*  This sets up the dimensions of the area usable for graphics.
       It subtracts a little bit to leave a suitable margin from the
       border of the window.  */

   if (!SPgraphicsEnabled) return FALSE;
   XGetWindowAttributes(display,window,&winAtts);
   SPgraphics.pix.xLo = MARGIN_SIZE;
   SPgraphics.pix.yLo = MARGIN_SIZE;
   SPgraphics.pix.xHi = winAtts.width - MARGIN_SIZE;
   SPgraphics.pix.yHi = winAtts.height - MARGIN_SIZE;
   SPgraphics.pix.xMid = (SPgraphics.pix.xLo + SPgraphics.pix.xHi) / 2;
   SPgraphics.pix.yMid = (SPgraphics.pix.yLo + SPgraphics.pix.yHi) / 2;
   SPgraphics.xWindowMax = winAtts.width;
   SPgraphics.yWindowMax = winAtts.height;
   changed = winAtts.height != lastWinAtts.height ||
             winAtts.width  != lastWinAtts.width;
   lastWinAtts = winAtts;
   return changed;
}

#define FAIL(text) { errorText = text; goto failure; }

void SPrgbColors(Boolean colorsOn)
{
   static XColor          blackcolor[40];
   static Boolean         firstTime = TRUE;
   float                  hsv[3];
   static Int32           hueCodes[7] = {0,4,6,11,19,26,34};
   ColorTyp               i;
   float                  rgb[3];
   static XColor          rgbcolor[7];

   if (firstTime) {
      for (i=0; i<7; i+=1) {
         hsv[0] = (float) hueCodes[i] / 40.;
         hsv[1] = .95;   /* saturation */
         hsv[2] = .95;   /* value      */
         hsv_to_rgb(hsv,rgb);
         rgbcolor[i].red = (int) (rgb[0] * 65535.) ;
         rgbcolor[i].green = (int) (rgb[1] * 65535.) ;
         rgbcolor[i].blue = (int) (rgb[2] * 65535.) ;
         rgbcolor[i].flags = DoRed | DoGreen | DoBlue;
         blackcolor[i].red = blackcolor[i].green = blackcolor[i].blue = 0;
         blackcolor[i].flags = DoRed | DoGreen | DoBlue;
         blackcolor[i].pixel = codeColors[i];
      }
      firstTime = FALSE;
   }
   for (i=0; i<7; i+=1) {
      if (colorsOn) {
         if (!XAllocColor(display,colormap,&rgbcolor[i])) {
            fprintf(stderr,"couldn't alloc color");
            exit(1);
         }
      }
      else {
         if (!XAllocColor(display,colormap,&blackcolor[i])) {
            fprintf(stderr,"couldn't alloc color");
            exit(1);
         }
      }
      codeColors[i] = rgbcolor[i].pixel;
   }
}

const char *SPinitWindow()
{
   int                    borderwidth = 3;
   const char             *errorText = NULL;
   XGCValues              gcvalues;
   Int32                  j;
   static Pixmap          pixmap;
   static Pixmap          pixmapTile1;
   static Pixmap          pixmapTile2;
   unsigned long          planeMask;
   XSizeHints             sizehints;
   XSetWindowAttributes   windowattrs;
   unsigned long          windowmask;
   XWMHints               wmhints;

   initFaceDispInfo();

   SPcurrentColor = NO_COLOR;

   lastWinAtts.width = -19;
   lastWinAtts.height = -19;

   display = XOpenDisplay(NULL);
   if (display == NULL) FAIL("couldn't open display");

   screen     = DefaultScreen(display);
   depth      = DefaultDepth(display,screen);
   blackpixel = BlackPixel(display,screen);
   whitepixel = BlackPixel(display,screen);
   colormap   = DefaultColormap(display,screen);

   windowattrs.border_pixel = whitepixel;
   windowattrs.background_pixel = blackpixel;
   windowattrs.backing_store = Always;
   windowmask = (CWBackPixel | CWBorderPixel | CWBackingStore);


   window = XCreateWindow( display,
                           RootWindow(display,screen),
                           XULC, YULC,         /* upper-left corner */
                           WWIDTH, WHEIGHT,
                           borderwidth,
                           depth,
                           InputOutput,
                           CopyFromParent,
                           windowmask,
                           &windowattrs);


   sizehints.flags  = PPosition | PSize | PMinSize | PMaxSize;
   sizehints.x      = XULC;
   sizehints.y      = YULC;
   sizehints.width  = WWIDTH;
   sizehints.height = WHEIGHT;
   sizehints.min_width = sizehints.min_height = 19;
   sizehints.max_width = sizehints.max_height = 1919;

   wmhints.initial_state =
    getenv("SPICON") == NULL ? NormalState : IconicState;

   wmhints.flags = StateHint;

#ifndef SKIP_THIS_LINE_FOR_MWM
   XSetTransientForHint(display,window,RootWindow(display,screen));
#endif

#ifdef TEUTONOPHILE
   XStoreName(display,window,"Vielfl\344cher");
#else
   XStoreName(display,window,"Polyhedron");
#endif

   XSetWMNormalHints(display,window,&sizehints);

#ifndef NO_ICON
   pixmap = XCreateBitmapFromData(display,
                                  window,
                                  (char *)polyicon_bits,
                                  polyicon_width,
                                  polyicon_height);

   wmhints.icon_pixmap = pixmap;
   wmhints.flags |= IconPixmapHint;
#endif
   pixmapTile1 = XCreateBitmapFromData(display,
                                       window,
                                       (char *)tile1_bits,
                                       tile1_width,
                                       tile1_height);

   pixmapTile2 = XCreateBitmapFromData(display,
                                       window,
                                       (char *)tile2_bits,
                                       tile2_width,
                                       tile2_height);

   XSetWMHints(display,window,&wmhints);

   XMapRaised(display,window);

   gcs[0] = XCreateGC(display,window,(unsigned long) 0,&gcvalues);
   gcvalues.fill_style = FillStippled;
   gcvalues.line_style = LineOnOffDash;
   gcvalues.cap_style = CapButt;
   gcvalues.stipple = pixmapTile1;
   gcvalues.dashes = 1;
   gcs[1] = XCreateGC(display,window,
                      GCStipple | GCFillStyle
//                    | GCLineStyle | GCCapStyle
                      ,
                      &gcvalues);
   gcvalues.stipple = pixmapTile2;
   gcs[2] = XCreateGC(display,window,
                      GCStipple | GCFillStyle
//                    | GCLineStyle | GCCapStyle
                      ,
                      &gcvalues);

   SPsetGC(0);

   SPrgbColors(TRUE);

   for (j=0; fontName[j][0] != '\0'; j+=1) {
      fontStruct = XLoadQueryFont(display,fontName[j]);
      if (fontStruct != NULL) {
         XSetFont(display,gc,fontStruct->fid);
         SPfontAvailable = TRUE;
         goto gotFont;
      }
   }

   SPfontAvailable = FALSE;

gotFont:;
   XSetBackground(display, gc, blackpixel);
   XDefineCursor(display,window,XCreateFontCursor(display,XC_left_ptr));
   XFlush(display);
   (void) SPgetWindowSize();

   SPgraphicsEnabled = TRUE;

failure:;
   return errorText;
}


void SPsetFaceColor(Int32 faceColor)
{
   if (!SPgraphicsEnabled) return;
   if (faceColor != SPcurrentColor) {
      if (faceColor >= 0 && faceColor < 7) {
         XSetForeground(display, gc, codeColors[faceColor]);
         SPcurrentColor = faceColor;
      }
   }
}

void SPsetBlack()
{
   XSetForeground(display, gc, blackpixel);
   SPcurrentColor = 1919191919;
}

ColorTyp SPgetCurrentColor() { return SPcurrentColor; }

void SPclearWindow()
{
   if (!SPgraphicsEnabled) return;
   (void) SPgetWindowSize();
   XClearArea(display,window,0,0,1919,1919,FALSE);
}

void SPdrawRectangle(const PixRect &rect)
{
   if (!SPgraphicsEnabled) return;
   XDrawRectangle(display,window,gc,rect.xLo         ,rect.yLo,
                                    rect.xHi-rect.xLo,rect.yHi-rect.yLo);
}

void SPcloseWindow()
{
   if (!SPgraphicsEnabled) return;
   XDestroyWindow(display,window);
   if (SPfontAvailable) XFreeFont(display,fontStruct);
   XFlush(display);
}

void SPflushDisplay()
{
   if (!SPgraphicsEnabled) return;
   XFlush(display);
}


void SPclearRect(PixCoord x,PixCoord y,PixCoord width,PixCoord height)
{
   if (!SPgraphicsEnabled) return;
   XClearArea(display,window,x,y,width,height,False);
}

void SPdrawPoint(PixCoord x,PixCoord y)
{
   XDrawPoint(display,window,gc,x,y);
}

void SPdrawLine(PixCoord x1,PixCoord y1,PixCoord x2,PixCoord y2)
{
   if (!SPgraphicsEnabled) return;
   XDrawLine(display,window,gc,x1,y1,x2,y2);
}

void SPdrawText(PixCoord x,PixCoord y,char *textBuf)
{
   if (!SPgraphicsEnabled || !SPfontAvailable) return;
   XDrawImageString(display,window,gc,x,y,textBuf,strlen(textBuf));
}

void SPdrawLegends(Int32 faceNr)
{
   const PixCoord LEGEND_WIDTH = 25;

   char  faceString[2];

   if (!SPgraphicsEnabled || !SPfontAvailable) return;
   faceString[0] = FACE_CHAR(faceNr);
   faceString[1] = '\0';
   SPsetFaceColor(faceNr);
   XDrawImageString(display,window,gc,
                    SPgraphics.pix.xHi + LEGEND_WIDTH * (faceNr - FACE_COUNT),
                    SPgraphics.pix.yHi - 2,
                    faceString,1);
}

void SPsetGC(Int32 gcNr)
{
   gc = gcs[gcNr];
}

void SPdrawLines(SPXPoint *points,Int32 npoints)
{
   XDrawLines(display,window,gc,(XPoint*)points,npoints,CoordModeOrigin);
}

void SPfillPolygon(SPXPoint *points,Int32 npoints)
{
   XFillPolygon(display,window,gc,(XPoint*)points,npoints,Complex,
                CoordModeOrigin);
}
