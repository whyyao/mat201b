#include "common.h"

#include "alloutil/al_OmniStereoGraphicsRenderer.hpp"

using namespace al;
using namespace std;
// Mesh sphere;

struct MyApp : OmniStereoGraphicsRenderer {
  cuttlebone::Taker<State> taker;
  State* state = new State;

  Material material;
  Light light;
  vector<enPlanet> planets;

  Texture gameoverText;

  mePlanet myPlanet;
  bool simulate = true;

  // background related
  Mesh bgMesh;
  Texture bgTexture;

  MyApp() {
    memset(state, 0, sizeof(State));
    addSphereWithTexcoords(bgMesh);
    // load image into texture print out error and exit if failure
    Image image;
    if (image.load(fullPathOrDie("cell2.jpg"))) {
      cout << "Read image from " << endl;
    } else {
      cout << "Failed to read image from "
           << "!!!" << endl;
      exit(-1);
    }

    bgTexture.allocate(image.array());

     if (image.load(fullPathOrDie("gameover.png"))) {
      cout << "Read image from " << endl;
    } else {
      cout << "Failed to read image from "
           << "!!!" << endl;
      exit(-1);
    }
    gameoverText.allocate(image.array());

    // initial pos/light/lens
    light.pos(0, 0, 0);
    nav().pos(0, 0, 0);
    lens().near(0.1);
    lens().far(5000);
    lens().far(400);

    planets.resize(particleCount);
  }

  void onAnimate(double dt) {
    nav().pos(0,0,0);
    if (taker.get(*state) > 0) pose = state->pose;

    for (unsigned i = 0; i < planets.size(); i++) {
      planets[i].position = state->position[i];
      planets[i].volume = state->volume[i];
    }
    myPlanet.position = state->myPosition;
    myPlanet.volume = state->myVol;

    //nav().faceToward(myPlanet.position, Vec3d(0, 1, 0), 0.05);

    for (auto& p : planets) {
      p.updateVolume();
      p.updateColor(myPlanet);
    }
    myPlanet.updateVolume();
  }

  void onDraw(Graphics& g) {
    shader().uniform("lighting", 0.0);
    shader().uniform("texture", 1.0);

    g.lighting(false);  // turn off lighting
    //// disable depth buffer, so that background will be drawn over
    g.depthMask(false);

  g.blending(true);
    g.blendModeTrans();
    g.pushMatrix();
    g.translate(nav().pos());
    g.rotate(180, 0, 0, 1);
    bgTexture.bind();
    g.color(1, 1, 1);
    g.draw(bgMesh);
    bgTexture.unbind();
    g.popMatrix();

      if(myPlanet.volume <= 0 ){
      g.pushMatrix();
      g.translate(myPlanet.position + Vec3f(2,2,2));
      g.scale(50);
      g.rotate(Quatd::getBillboardRotation((myPlanet.position - nav().pos()), nav().uu()));
      gameoverText.quad(g);
      g.popMatrix();
    }
     g.blending(false);
    g.depthMask(true);  // turn depth mask back on
   
    // g.lighting(true);

    // material();
    light();
    shader().uniform("lighting", 0.5);
    shader().uniform("texture", 0.0);

    // bgTexture.quad(g);
    g.color(1, 1, 1);
    g.scale(scaleFactor);
    myPlanet.draw(g);
    for (auto& b : planets) {
      if (b.rad > 0) {
        b.draw(g);
      }
    }
  }
};

int main() {
  MyApp app;
  app.taker.start();
  app.start();
}
