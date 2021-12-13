//  sp/display.cc   Tom Ace   crux@qnet.com

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
// #include <X11/Xlib.h>
// #include <X11/Xutil.h>
// #include <X11/cursorfont.h>
#include <math.h>
#include "sp1.h"
#include "spprocs.h"
#include "graphics.h"

static Int32     polyNr = 0;
static Boolean   useBlack = TRUE;

void SPxyify(Int32 faceNr,Poly &poly,Face &xyFace,Boolean usePrevTranslation)
{
   // Returns xyFace, all of whose points have identical z-coordinates.
   // This face is calculated as a rotation of the original face (which had
   // any orientation in 3-space, according to its position in the polyhedron.)
   // The six vertices of the original face are assumed to be coplanar.
   // It is assumed that poly's vertices and planes are in agreement, and
   // that the fields in poly calculated by poly::deriveMisc() are current.

   XYZ         dirCosines[3];
   static XYZ  prevDirCosines[3];
   Face        face;
   Int32       i,j;
   static Real prevXLo = 0;
   static Real prevYLo = 0;
   Real        xLo,yLo,xHi,yHi;
   Real        x,y;

   //  To do the rotation, we need three unit vectors ("direction cosines"):
   //    0:  Parallel to the edge specified by the face's base vertices;
   //    1:  Normal to #0, and lying in the plane of the face;
   //    2:  The face's normal vector.
   //
   //  We calculate them in the order 2, 0, 1.
   //  2 is expected to be precalculated, e.g. by Poly::derivePlanes();
   //  0 is trivial; 1 is the cross product of 0 and 2.

   for (j=0; j<FACE_THINGS; j++) {
      face.vertices[j] = poly.vertices[SPfaceConfig[faceNr].vertexNr[j]];
   }

   if (usePrevTranslation) {
      for (i=0; i<3; i++) dirCosines[i] = prevDirCosines[i];
   }
   else {
      dirCosines[2] = poly.facePlanes[faceNr].normal;

      for (i=0; i<3; i++) {
         dirCosines[0][i] =
          face.vertices[SPfaceConfig[faceNr].keyVertices[0]][i] -
          face.vertices[SPfaceConfig[faceNr].keyVertices[1]][i];
      }
      dirCosines[0].unitVector(PRESERVE_DIR);

      dirCosines[0].crossProduct(dirCosines[2],dirCosines[1]);
      for (i=0; i<3; i++) prevDirCosines[i] = dirCosines[i];
   }

   // Now use the three direction cosine unit vectors to rotate the
   // coordinates of the six points that comprise the face.

   xLo = yLo = HUGE_REAL;
   for (i=0; i<FACE_THINGS; i++) {
      for (j=0; j<3; j++) {
         xyFace.vertices[i][j] = face.vertices[i].dotProduct(dirCosines[j]);
      }
      x = xyFace.vertices[i][0];
      y = xyFace.vertices[i][1];
      if (x < xLo) xLo = x;
      if (y < yLo) yLo = y;
   }

   xHi = yHi = -HUGE_REAL;

   if (usePrevTranslation) { xLo = prevXLo; yLo = prevYLo; }
   else                    { prevXLo = xLo; prevYLo = yLo; }

   for (i=0; i<FACE_THINGS; i++) {
      x = (xyFace.vertices[i][0] -= xLo);
      y = (xyFace.vertices[i][1] -= yLo);
      if (x > xHi) xHi = x;
      if (y > yHi) yHi = y;
   }

   xyFace.xHi = xHi;
   xyFace.yHi = yHi;
}

static void drawFaces()
{
   Real            faceScale;
   Int32           i,j,jj;
   SPXPoint        points[FACE_THINGS + 1];
   Real            thisFaceScale;
   Real            windowScale;
   Face            xyFaces[FACE_COUNT];

   (void) SPgetWindowSize();

   for (i=0; i<FACE_COUNT; i++) SPdrawLegends(i);

   windowScale = SPmin(Real(SPgraphics.pix.xHi - SPgraphics.pix.xLo) / 1024.,
                       Real(SPgraphics.pix.yHi - SPgraphics.pix.yLo) / 800.);

   SPpoly.derivePlanes();  // ensures appropriate normal vector orientations
   SPpoly.deriveMisc();
   faceScale = HUGE_REAL;
   for (i=0; i<FACE_COUNT; i++) {
      SPxyify(i,SPpoly,xyFaces[i],FALSE);
      thisFaceScale = SPmin(SPfaceDispInfo[i].stdWidth  / xyFaces[i].xHi,
                            SPfaceDispInfo[i].stdHeight / xyFaces[i].yHi);
      if (thisFaceScale < faceScale) faceScale = thisFaceScale;
   }

   for (i=0; i<FACE_COUNT; i++) {
      for (j=0; j<FACE_THINGS; j++) {
         jj = SPfaceConfig[i].drawVertices[j];
         points[j].x = (Int32)((xyFaces[i].vertices[jj][0] * faceScale  +
          SPfaceDispInfo[i].stdZone.xLo) * windowScale) + SPgraphics.pix.xLo;
         points[j].y = SPgraphics.pix.yHi - (Int32)
         ((xyFaces[i].vertices[jj][1] * faceScale +
           SPfaceDispInfo[i].stdZone.yLo) * windowScale);
      }
      points[FACE_THINGS].x = points[0].x;
      points[FACE_THINGS].y = points[0].y;
      SPsetFaceColor(i);
//    SPdrawLines(points,FACE_THINGS + 1);
      SPfillPolygon(points,FACE_THINGS + 1);
   }
}

static void extendXY(Real x,Real y,Real otherX,Real otherY,Real extend,
                     Real &newX,Real &newY)
{
   // (x,y)           at t = 0
   // (otherX,otherY) at t = 1
   // (newX,newY)     at t = tt  extended by length extend
   // x = (otherX - x) * t + x
   // y = (otherY - y) * t + y

   // extend requires deltaT of sqrt(extend^2 / ((otherX-x)^2 + (otherY-y)^2))

   Real    xCoeff = otherX - x;
   Real    yCoeff = otherY - y;

   Real    tt = extend / (-sqrt(xCoeff * xCoeff + yCoeff * yCoeff));
   newX = x + xCoeff * tt;
   newY = y + yCoeff * tt;
}

#ifdef COLOR_TEST_CODE
   for (Int32 i=0; i<40; i++) {
      SPsetColor(i);
      SPdrawLine(19,15 + i*15,239,15+i*15);
      char textBuf[19];
      sprintf(textBuf,"%d",i);
      SPdrawText(242,15+i*15,textBuf);
   };
#endif

static XYZ eye;

static void drawProjPoly(Poly &poly)
{
   Real        closestDist[FACE_COUNT];
   Real        farthestDist[FACE_COUNT];
   XYZ         dirCosines[3];
   Boolean     doIt;
   Int32       i,ii,j,jj;
   Real        meanDist[FACE_COUNT];
   SPXPoint    points[FACE_THINGS + 1];
   XYZ         projXYMin,projXYMax,projXYMid;
   static Real scale = -1.;
   Real        t;          // used in parametric equations for lines
   XYZ         vertexDisp[VERTEX_COUNT];
   XYZ         vertexProj[VERTEX_COUNT];
   XYZ         vertexProjXY[VERTEX_COUNT];
   Plane       windowPlane;

   SPpoly.derivePlanes();  // ensures appropriate normal vector orientations
   poly.deriveMisc();

   for (i=0; i<VERTEX_COUNT; i++) {
      for (j=0; j<3; j++) vertexDisp[i][j] = poly.vertices[i][j] - eye[j];
      poly.eyeDist[i] = vertexDisp[i].magnitude();
      if (poly.eyeDist[i] < poly.minEyeDist) {
         poly.minEyeDist = poly.eyeDist[i];
      }
   }

   // get us a unit vector pointing from the eye to the CG (for window plane)

   for (i=0; i<3; i++) windowPlane.normal[i] = eye[i] - poly.cg[i];
   windowPlane.normal.unitVector(PRESERVE_DIR);

   // generate a suitable locator for the window plane
   windowPlane.locator = (windowPlane.normal.dotProduct(eye) +
                          windowPlane.normal.dotProduct(poly.cg)) / 2.;

   // Project all vertices onto the window plane, and xy-ify them
   // dirCosines:  [0]:  on plane, towards origin
   //              [2]:  plane normal
   //              [1]:  cross product of [0] and [2]

   dirCosines[2] = windowPlane.normal;
   dirCosines[2].crossProduct(eye,dirCosines[1]);
   dirCosines[1].unitVector(PRESERVE_DIR);
   dirCosines[1].crossProduct(dirCosines[2],dirCosines[0]);

   projXYMin.setCoords( HUGE_REAL, HUGE_REAL, HUGE_REAL);
   projXYMax.setCoords(-HUGE_REAL,-HUGE_REAL,-HUGE_REAL);

   for (i=0; i<VERTEX_COUNT; i++) {
      t = (windowPlane.locator - windowPlane.normal.dotProduct(eye)) /
       windowPlane.normal.dotProduct(vertexDisp[i]);
      for (j=0; j<3; j++) vertexProj[i][j] = eye[j] + t * vertexDisp[i][j];
      for (j=0; j<3; j++) {
         vertexProjXY[i][j] = vertexProj[i].dotProduct(dirCosines[j]);
         if (vertexProjXY[i][j] < projXYMin[j]) {
            projXYMin[j] = vertexProjXY[i][j];
         }
         if (vertexProjXY[i][j] > projXYMax[j]) {
            projXYMax[j] = vertexProjXY[i][j];
         }
      }
   }

   for (j=0; j<3; j++) projXYMid[j] = (projXYMin[j] + projXYMax[j]) / 2.;

   // determine the scale
   if (scale < 0.) {
      scale = SPmin(Real(SPgraphics.pix.xHi - SPgraphics.pix.xLo) /
                          (projXYMax[0] - projXYMin[0]),
                    Real(SPgraphics.pix.yHi - SPgraphics.pix.yLo) /
                          (projXYMax[1] - projXYMin[1]));
      scale = scale / 1.5;
   }

   // determine the draw sequence
   for (i=2; i<=3; i++) {
      closestDist [i] =  HUGE_REAL;
      farthestDist[i] = -HUGE_REAL;
      meanDist[i] = 0;
      for (j=0; j<FACE_THINGS; j++) {
         Real dist = poly.eyeDist[SPfaceConfig[i].vertexNr[j]];
         if (dist < closestDist [i]) closestDist [i] = dist;
         if (dist > farthestDist[i]) farthestDist[i] = dist;
         meanDist[i] += dist;
      }
      meanDist[i] /= Real(FACE_THINGS);
   }

// char *drawSequence =
//  closestDist[2] < closestDist[3] ? "\6\3\2\5\4\1\0" : "\6\2\3\5\4\1\0";

// char *drawSequence =
//  farthestDist[2] < farthestDist[3] ? "\6\3\2\5\4\1\0" : "\6\2\3\5\4\1\0";

   const char *drawSequence =
    meanDist[2] < meanDist[3] ? "\6\3\2\5\4\1\0" : "\6\2\3\5\4\1\0";

   if (useBlack) SPrgbColors(FALSE);   // next best thing to double-buffering

   SPflushDisplay();
   for (i=0; i<FACE_COUNT; i++) {
      ii = drawSequence[i];
      // draw the plane iff the eye is on the outside side of it
      doIt = poly.facePlanes[ii].normal.dotProduct(eye) >
             poly.facePlanes[ii].locator;
      if (SPfaceConfig[ii].mirror2) doIt = !doIt;
      if (doIt) {
         SPdrawLegends(ii);
         for (j=0; j<FACE_THINGS; j++) {
            jj = SPfaceConfig[ii].vertexNr[SPfaceConfig[ii].drawVertices[j]];
            points[j].x = SPgraphics.pix.xMid +
             (Int16)((vertexProjXY[jj][0] - projXYMid[0]) * scale);
            points[j].y = SPgraphics.pix.yMid -
             (Int16)((vertexProjXY[jj][1] - projXYMid[1]) * scale);
         }
         points[FACE_THINGS].x = points[0].x;
         points[FACE_THINGS].y = points[0].y;
         SPsetFaceColor(ii);
         SPfillPolygon(points,FACE_THINGS + 1);
         SPsetBlack();
         SPdrawLines(points,FACE_THINGS + 1);
      }
   }

   if (useBlack) SPrgbColors(TRUE);

   // caller will flush the display
}

static Boolean displayFaces = TRUE;

void SPdrawDisplay()
{
   if (!SPgraphicsEnabled) return;

   SPclearWindow();

   if (displayFaces) drawFaces();
   else              drawProjPoly(SPpoly);

   SPflushDisplay();
}

// void SPdrawDisplay()
// {
//    if (!SPgraphicsEnabled) return;
//
//    Real    theta = 0.1;
//    while(TRUE) {
//       SPclearWindow();
//       eye[0] = sin(theta / 1.1);
//       eye[1] = sin(theta / 1.2);
//       eye[2] = sin(theta / 1.3);
//       eye.unitVector(PRESERVE_DIR);
//       eye.scale(100.);
//       drawProjPoly(SPpoly);
//       SPflushDisplay();
//       SPsleep(1);
//       theta += .2;
//    }
// }

void SPinitGraphics()
{
   const char *errorText = SPinitWindow();
   if (errorText != NULL) SP_ERROR(errorText);

   eye.setCoords(100.,100.,100.);
}


#define TOKEN_TEXT(x) (strcmp(token.data.text,(x)) == 0)

typedef enum {unrecognizedToken,decValue,alphaString,comma,plus,dash,
              star,openParen,closeParen,pound,endOfLine} tokenVariety;

class TokenTyp {
public:
   tokenVariety  typ;
   union {
      Int32  value;
      char   *text;
   } data;
};

static Int16      charIndex;
static char       inputLine[1000];
static char       lastLine[1000];
static char       *lineBuf;
static Int16      lineCount;
static char       promptEquiv[] = "         ";

static void parseError(Int32 errNr)
{
   Int16   i;
   static const char *errtext[]={
    /*  0 */ "",
    /*  1 */ "unrecognized command (try HELP)",
    /*  2 */ "eye command parameters must be 3 integer coordinates",
    /*  3 */ "scale command example:  SCALE 119 AGC-GDB [EAG-FGB]",
    /*  4 */ "move vertex example:  MV ABC BCE-BFG moves ABD in the dir BCE-BFG",
    /*  5 */ "revert command requires a saved poly number",
    /*  6 */ "only thing allowed after last vertex is an integer percentage",
    /*  7 */ "randomize:  Z <% std extend> <% tweak normals> <% tweak locators>",
    /*  8 */ "couldn't open poly file (in directory \"polys\") for input",
    /*  9 */ "couldn't open poly file (in directory \"polys\") for output",
    /* 10 */ "syntax error in polyhedron vertex file",
    /* 11 */ "vertex file with that name exists; there is no overwrite function",
    /* 12 */ "poly number out of range",
    /* 13 */ "syntax:  MMDA <% std extend> <% tweak normals> <% tweak locators>",
    /* 14 */ "thickness (integer number of mils) required",
    /* 15 */ "planify precision spec must be an integer from 4-10",
    /* 16 */ "you must specify two different vertices",
    /* 17 */ "oblique command example:  OB 19 AGC-GDB EAG-FGB",
    /* 11 */ "Postscript file with that name exists; there is no overwrite function",
    /* 19 */ "couldn't open Postscript file (in directory \"ps\") for output",
    /* 20 */ "you must specify a file name",
    /* 21 */ "faces are not flat enough; invoke \"PL 10\" before printing",
    /* 22 */ "the two lines may not be parallel (nor the same line)",
    /* 23 */ "",
    /* NN */ ""};
   char    spaceBuf[200];

   if (lineBuf != inputLine) {
      fprintf(stderr,"An error was detected in the following subexpression:\n");
      fprintf(stderr,"%s%s\n",promptEquiv,lineBuf);
   }
   for (i=1; i<charIndex; i+=1) spaceBuf[i-1] = ' ';
   spaceBuf[charIndex-1] = '\0';
   fprintf(stderr,"%s%s^\n",promptEquiv,spaceBuf);
   fprintf(stderr,"Error: %s\n",errtext[errNr]);
}


static Boolean getLine()
{
   Int16  ch;

   lineBuf = inputLine;
   do {
      ch = getc(stdin);
      if (ch == EOF) {
         fprintf(stderr,"Warning:  unexpected EOF while reading commands\n");
         return FALSE;
      }
      if (ch == '\n') *lineBuf = '\0';
      else            *lineBuf++ = ch;
   } while (ch != '\n');
   lineBuf = inputLine;
   charIndex = 0;
   lineCount += 1;
   return TRUE;
}

static char rawNextC()
{
   Int16  ch;
   do {
      ch = lineBuf[charIndex++];
   } while (ch == ' ' || ch == '\t');
   return ch;
}

static inline char nextC()
{
   return toupper(rawNextC());
}

static char *restOfLine()
{
   static char      textBuf[100];
   char             *stringPtr = textBuf;

   while ((*stringPtr++ = rawNextC()) != '\0') {}
   return textBuf;
}

static TokenTyp *nextToken()
{
   Int16            ch;
   Boolean          moreDigits;
   Boolean          moreLetters;
   Boolean          negated = FALSE;
   char             *stringPtr;
   static char      textBuf[100];
   static TokenTyp  token;

#define IS_ALPHA(x) \
 (isalpha(x) || isdigit(x) || (x) == '!' || (x) == '/' || (x) == '.')
/*  As you can see, our concept of "alpha" is somewhat broad.  */

   ch = nextC();
   if (ch == '-') {
      negated = TRUE;
      ch = nextC();
   }
   if (isdigit(ch)) {
      token.typ = decValue;
      token.data.value = 0;
      do {
         token.data.value = token.data.value * 10 + ch - '0';
         moreDigits = isdigit(lineBuf[charIndex]);
         if (moreDigits) ch = nextC();
      } while (moreDigits);
      if (negated) token.data.value = -token.data.value;
      return &token;
   }
   if (IS_ALPHA(ch)) {
      token.typ = alphaString;
      stringPtr = textBuf;
      do {
         *stringPtr++ = ch;
         moreLetters = IS_ALPHA(lineBuf[charIndex]);
         if (moreLetters) ch = nextC();
      } while (moreLetters);
      *stringPtr = '\0';
      token.data.text = textBuf;
      return &token;
   }
   switch (ch) {
      case ',':
         token.typ = comma;
         break;
      case '-':
         token.typ = dash;
         break;
      case '+':
         token.typ = plus;
         break;
      case '*':
         token.typ = star;
         break;
      case '(':
         token.typ = openParen;
         break;
      case ')':
         token.typ = closeParen;
         break;
      case '#':
         token.typ = pound;
         break;
      case '\0':
         token.typ = endOfLine;
         break;
      default:
         token.typ = unrecognizedToken;
         break;
      }
   return &token;
}

static char *psFileName(char *fileName)
{
   static char     pfn[239];

   if (*fileName == '\0') return NULL;
   strcpy(pfn,"ps/");
   strcpy(pfn+3,fileName);
   return pfn;
}

static Boolean vertexOnFace(Int32 vertexNr,Int32 faceNr)
{
   Int32         i;

   for (i=0; i<VERTEX_THINGS; i++) {
      if (SPvertexConfig[vertexNr].faceNr[i] == faceNr) return TRUE;
   }

   return FALSE;
}

static void printFaces(Boolean thickness)
{
   Real           edgeLen[FACE_COUNT];
   Real           faceScale;
   char           *fileName;
   Int32          i,ii,j,jj,j1,j2;
   Real           locatorAdjust;
   Real           letterCount;
   FILE           *pf;
   Real           printWidth,printHeight;
   Poly           tempPoly;
   Real           thisFaceScale;
   Real           x,y;
   Real           x1,y1;
   Real           x2,y2;
   Real           xp1,yp1;
   Real           xp2,yp2;
   Face           xyFace,xyFace2;

   fileName = psFileName(restOfLine());
   if (!fileName) {
      parseError(20);
      return;
   }
   pf = fopen(fileName,"r");
   if (pf != NULL) {
      parseError(18);
      return;
   }
   fclose(pf);
   pf = fopen(fileName,"w");
   if (pf == NULL) {
      parseError(19);
      return;
   }

   if (SPpoly.maxDeviation() > .000000001) {
      parseError(21);
      return;
   }

   printf("writing Postscript for face patterns to file \"%s\"\n",fileName);

   SPpoly.derivePlanes();  // ensures appropriate normal vector orientations
   SPpoly.deriveMisc();

   printWidth = 7.5;
   printHeight = 10.;

   faceScale = HUGE_REAL;
   for (i=0; i<FACE_COUNT; i++) {
      SPxyify(i,SPpoly,xyFace,FALSE);
      thisFaceScale = SPmin(printWidth  / xyFace.xHi,
                            printHeight / xyFace.yHi);
      if (thisFaceScale < faceScale) faceScale = thisFaceScale;
   }

   fprintf(pf,"%%!PS-Adobe-1.0\n");
   fprintf(pf,"/mv {moveto} def\n");
   fprintf(pf,"/rmv {rmoveto} def\n");
   fprintf(pf,"/ls {lineto stroke} def\n");
   fprintf(pf,"/lt {lineto} def\n");
   fprintf(pf,"/np {newpath} def\n");
   fprintf(pf,"/cp {currentpoint} def\n");

   for (i=0; i<FACE_COUNT; i++) {
      fprintf(pf,"72.19 72.32 scale   %% inches on Apple 360 printer\n");
      fprintf(pf,"0.5 0.5 translate\n");
      fprintf(pf,"1 setlinejoin 1 setlinecap\n");
      fprintf(pf,"/Helvetica-Bold findfont .5 scalefont setfont\n");
      fprintf(pf,"np [] 0 setdash 1 300 div setlinewidth\n");
      SPxyify(i,SPpoly,xyFace,FALSE);
      for (j=0; j<FACE_THINGS; j++) {
         jj = SPfaceConfig[i].drawVertices[j];
         x = xyFace.vertices[jj][0] * faceScale;
         y = xyFace.vertices[jj][1] * faceScale;
         fprintf(pf,"%.4f %.4f %s\n",x,y,j == 0 ? "mv" : "lt");
      }
      fprintf(pf,"closepath .8 setgray fill\n");

      SPxyify(i,SPpoly,xyFace,FALSE);

      ii = 0;
      fprintf(pf,"0 setgray 1 300 div setlinewidth\n");
      for (j=0; j<FACE_THINGS; j++) {
         jj = SPfaceConfig[i].drawVertices[j];
         x1 = xyFace.vertices[jj][0] * faceScale;
         y1 = xyFace.vertices[jj][1] * faceScale;
         jj = SPfaceConfig[i].drawVertices[(j + 1) % FACE_THINGS];
         x2 = xyFace.vertices[jj][0] * faceScale;
         y2 = xyFace.vertices[jj][1] * faceScale;
         extendXY(x1,y1,x2,y2,.5,xp1,yp1);
         extendXY(x2,y2,x1,y1,.5,xp2,yp2);
         fprintf(pf,"np %.4f %.4f mv %.4f %.4f ls\n",
                 xp1,yp1,xp2,yp2);
      }

      if (SPthickness > 0.) {
         for (ii=0; ii<FACE_COUNT; ii++) {
            // find alternate position for each edge according to face thickness
            // (this entails sliding each other face inward by face thickness)
            if (ii != i) {
               locatorAdjust = SPthickness / faceScale;
               if (SPfaceConfig[ii].mirror2) locatorAdjust = -locatorAdjust;
               tempPoly = SPpoly;
               tempPoly.facePlanes[ii].locator -= locatorAdjust;
               tempPoly.deriveVertices();
               tempPoly.deriveMisc();
               SPxyify(i,tempPoly,xyFace2,TRUE);
               for (j=0; j<FACE_THINGS; j++) {
                  j1 = SPfaceConfig[i].drawVertices[j];
                  j2 = SPfaceConfig[i].drawVertices[(j + 1) % FACE_THINGS];
                  if (vertexOnFace(SPfaceConfig[i].vertexNr[j1],ii) &&
                      vertexOnFace(SPfaceConfig[i].vertexNr[j2],ii)) {
                     x1 = xyFace2.vertices[j1][0] * faceScale;
                     y1 = xyFace2.vertices[j1][1] * faceScale;
                     x2 = xyFace2.vertices[j2][0] * faceScale;
                     y2 = xyFace2.vertices[j2][1] * faceScale;
                     extendXY(x1,y1,x2,y2,.5,xp1,yp1);
                     extendXY(x2,y2,x1,y1,.5,xp2,yp2);
                     fprintf(pf,"np %.4f %.4f mv %.4f %.4f ls\n",
                             xp1,yp1,xp2,yp2);
                  }
               }
            }
         }
      }

      // draw face ID char at CG of the 'letter vertices'
      x = y = letterCount = 0.;
      for (j=0; j<4; j++) {
         jj = SPfaceConfig[i].letterVertices[j];
         if (jj >= 0) {
            x += xyFace.vertices[jj][0] * faceScale;
            y += xyFace.vertices[jj][1] * faceScale;
            letterCount += 1.;
         }
      }
      x /= letterCount;
      y /= letterCount;

      fprintf(pf,".9 setgray\n");
      fprintf(pf,"/Helvetica-Bold findfont 36 72 div scalefont setfont\n");
      fprintf(pf,"%.4f %.4f mv (%c) show\n",x - .1,y - .1,FACE_CHAR(i));

      fprintf(pf,"/Helvetica-Bold findfont 24 72 div scalefont setfont\n");

      for (j=0; j<FACE_THINGS; j++) {
         Int32 j0 = SPfaceConfig[i].drawVertices[j];
         Int32 j1 = SPfaceConfig[i].drawVertices[(j + 1) % FACE_THINGS];
         x = (xyFace.vertices[j0][0] + xyFace.vertices[j1][0]) * faceScale / 2.;
         y = (xyFace.vertices[j0][1] + xyFace.vertices[j1][1]) * faceScale / 2.;
         Real deltaX = xyFace.vertices[j0][0] - xyFace.vertices[j1][0];
         Real deltaY = xyFace.vertices[j0][1] - xyFace.vertices[j1][1];
         edgeLen[SPfaceConfig[i].otherFaces[j]] =
          sqrt(deltaX * deltaX + deltaY * deltaY) * faceScale;
         y = (xyFace.vertices[j0][1] + xyFace.vertices[j1][1]) * faceScale / 2.;
         fprintf(pf,"%.4f %.4f mv (%c) show\n",x - .1,y - .1,
                 FACE_CHAR(SPfaceConfig[i].otherFaces[j]));
      }



      fprintf(pf,".5 9.8 mv 0 setgray\n");
      fprintf(pf,"cp (face %c) show moveto 0 -.3 rmv\n",FACE_CHAR(i));
      fprintf(pf,"/Helvetica findfont 12 72 div scalefont setfont\n");
      fprintf(pf,"cp (Szilassi polyhedron pattern) show mv 0 -.2 rmv\n");
      fprintf(pf,"cp (Tom Ace    crux@qnet.com) show mv 0 -.2 rmv\n");
      fprintf(pf,"cp (http://www.qnet.com/~crux/) show mv 0 -.2 rmv\n");

      if (SPthickness > 0.) {
         fprintf(pf,"0 -.1 rmv (material thickness = %.3f\") show\n",
                 SPthickness);
      }

      fprintf(pf,"3.5 9.8 mv\n");
      for (ii=0; ii<FACE_COUNT; ii++) {
         if (ii != i) {
            Real dihedralRad = acos(-SPpoly.facePlanes[i ].normal.dotProduct(
                                     SPpoly.facePlanes[ii].normal));
            Real dihedralDeg = dihedralRad * 180. / 3.1415926535897932384;
            fprintf(pf,
             "cp ( angle with face %c = %5.1f    edge length = %.3f\")\n",
             FACE_CHAR(ii),dihedralDeg,edgeLen[ii]);
            fprintf(pf,"show mv 0 -0.2 rmv\n");
         }
      }

      fprintf(pf,"showpage\n");
   }
   fprintf(pf,"%c",'\4');
   fclose(pf);
}

static void dispFaces()
{
   displayFaces = TRUE;
}

static void disp3D()
{
   displayFaces = FALSE;
}

static void setEye()
{
   XYZ     tempEye;

   TokenTyp  token = *nextToken();
   if (token.typ == endOfLine) {
      printf("eye: %10.5f,%10.5f,%10.5f\n",eye[0],eye[1],eye[2]);
   }
   else {
      if (token.typ != decValue) goto eyeError;
      tempEye[0] = (Real)token.data.value;
      token = *nextToken();
      if (token.typ != decValue) goto eyeError;
      tempEye[1] = (Real)token.data.value;
      token = *nextToken();
      if (token.typ != decValue) goto eyeError;
      tempEye[2] = (Real)token.data.value;

      // fix any eye locations too close to the origin
      while (tempEye.magnitude() < 50.) {
         tempEye[0] += .0001;
         tempEye.scale(2.);
      }
      eye = tempEye;
      disp3D();
      SPdrawDisplay();
   }
   return;

   eyeError:;
   parseError(2);
}

const Int32  maxPolys = 1919;  // should be plenty
static Poly *savedPolys;

static void nextPoly()
{
   polyNr = (polyNr + 1) % maxPolys;
}

static Int32 getVertexNr(char *vertexText)
{
   Int32       faceNr[VERTEX_THINGS];
   Int32       i,j;
   Int32       vertexNr;

   if (strlen(vertexText) < VERTEX_THINGS) return -19;
   for (i=0; i<VERTEX_THINGS; i++) {
      faceNr[i] = toupper(vertexText[i]) - 'A';
      if (faceNr[i] < 0 || faceNr[i] >= VERTEX_COUNT) return -19;
   }
   for (i=0; i<VERTEX_THINGS; i++) {
      if (faceNr[i] == faceNr[(i + 1) % VERTEX_THINGS]) return -19;
   }
   for (vertexNr=0; vertexNr<VERTEX_COUNT; vertexNr++) {
      for (i=0; i<VERTEX_THINGS; i++) {
         for (j=0; j<VERTEX_THINGS; j++) {
            if (SPvertexConfig[vertexNr].faceNr[j] == faceNr[i]) goto gotFace;
         }
         goto nextVertex;
         gotFace:;
      }
      goto gotVertex;
      nextVertex:;
   }
   return -19;

   gotVertex:;
   return vertexNr;
}

static void scalePoly()
{
   XYZ         dirCosines[3],invDirCosines[3];
   Int32       i,j;
   TokenTyp    token;
   Real        scale;
   XYZ         scratchXYZ,scratchXYZ2;
   Int32       v1,v2;

   token = *nextToken();
   if (token.typ != decValue) goto scaleError;
   scale = (Real)token.data.value / 100.;

   token = *nextToken();
   if (token.typ != alphaString) goto scaleError;
   v1 = getVertexNr(token.data.text);
   if (v1 < 0) goto scaleError;

   token = *nextToken();
   if (token.typ != alphaString) goto scaleError;
   v2 = getVertexNr(token.data.text);
   if (v2 < 0) goto scaleError;
   if (v2 == v1) goto scaleError2;

   token = *nextToken();
   if (token.typ == endOfLine) {
      SPpoly.vertices[v1].subtract(SPpoly.vertices[v2],dirCosines[0]);
   }
   else {
      //  If two vectors are specified, scale along their cross product.
      SPpoly.vertices[v1].subtract(SPpoly.vertices[v2],scratchXYZ);
      if (token.typ != alphaString) goto scaleError;
      v1 = getVertexNr(token.data.text);
      if (v1 < 0) goto scaleError;

      token = *nextToken();
      if (token.typ != alphaString) goto scaleError;
      v2 = getVertexNr(token.data.text);
      if (v2 < 0) goto scaleError;
      if (v2 == v1) goto scaleError2;
      SPpoly.vertices[v1].subtract(SPpoly.vertices[v2],scratchXYZ2);
      scratchXYZ.crossProduct(scratchXYZ2,dirCosines[0]);
      if (dirCosines[0].magnitude() < 2. * LITTLE_REAL) goto scaleError3;
   }

   //  Rotate so that scaling axis is the x axis.  Generate three unit vectors:
   //    0:  Parallel to scaling axis
   //    1:  Some arbitrary normal to #0
   //    2:  normal to 0 and 1

   dirCosines[0].unitVector(PRESERVE_DIR);  // set up above; adjust length
   dirCosines[0].crossProduct(SPoneOneOne,dirCosines[1]);
   if (dirCosines[1].magnitude() < .01) {
      printf("scale axis nearly parallel to <1,1,1> (not an error)\n");
      scratchXYZ.setCoords(-1.,1.,2.);
      dirCosines[0].crossProduct(scratchXYZ,dirCosines[1]);
   }
   dirCosines[1].unitVector(PRESERVE_DIR);
   dirCosines[0].crossProduct(dirCosines[1],dirCosines[2]);

   // Generate the set of dir cosines that inverts the rotation.

   for (i=0; i<3; i++) {
      for (j=0; j<3; j++) {
         invDirCosines[i][j] = dirCosines[j][i];
      }
   }

// printf("product of dirCosines and its alleged inverse:\n");
// for (i=0; i<3; i++) {
//    for (j=0; j<3; j++) {
//       for (Int32 k=0; k<3; k++) {
//          scratchXYZ[k] = invDirCosines[k][j];
//       }
//       printf("%10.5f",dirCosines[i].dotProduct(scratchXYZ));
//    }
//    printf("\n");
// }

   // Use the direction cosine matrix to rotate the coordinates
   // of all the polyhedron's vertices.  Scale the rotated x coords.
   // Then do the inverse rotation.

   for (i=0; i<VERTEX_COUNT; i++) {
      for (j=0; j<3; j++) {
         scratchXYZ[j] = SPpoly.vertices[i].dotProduct(dirCosines[j]);
      }
      scratchXYZ[0] *= scale;
      for (j=0; j<3; j++) {
         SPpoly.vertices[i][j] = scratchXYZ.dotProduct(invDirCosines[j]);
      }
   }

   nextPoly();
   SPdrawDisplay();
   return;

   scaleError:;
   parseError(3);
   return;

   scaleError2:;
   parseError(16);
   return;

   scaleError3:;
   parseError(22);
   return;
}

static void obliquePoly()
{
   XYZ         dirCosines[3],invDirCosines[3];
   Int32       i,j;
   TokenTyp    token;
   Real        scale;
   XYZ         scratchXYZ;
   Int32       v1,v2;

   token = *nextToken();
   if (token.typ != decValue) goto obliqueError;
   scale = (Real)token.data.value / 100.;

   //   ------       -----
   //   |    |  ->  /    /
   //   ------      -----
   //  two vectors:  first is like x in the above diagram, second is like y

   token = *nextToken();
   if (token.typ != alphaString) goto obliqueError;
   v1 = getVertexNr(token.data.text);
   if (v1 < 0) goto obliqueError;

   token = *nextToken();
   if (token.typ != alphaString) goto obliqueError;
   v2 = getVertexNr(token.data.text);
   if (v2 < 0) goto obliqueError;
   if (v2 == v1) goto obliqueError2;
   SPpoly.vertices[v2].subtract(SPpoly.vertices[v1],dirCosines[0]);
   dirCosines[0].unitVector(PRESERVE_DIR);

   token = *nextToken();
   if (token.typ != alphaString) goto obliqueError;
   v1 = getVertexNr(token.data.text);
   if (v1 < 0) goto obliqueError;

   token = *nextToken();
   if (token.typ != alphaString) goto obliqueError;
   v2 = getVertexNr(token.data.text);
   if (v2 < 0) goto obliqueError;
   if (v2 == v1) goto obliqueError2;
   SPpoly.vertices[v2].subtract(SPpoly.vertices[v1],scratchXYZ);
   dirCosines[0].crossProduct(scratchXYZ,dirCosines[2]);
   dirCosines[2].unitVector(PRESERVE_DIR);
   dirCosines[2].crossProduct(dirCosines[0],dirCosines[1]);

   //  Our three direction cosine unit vectors are now as follows:
   //    0:  Parallel to first vector
   //    1:  parallel to second vector
   //    2:  normal to 0 and 1

   // Generate the set of dir cosines that inverts the rotation.

   for (i=0; i<3; i++) {
      for (j=0; j<3; j++) {
         invDirCosines[i][j] = dirCosines[j][i];
      }
   }

// printf("product of dirCosines and its alleged inverse:\n");
// for (i=0; i<3; i++) {
//    for (j=0; j<3; j++) {
//       for (Int32 k=0; k<3; k++) {
//          scratchXYZ[k] = invDirCosines[k][j];
//       }
//       printf("%10.5f",dirCosines[i].dotProduct(scratchXYZ));
//    }
//    printf("\n");
// }

   // Use the direction cosine matrix to rotate the coordinates
   // of all the polyhedron's vertices.  Oblique the rotated x coords.
   // Then do the inverse rotation.

   for (i=0; i<VERTEX_COUNT; i++) {
      for (j=0; j<3; j++) {
         scratchXYZ[j] = SPpoly.vertices[i].dotProduct(dirCosines[j]);
      }
      scratchXYZ[0] += scratchXYZ[1] * scale;
      for (j=0; j<3; j++) {
         SPpoly.vertices[i][j] = scratchXYZ.dotProduct(invDirCosines[j]);
      }
   }

   nextPoly();
   SPdrawDisplay();
   return;

   obliqueError:;
   parseError(17);
   return;

   obliqueError2:;
   parseError(16);
}

static Boolean fixedVertices[VERTEX_COUNT];

static void moveVertex()
{
   XYZ           diff;
   Int32         dvTo,dvFrom,vertexNr;
   Int32         mvErrorNr = 4;
   Real          percentage = 100.;
   TokenTyp      token;

   token = *nextToken();
   if (token.typ != alphaString) goto mvError;
   vertexNr = getVertexNr(token.data.text);
   if (vertexNr < 0) goto mvError;

   token = *nextToken();
   if (token.typ != alphaString) goto mvError;
   dvFrom = getVertexNr(token.data.text);
   if (dvFrom < 0) goto mvError;

   token = *nextToken();
   if (token.typ != alphaString) goto mvError;
   dvTo = getVertexNr(token.data.text);
   if (dvTo < 0) goto mvError;
   if (dvFrom == dvTo) goto mvError2;

   token = *nextToken();
   if (token.typ != endOfLine) {
      if (token.typ != decValue) {
         mvErrorNr = 5;
         goto mvError;
      }
      percentage = (Real)token.data.value;
   }

   SPpoly.vertices[dvTo].subtract(SPpoly.vertices[dvFrom],diff);
   diff.unitVector(PRESERVE_DIR);
   diff.scale(percentage * SPpoly.maxCGDist / 1900.);

   SPpoly.vertices[vertexNr].add(diff,SPpoly.vertices[vertexNr]);
   fixedVertices[vertexNr] = TRUE;
   SPdrawDisplay();
   nextPoly();
   return;

   mvError:;
   parseError(mvErrorNr);
   return;

   mvError2:;
   parseError(16);
}

static void clearFixedVertices()
{
   for (Int32 i=0; i<VERTEX_COUNT; i++) fixedVertices[i] = FALSE;
}

static void planify() {
   Real      precision = 1.E-4;
   TokenTyp  token = *nextToken();
   if (token.typ == decValue) {
      if (token.data.value < 4 || token.data.value > 10) {
         parseError(15);
         return;
      }
      precision = 1.;
      for (Int32 i=0; i<token.data.value; i++) precision *= .1;
   }
   else if (token.typ != endOfLine) {
      parseError(15);
      return;
   }
   SPpoly.planify(fixedVertices,precision);
   clearFixedVertices();
   nextPoly();
   SPdrawDisplay();
}

static void revert()
{
   TokenTyp  token = *nextToken();
   if (token.typ != decValue) {
      parseError(5);
      return;
   }
   if (token.data.value < 0 || token.data.value > polyNr) {
      parseError(12);
      return;
   }
   SPpoly = savedPolys[token.data.value];
   nextPoly();
   SPdrawDisplay();
   return;
}

static void eyeRotateA()
{
   XYZ    dirCosines[3];
   XYZ    rotEye;

   dirCosines[0].setCoords(0.,-1.,0.);
   dirCosines[1].setCoords(0.,0.,1.);
   dirCosines[2].setCoords(1.,0.,0.);

   for (Int32 i=0; i<3; i++) rotEye[i] = eye.dotProduct(dirCosines[i]);
   eye = rotEye;
   disp3D();
   SPdrawDisplay();
}

static void eyeRotateB()
{
   XYZ    dirCosines[3];
   XYZ    rotEye;

   dirCosines[0].setCoords(0.,1.,0.);
   dirCosines[1].setCoords(-1.,0.,0.);
   dirCosines[2].setCoords(0.,0.,1.);

   for (Int32 i=0; i<3; i++) rotEye[i] = eye.dotProduct(dirCosines[i]);
   eye = rotEye;
   disp3D();
   SPdrawDisplay();
}

const Int32 ANIMATE_COSINES_COUNT = 3;
static XYZ   animateDirCosines[ANIMATE_COSINES_COUNT][3];

extern "C" void SPinitSig();
extern "C" int SPsigInt();

static void animateInit()
{
   animateDirCosines[0][0].setCoords(  // 01 -++    // 0 +++
    0.98787833990721308, 0.10976425998969035, 0.10976425998969035);
   animateDirCosines[0][1].setCoords(
    -0.11842292937033572, 0.99007076996827192, 0.075735594364749581);
   animateDirCosines[0][2].setCoords(
    -0.10036132393266421, -0.087816158441081177, 0.99106807383505902);

   animateDirCosines[1][0].setCoords( // 01 +--     // 0 +-+
    0.98787833990721308, -0.10976425998969035, 0.10976425998969035);
   animateDirCosines[1][1].setCoords(
    0.11842292937033572, 0.99007076996827192, -0.075735594364749581);
   animateDirCosines[1][2].setCoords(
    -0.10036132393266421, 0.087816158441081177, 0.99106807383505902);

   animateDirCosines[2][0].setCoords( // 01 -++     // 0 ++-
    0.98787833990721308, 0.10976425998969035, -0.10976425998969035);
   animateDirCosines[2][1].setCoords(
    -0.11842292937033572, 0.99007076996827192, -0.075735594364749581);
   animateDirCosines[2][2].setCoords(
    0.10036132393266421, 0.087816158441081177, 0.99106807383505902);

   SPinitSig();
}

static void animate()
{
   Int32     bestCosines;
   Int32     i,j;
   Real      minRotDot;
   Real      rotDot;
   XYZ       rotEye;

   disp3D();

   minRotDot = HUGE_REAL;

   for (i=0; i<ANIMATE_COSINES_COUNT; i++) {
      for (j=0; j<3; j++) {
         rotEye[j] = eye.dotProduct(animateDirCosines[i][j]);
      }
      rotDot = eye.dotProduct(rotEye);
      if (rotDot < minRotDot) {
         minRotDot = rotDot;
         bestCosines = i;
      }
   }

// printf("bestCosines = %d\n",bestCosines);

   while (!SPsigInt()) {
      for (j=0; j<3; j++) {
         rotEye[j] = eye.dotProduct(animateDirCosines[bestCosines][j]);
      }
      eye = rotEye;
      SPdrawDisplay();
      SPsleep(1);
   }
}

static Poly       randomInit;

static void saveRandomPolyInit()
{
   randomInit = SPpoly;
}

static void generateRandomPoly()
{
   //  tweakNormals:        +/- up to n% of oneOneOne      default n=5
   //  tweakLocators:       +/- n% of maxCGDist/19         default n=5

   Real       tweakNormals = 5.;
   Real       tweakLocators = 5.;
   Real       extendPercent = 100.;

   Int32      attemptCount;
   Boolean    goodPoly;
   Int32      i,j;
   Real       locatorDeltaMax;
   Real       normalDeltaMax;
   Poly       savePoly = SPpoly;

   TokenTyp  token = *nextToken();
   if (token.typ == endOfLine) goto gotParams;
   if (token.typ == decValue) extendPercent = (Real)token.data.value;
   else goto generateRandomParseError;
   token = *nextToken();
   if (token.typ == endOfLine) goto gotParams;
   if (token.typ == decValue) tweakNormals = (Real)token.data.value;
   else goto generateRandomParseError;
   token = *nextToken();
   if (token.typ == endOfLine) goto gotParams;
   if (token.typ == decValue) tweakLocators = (Real)token.data.value;
   else goto generateRandomParseError;

   gotParams:;

   normalDeltaMax  = tweakNormals / 100.;
   locatorDeltaMax = tweakLocators * randomInit.maxCGDist / 1900.;
   attemptCount = 0;

   do {
      attemptCount++;
      for (i=0; i<FACE_COUNT; i++) {
         for (j=0; j<3; j++) {
            SPpoly.facePlanes[i].normal[j] =
             randomInit.facePlanes[i].normal[j] +
             SPrandomPlusMinus(normalDeltaMax);
         }
         SPpoly.facePlanes[i].normal.unitVector(PRESERVE_DIR);
         SPpoly.facePlanes[i].locator = randomInit.facePlanes[i].locator +
          SPrandomPlusMinus(locatorDeltaMax);
      }
      SPpoly.deriveVertices();
      goodPoly = SPpoly.anyGood(extendPercent);
   } while (!goodPoly && !SPsigInt());

   printf("Z:  %s after %d attempts\n",
          goodPoly ? "success" : "interrupted",attemptCount);

   if (!goodPoly) {
      SPpoly = savePoly;
   }
   else {
      SPpoly.derivePlanes();  // ensures appropriate normal vector orientations
      nextPoly();
      SPdrawDisplay();
   }
   return;

   generateRandomParseError:;
   parseError(7);
}

static void maximizeMinDihedralAngle(Boolean retainLast)
{
   //  tweakNormals:        +/- up to n% of oneOneOne      default n=5
   //  tweakLocators:       +/- n% of maxCGDist/19         default n=5

   Real       tweakNormals = 5.;
   Real       tweakLocators = 5.;
   Real       extendPercent = 100.;

   Int32      attemptCount,changeCount;
   Poly       bestPoly = SPpoly;
   Int32      i,j;
   Real       locatorDeltaMax;
   Real       minDA,bestMinDA;
   Poly       mmdaInit = SPpoly;
   Real       normalDeltaMax;

   TokenTyp  token = *nextToken();
   if (token.typ == endOfLine) goto gotParams;
   if (token.typ == decValue) extendPercent = (Real)token.data.value;
   else goto mmdaError;;
   token = *nextToken();
   if (token.typ == endOfLine) goto gotParams;
   if (token.typ == decValue) tweakNormals = (Real)token.data.value;
   else goto mmdaError;;
   token = *nextToken();
   if (token.typ == endOfLine) goto gotParams;
   if (token.typ == decValue) tweakLocators = (Real)token.data.value;
   else goto mmdaError;;

   gotParams:;

   normalDeltaMax  = tweakNormals / 100.;
   locatorDeltaMax = tweakLocators * mmdaInit.maxCGDist / 1900.;
   attemptCount = changeCount = 0;

   bestMinDA = bestPoly.minDihedralAngle();

   do {
      attemptCount++;
      for (i=0; i<FACE_COUNT; i++) {
         for (j=0; j<3; j++) {
            SPpoly.facePlanes[i].normal[j] =
             mmdaInit.facePlanes[i].normal[j] +
             SPrandomPlusMinus(normalDeltaMax);
         }
         SPpoly.facePlanes[i].normal.unitVector(PRESERVE_DIR);
         SPpoly.facePlanes[i].locator = mmdaInit.facePlanes[i].locator +
          SPrandomPlusMinus(locatorDeltaMax);
      }
      SPpoly.deriveVertices();
      if (SPpoly.anyGood(extendPercent)) {
         SPpoly.derivePlanes();
         minDA = SPpoly.minDihedralAngle();
         if (minDA > bestMinDA) {
            bestPoly = SPpoly;
            if (retainLast) mmdaInit = bestPoly;
            bestMinDA = minDA;
            printf("min dihedral angle: %.3f\n",minDA);
            changeCount++;
         }
      }
   } while (!SPsigInt());

   printf("Z:  %d changes after %d attempts; min dihedral angle = %.3f\n",
          changeCount,attemptCount,bestMinDA);

   SPpoly = bestPoly;  // derivePlanes done already
   nextPoly();
   SPdrawDisplay();

   return;

   mmdaError:;
   parseError(13);
}

static void maximizeMeanLowDA(Boolean retainLast)
{
   //  tweakNormals:        +/- up to n% of oneOneOne      default n=5
   //  tweakLocators:       +/- n% of maxCGDist/19         default n=5

   Real       tweakNormals = 5.;
   Real       tweakLocators = 5.;
   Real       extendPercent = 100.;

   Int32      attemptCount,changeCount;
   Poly       bestPoly = SPpoly;
   Int32      i,j;
   Real       locatorDeltaMax;
   Real       mlda,bestMLDA;
   Poly       mmdaInit = SPpoly;
   Real       normalDeltaMax;

   TokenTyp  token = *nextToken();
   if (token.typ == endOfLine) goto gotParams;
   if (token.typ == decValue) extendPercent = (Real)token.data.value;
   else goto mmdaError;;
   token = *nextToken();
   if (token.typ == endOfLine) goto gotParams;
   if (token.typ == decValue) tweakNormals = (Real)token.data.value;
   else goto mmdaError;;
   token = *nextToken();
   if (token.typ == endOfLine) goto gotParams;
   if (token.typ == decValue) tweakLocators = (Real)token.data.value;
   else goto mmdaError;;

   gotParams:;

   normalDeltaMax  = tweakNormals / 100.;
   locatorDeltaMax = tweakLocators * mmdaInit.maxCGDist / 1900.;
   attemptCount = changeCount = 0;

   bestMLDA = bestPoly.meanLowDihedralAngle();

   do {
      attemptCount++;
      for (i=0; i<FACE_COUNT; i++) {
         for (j=0; j<3; j++) {
            SPpoly.facePlanes[i].normal[j] =
             mmdaInit.facePlanes[i].normal[j] +
             SPrandomPlusMinus(normalDeltaMax);
         }
         SPpoly.facePlanes[i].normal.unitVector(PRESERVE_DIR);
         SPpoly.facePlanes[i].locator = mmdaInit.facePlanes[i].locator +
          SPrandomPlusMinus(locatorDeltaMax);
      }
      SPpoly.deriveVertices();
      if (SPpoly.anyGood(extendPercent)) {
         SPpoly.derivePlanes();
         mlda = SPpoly.meanLowDihedralAngle();
         if (mlda > bestMLDA) {
            bestPoly = SPpoly;
            if (retainLast) mmdaInit = bestPoly;
            bestMLDA = mlda;
            printf("mean low dihedral angle: %.3f\n",mlda);
            changeCount++;
         }
      }
   } while (!SPsigInt());

   printf("Z:  %d changes after %d attempts; mean low dihedral angle = %.3f\n",
          changeCount,attemptCount,bestMLDA);

   SPpoly = bestPoly;  // derivePlanes done already
   nextPoly();
   SPdrawDisplay();

   return;

   mmdaError:;
   parseError(13);
}

static void setThickness()
{
   TokenTyp  token = *nextToken();

   if (token.typ == decValue && token.data.value >= 0) {
      SPthickness = (Real)token.data.value / 1000.;
   }
   else {
      parseError(14);
   }
}

static char *polyFileName(char *fileName)
{
   static char     pfn[239];

   if (*fileName == '\0') return NULL;
   strcpy(pfn,"polys/");
   strcpy(pfn+6,fileName);
   return pfn;
}

static void writePoly()
{
   char             *fileName;
   Int32            i,j;
   FILE             *polyFile;
   struct timeval   timeVal;
   struct timezone  timeZone;

   fileName = polyFileName(restOfLine());
   if (!fileName) {
      parseError(20);
      return;
   }
   polyFile = fopen(fileName,"r");
   if (polyFile != NULL) {
      parseError(11);
      return;
   }
   fclose(polyFile);
   polyFile = fopen(fileName,"w");
   if (polyFile == NULL) {
      parseError(9);
      return;
   }

   printf("writing vertex coordinates to file \"%s\"\n",fileName);

   gettimeofday(&timeVal,&timeZone);
   fprintf(polyFile,"%%poly #%d  \"%s\"  %s",
           polyNr,fileName,ctime((long int *)&timeVal.tv_sec));
   for (i=0; i<VERTEX_COUNT; i++) {
      fprintf(polyFile,"%%vertex ");
      for (j=0; j<VERTEX_THINGS; j++) {
         fprintf(polyFile,"%c",FACE_CHAR(SPvertexConfig[i].faceNr[j]));
      }
      fprintf(polyFile,":\n  ");
      for (j=0; j<3; j++) {
         fprintf(polyFile,"%.15f ",SPpoly.vertices[i][j]);
      }
      fprintf(polyFile,"\n");
   }
   fprintf(polyFile,"%%\n");
   fclose(polyFile);
}

static Int32 getHexByte (FILE *hexFile)
{
   Int32             ch;
   Int32             charNr = 0;
   Boolean           commentLine = FALSE;
   Int32             hexByte = 0;

   while (charNr < 2) {
      ch = getc(hexFile);
      if (ch == EOF) goto ng;
      if (commentLine) {
         if (ch == '\n') commentLine = FALSE;
      }
      else {
         if (ch != '\n' && ch != '\t' && ch != ' ') {
            ch = toupper(ch);
            if (ch >= '0' && ch <= '9') {
               hexByte |= (ch - '0') << (charNr++ == 0 ? 4 : 0);
            }
            else if (ch >= 'A' && ch <= 'F') {
               hexByte |= (10 + ch - 'A') << (charNr++ == 0 ? 4 : 0);
            }
            else if (ch == '%') commentLine = TRUE;
            else goto ng;
         }
      }
   }
   return hexByte;

   ng:;
   return -19;
}

static void printHelp()
{
   static const char *helpText[] = {

"There should be a window on your display labeled 'polyhedron'.  For",
"openers, try the DF, D3, Z, ERA, and AN commands, and omit any optional",
"parameters.  Commands are as follows (everything is case-insensitive):",
"",
"AN              start animated (rotating) 3D display; ^C terminates",
"DF              display faces",
"D3              display 3D polyhedron in perspective",
"ERA             rotate eye location about origin (style A)",
"ERB             rotate eye location about origin (style B)",
"EYE             report current eye location for 3D display",
"EYE x y z       set eye location for 3D display",
"HELP            print a possibly useful summary of command syntax",
"MV v1 v2-v3 n   move vertex v1 in the direction v2->v3",
"                vertices are three letters (A-G), specifying 3 faces",
"                (optional n = distance; default 100--don't ask what units)",
"PL              'planify'; used after one or more MV commands to tweak",
"                the vertices you haven't moved to make the faces flat again",
"PL 10           industrial-strength planify; use this before PRINT",
"PRINT           generate Postscript for 7 faces in file poly.ps",
"QUIT            quit",
"R               redraw display",
"RD filename     read coordinates of vertices from ./polys/filename",
"RV n            revert to poly as shown at prompt number n",
"S               print status (dihedral angles, max vertex deviation)",
"SCALE n v1-v2   adjust scale by n percent; v1-v2 defines axis to scale along",
"SCALE n v1-v2 v3-v4   like the previous scale command, except that it scales",
"                along an axis perpendicular to both v1-v2 and v3-v4",
"SETT            set material thickness (affects Postscript face output only)",
"WR filename     write coordinates of vertices to ./polys/filename",
"Z s n l         randomly alter polyhedron; ^C interrupts; optional parameters:",
"                s  'strictness' in percentage of default strictness",
"                n  coefficient for faces' normal vector tweak (default 5)",
"                l  coefficient for faces' locator tweak (default 5)",
"                ^C interrupts",
"ZSAVE           save current poly for initial state in subsequent Z cmds",
"",
"Note:  this program is not polished, not awfully user-friendly, and is not",
"bug-free.  For example, rotation (AN) works poorly for some initial eye",
"positions.  If AN rotates slowly or not at all, try ERA or ERB before AN.",
"Further notes on the program can be found in the README file.",
"",
"%end"};

   for (const char **text = helpText; **text != '%'; text++) printf("%s\n",*text);
}

static void readPoly()
{
   Int32            ch;
   char             *fileName;
   Int32            hexByte;
   Int32            i,j;
   UInt32           k;
   FILE             *polyFile;
   Poly             savePoly = SPpoly;
   struct timeval   timeVal;
   struct timezone  timeZone;

   gettimeofday(&timeVal,&timeZone);

   fileName = polyFileName(restOfLine());
   polyFile = fopen(fileName,"r");
   if (polyFile == NULL) {
      parseError(8);
      return;
   }

   // attempt to read obsolete hex format

   for (i=0; i<VERTEX_COUNT; i++) {
      for (j=0; j<3; j++) {
         char *c = (char*) &SPpoly.vertices[i][j];
         for (k=0; k<sizeof(Real); k++) {
            hexByte = getHexByte(polyFile);
            if (hexByte < 0) goto hexPolyError;
            *c++ = hexByte;
         }
      }
   }
   SPpoly.derivePlanes();  // ensures appropriate normal vector orientations
   SPpoly.deriveMisc();
   nextPoly();
   SPdrawDisplay();
   fclose(polyFile);
   return;

   hexPolyError:;
   fclose(polyFile);
   polyFile = fopen(fileName,"r");
   SPpoly = savePoly;

   for (i=0; i<VERTEX_COUNT; i++) {
      for (j=0; j<3; j++) {
         while (ch = getc(polyFile),ch == '%') {
            if (ch == EOF) goto decPolyError;
            do {
               ch = getc(polyFile);
               if (ch == EOF) goto decPolyError;
            } while (ch != '\n');
         }
         if (fscanf(polyFile,"%lf",&SPpoly.vertices[i][j]) == EOF) {
            goto decPolyError;
         }
      }
      while (ch = getc(polyFile),ch != '\n') {
         if (ch == EOF) goto decPolyError;
      }
   }

   SPpoly.derivePlanes();  // ensures appropriate normal vector orientations
   SPpoly.deriveMisc();
   nextPoly();
   SPdrawDisplay();
   fclose(polyFile);
   return;

   decPolyError:;
   fclose(polyFile);
   SPpoly = savePoly;
   parseError(10);
   return;
}

static void printStatus()
{
   printf("material thickness: %.3f\"\n",SPthickness);
   printf("maximum vertex deviation: %.11f\n",SPpoly.maxDeviation());
   SPpoly.printSummary();
}

static void toggleBlack()
{
   useBlack = !useBlack;
   printf("black %sabled\n",useBlack ? "en" : "dis");
   if (useBlack) SPrgbColors(TRUE);
}

void SPdrawCmd()
{
   Boolean      done;
   static char  promptBuf[19];
   TokenTyp     token;
   Boolean      quit;

   if (!SPgraphicsEnabled) return;
   animateInit();

   lineCount = 0;
   lastLine[0] = '\0';
   done = FALSE;
   saveRandomPolyInit();  // save stock poly as random starting point
   SPdrawDisplay();
   savedPolys = (Poly*) malloc(sizeof(Poly) * maxPolys);
   if (savedPolys == NULL) SP_ERROR("couldn't alloc poly history buffer");
   do {
      savedPolys[polyNr] = SPpoly;
      sprintf(promptBuf,"poly%3d: ",polyNr);
      printf("%s",promptBuf);
      quit = FALSE;
      if (getLine()) {
         token = *nextToken();
         if (token.typ == alphaString && TOKEN_TEXT("LAST")) {
            printf("redoing: %s\n",lastLine);
            strcpy(inputLine,lastLine);
            charIndex = 0;
            token = *nextToken();
         }
         else {
            strcpy(lastLine,inputLine);
         }
         switch (token.typ) {
            case alphaString:
               if      (TOKEN_TEXT("DF")) { dispFaces(); SPdrawDisplay(); }
               else if (TOKEN_TEXT("D3")) { disp3D();    SPdrawDisplay(); }
               else if (TOKEN_TEXT("SCALE")) scalePoly();
               else if (TOKEN_TEXT("OB")) obliquePoly();
               else if (TOKEN_TEXT("EYE")) setEye();
               else if (TOKEN_TEXT("AN")) animate();
               else if (TOKEN_TEXT("ERA")) eyeRotateA();
               else if (TOKEN_TEXT("ERB")) eyeRotateB();
               else if (TOKEN_TEXT("MV")) moveVertex();
               else if (TOKEN_TEXT("PL")) planify();
               else if (TOKEN_TEXT("R")) SPdrawDisplay();
               else if (TOKEN_TEXT("RV")) revert();
               else if (TOKEN_TEXT("PRINT")) printFaces(FALSE);
               else if (TOKEN_TEXT("QUIT")) done = TRUE;
               else if (TOKEN_TEXT("WR")) writePoly();
               else if (TOKEN_TEXT("RD")) readPoly();
               else if (TOKEN_TEXT("ZSAVE")) saveRandomPolyInit();
               else if (TOKEN_TEXT("Z")) generateRandomPoly();
               else if (TOKEN_TEXT("S")) printStatus();
               else if (TOKEN_TEXT("MMDA")) maximizeMinDihedralAngle(FALSE);
               else if (TOKEN_TEXT("MMDAR")) maximizeMinDihedralAngle(TRUE);
               else if (TOKEN_TEXT("MMLDA")) maximizeMeanLowDA(FALSE);
               else if (TOKEN_TEXT("MMLDAR")) maximizeMeanLowDA(TRUE);
               else if (TOKEN_TEXT("SETT")) setThickness();
               else if (TOKEN_TEXT("BLACK")) toggleBlack();
               else if (TOKEN_TEXT("H") || TOKEN_TEXT("HELP")) printHelp();
               else parseError(1);
               break;
            case endOfLine:
            case pound:
               /*  do nothing  */
               break;
            default:
               parseError(1);
               break;
         }  /* end of switch */
      }
      else {    /* e-o-f encountered by getLine */
         done = TRUE;
      }
   } while (!done);
}

