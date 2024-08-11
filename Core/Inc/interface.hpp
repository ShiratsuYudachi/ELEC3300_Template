// interface.hpp

#ifdef __cplusplus
extern "C" {
#endif

void myfunc();

void setActiveScreen0();
void setActiveScreen1();

#ifdef __cplusplus
}

#include "EasyUI.hpp"
extern const float (*targetGcode)[4];
extern int targetGcodeLength;
extern const float* targetGcodeCenterOfMass;
extern Screen operationScreen;

#endif
