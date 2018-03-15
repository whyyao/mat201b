#include "allocore/io/al_App.hpp"
#include "Cuttlebone/Cuttlebone.hpp"
#include <cmath> 

using namespace al;
using namespace std;

unsigned particleCount = 50;   
double maximumAcceleration = 10;  
double sphereRadius = 10;  
double placeholderSize = 300;
float scaleFactor = 0.1;   
float maxspeed = 40;
float maxforce = 0.1;

Vec3f r() { return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()); }
// Vec3f toVec(Quatf q) { return Vec3f(q.x, q.y, q.z); }

struct Planet{  
  Vec3f position, velocity, acceleration;
  Color c;
  float volume, rad;
  bool myself;
  Mesh mesh;
  float speed;
  
  //default - not a "me" planet
  Planet(){
    //setting up basic info
    myself = false;
    velocity = r();
    position = (r()*placeholderSize).normalize(placeholderSize);
    speed = rnd::uniform(0.005);
    rad = (rnd::uniformS()*4)+sphereRadius;
    volume = 3.14* 4/3 *(rad) *(rad) *(rad);

    //adding mesh
    addSphere(mesh, rad);
    mesh.generateNormals();
    c = HSV(0, 0.7, 1);
  }

  //use to set up "me" planet info
  void setMe(){
    myself= true;
    //has a default speed and deacceleration when click on mouse
    velocity.zero();
    speed = (0.01);
    acceleration = Vec3f(-0.05,-0.05,-0.05);
    //default size and redraw mesh
    rad = sphereRadius;
    mesh.reset();
    addSphere(mesh, rad);
    mesh.generateNormals();
    //blue color
    c = HSV(200.0/360.0f, 1 , 1);
    //always starts at the default position
    position = (Vec3f(0,0,-200)).normalize(placeholderSize);
  }

  void update(Planet my){
    //used to rotate and updated the position on the surface

    velocity += acceleration;
    updateVolume();

    //cout<<velocity<<endl;

    if(myself == false){
      
      Quatf q;
      q.fromAxisAngle(speed, velocity);
      q.normalize();
      position = q.rotate(position);
      // if planet volume is greater, change color to red
      // if not, change color to green
      updateColor(my);
    }else{
      Quatf q;
      q.fromAxisAngle(speed, velocity);
      speed *= 0.9;
      q.normalize();
      position = q.rotate(position);
      // "me" planet
    }
  }

  void updateColor(Planet my){
    if(myself == false){
    if(my.volume > volume){
            c = HSV(120/360.0f, 1, 1);
    }else{
            c =  HSV(0, 0.7, 1);
    }
  }
  }
  void draw(Graphics& g) {
    g.pushMatrix();
    g.translate(position);
    g.color(c);
    g.draw(mesh);
    g.popMatrix();
  }

  //function to see if otherPlanet is colliding with self
  bool ifCollide(Planet otherPlanet){
      if((position - otherPlanet.position).mag() <= (rad + otherPlanet.rad)){
        return true;
      }
    return false;
  }

//absorb with animate according with speed
  void absorb(Planet& otherPlanet){
    float absorbtionSpeed = speed+otherPlanet.speed;
    float deltaVolume = absorbtionSpeed * 50000;
    // cout<<"devolume"<<deltaVolume<<endl;
    // cout<<"volume"<<volume<<endl;
    volume += deltaVolume;
    otherPlanet.volume -= deltaVolume;
  }

//update rad with new volume
  void updateVolume(){
    rad = pow((volume *3 /4 /3.14),1.0/3);
    mesh.reset();
    addSphere(mesh,rad);
    mesh.generateNormals();
  }
};