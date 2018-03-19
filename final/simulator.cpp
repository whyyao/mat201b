#include "common.h"

#include <unistd.h>
#include "Gamma/SamplePlayer.h"
#include "alloutil/al_AlloSphereAudioSpatializer.hpp"
#include "alloutil/al_Simulator.hpp"

using namespace al;
using namespace std;
using namespace gam;
// Mesh sphere;

struct MyApp : App, AlloSphereAudioSpatializer, InterfaceServerClient {
  State* state = new State;
  cuttlebone::Maker<State> maker;

  SoundSource aSoundSource;

  Material material;
  Light light;
  Texture gameoverText;

  vector<enPlanet> planets;

  mePlanet myPlanet;
  bool simulate = true;
  bool gameRestart = false;

  Vec3f savePos;

  // background related
  Mesh bgMesh;
  Texture bgTexture;

  SamplePlayer<> bgPlayer;
  SamplePlayer<> absorbPlayer;

  MyApp()
      : maker(Simulator::defaultBroadcastIP()),
        InterfaceServerClient(Simulator::defaultInterfaceServerIP()) {
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
    gameRestart = false;
    // initial pos/light/lens
    light.pos(0, 0, -200);
    nav().pos(0, 0, 0);
    lens().far(400);

    planets.resize(particleCount);

    background(Color(0.07));
    initWindow();
App::background(Color(0.7, 1.0));
    // audio
    bgPlayer.load(fullPathOrDie("bg.wav").c_str());
    absorbPlayer.load(fullPathOrDie("absorb.wav").c_str());
    bgPlayer.reset();
    absorbPlayer.finish();

    AlloSphereAudioSpatializer::initAudio();
    AlloSphereAudioSpatializer::initSpatialization();
    // if gamma
    gam::Sync::master().spu(AlloSphereAudioSpatializer::audioIO().fps());
    scene()->addSource(aSoundSource);
    aSoundSource.dopplerType(DOPPLER_NONE);
    scene()->usePerSampleProcessing(false);
  }

  void onAnimate(double dt) {
    while (InterfaceServerClient::oscRecv().recv())
      ;  // XXX
    nav().pos(0,0,0);

    // pressed s to pause/resume the game
    if (!simulate) return;

    for (int i = 0; i < planets.size(); i++) {
      for (int j = i + 1; j < planets.size(); j++) {
        if (planets[i].ifCollide(planets[j])) {
          if (planets[i].volume > planets[j].volume) {
            planets[i].absorb(planets[j]);

          } else {
            planets[j].absorb(planets[i]);
          }
        }
      }
    }

    for (int i = 0; i < planets.size(); i++) {
      if (myPlanet.ifCollide(planets[i])) {
        if (myPlanet.volume > planets[i].volume) {
          absorbPlayer.reset();
          myPlanet.absorb(planets[i]);
        } else {
          planets[i].absorb(myPlanet);
        }
      }
    }
    // update function for each planet
    for (auto& p : planets) p.update(myPlanet);
    myPlanet.update(myPlanet);

    nav().faceToward(myPlanet.position, Vec3d(0, 1, 0), 0.05);


    //check if lose
    if(myPlanet.volume<0){
      //usleep(3000);
      simulate = false;

      if(gameRestart == true){
        restart();
      }
    }

    //check if win
    for(auto& planet:planets){

    }

    // cuttlebone settings

    for (unsigned i = 0; i < planets.size(); i++) {
      state->position[i] = planets[i].position;
      state->volume[i] = planets[i].volume;
    }
    state->myPosition = myPlanet.position;
    state->myVol = myPlanet.volume;
    state->pose = nav();
    maker.set(*state);
  }

  virtual void onMouseDown(const ViewpointWindow& w, const Mouse& m) {
    Rayd r = getPickRay(w, m.x(), m.y());
    // cout<<"r: "<<r.direction()<<endl;
    float t = r.intersectSphere(Vec3d(0, 0, 0), placeholderSize);
    if (t > 0.f) {
    }
    // r(t)

    Vec3f newVelocity =
        cross(Vec3f(r(placeholderSize)), myPlanet.position).normalize();
    myPlanet.velocity = newVelocity;
    myPlanet.speed = 0.015;
    //myPlanet.clicked();
    savePos = r(placeholderSize);
    // myPlanet.speed = 0.01;
  }

  void onDraw(Graphics& g) {
    g.lighting(false);  // turn off lighting
    g.depthMask(
        false);  // disable depth buffer, so that background will be drawn over
    g.blending(true);
    g.blendModeTrans();
    g.pushMatrix();
    g.translate(nav().pos());
    g.rotate(180, 0, 0, 1);
    bgTexture.bind();
    g.color(1, 1, 1);
    //g.draw(bgMesh);
    bgTexture.unbind();
    g.popMatrix();

    if(myPlanet.volume <= 0 ){
      g.pushMatrix();
      g.translate(myPlanet.position + Vec3f(2,2,2));
      g.rotate(Quatd::getBillboardRotation((myPlanet.position - nav().pos()), nav().uu()));
      //g.rotate(Quatd::getBillboardRotation(myPlanet.position, nav().uu()));
      g.scale(50);
      gameoverText.quad(g);
      g.popMatrix();
    }
   

    g.blending(false);

    g.depthMask(true);  // turn depth mask back on

    material();
    light();
    //bgTexture.quad(g);
    g.scale(scaleFactor);
    myPlanet.draw(g);
    for (auto& b : planets) {
      if (b.rad > 0) {
        b.draw(g);
      }
    }
    Mesh& m = g.mesh();
    addCone(m);
    g.translate(savePos);
    g.draw(m);

   
  }

  void onKeyDown(const ViewpointWindow&, const Keyboard& k) {
    switch (k.key()) {
      default:
        break;
      case 'l':
        simulate = !simulate;
        cout<<"l is pressed"<<endl;
        break;
      case 'o':
        simulate = true;  
        gameRestart = !gameRestart;
        
        break;
    }
  }

  virtual void onSound(AudioIOData& io) {
    // aSoundSource.pose(nav());
    while (io()) {
      // aSoundSource.writeSample(bgPlayer());
      // aSoundSource.writeSample(absorbPlayer());
      io.out(0) = io.out(1) = bgPlayer() + absorbPlayer();
    }
    // listener()->pose(nav());
    // scene()->render(io);
  }

  void restart(){ 
    //simulate = true;
    //myPlanet = mePlanet();
    planets.clear();
    planets.resize(particleCount);
    myPlanet.rad = sphereRadius;
    myPlanet.mesh.reset();
    addSphere(myPlanet.mesh, myPlanet.rad);
    myPlanet.mesh.generateNormals();
    myPlanet.volume = 3.14 * 4 / 3 * (myPlanet.rad) * (myPlanet.rad) * (myPlanet.rad);
  }

};


int main() {
  MyApp app;
  app.AlloSphereAudioSpatializer::audioIO().start();
  app.InterfaceServerClient::connect();
  app.maker.start();
  app.start();
}
