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

string fullPathOrDie(string fileName, string whereToLook = ".") {
  SearchPaths searchPaths;
  searchPaths.addSearchPath(whereToLook);
  string filePath = searchPaths.find(fileName).filepath();
  assert(filePath != "");
  return filePath;
}

struct State {
  Vec3f position[50];
  float volume[50];
  Vec3f myPosition;
  float myVol;
  bool simulate;
  Pose pose;
};

#endif
