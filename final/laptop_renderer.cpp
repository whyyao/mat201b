#include "common.h"

using namespace al;
using namespace std;
// Mesh sphere;

struct MyApp : App {
  cuttlebone::Taker<State> taker;
  State* state = new State;

  Material material;
  Light light;

  vector<Planet> planets;
  Planet special;
  Planet myPlanet;
  bool simulate = true;

  // background related
  Mesh bgMesh;
  Texture bgTexture;

  MyApp() {
    addSphereWithTexcoords(bgMesh);
    // load image into texture print out error and exit if failure
    Image image;
    if (image.load(fullPathOrDie("cell.jpg"))) {
      cout << "Read image from " << endl;
    } else {
      cout << "Failed to read image from "
           << "!!!" << endl;
      exit(-1);
    }

    bgTexture.allocate(image.array());

    // initial pos/light/lens
    light.pos(0, 0, 0);
    nav().pos(0, 0, 0);
    lens().far(400);

    planets.resize(particleCount);
    myPlanet.setMe();

    background(Color(0.07));
    initWindow();
    initAudio();
  }

  void onAnimate(double dt) {
    taker.get(*state);
    simulate = state->simulate;
    // pressed s to pause/resume the game
    if (!simulate) return;

    for (unsigned i = 0; i < planets.size(); i++) {
      planets[i].position = state->position[i];
      planets[i].rad = state->rad[i];
    }
    myPlanet.position = state->myPosition;
    myPlanet.rad = state->myRad;

    nav().faceToward(myPlanet.position, Vec3d(0,1,0), 0.05);

    
    for(auto& p: planets){
      p.updateVolume();
      p.updateColor(myPlanet);
    }
  }

  void onDraw(Graphics& g) {
    g.lighting(false);  // turn off lighting
    g.depthMask(
        false);  // disable depth buffer, so that background will be drawn over

    g.pushMatrix();
    g.translate(nav().pos());
    g.rotate(180, 0, 0, 1);
    bgTexture.bind();
    g.color(1, 1, 1);
    g.draw(bgMesh);
    bgTexture.unbind();
    g.popMatrix();

    g.depthMask(true);  // turn depth mask back on

    material();
    light();
    // bgTexture.quad(g);
    g.scale(scaleFactor);
    myPlanet.draw(g);
    for (auto& b : planets) b.draw(g);
  }

  void onKeyDown(const Keyboard& k) {
    switch (k.key()) {
      case 's':
        cout << "pressed s" << endl;
        simulate = !simulate;
        break;
    }
  }

  // check if any planet has volume less than 0. If yes, delete them
  void checkIfExist(vector<Planet>& planets, Planet& myPlanet) {
    vector<int> deletingPlanet;
    for (int i = 0; i < planets.size(); i++) {
      Planet planet = planets[i];
      if (planet.volume < 0) {
        deletingPlanet.push_back(i);
      }
    }
    for (auto index : deletingPlanet) {
      planets.erase(planets.begin() + index);
    }
    if (myPlanet.volume < 0) {
      simulate = false;
    }
  }
};

int main() {
  MyApp app;
  app.taker.start();
  app.start();
}
