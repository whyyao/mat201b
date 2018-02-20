//Yuan Yao, MAT201B, Feb 6th, yuanyao00@ucsb.edu
#include "allocore/io/al_App.hpp"
#include "Cuttlebone/Cuttlebone.hpp"
#include "agents_common.hpp"
#include "Gamma/Delay.h"
#include "Gamma/Filter.h"
#include "Gamma/Oscillator.h"
using namespace al;
using namespace std;
using namespace gam;

// some of these must be carefully balanced; i spent some time turning them.
// change them however you like, but make a note of these settings.
unsigned particleCount = 50;     // try 2, 5, 50, and 5000
double maximumAcceleration = 30;  // prevents explosion, loss of particles
double initialRadius = 50;        // initial condition
double initialSpeed = 50;         // initial condition
double gravityFactor = 1e6;       // see Gravitational Constant
double timeStep = 0.0625;         // keys change this value for effect
double scaleFactor = 0.1;         // resizes the entire scene
double sphereRadius = 3;  // increase this to make collisions more frequent
double springConstant = 80;

Mesh sphere;  // global prototype; leave this alone

// helper function: makes a random vector
Vec3f r() { return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()); }

struct Boid{
  Vec3f location, velocity, acceleration;
  float maxspeed, maxforce, rad;
  Color c;
  
  Boid(){
    // acceleration = Vec3f(0,0,0);
    acceleration = Vec3f(0,0,0);
    velocity = Vec3f(0,0,0);
    location = r()*initialRadius;
    rad = 3.0;
    maxspeed = 4;
    maxforce = 0.1;
    c = HSV(rnd::uniform(), 0.7, 1);
  }

  //Euler method
  void update(){
    this->velocity += this->acceleration;
    if (velocity.mag() > maxspeed){
          velocity.normalize(maxspeed);
    }
    this->location += this->velocity;
    acceleration.zero(); 
  }

  Vec3f seek(Vec3f target){
    Vec3f desired = target - this->location;
    desired.normalize();
    desired = desired * maxspeed ;
    Vec3f steer = desired - this->velocity;
    
    //transfer steer.limit(maxforce);
    if (steer.mag() > maxforce){
          steer.normalize(maxforce);
          applyForce(steer);
          return steer;
    }else{
      return Vec3f(0,0,0);
    }
  }

  void applyForce(Vec3f force){
    acceleration += force;
  }

  void draw(Graphics& g) {
    g.pushMatrix();
    g.translate(location);
    g.color(c);
    g.draw(sphere);
    g.popMatrix();
  }

  void flock(vector<Boid> boids){
    Vec3f sep = separate(boids);
    Vec3f ali = align(boids);
    Vec3f coh = cohesion(boids);
    sep = sep * 1.5;
    ali = ali * 1.0;
    coh = coh * 1.0;
    applyForce(sep);
    applyForce(ali);
    applyForce(coh);
  }

  Vec3f separate(vector<Boid> boids){
    float desiredseparation = 20;
    Vec3f sum(0,0,0);
    int count = 0;
    for (auto boid: boids){
      Vec3f temp = (this->location - boid.location);
      float d = temp.mag();
      if ((d>0) && (d< desiredseparation)){
        Vec3f diff =  this->location - boid.location;
        diff /= d;
        sum += diff;
        count++;
      }
    }

    if(count > 0){
      sum /= count;
      sum.normalize();
      sum *= maxspeed;
      Vec3f steer = sum - velocity;
      if (steer.mag() > maxforce){
            steer.normalize(maxforce);
      }
      applyForce(steer);
      return steer;
    }else{
      return Vec3f(0,0,0);
    }
  }

  Vec3f align(vector<Boid> boids){
    float neighboardist = 50;
    Vec3f sum(0,0,0);
    int count = 0;
    
    for(Boid boid: boids){
      Vec3f temp = (this->location - boid.location);
      float d = temp.mag();
      if((d>0) && (d<neighboardist)){
        sum += boid.velocity;
        count++;
      }
    }
    if (count > 0){
      sum /= boids.size();
      sum.normalize();
      sum *= maxspeed;
      Vec3f steer = sum - velocity;
      if (steer.mag() > maxforce){
            steer.normalize(maxforce);
      }
      return steer;
    }else{
      return Vec3f(0,0,0);
    }
  
  }

  Vec3f cohesion(vector<Boid> boids){
    float neighboardist = 50;
    Vec3f sum(0,0,0);
    int count = 0;
    for(Boid boid: boids){
      Vec3f temp = (this->location - boid.location);
      float d = temp.mag();
      if((d>0) && (d<neighboardist)){
        sum += boid.location;
        count++;
      }
    }
    if (count > 0){
      sum /= boids.size();
      return seek(sum);
    }else{
      return Vec3f(0,0,0);
    }
  }
};

struct MyApp : App {


  Accum<> tmr;  // Timer for triggering sound
  SineD<> src;  // Sine grain
  Delay<> delay;  // Delay line
  OnePole<> lpf;

  State* state = new State;
  cuttlebone::Maker<State> maker;


  Material material;
  Light light;
  bool simulate = true;
  vector<Boid> boids;
  MyApp(): maker("127.0.0.1"){
    delay.maxDelay(0.2);

    tmr.period(4);
    tmr.phaseMax();

    // Configure a short cosine grain
    src.set(1000, 0.8, 0.04, 0.25);

    // Set up low-pass filter
    lpf.type(gam::LOW_PASS);
    lpf.freq(2000);

    addSphere(sphere, sphereRadius);
    sphere.generateNormals();
    light.pos(0, 0, 0);              // place the light
    nav().pos(0, 0, 30);             // place the viewer
    lens().far(400);                 // set the far clipping plane
    boids.resize(particleCount);  // make all the particles
    background(Color(0.07));

    initWindow();
    initAudio();
  }

  void onAnimate(double dt) {
    // if (!simulate)
    //   // skip the rest of this function
    //   return;
    for (auto& b: boids) {
      b.flock(boids);
      b.update();
    }

    maker.set(*state);
    for (unsigned i = 0; i < boids.size(); i++)
       state->position[i] = boids[i].location;
  }

  void onDraw(Graphics& g) {
    material();
    light();
    g.scale(scaleFactor);
    for (auto& b : boids) b.draw(g);
  }

  void onSound(AudioIOData& io) {
    gam::Sync::master().spu(audioIO().fps());
    while(io()){

      if(tmr()) src.reset();

      float s = src();

      // Read the end of the delay line to get the echo
      float echo = delay();

      // Low-pass filter and attenuate the echo
      echo = lpf(echo) * 0.8;

      // Write sum of current sample and echo to delay line
      delay(s + echo);

      // Finally output sum of dry and wet signals
      s += echo;
    
      io.out(0) = io.out(1) = s;
    }
  }

  void onKeyDown(const ViewpointWindow&, const Keyboard& k) {
    switch (k.key()) {
      default:
      case '1':
        // reverse time
        timeStep *= -1;
        break;
      case '2':
        // speed up time
        if (timeStep < 1) timeStep *= 2;
        break;
      case '3':
        // slow down time
        if (timeStep > 0.0005) timeStep /= 2;
        break;
      case '4':
        // pause the simulation
        simulate = !simulate;
        break;
    }
  }
};

int main() {   
  MyApp app;
  app.maker.start();
  app.start(); }

