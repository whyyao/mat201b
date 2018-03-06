//Yuan Yao, MAT201B, Feb 29th, yuanyao00@ucsb.edu
#include "allocore/io/al_App.hpp"
#include "Cuttlebone/Cuttlebone.hpp"
#include <cmath> 
using namespace al;
using namespace std;

unsigned particleCount = 20;   
double maximumAcceleration = 10;  
double sphereRadius = 10;  
double placeholderSize = 200;
float scaleFactor = 0.1;   

Mesh sphere;  

// helper to make random
Vec3f r() { return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()); }

struct Planet{
  Vec3f location, velocity, acceleration;
  float maxspeed, maxforce, rad;
  Color c;
  float volume;
  bool myself;
  Mesh mesh;
  //default to be a not me planet
  Planet(){
    velocity = r();
    location = (r()*placeholderSize).normalize(placeholderSize);
    rad = (rnd::uniformS()*1.5)+sphereRadius;
    addSphere(mesh, rad);
    mesh.generateNormals();
    maxspeed = 4;
    maxforce = 0.1;
    c = HSV(0, 0.7, 1);
    myself = false;
    volume = 3.14* 4/3 *(rad) *(rad) *(rad);
  }

  void setMe(){
    myself= true;
    c = HSV(200.0/360.0f, 1 , 1);
    velocity.zero();
  }

  //Euler method
  void update(){
    this->velocity += this->acceleration;
    if (velocity.mag() > maxspeed){
      velocity = velocity.normalize(maxspeed);
    }
    this->location += this->velocity;
    location = location.normalize(placeholderSize);
  }

  void applyForce(Vec3f force){
    acceleration += force;
  }

  void draw(Graphics& g) {
    g.pushMatrix();
    g.translate(location);
    g.color(c);
    g.draw(mesh);
    g.popMatrix();
  }

  bool ifCollide(Planet otherPlanet){
    if((location - otherPlanet.location).mag() <= (rad + otherPlanet.rad)){
      return true;
    }
  return false;
  }

  void absorb(Planet otherPlanet){
    volume += otherPlanet.volume;
    rad = pow((volume *3 /4/3.14),1.0/3);
    mesh.reset();
    addSphere(mesh,rad);
    mesh.generateNormals();
  }

};

struct MyApp : App {

  // State* state = new State;
  //cuttlebone::Maker<State> maker;

  Material material;
  Light light;

  vector<Planet> planets;
  Planet myPlanet;

  MyApp(){

    light.pos(0, 0, 0);         
    nav().pos(0, 0, 50);        
    lens().far(400);             

    planets.resize(particleCount);
    myPlanet.setMe();

    background(Color(0.07));
    initWindow();
    initAudio();
  }

  void onAnimate(double dt) {

    for (auto& b: planets) {
      b.update();
    }
    myPlanet.update();

    vector<int> deletingPlanet;
    for(int i = 0; i<planets.size(); i++){
      for (int j = i + 1;j<planets.size(); j++){
          if(planets[i].ifCollide(planets[j])){
            if(planets[i].volume>planets[j].volume){
              planets[i].absorb(planets[j]);
              deletingPlanet.push_back(j);
            }else{
              planets[j].absorb(planets[i]);
              deletingPlanet.push_back(i);
            }
          }
      }
    }

    for(auto index: deletingPlanet){
      planets.erase(planets.begin()+index);
    }

    //maker.set(*state);
    // for (unsigned i = 0; i < boids.size(); i++)
    //    state->position[i] = boids[i].location;
  }

  void onDraw(Graphics& g) {
    material();
    light();
    g.scale(scaleFactor);
    myPlanet.draw(g);
    for (auto& b : planets) b.draw(g);
    
  }


 void onKeyDown(const Keyboard& k){

		// Use a switch to do something when a particular key is pressed
		switch(k.key()){

		// For printable keys, we just use its character symbol:
		case 'i': 
      myPlanet.velocity.y += 2;
      break;
    case 'j':
      myPlanet.velocity.z += 2;
      break;
    case 'l':
      myPlanet.velocity.z -= 2;
      break;
    case 'k':
      myPlanet.velocity.y -= 2;
      break;
		}
	}
};

int main() {   
  MyApp app;
  //app.maker.start();
  app.start(); }


