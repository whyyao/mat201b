//Yuan Yao, MAT201B, Feb 29th, yuanyao00@ucsb.edu
#include "allocore/io/al_App.hpp"
#include "Cuttlebone/Cuttlebone.hpp"
using namespace al;
using namespace std;

unsigned particleCount = 200;     // try 2, 5, 50, and 5000
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

struct Planet{
  Vec3f location, velocity, acceleration;
  float maxspeed, maxforce, rad;
  Color c;

  Planet(){
    // acceleration = Vec3f(0,0,0);
    acceleration = Vec3f(0,0,0);
    velocity = r();
    location = (r()*200).normalize(200);
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
    if (location.mag() > 200){
      location = location.normalize(200);
    }
    acceleration.zero(); 
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

};

struct MyApp : App {

  // State* state = new State;
  //cuttlebone::Maker<State> maker;


  Material material;
  Light light;
  bool simulate = true;
  vector<Planet> boids;
  MyApp(){
    addSphere(sphere, sphereRadius);
    sphere.generateNormals();
    light.pos(0, 0, 0);              // place the light
    nav().pos(0, 0, 100);             // place the viewer
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
      b.update();
    }

    //maker.set(*state);
    // for (unsigned i = 0; i < boids.size(); i++)
    //    state->position[i] = boids[i].location;
  }

  void onDraw(Graphics& g) {
    material();
    light();
    g.scale(scaleFactor);
    for (auto& b : boids) b.draw(g);
  }

  void onSound(AudioIO& io) {
    while (io()) {
      io.out(0) = 0;
      io.out(1) = 0;
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
  //app.maker.start();
  app.start(); }

