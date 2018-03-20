#include "common.h"
#include <unistd.h>
#include "Gamma/SamplePlayer.h"
#include "alloutil/al_AlloSphereAudioSpatializer.hpp"
#include "alloutil/al_Simulator.hpp"

using namespace al;
using namespace std;
using namespace gam;


struct MyApp : App, AlloSphereAudioSpatializer, InterfaceServerClient {
  State* state = new State;
  cuttlebone::Maker<State> maker;

  SoundSource aSoundSource;
  SamplePlayer<> bgPlayer;
  SamplePlayer<> absorbPlayer;

  Material material;
  Light light;

  vector<enPlanet> planets;
  mePlanet myPlanet;
  bool simulate = true;
  bool gameRestart = false;

  Vec3f pointer;

  Mesh bgMesh;
  Texture bgTexture;
  Texture gameoverText;
  Texture winText;

  MyApp()
      : maker(Simulator::defaultBroadcastIP()),
        InterfaceServerClient(Simulator::defaultInterfaceServerIP()) {
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
   
    gameRestart = false;
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
  
    gam::Sync::master().spu(AlloSphereAudioSpatializer::audioIO().fps());
    scene()->addSource(aSoundSource);
    aSoundSource.dopplerType(DOPPLER_NONE);
    scene()->usePerSampleProcessing(false);
  }

  void onAnimate(double dt) {
    while (InterfaceServerClient::oscRecv().recv())
      ;
    //nav().pos(0,0,50);

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
      simulate = false;
      if(gameRestart == true){
        restart();
      }
    }

    // cuttlebone settings

    for (unsigned i = 0; i < planets.size(); i++) {
      state->position[i] = planets[i].position;
      state->volume[i] = planets[i].volume;
    }
    state->myPosition = myPlanet.position;
    state->myVol = myPlanet.volume;
    maker.set(*state);
  }

  virtual void onMouseDown(const ViewpointWindow& w, const Mouse& m) {
    Rayd r = getPickRay(w, m.x(), m .y());
    float t = r.intersectSphere(Vec3d(0, 0, 0), placeholderSize);
    Vec3f newVelocity = cross(Vec3f(r(placeholderSize)), myPlanet.position).normalize();
    myPlanet.velocity = newVelocity;
    myPlanet.speed = 0.015;
    pointer = r(placeholderSize);
  
  }

  void onDraw(Graphics& g) {
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
    material();
    light();
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
    planets.clear();
    planets.resize(particleCount);
    myPlanet.rad = sphereRadius;
    myPlanet.mesh.reset();
    addSphere(myPlanet.mesh, myPlanet.rad);
    myPlanet.mesh.generateNormals();
    myPlanet.volume = 3.14 * 4 / 3 * (myPlanet.rad) * (myPlanet.rad) * (myPlanet.rad);
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
      g.translate(myPlanet.position + Vec3f(50,50,5git 0));
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
  app.AlloSphereAudioSpatializer::audioIO().start();
  app.InterfaceServerClient::connect();
  app.maker.start();
  app.start();
}
