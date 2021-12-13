// sp/poly.cc  Tom Ace  crux@qnet.com

#include <stdio.h>
#include <math.h>
#include "sp1.h"
#include "spprocs.h"

Boolean Poly::deriveVertices()
{
   XYZ         a[3];
   XYZ         b;
   Int32       i;
   XYZ         intersection;
   Int32       planeNr;
   Int32       vertexNr;

   for (vertexNr=0; vertexNr<VERTEX_COUNT; vertexNr++) {
      for (i=0; i<VERTEX_THINGS; i++) {
         planeNr = SPvertexConfig[vertexNr].faceNr[i];
         a[i] = facePlanes[planeNr].normal;
         b[i] = facePlanes[planeNr].locator;
      }
      
      // Solve the system of 3 equations.  This returns the coordinates
      // of the point of intersection of the three planes.  

      if (!SPsolve3Eqs(a,b,intersection)) goto precisionProblem;

      for (i=0; i<3; i++) vertices[vertexNr][i] = intersection[i];
   }
   return TRUE;

   precisionProblem:;
   return FALSE;
}

void Poly::derivePlanes()
{
   XYZ         a[3];
   Int32       faceNr;
   Int32       i;
   XYZ         someVertex;

   for (faceNr=0; faceNr<FACE_COUNT; faceNr++) {
      for (i=0; i<3; i++) {
         a[i] = vertices[SPfaceConfig[faceNr].vertexNr[
          SPfaceConfig[faceNr].keyVertices[i]]];
      }
      someVertex = a[0];
      (void) SPsolve3Eqs(a,SPoneOneOne,facePlanes[faceNr].normal);
      if (SPfaceConfig[faceNr].mirror) facePlanes[faceNr].normal.invertDir();
      facePlanes[faceNr].normal.unitVector(PRESERVE_DIR);
      facePlanes[faceNr].locator = 
       facePlanes[faceNr].normal.dotProduct(someVertex);
   }
}

void Poly::deriveMisc()
{
   XYZ         diff;
   Real        dist;
   Int32       i;
   Int32       vertexNr;

   cg.setCoords(0.,0.,0.);
   for (vertexNr=0; vertexNr<VERTEX_COUNT; vertexNr++) {
      for (i=0; i<3; i++) cg[i] += vertices[vertexNr][i];
   }
   for (i=0; i<3; i++) cg[i] /= (Real)VERTEX_COUNT;
   maxCGDist = 0.;
   for (vertexNr=0; vertexNr<VERTEX_COUNT; vertexNr++) {
      for (i=0; i<3; i++) diff[i] = cg[i] - vertices[vertexNr][i];
      dist = diff.magnitude();
      if (dist > maxCGDist) maxCGDist = dist;
   }
}

Real Poly::maxDeviation()
{
   Real            currDev;
   Boolean         fixedPoints[6];
   Int32           i,j;
   Real            maxDev;
   Real            planeDev;
   Int32           planeNr;
   Plane           planes[FACE_COUNT];
   XYZ             sixPoints[6];
   Int32           vertexNr;

   for (i=0; i<FACE_COUNT; i++) {
      for (j=0; j<FACE_THINGS; j++) {
         vertexNr = SPfaceConfig[i].vertexNr[j];
         sixPoints[j] = vertices[vertexNr];
         fixedPoints[j] = FALSE;
      }
      planes[i].computePlane6(sixPoints,fixedPoints);
   }

   // Find the point which most deviates from the computed planes.
   // Deviation is calculated by summing the square-deviations of 
   // each point from the three planes to which it belongs.  (Where 
   // "square-deviation" of a point from a plane is defined as the 
   // square of the difference of AX + BY + CZ and K.)

   maxDev = -1.0;
   for (i=0; i<VERTEX_COUNT; i++) {
      currDev = 0.0;
      for (j=0; j<VERTEX_THINGS; j++) {
         planeNr = SPvertexConfig[i].faceNr[j];
         planeDev = planes[planeNr].locator - 
                    vertices[i].dotProduct(planes[planeNr].normal);
         currDev += planeDev * planeDev;
      }
      if (currDev > maxDev) maxDev = currDev;
   }
   return maxDev;
}

void Poly::planify(Boolean fixedVertices[VERTEX_COUNT],Real precision)
{
   XYZ             a[3];
   XYZ             b;
   Real            currDev;
   Boolean         fixedPoints[6];
   Int32           i,j;
   XYZ             intersection;
   Int32           iterCount = 0;
   Real            maxDev;
   Real            planeDev;
   Int32           planeNr;
   Plane           planes[FACE_COUNT];
   XYZ             sixPoints[6];
   Int32           vertexNr;
   Int32           worstVertex;

   // Mark all planes as needing to be computed.
   for (i=0; i<FACE_COUNT; i++) planes[i].valid = FALSE;

   do {
      // Compute equations of changed planes 
      for (i=0; i<FACE_COUNT; i++) {
         if (!planes[i].valid) {
            for (j=0; j<FACE_THINGS; j++) {
               vertexNr = SPfaceConfig[i].vertexNr[j];
               sixPoints[j] = vertices[vertexNr];
               fixedPoints[j] = fixedVertices[vertexNr];
            }
            planes[i].computePlane6(sixPoints,fixedPoints);
            planes[i].valid = TRUE;
//          printf("face plane %d: ",i);
//          planes[i].dump();
         }
      }

      // Find the point which most deviates from the computed planes.
      // Deviation is calculated by summing the square-deviations of 
      // each point from the three planes to which it belongs.  (Where 
      // "square-deviation" of a point from a plane is defined as the 
      // square of the difference of AX + BY + CZ and K.)

      maxDev = -1.0;
      for (i=0; i<VERTEX_COUNT; i++) {
         if (!fixedVertices[i]) {
            currDev = 0.0;
            for (j=0; j<VERTEX_THINGS; j++) {
               planeNr = SPvertexConfig[i].faceNr[j];
               planeDev = planes[planeNr].locator - 
                          vertices[i].dotProduct(planes[planeNr].normal);
               currDev += planeDev * planeDev;
            }
            if (currDev > maxDev) {
               maxDev = currDev;
               worstVertex = i;
            }
         }
      }

      // Now that we know the most deviant point, figure out where we guess it
      // ought to be (i.e., where the three calculated planes containing it
      // intersect) and move it halfway to that point.

      // Build a matrix of three normal vectors to pass to solve3Eqs.
      // Also build the B vector (three locators for the three planes).

      for (i=0; i<VERTEX_THINGS; i++) {
         planeNr = SPvertexConfig[worstVertex].faceNr[i];
         a[i] = planes[planeNr].normal;
         b[i] = planes[planeNr].locator;
      }

      // Solve the system of 3 equations.  This returns the coordinates
      // of the point of intersection of the three planes.  We then
      // average those coordinates with those of WorstVertex, and mark
      // the pertinent planes to be recomputed. 
      if (!SPsolve3Eqs(a,b,intersection)) goto precisionProblem;

      for (i=0; i<3; i++) {
         vertices[worstVertex][i] = 
          (vertices[worstVertex][i] + intersection[i]) / 2.0;
      }

      for (i=0; i<VERTEX_THINGS; i++) {
         planes[SPvertexConfig[worstVertex].faceNr[i]].valid = FALSE;
      }
//    printf("worstVertex=%d  maxDev=%.5f\n",worstVertex,maxDev);
   } while (maxDev > precision && ++iterCount < 1919);    

   if (maxDev > precision) {
      printf("Poly::planify failed; desired precision=%.10f achieved=%.10f\n",
             precision,maxDev);
   }

   precisionProblem:;

// for (i=0; i<VERTEX_COUNT; i++) {
//    printf(" Vertex %d:  (%.5f,%.5f,%.5f)\n",i,
//           vertices[i][0],vertices[i][1],vertices[i][2]);
// }
}

Boolean Poly::anyGood(Real extendPercent)
{
   Int32          i,j,k;
   Face           xyFace;
   Real           deltaX;
   Real           deltaY;
   Real           dist;
   Real           extend;
   Real           extendT;
   Real           extendX;
   Real           extendY;
   XYZ            extendedEndpoints[FACE_THINGS][2];
   XYZ            v0,v1;

   deriveMisc();
   extend = extendPercent * maxCGDist / 1900.;
   for (i=0; i<FACE_COUNT; i++) {
      SPxyify(i,*this,xyFace,FALSE);
      v0 = xyFace.vertices[SPfaceConfig[i].drawVertices[FACE_THINGS - 1]];
      for (j=0; j<FACE_THINGS; j++) {
         v1 = xyFace.vertices[SPfaceConfig[i].drawVertices[j]];
         deltaX = v1[0] - v0[0];
         deltaY = v1[1] - v0[1];
         dist = sqrt(deltaX * deltaX + deltaY * deltaY);
         extendT = extend / dist;
         extendX = deltaX * extendT;
         extendY = deltaY * extendT;
         extendedEndpoints[j][0][0] = v0[0] - extendX;
         extendedEndpoints[j][0][1] = v0[1] - extendY;
         extendedEndpoints[j][1][0] = v1[0] + extendX;
         extendedEndpoints[j][1][1] = v1[1] + extendY;
         v0 = v1;
      }
      for (j=0; j<FACE_THINGS-2; j++) {
         for (k=j+2; k<(j==0?FACE_THINGS-1:FACE_THINGS); k++) {
            // check to see if the segments described by extendedEndpoints[j]
            // and extendedEndpoints[k] intersect.
            Real jX0 = extendedEndpoints[j][0][0];
            Real jX1 = extendedEndpoints[j][1][0];
            Real kX0 = extendedEndpoints[k][0][0];
            Real kX1 = extendedEndpoints[k][1][0];
            Real jY0 = extendedEndpoints[j][0][1];
            Real jY1 = extendedEndpoints[j][1][1];
            Real kY0 = extendedEndpoints[k][0][1];
            Real kY1 = extendedEndpoints[k][1][1];
            if (((jX0 - kX0) * (kY1 - kY0) > (jY0 - kY0) * (kX1 - kX0)) !=
                ((jX1 - kX0) * (kY1 - kY0) > (jY1 - kY0) * (kX1 - kX0)) &&
                ((kX0 - jX0) * (jY1 - jY0) > (kY0 - jY0) * (jX1 - jX0)) !=
                ((kX1 - jX0) * (jY1 - jY0) > (kY1 - jY0) * (jX1 - jX0))) {
               return FALSE;
            }
         }
      }
   }
   return TRUE;
}

void Poly::printSummary()
{
   Real         dihedral;
   Int32        i,j;
   Real         minDihedral = HUGE_REAL;
   Int32        minI,minJ;

   printf("dihedral angles at the polyhedron's edges (in degrees):\n");
   for (i=0; i<FACE_COUNT; i++) {
      for (j=0; j<=i; j++) {
         if (j == i) printf ("  --%c--",FACE_CHAR(i));
         else if (j < i) {
            dihedral = acos(-facePlanes[i].normal.dotProduct(
             facePlanes[j].normal)) * 180. / 3.1415926535897932384;
            if (dihedral < minDihedral) {
               minDihedral = dihedral;
               minI = i;
               minJ = j;
            }
            printf("%7.1f",dihedral);
         }
      }
      printf("\n");
   }
   printf("minimum dihedral angle %5.1f at edge %c%c\n",
          minDihedral,FACE_CHAR(minJ),FACE_CHAR(minI));
}

Real Poly::minDihedralAngle()
{
   Real         dihedral;
   Int32        i,j;
   Real         minDihedral = HUGE_REAL;

   for (i=0; i<FACE_COUNT; i++) {
      for (j=0; j<i; j++) {
         dihedral = 
          acos(-facePlanes[i].normal.dotProduct(facePlanes[j].normal));
         if (dihedral < minDihedral) minDihedral = dihedral;
      }
   }
   return minDihedral * 180. / 3.1415926535897932384;
}


Real Poly::meanLowDihedralAngle()
{
   Int32        count = 0;
   Real         dihedral;
   Int32        i,j;
   Real         sigma = 0.;
   const Real   LOW_DA = 50.;

   for (i=0; i<FACE_COUNT; i++) {
      for (j=0; j<i; j++) {
         dihedral = acos(-facePlanes[i].normal.dotProduct(
          facePlanes[j].normal)) * 180. / 3.1415926535897932384;
         if (dihedral < LOW_DA) {
            sigma += dihedral;
            count++;
         }
      }
   }
   return sigma / Real(count);
}


