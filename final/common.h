#ifndef __COMMON__
#define __COMMON__

#include "allocore/io/al_App.hpp"

#include <cmath>
#include "Cuttlebone/Cuttlebone.hpp"
#include "allocore/io/al_App.hpp"
#include "allocore/math/al_Ray.hpp"
#include "planet.cpp"

using namespace al;


// Common definition of application state

  const unsigned particleCount = 100;     
  double sphereRadius = 10;  
  double placeholderSize = 300;
  double scaleFactor = 0.1;

string fullPathOrDie(string fileName, string whereToLook = ".") {
  SearchPaths searchPaths;
  searchPaths.addSearchPath(whereToLook);
  string filePath = searchPaths.find(fileName).filepath();
  assert(filePath != "");
  return filePath;
}

struct State {
  Vec3f position[particleCount];
  float volume[particleCount];
  Vec3f myPosition;
  float myVol;
};

#endif
