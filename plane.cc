// sp/plane.cc  Tom Ace  crux@qnet.com

#include <stdio.h>
#include <math.h>
#include "sp1.h"
#include "spprocs.h"


class VectorOrdering {
public:
   Real          diffMag;   
   Int16         index;
};

void Plane::computePlane6(XYZ points[6],Boolean fixedPoints[6])
{   
// Given six points, compute a normal vector and locator (scalar)
// that describe a new plane based on the 6 points.  

   XYZ              cross;
   XYZ              diff12,diff13,diff23;
   Int32            i,j1,j2,j3;
   XYZ              newNormal;
   Real             weight,totalWeight;

   newNormal.setCoords(0.,0.,0.);
   for (j1=0; j1<4; j1++) {
      for (j2=j1+1; j2<5; j2++) {
         for (j3=j2+1; j3<6; j3++) {
            points[j1].subtract(points[j2],diff12);
            points[j1].subtract(points[j3],diff13);
            points[j2].subtract(points[j3],diff23);
            diff12.crossProduct(diff13,cross);
            if (normal.dotProduct(cross) < 0) cross.invertDir();
            if (fixedPoints[j1]) cross.scale(3.);
            newNormal.add(cross,newNormal);
            diff12.crossProduct(diff23,cross);
            if (normal.dotProduct(cross) < 0) cross.invertDir();
            if (fixedPoints[j2]) cross.scale(3.);
            newNormal.add(cross,newNormal);
            diff13.crossProduct(diff23,cross);
            if (normal.dotProduct(cross) < 0) cross.invertDir();
            if (fixedPoints[j3]) cross.scale(3.);
            newNormal.add(cross,newNormal);
         }
      }
   }

   newNormal.unitVector(ADJUST_DIR);
   normal = newNormal;

   // Calculate the locator (the K in AX + BY + CZ = K) by calculating
   // a weighted mean of the six locators determined by the six points 
   // and our calculated mean normal vector.

   locator = totalWeight = 0.;
   for (i=0; i<6; i++) {
      weight = fixedPoints[i] ? 3. : 1.;
      locator += normal.dotProduct(points[i]) * weight;
      totalWeight += weight;
   }

   locator /= totalWeight;
}

void Plane::dump()
{
   printf("Normal vector=<%.5f,%.5f,%.5f>  locator=%.5f\n",
          normal[0],normal[1],normal[2],locator);
}

