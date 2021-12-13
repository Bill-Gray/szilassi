// sp/first.cc  Tom Ace  crux@qnet.com
 
#include <stdio.h>
#include <math.h>
#include "sp1.h"
#include "spprocs.h"

void SPeverythingElse()
{
   Boolean         fixedVertices[VERTEX_COUNT];
   Int32           i;

#define SZILASSI_COORDS
#ifdef SZILASSI_COORDS
   // initialize SPpoly with modified (mirrored) Szillasi coordinates
   SPpoly.vertices[ 8].setCoords( 12.0,  0.0,-12.0);
   SPpoly.vertices[ 4].setCoords(-12.0,  0.0,-12.0);
   SPpoly.vertices[ 1].setCoords(  0.0, 12.6, 12.0);
   SPpoly.vertices[ 0].setCoords(  0.0,-12.6, 12.0);
   SPpoly.vertices[11].setCoords( 3.75, 3.75,  3.0);
   SPpoly.vertices[10].setCoords(-3.75,-3.75,  3.0);
   SPpoly.vertices[ 6].setCoords(  2.0, -5.0,  8.0);
   SPpoly.vertices[ 3].setCoords( -2.0,  5.0,  8.0);
   SPpoly.vertices[ 9].setCoords(  7.0,  2.5, -2.0); 
   SPpoly.vertices[ 5].setCoords( -7.0, -2.5, -2.0);
   SPpoly.vertices[ 7].setCoords(  7.0,  0.0, -2.0); 
   SPpoly.vertices[ 2].setCoords( -7.0,  0.0, -2.0); 
   SPpoly.vertices[13].setCoords(  4.5, -2.5, -2.0);
   SPpoly.vertices[12].setCoords( -4.5,  2.5, -2.0);
#else
   // initialize SPpoly with manually measured coordinates
   // note:  displays don't work right with these.  not sure why; they 
   // should have the same mirror-ness as the modified Szilassi coordinates.
   SPpoly.vertices[ 0].setCoords( 0.0, 0.0, 0.0);
   SPpoly.vertices[ 1].setCoords( 5.0, 0.0, 0.0);
   SPpoly.vertices[ 2].setCoords( 2.5,3.18, 0.0);
   SPpoly.vertices[ 3].setCoords( 3.5, 0.8, 0.0);
   SPpoly.vertices[ 4].setCoords( 2.5, 5.4, 0.0);
   SPpoly.vertices[ 5].setCoords(1.95,3.18, 0.0);
   SPpoly.vertices[ 6].setCoords(1.42, 0.5,0.85);
   SPpoly.vertices[ 7].setCoords( 2.5, 1.8,2.65);
   SPpoly.vertices[ 8].setCoords( 2.5, 3.0, 4.3);
   SPpoly.vertices[ 9].setCoords(3.05, 1.8,2.65);
   SPpoly.vertices[10].setCoords( 1.8, 1.9,0.25);
   SPpoly.vertices[11].setCoords(3.15,1.15,1.55);
   SPpoly.vertices[12].setCoords(2.93,2.95, 0.4);
   SPpoly.vertices[13].setCoords( 2.1, 2.0, 2.1);
#endif

   // Add a constant to each of the coordinates in Poly.  This will remove 
   // zeroes from the data and make the linear algebra routines happier.

   for (i=0; i<VERTEX_COUNT; i++) SPpoly.vertices[i].translate(SPoneOneOne);
   SPpoly.derivePlanes();

   for (i=0; i<VERTEX_COUNT; i++) fixedVertices[i] = FALSE;

   SPpoly.planify(fixedVertices,1.E-10);

   SPdrawCmd();

   SPexit(0);
}

