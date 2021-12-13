// sp/misc.cc  Tom Ace  crux@qnet.com

// assorted stuff

#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include "sp1.h"
#include "spprocs.h"

void SPsleep(Int32 secs)
{
   (void) sleep((UInt32)secs);
}

void SPerror(const char *fileName, const Int32 lineNr, const char *errorText)
{
   fprintf(stderr,"Error at %s:%d; %s\n",fileName,lineNr,errorText);
// fprintf(stderr,"sleeping in SPerror to keep process alive for debugging\n");
// while (TRUE) SPsleep(19);
   SPexit(1);
}

void SPexit(Int32 rc)
{
   exit(rc);
}

#ifdef IN_POLY_NOW
void SPprintSummary(Poly &poly)
{
   Real         dihedral;
   Int32        i,j;
   Real         minDihedral = HUGE_REAL;
   Int32        minI,minJ;

   for (i=0; i<FACE_COUNT; i++) {
      for (j=0; j<FACE_COUNT; j++) {
         if (j == i) printf ("  --%c--",FACE_CHAR(i));
         else if (j < i) {
            dihedral = acos(-poly.facePlanes[i].normal.dotProduct(
             poly.facePlanes[j].normal)) * 180. / 3.1415926535897932384;
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
#endif
