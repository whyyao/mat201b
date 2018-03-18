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

  vector<enPlanet> planets;

  myPlanet myPlanet;
  bool simulate = true;

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

    background(Color(0.07));
    initWindow();

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
    // nav().pos(0,0,0);

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

    if (myPlanet.volume < 0) {
      usleep(1000);
      simulate = false;
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

    savePos = r(placeholderSize);
    // myPlanet.speed = 0.01;
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

  void onKeyDown(const Keyboard& k) {
    switch (k.key()) {
      case 's':
        cout << "pressed s" << endl;
        simulate = !simulate;
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
};

int main() {
  MyApp app;
  app.AlloSphereAudioSpatializer::audioIO().start();
  app.InterfaceServerClient::connect();
  app.maker.start();
  app.start();
}
