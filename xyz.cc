// sp/xyz.cc  Tom Ace  crux@qnet.com

#include <math.h>
#include "sp1.h"
#include "spprocs.h"

void XYZ::unitVector(AdjustDir adjustDir)
{
   // Adjusts the scale of the vector so that it becomes a unit vector.  
   // If the caller so requests (i.e., the second argument is "ADJUST_DIR" 
   // (the alternative is PRESERVE_DIR)), it will ensure that the first
   // nonzero (well, nontiny) coordinate is positive.

   // It is legal for the output to be the given vector.

   Real         divisor = magnitude();
   Int32        i;

   if (divisor < LITTLE_REAL) SP_ERROR("minuscule vector in SPunitVector");
   if (adjustDir == ADJUST_DIR) {
      if (xyz[0] < -LITTLE_REAL) divisor = -divisor;
      else {
         if (xyz[0] < LITTLE_REAL) {
            if (xyz[1] < -LITTLE_REAL) divisor = -divisor;
            else {
               if (xyz[1] < LITTLE_REAL) {
                  if (xyz[2] < -LITTLE_REAL) divisor = -divisor;
               }
            }
         }
      }
   }
   for (i=0; i<3; i++) xyz[i] /= divisor;
}

void XYZ::crossProduct(XYZ &other,XYZ &product)
{
   product[0] = xyz[1] * other[2] - xyz[2] * other[1];
   product[1] = xyz[2] * other[0] - xyz[0] * other[2];
   product[2] = xyz[0] * other[1] - xyz[1] * other[0];
}

Real XYZ::dotProduct(XYZ &other)
{
   return xyz[0] * other[0] + xyz[1] * other[1] + xyz[2] * other[2];
}

