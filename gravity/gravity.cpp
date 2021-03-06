//Yuan Yao, MAT201B, Feb 6th, yuanyao00@ucsb.edu
#include "allocore/io/al_App.hpp"
#include "Cuttlebone/Cuttlebone.hpp"
#include "Gamma/Filter.h"
#include "Gamma/Oscillator.h"
#include "gravity_common.hpp"
using namespace al;
using namespace std;
using namespace gam;

// some of these must be carefully balanced; i spent some time turning them.
// change them however you like, but make a note of these settings.
unsigned particleCount = 10;     // try 2, 5, 50, and 5000
double maximumAcceleration = 30;  // prevents explosion, loss of particles
double initialRadius = 50;        // initial condition
double initialSpeed = 50;         // initial condition
double gravityFactor = 1e6;       // see Gravitational Constant
double timeStep = 0.0625;         // keys change this value for effect
// double timeStep = 0.01;
double scaleFactor = 0.1;         // resizes the entire scene
double sphereRadius = 20;  // increase this to make collisions more frequent
double springConstant = 1000;
double dragFactor = 0.001;

Mesh sphere;  // global prototype; leave this alone

// helper function: makes a random vector
Vec3f r() { return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()); }

// Phasor and Sawtooth are from the class example
struct Phasor {
  float phase = 0, increment = 0.001;
  void frequency(float hz, float sampleRate) { increment = hz / sampleRate; }
  float getNextSample() {
    float returnValue = phase;
    phase += increment;
    if (phase > 1) phase -= 1;
    return returnValue;
  }
  float operator()() { return getNextSample(); }
};

struct Sawtooth : Phasor {
  float getNextSample() { return 2 * Phasor::getNextSample() - 1; }
  float operator()() { return getNextSample(); }
};

struct Particle { 
  Vec3f position, velocity, acceleration;
  Color c;
  Particle() {
    position = r() * initialRadius;
    velocity =
        // this will tend to spin stuff around the y axis
        Vec3f(0, 1, 0).cross(position).normalize(initialSpeed);
    c = HSV(rnd::uniform(), 0.7, 1);
  }
  void draw(Graphics& g) {
    g.pushMatrix();
    g.translate(position);
    g.color(c);
    g.draw(sphere);
    g.popMatrix();
  }
};

struct MyApp : App{

  Material material;
  Light light;
  bool simulate = true;

  vector<Particle> particle;
  State* state = new State;
  cuttlebone::Maker<State> maker;

  Sawtooth saw;
  Phasor frequency;

  MyApp() : maker("127.0.0.1"){
    
    addSphere(sphere, sphereRadius);
    sphere.generateNormals();
    light.pos(0, 0, 0);              // place the light
    nav().pos(0, 0, 30);             // place the viewer
    lens().far(400);                 // set the far clipping plane
    particle.resize(particleCount);  // make all the particles
    background(Color(0.07));

    initWindow();
    initAudio();

    
  }

  void onAnimate(double dt) {
    if (!simulate)
      // skip the rest of this function
      return;

    //
    //  Detect Collisions Here
    //

    for (unsigned i = 0; i < particle.size(); ++i)
      for (unsigned j = 1 + i; j < particle.size(); ++j) {
        Particle& a = particle[i];
        Particle& b = particle[j];
        //collide if the positions are equal
        Vec3f diff = (b.position - a.position);
        if (diff.mag() < (2*sphereRadius)){
          cout<<"collide"<<endl;
          Vec3f acceleration = (diff - 2 * sphereRadius)  * springConstant;
          a.acceleration -= acceleration;
          b.acceleration += acceleration;
        }else{
          double d = diff.mag();

        // F = ma where m=1
          Vec3f acceleration = diff / (d * d * d) * gravityFactor;
        // equal and opp osite force (symmetrical)
          a.acceleration += acceleration;
          b.acceleration -= acceleration;
        }
      }
    
      //drag 
      for(auto& p: particle)
        p.acceleration += p.velocity  * -dragFactor;
    

    // Limit acceleration
    for (auto& p : particle)
      if (p.acceleration.mag() > maximumAcceleration)
        p.acceleration.normalize(maximumAcceleration);

    // Euler's Method; Keep the time step small
    for (auto& p : particle) p.position += p.velocity * timeStep;
    for (auto& p : particle) p.velocity += p.acceleration * timeStep;
    for (auto& p : particle) p.acceleration.zero();


    maker.set(*state);
    for (unsigned i = 0; i < particle.size(); i++)
       state->position[i] = particle[i].position;


  }

  void onDraw(Graphics& g) {
    material();
    light();
    g.scale(scaleFactor);
    for (auto p : particle) p.draw(g);
  }



  void onSound(AudioIOData& io) {
    while (io()) {
      saw.frequency(200 + 20 * frequency(), 24100);
      float s = saw() * 0.3;
      for (unsigned i = 0; i < particle.size(); ++i)
      for (unsigned j = 1 + i; j < particle.size(); ++j) {
        Particle& a = particle[i];
        Particle& b = particle[j];
        //collide if the positions are equal
        Vec3f diff = (b.position - a.position);
        if (diff.mag() < (2*sphereRadius)){
                  io.out(0) = s;
                  io.out(1) = s;
                  //cout << "collide" << endl;
          }     
        }
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
