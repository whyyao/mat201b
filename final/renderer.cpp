#include "common.h"
#include "alloutil/al_OmniStereoGraphicsRenderer.hpp"

using namespace al;
using namespace std;


struct MyApp : OmniStereoGraphicsRenderer {
  cuttlebone::Taker<State> taker;
  State* state = new State;

  Material material;
  Light light;

  vector<enPlanet> planets;
  mePlanet myPlanet;
  bool simulate = true;

  Vec3f pointer;
  
  Mesh bgMesh;
  Texture bgTexture;
  Texture gameoverText;
  Texture winText;

  MyApp() {
    memset(state, 0, sizeof(State));
    addSphereWithTexcoords(bgMesh);
    // load image into texture print out error and exit if failure
    Image image;
    if (image.load(fullPathOrDie("cell2.jpg"))) {
      bgTexture.allocate(image.array());
    } else {
      exit(-1);
    }

    if (image.load(fullPathOrDie("gameover.png"))) {
      gameoverText.allocate(image.array());
    } else {
      exit(-1);
    }

    if (image.load(fullPathOrDie("win.png"))){
      winText.allocate(image.array());
    }else{
      exit(-1);
    }

    light.pos(0, 0, -100);
    nav().pos(0, 0, 0);
    lens().near(0.1);
    lens().far(5000);
    lens().far(400);

    planets.resize(particleCount);
  }

  void onAnimate(double dt) {
    nav().pos(0,0,0);
    taker.get(*state);

    for (unsigned i = 0; i < planets.size(); i++) {
      planets[i].position = state->position[i];
      planets[i].volume = state->volume[i];
    }
    myPlanet.position = state->myPosition;
    myPlanet.volume = state->myVol;
    pointer =  state->pointer;

    for (auto& p : planets) {
      p.updateRadius();
      p.updateColor(myPlanet);
    }
    myPlanet.updateRadius();
  }

  void onDraw(Graphics& g) {
    shader().uniform("lighting", 0.0);
    shader().uniform("texture", 1.0);

    g.lighting(false); 
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

    lose(g);
    win(g);
    g.blending(false);
    g.depthMask(true); 
    light();

    shader().uniform("lighting", 0.5);
    shader().uniform("texture", 0.0);

    g.color(1, 1, 1);
    g.scale(scaleFactor);
    myPlanet.draw(g);
    for (auto& b : planets) {
      if (b.rad > 0) {
        b.draw(g);
      }
    }
    Mesh& m = g.mesh();
    addCone(m);
    g.translate(pointer);
    g.draw(m);
  }

  void win(Graphics& g){
    bool ifWin = true;
    for(auto& planet: planets){
      if (planet.volume > 0){
        ifWin = false;
        break;
      }
    }
    if(ifWin == true){
      g.pushMatrix();
      g.translate(myPlanet.position + Vec3f(50,50,50));
      Vec3d forward = Vec3d(nav().pos() - myPlanet.position).normalize();
      Quatd rot = Quatd::getBillboardRotation(forward, nav().uu());
      g.rotate(rot);
      g.scale(100);
      winText.quad(g);
      g.popMatrix();
    }
  }

  void lose(Graphics& g){
    bool ifLose = true;
    for(auto& planet: planets){
      if(planet.volume < myPlanet.volume){
        ifLose = false;
      }
    }
    if(myPlanet.volume < 0 ){
      ifLose = true;
    }

    if(ifLose){
      g.pushMatrix();
      g.translate(myPlanet.position + Vec3f(10,10,10));
      Vec3d forward = Vec3d(nav().pos() - myPlanet.position).normalize();
      Quatd rot = Quatd::getBillboardRotation(forward, nav().uu());
      g.rotate(rot);
      g.scale(100);
      gameoverText.quad(g);
      g.popMatrix();
    }
  }
};

int main() {
  MyApp app;
  app.taker.start();
  app.start();
}
