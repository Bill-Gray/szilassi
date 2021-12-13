// sp/sp1.h  Tom Ace  crux@qnet.com
 

#ifdef IN_SPMAIN
#define EXT
#else
#define EXT extern
#endif

typedef char                Int8;
typedef unsigned char       UInt8;
typedef short int           Int16;
typedef unsigned short int  UInt16;
typedef int                 Int32;
typedef unsigned int        UInt32;
typedef int                 IntP;
typedef double              Real;

typedef char                Boolean8;
typedef short int           Boolean16;
typedef int                 Boolean;

const Int32 MAX_INT32 = 2147483647;
const Real  TINY_REAL = 1.E-12;
const Real  LITTLE_REAL = 1.E-9;
const Real  HUGE_REAL = 1.E35;

#ifndef NULL
#define NULL 0
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

class RealRect {
public:
   Real      xLo,yLo,xHi,yHi;
};

#define SP_ERROR(errorText) SPerror(__FILE__,__LINE__,errorText)

typedef enum {ADJUST_DIR,PRESERVE_DIR} AdjustDir;

class XYZ {
   Real       xyz[3];
public:
   Real       &operator[](const Int32 index)   { return xyz[index]; }
// because of this operator (brackets) I gave up on const correctness of XYZs.

   Real       magnitude()   {    
     return sqrt(xyz[0] * xyz[0] + xyz[1] * xyz[1] + xyz[2] * xyz[2]);
   }

   void       setCoords(Real x,Real y,Real z) {
      xyz[0] = x; xyz[1] = y; xyz[2] = z;
   }

   void       unitVector(AdjustDir adjustDir);

   void       translate(XYZ translation) {
      xyz[0] += translation.xyz[0]; 
      xyz[1] += translation.xyz[1]; 
      xyz[2] += translation.xyz[2];
   }

   void       scale(Real scaleFactor) {
      xyz[0] *= scaleFactor; xyz[1] *= scaleFactor; xyz[2] *= scaleFactor;
   }

   void invertDir() { xyz[0] = -xyz[0]; xyz[1] = -xyz[1]; xyz[2] = -xyz[2]; }

   void crossProduct(XYZ &other,XYZ &product);

   void add(XYZ &other,XYZ &sum) { 
      sum[0] = xyz[0] + other[0];
      sum[1] = xyz[1] + other[1];
      sum[2] = xyz[2] + other[2];
   }

   void subtract(XYZ &other,XYZ &difference) { 
      difference[0] = xyz[0] - other[0];
      difference[1] = xyz[1] - other[1];
      difference[2] = xyz[2] - other[2];
   }

   Real dotProduct(XYZ &other);
};

class Plane {
public:
   XYZ      normal;
   Real     locator;
   Boolean  valid;

   void     computePlane6(XYZ points[6],Boolean fixedPoints[6]);
   void     dump();
};

const Int32   VERTEX_COUNT = 14;
const Int32   FACE_COUNT = 7;
const Int32   EDGE_COUNT = 21;

const Int32 EDGE_THINGS = 2;
const Int32 FACE_THINGS = 6;
const Int32 VERTEX_THINGS = 3;

class Edge {
public:
   XYZ         vertices[2];
};

class Face {
public:
   XYZ         vertices[6];
   Real        xHi,yHi;
};

class EdgeConfig {
public:
   Int32       faceNr[EDGE_THINGS];
   Int32       vertexNr[EDGE_THINGS];
};

class FaceConfig {
public:
   Int32       edgeNr[FACE_THINGS];
   Int32       vertexNr[FACE_THINGS];
   Int32       keyVertices[3];              //  [0,5]; indices to vertexNr
   Int32       drawVertices[FACE_THINGS];   //  [0,5]; indices to vertexNr
   Int32       letterVertices[4];           //  [0,5]; indices to vertexNr
   Int32       otherFaces[FACE_THINGS];     // in draw order, for labeling only
   Boolean     mirror;                      // for drawing faces
   Boolean     mirror2;                     // for drawing 3D poly

   void setDrawVertices(Int32 v0,Int32 v1,Int32 v2,Int32 v3,Int32 v4,Int32 v5){
      drawVertices[0] = v0; drawVertices[1] = v1; drawVertices[2] = v2;
      drawVertices[3] = v3; drawVertices[4] = v4; drawVertices[5] = v5;
   }

   void setOtherFaces (Int32 f0,Int32 f1,Int32 f2,Int32 f3,Int32 f4,Int32 f5) {
      otherFaces[0] = f0; otherFaces[1] = f1; otherFaces[2] = f2;
      otherFaces[3] = f3; otherFaces[4] = f4; otherFaces[5] = f5;
   }

   void setLetterVertices(Int32 v0,Int32 v1,Int32 v2,Int32 v3) {
      letterVertices[0] = v0; letterVertices[1] = v1; 
      letterVertices[2] = v2; letterVertices[3] = v3;
   }
};

class VertexConfig {
public:
   Int32       edgeNr[VERTEX_THINGS];
   Int32       faceNr[VERTEX_THINGS];
};

class Poly {
public:
   XYZ         vertices[VERTEX_COUNT];
   Real        eyeDist[VERTEX_COUNT];       // elements correspond to vertices
   Real        minEyeDist;

   Plane       facePlanes[FACE_COUNT];
   XYZ         cg;
   Real        maxCGDist;

   Boolean deriveVertices();
   void derivePlanes();
   void deriveMisc();
   void planify(Boolean fixedVertices[VERTEX_COUNT],Real precision);
   Boolean anyGood(Real extendPercent);
   void printSummary();
   Real maxDeviation();
   Real minDihedralAngle();
   Real meanLowDihedralAngle();
};

class SPXPoint {
public:
   Int16       x,y;
};

EXT EdgeConfig   SPedgeConfig  [EDGE_COUNT];
EXT FaceConfig   SPfaceConfig  [FACE_COUNT];
EXT VertexConfig SPvertexConfig[VERTEX_COUNT];

EXT XYZ SPoneOneOne;

EXT Poly SPpoly;

#define FACE_CHAR(x) (x + 'A')

EXT Real SPthickness
#ifdef IN_SPMAIN
= 0.
#endif
;
