#ifndef __COMMON__
#define __COMMON__

#include "allocore/io/al_App.hpp"
using namespace al;

// Common definition of application state

struct State {
  Vec3f position[50];
  float rad[50];
  Vec3f myPosition;
  float myRad;
  bool simulate;
};

#endif
