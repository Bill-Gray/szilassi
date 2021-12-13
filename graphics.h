//  graphics.h

/*  note:  DO NOT include this file from any files except draw.cc and
    graphics.c .  Only draw.cc is to make calls to graphics.c functions.  */

/*  graphics.h:  designed to be include-able from either C or C++  */

#ifdef IN_GRAPHICS
#define EXT_GRAPHICS
#else
#define EXT_GRAPHICS extern
#endif

#define COORD_AREA_SIZE 27
#define FULL_CIRCLE (360*64)
#define MARGIN_SIZE 16

typedef Int32  ColorTyp;
typedef Int32  PixCoord;

class PixRect { public: PixCoord xLo,yLo,xHi,yHi,xMid,yMid; };

EXT_GRAPHICS Boolean SPfontAvailable;

EXT_GRAPHICS  const char *SPcolorNames[]
#ifdef IN_GRAPHICS
= { /*   0 */ "",             // zero & below are reserved for code colors
    /*   1 */ "white",
    /*   2 */ "lightGray",
    /*   3 */ "limeGreen",
    /*   4 */ "goldenrod",
    /*   5 */ "maroon",
    /*   6 */ "gray69",
    /*   7 */ "white",
    /*   8 */ "blue",
    /*   9 */ "purple",
    /*  10 */ "orangered",
    /*  11 */ "orange",
    /*  12 */ "yellow",
    /*  13 */ "cyan",
    /*  14 */ "darkslategray2",
    /*  15 */ "darkorchid",
    /*  16 */ "tan",
    /*  17 */ "orangered",
    /*  18 */ "RoyalBlue",
    /*  19 */ "pink",
    /*  20 */ "black",
    /* end */ ""   /*   null string for end marker (necessary)   */
  }
#endif
;

EXT_GRAPHICS  unsigned long  SPcolors[30];   /* leave room for all names */

#define GENERAL_LEGEND_COLOR 13
#define BIN_BOUNDARY_LINE_COLOR 6
#define BIN_COORDINATE_TEXT_COLOR 6
#define COORDINATE_LINE_MINOR_COLOR 8
#define COORDINATE_TEXT_MINOR_COLOR 8
#define COORDINATE_LINE_MAJOR_COLOR 9
#define COORDINATE_TEXT_MAJOR_COLOR 9
#define BLACKOUT_COLOR 20
#define ALL_Z_DOGS_COLOR 16
#define TARGET_COLOR 10
#define SOURCE_COLOR 8
#define VIA_SITE_COLOR 17
#define GENERIC_LEGEND_COLOR 2
#define NO_COLOR 1919

class SPgraphicsClass {
public:
   PixCoord     xWindowMax,yWindowMax;
   PixRect      pix;                         /*  bin drawing area  */
   Int32        pixSize;
   Real         scale;
};

EXT_GRAPHICS SPgraphicsClass SPgraphics;

EXT_GRAPHICS ColorTyp SPcurrentColor;

EXT_GRAPHICS Boolean SPgraphicsEnabled
#ifdef IN_GRAPHICS
= FALSE
#endif
;

const Real ZONE_MARGIN = 3.;

class FaceDispInfo {
public:
   RealRect    stdZone;
   Real        stdWidth,stdHeight;

   void initStd(Real xLo,Real yLo,Real xHi,Real yHi) {
      stdZone.xLo = xLo + ZONE_MARGIN; stdZone.yLo = yLo + ZONE_MARGIN;
      stdZone.xHi = xHi - ZONE_MARGIN; stdZone.yHi = yHi - ZONE_MARGIN;
      stdWidth  = stdZone.xHi - stdZone.xLo;
      stdHeight = stdZone.yHi - stdZone.yLo;
   }
};

EXT_GRAPHICS FaceDispInfo     SPfaceDispInfo[FACE_COUNT];

Boolean SPgetWindowSize();
const char *SPinitWindow();
void SPrgbColors(Boolean);
void SPsetFaceColor(Int32);
void SPsetBlack();
ColorTyp SPgetCurrentColor();
void SPclearWindow();
void SPcloseWindow();
void SPflushDisplay();
void SPclearRect(PixCoord,PixCoord,PixCoord,PixCoord);
void SPdrawRectangle(const PixRect &);
void SPdrawPoint(PixCoord,PixCoord);
void SPdrawLine(PixCoord,PixCoord,PixCoord,PixCoord);
void SPdrawText(PixCoord,PixCoord,char*);
void SPdrawLegends(Int32);
void SPsetGC(Int32);
void SPdrawLines(SPXPoint *,Int32);
void SPfillPolygon(SPXPoint *points,Int32 npoints);

