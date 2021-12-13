// sp/spprocs.h      Tom Ace  crux@qnet.com


// display.cc

void SPxyify(Int32 faceNr,Poly &poly,Face &xyFace,Boolean usePrevTranslation);
void SPinitGraphics();
void SPdrawCmd();


// first.cc

void SPeverythingElse();


// linear.cc

Boolean SPsolve3Eqs(XYZ coeffs[3],XYZ &sigma,XYZ &xyz);


// misc.c

void SPsleep(Int32);
void SPerror( const char*, const Int32, const char*);
void SPexit(Int32);


// inlines

inline Int32 SPabs(const Int32 a) { return a >= 0 ? a : -a; }
inline Int16 SPabs(const Int16 a) { return a >= 0 ? a : -a; }

inline Real  SPmin(const Real  a,const Real  b) { return a <= b ? a : b; }
inline Real  SPmax(const Real  a,const Real  b) { return a >= b ? a : b; }
inline Int32 SPmin(const Int32 a,const Int32 b) { return a <= b ? a : b; }
inline Int32 SPmax(const Int32 a,const Int32 b) { return a >= b ? a : b; }
inline Int16 SPmin(const Int16 a,const Int16 b) { return a <= b ? a : b; }
inline Int16 SPmax(const Int16 a,const Int16 b) { return a >= b ? a : b; }


extern "C" long random();

inline Real SPrandomPlusMinus(Real deltaMax)
{
   // return a real in the range [-deltaMax,+deltaMax]

   return deltaMax * Real(random()) / 1073741824. - deltaMax;
}

