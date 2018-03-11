#include "allocore/io/al_App.hpp"
#include "Cuttlebone/Cuttlebone.hpp"
#include <cmath> 

using namespace al;
using namespace std;

unsigned particleCount = 30;   
double maximumAcceleration = 10;  
double sphereRadius = 10;  
double placeholderSize = 300;
float scaleFactor = 0.1;   

Vec3f r() { return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()); }
Vec3f toVec(Quatf q) { return Vec3f(q.x, q.y, q.z); }

struct Planet{  
  Vec3f position, velocity, acceleration;
  float maxspeed, maxforce, rad;
  Color c;
  float volume;
  bool myself;
  Mesh mesh;
  float speed;
  //phase = 0;
  
  //default to be a not me planet
  Planet(){
    velocity = r();
    position = (r()*placeholderSize).normalize(placeholderSize);
    //speed = rnd::uniform(0.005, 0.06);
    speed = rnd::uniform(0.001);
    rad = (rnd::uniformS()*4)+sphereRadius;
    addSphere(mesh, rad);
    mesh.generateNormals();
    maxspeed = 40;
    maxforce = 0.1;
    c = HSV(0, 0.7, 1);
    myself = false;
    volume = 3.14* 4/3 *(rad) *(rad) *(rad);
  }

  void setMe(){
    myself= true;
    rad = sphereRadius;
    mesh.reset();
    addSphere(mesh, rad);
    mesh.generateNormals();
    c = HSV(200.0/360.0f, 1 , 1);
    velocity.zero();
    position = (Vec3f(0,0,200)).normalize(placeholderSize);
  }

  //Euler method
  void update(Planet my){

    Quatf q;
    q.fromAxisAngle(speed, velocity);
    q.normalize();
    position = q.rotate(position);

    if(myself == false){
      if(position.mag()>placeholderSize){
          
      }
      if(my.volume > volume){
        c = HSV(120/360.0f, 1, 1);
      }else{
        c =  HSV(0, 0.7, 1);
      }
    }else{
       if(position.mag()>placeholderSize){
        position.normalize(placeholderSize);
      }
    }
  }

  void applyForce(Vec3f force){
    acceleration += force;
  }

  void draw(Graphics& g) {
    g.pushMatrix();
    g.translate(position);
    g.color(c);
    g.draw(mesh);
    g.popMatrix();
  }

  bool ifCollide(Planet otherPlanet){
    if((position - otherPlanet.position).mag() <= (rad + otherPlanet.rad)){
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