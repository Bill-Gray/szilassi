// sp/linear.cc  Tom Ace  crux@qnet.com
 
// linear algebra functions and related stuff

#include <math.h>
#include <stdio.h>
#include "sp1.h"
#include "spprocs.h"

static void ludcmp(XYZ a[3],Int32 indx[3],Real &d)
// static void ludcmp(XYZ *a,Int32 *indx,Real &d)
{
   Real            av;
   Int32           k,j,iMax,i;
   Real            sum,dum,big;
   XYZ             vv;

//  LU decomposition routine
   d = 1.0;
   for (i=0; i<3; i++) {
      big = 0.0;
      for (j=0; j<3; j++) {
         av = fabs(a[i][j]); 
         if (av > big) big = av;
      }
      if (big < TINY_REAL) SP_ERROR("singular matrix in ludcmp");
      vv[i] = 1.0 / big;
   }
   for (j=0; j<3; j++) {
      if (j > 0) {
         for (i=0; i<j; i++) {
            sum = a[i][j];
            if (i > 0) {
               for (k=0; k<i; k++) sum -= a[i][k] * a[k][j];
               a[i][j] = sum;
            }
         }
      }
      big = 0.0;
      for (i=j; i<3; i++) {
         sum = a[i][j];
         if (j > 0) {
            for (k=0; k<j; k++) sum -= a[i][k] * a[k][j];
            a[i][j] = sum;
         }
         dum = vv[i] * fabs(sum);
         if  (dum > big) {
            big = dum;
            iMax = i;
         }
      }
      if (j != iMax) {
         for (k=0; k<3; k++) {
            dum = a[iMax][k];
            a[iMax][k] = a[j][k];
            a[j][k] = dum;
         }
         d = -d;
         vv[iMax] = vv[j];
      }
      indx[j] = iMax;
      if (j != 2) {
         if (fabs(a[j][j]) < TINY_REAL) a[j][j] = TINY_REAL;
         dum = 1.0 / a[j][j];
         for (i=j+1; i<3; i++) a[i][j] *= dum;
      }
   }
   if (fabs(a[2][2]) < TINY_REAL) a[2][2] = TINY_REAL;
}

// static void lubksp(XYZ *a,Int32 *indx,XYZ &b)
static void lubksp(XYZ a[3],Int32 indx[3],XYZ &b)
{
   Int32        j,ip,ii,i;
   Real         sum;

//  LU back substitution routine  }

   ii = -1;
   for (i=0; i<3; i++) {
      ip = indx[i];
      sum = b[ip];
      b[ip] = b[i];
      if (ii >= 0) {
         for (j=ii; j<=i-1; j++) sum -= a[i][j] * b[j];
      }
      else if (fabs(sum) > TINY_REAL) ii = i;
      b[i] = sum;
   }
   for (i=2; i>=0; i--) {
      sum = b[i];
      if (i < 2) {
         for (j=i+1; j<3; j++) sum -= a[i][j] * b[j];
      }
      b[i] = sum / a[i][i];
   }
}

Boolean SPsolve3Eqs(XYZ coeffs[3],XYZ &sigma,XYZ &xyz)
// This procedure is here so no one need ever bother with the
// stupid names and calling sequences of ludcmp() and lubskp().

// input:   coefficient matrix coeffs:    coeffs[n] = <xn, yn, zn>
//          sum vector sigma              sigma = <sigmax, sigmay, sigmaz>
// output:  variable vector xyz

// equations written as matrix mult.:  coeffs <xyz column vector> = sigma   

{
   Int32       indx[3];
   Real        d;
   XYZ         hold;

   XYZ         holdCoeffs[3];
   Real        diff;

   Int32   i;
   for (i=0; i<3; i++) holdCoeffs[i] = coeffs[i];

   ludcmp(coeffs,indx,d);

   hold[0] = sigma[0]; hold[1] = sigma[1]; hold[2] = sigma[2]; 
   
   lubksp(coeffs,indx,hold);

   xyz[0]  = hold[0];  xyz[1]  = hold[1];  xyz[2]  = hold[2]; 

   // check, 'cause we're suspicious.

   for (i=0; i<3; i++) {
      diff = xyz.dotProduct(holdCoeffs[i]) - sigma[i];
      if (fabs(diff) > TINY_REAL) {
         printf("solve3Eqs result lacking in precision\n");
//       SP_ERROR("solve3Eqs failure");
         return FALSE;
      }
   }
   return TRUE;
}


