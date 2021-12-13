// sp/main.cc  Tom Ace  crux@qnet.com

#define IN_SPMAIN

#include <stdio.h>
#include <math.h>
#include "sp1.h"
#include "spprocs.h"

static void initialize()
{
   Int32        edgeNr;
   Int32        i,j;

   // Initialize various things.

   SPoneOneOne.setCoords(1.0,1.0,1.0);

   // Initialize the arrays that describe the configuration of the polyhedron.
   // These arrays are never changed once they've been initialized. 

   // Initialize the edges-to-faces array (trivial)
   edgeNr = 0;
   for (i=0; i<6; i++) {
      for (j=i+1; j<7; j++) {
         SPedgeConfig[edgeNr].faceNr[0] = i;
         SPedgeConfig[edgeNr].faceNr[1] = j;
         edgeNr++;
      }
   }

   // Initialize the faces-to-vertices array.  In addition to the
   // expected ".vertexNr" array, there's also a ".keyVertices"
   // array which specifies particular vertices (individually selected
   // for each face, used in preparing 2-D displays.)
   // The keyVertices in SPfaceConfig are from the range [0,5]; i.e., they
   // index vertices within a face, not within the whole polyhedron.
   // The [0] [1] and [2] elements of keyVertices are used in 2-D code
   // to derive the normal vector for the plane.  The sequence of the
   // three vectors is chosen to get the desired orientation of the face
   // in the 2-D display.
   // The [0] and [1] elements are two "base vertices"; they are the
   // endpoints of the edge which will be horizontal in a 2-D display.

   for (i=0; i<6; i++) SPfaceConfig[0].vertexNr[i] = i;
   SPfaceConfig[0].keyVertices[0] = 1;   // i.e., vertex 0
   SPfaceConfig[0].keyVertices[1] = 0;   // i.e., vertex 1
   SPfaceConfig[0].keyVertices[2] = 4;   // i.e., vertex 4
   SPfaceConfig[0].setDrawVertices(0,2,5,4,3,1);  // i.e., 0,2,5,4,3,1
   SPfaceConfig[0].setOtherFaces(2,6,4,5,3,1);    // i.e., C,G,E,F,D,B
   SPfaceConfig[0].setLetterVertices(0,1,2,5);    // i.e., ...
   SPfaceConfig[0].mirror  = TRUE;
   SPfaceConfig[0].mirror2 = TRUE;

   SPfaceConfig[1].vertexNr[0] = 0;
   SPfaceConfig[1].vertexNr[1] = 1;
   SPfaceConfig[1].vertexNr[2] = 6;
   SPfaceConfig[1].vertexNr[3] = 7;
   SPfaceConfig[1].vertexNr[4] = 8;
   SPfaceConfig[1].vertexNr[5] = 9;
   SPfaceConfig[1].keyVertices[0] = 0;   // i.e., vertex 0
   SPfaceConfig[1].keyVertices[1] = 1;   // i.e., vertex 1
   SPfaceConfig[1].keyVertices[2] = 4;   // i.e., vertex 8
   SPfaceConfig[1].setDrawVertices(1,3,5,4,2,0);  // i.e., 1,7,9,8,6,0
   SPfaceConfig[1].setOtherFaces(3,6,5,4,2,0);    // i.e., D,G,F,E,C,A
   SPfaceConfig[1].setLetterVertices(0,1,3,5);    // i.e., ...
   SPfaceConfig[1].mirror  = TRUE;
   SPfaceConfig[1].mirror2 = TRUE;

   SPfaceConfig[2].vertexNr[0] = 0;
   SPfaceConfig[2].vertexNr[1] = 2;
   SPfaceConfig[2].vertexNr[2] = 6;
   SPfaceConfig[2].vertexNr[3] = 10;
   SPfaceConfig[2].vertexNr[4] = 11;
   SPfaceConfig[2].vertexNr[5] = 12;
   SPfaceConfig[2].keyVertices[0] = 4;   // i.e., vertex 11
   SPfaceConfig[2].keyVertices[1] = 3;   // i.e., vertex 10
   SPfaceConfig[2].keyVertices[2] = 5;   // i.e., vertex 12
   SPfaceConfig[2].setDrawVertices(0,2,3,4,5,1);  // i.e., 0,6,10,11,12,2
   SPfaceConfig[2].setOtherFaces(1,4,3,5,6,0);    // i.e., B,E,D,F,G,A
   SPfaceConfig[2].setLetterVertices(1,3,4,5);    // i.e., ...
   SPfaceConfig[2].mirror  = FALSE;
   SPfaceConfig[2].mirror2 = TRUE;

   SPfaceConfig[3].vertexNr[0] = 1;
   SPfaceConfig[3].vertexNr[1] = 3;
   SPfaceConfig[3].vertexNr[2] = 7;
   SPfaceConfig[3].vertexNr[3] = 10;
   SPfaceConfig[3].vertexNr[4] = 11;
   SPfaceConfig[3].vertexNr[5] = 13;
   SPfaceConfig[3].keyVertices[0] = 3;   // i.e., vertex 10
   SPfaceConfig[3].keyVertices[1] = 4;   // i.e., vertex 11
   SPfaceConfig[3].keyVertices[2] = 5;   // i.e., vertex 13
   SPfaceConfig[3].setDrawVertices(0,1,4,3,5,2);  // i.e., 1,3,11,10,13,7
   SPfaceConfig[3].setOtherFaces(0,5,2,4,6,1);    // i.e., A,F,C,E,G,B
   SPfaceConfig[3].setLetterVertices(2,3,4,5);    // i.e., ...
   SPfaceConfig[3].mirror  = FALSE;
   SPfaceConfig[3].mirror2 = TRUE;

   SPfaceConfig[4].vertexNr[0] = 4;
   SPfaceConfig[4].vertexNr[1] = 5;
   SPfaceConfig[4].vertexNr[2] = 6;
   SPfaceConfig[4].vertexNr[3] = 8;
   SPfaceConfig[4].vertexNr[4] = 10;
   SPfaceConfig[4].vertexNr[5] = 13;
   SPfaceConfig[4].keyVertices[0] = 3;   // i.e., vertex 8
   SPfaceConfig[4].keyVertices[1] = 0;   // i.e., vertex 4
   SPfaceConfig[4].keyVertices[2] = 2;   // i.e., vertex 6
   SPfaceConfig[4].setDrawVertices(0,1,5,4,2,3);  // i.e., 4,5,13,10,6,8
   SPfaceConfig[4].setOtherFaces(0,6,3,2,1,5);    // i.e., A,G,D,C,B,F
   SPfaceConfig[4].setLetterVertices(0,1,3,5);    // i.e., ...
   SPfaceConfig[4].mirror  = TRUE;
   SPfaceConfig[4].mirror2 = TRUE;

   SPfaceConfig[5].vertexNr[0] = 3;
   SPfaceConfig[5].vertexNr[1] = 4;
   SPfaceConfig[5].vertexNr[2] = 8;
   SPfaceConfig[5].vertexNr[3] = 9;
   SPfaceConfig[5].vertexNr[4] = 11;
   SPfaceConfig[5].vertexNr[5] = 12;
   SPfaceConfig[5].keyVertices[0] = 1;   // i.e., vertex 4
   SPfaceConfig[5].keyVertices[1] = 2;   // i.e., vertex 8
   SPfaceConfig[5].keyVertices[2] = 0;   // i.e., vertex 3
   SPfaceConfig[5].setDrawVertices(2,3,5,4,0,1);  // i.e., 8,9,12,11,3,4
   SPfaceConfig[5].setOtherFaces(1,6,2,3,0,4);    // i.e., B,G,C,D,A,E
   SPfaceConfig[5].setLetterVertices(1,2,3,5);   // i.e., ...
   SPfaceConfig[5].mirror  = TRUE;
   SPfaceConfig[5].mirror2 = TRUE;

   SPfaceConfig[6].vertexNr[0] = 2;
   SPfaceConfig[6].vertexNr[1] = 5;
   SPfaceConfig[6].vertexNr[2] = 7;
   SPfaceConfig[6].vertexNr[3] = 9;
   SPfaceConfig[6].vertexNr[4] = 12;
   SPfaceConfig[6].vertexNr[5] = 13;
   SPfaceConfig[6].keyVertices[0] = 1;   // i.e., vertex 5
   SPfaceConfig[6].keyVertices[1] = 5;   // i.e., vertex 13
   SPfaceConfig[6].keyVertices[2] = 4;   // i.e., vertex 12
   SPfaceConfig[6].setDrawVertices(1,0,4,3,2,5);  // i.e., 5,2,12,9,7,13
   SPfaceConfig[6].setOtherFaces(0,2,5,1,3,4);    // i.e., A,C,F,B,D,E
   SPfaceConfig[6].setLetterVertices(0,2,4,5);    // i.e., ...
   SPfaceConfig[6].mirror  = FALSE;
   SPfaceConfig[6].mirror2 = TRUE;

   //  Initialize the vertices-to-faces array.
   SPvertexConfig[0].faceNr[0] = 0;
   SPvertexConfig[0].faceNr[1] = 1;
   SPvertexConfig[0].faceNr[2] = 2;

   SPvertexConfig[1].faceNr[0] = 0;
   SPvertexConfig[1].faceNr[1] = 1;
   SPvertexConfig[1].faceNr[2] = 3;

   SPvertexConfig[2].faceNr[0] = 0;
   SPvertexConfig[2].faceNr[1] = 2;
   SPvertexConfig[2].faceNr[2] = 6;

   SPvertexConfig[3].faceNr[0] = 0;
   SPvertexConfig[3].faceNr[1] = 3;
   SPvertexConfig[3].faceNr[2] = 5;

   SPvertexConfig[4].faceNr[0] = 0;
   SPvertexConfig[4].faceNr[1] = 4;
   SPvertexConfig[4].faceNr[2] = 5;

   SPvertexConfig[5].faceNr[0] = 0;
   SPvertexConfig[5].faceNr[1] = 4;
   SPvertexConfig[5].faceNr[2] = 6;

   SPvertexConfig[6].faceNr[0] = 1;
   SPvertexConfig[6].faceNr[1] = 2;
   SPvertexConfig[6].faceNr[2] = 4;

   SPvertexConfig[7].faceNr[0] = 1;
   SPvertexConfig[7].faceNr[1] = 3;
   SPvertexConfig[7].faceNr[2] = 6;

   SPvertexConfig[8].faceNr[0] = 1;
   SPvertexConfig[8].faceNr[1] = 4;
   SPvertexConfig[8].faceNr[2] = 5;

   SPvertexConfig[9].faceNr[0] = 1;
   SPvertexConfig[9].faceNr[1] = 5;
   SPvertexConfig[9].faceNr[2] = 6;

   SPvertexConfig[10].faceNr[0] = 2;
   SPvertexConfig[10].faceNr[1] = 3;
   SPvertexConfig[10].faceNr[2] = 4;

   SPvertexConfig[11].faceNr[0] = 2;
   SPvertexConfig[11].faceNr[1] = 3;
   SPvertexConfig[11].faceNr[2] = 5;

   SPvertexConfig[12].faceNr[0] = 2;
   SPvertexConfig[12].faceNr[1] = 5;
   SPvertexConfig[12].faceNr[2] = 6;

   SPvertexConfig[13].faceNr[0] = 3;
   SPvertexConfig[13].faceNr[1] = 4;
   SPvertexConfig[13].faceNr[2] = 6;

   // edges-to-vertices array code will be written if that array is needed.
   // faces-to edges array code will be written if that array is needed.
   // vertices-to-edges array code will be written if that array is needed.
   // Note that any of the three not-yet-initialized arrays can be derived 
   // from the three which have been initialized.  
}


int main(int argc,char **argv)
{
   printf("sp version 1.0; type HELP for info\n");
   initialize();
   SPinitGraphics();
   SPeverythingElse();
}
