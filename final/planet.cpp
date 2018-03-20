#include <cmath>
#include "Cuttlebone/Cuttlebone.hpp"
#include "allocore/io/al_App.hpp"

using namespace al;
using namespace std;

Vec3f r() { return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()); }
// Vec3f toVec(Quatf q) { return Vec3f(q.x, q.y, q.z); }

struct Planet {
  const unsigned particleCount = 100;
  double sphereRadius = 10;
  double placeholderSize = 300;

  Vec3f position, velocity, acceleration;
  Color c;
  float volume, rad, speed;
  Mesh mesh;
  
  Planet() {}

  //update color according to the volume
  void updateColor(Planet my) {
      if (my.volume > volume) {
        c = HSV(120 / 360.0f, 1, 1);
      } else {
        c = HSV(0, 0.7, 1);
      }
  }

  //called in onDraw
  void draw(Graphics& g) {
    g.pushMatrix();
    g.translate(position);
    g.color(c);
    g.draw(mesh);
    g.popMatrix();
  }

  // see if otherPlanet is colliding with self
  bool ifCollide(Planet otherPlanet) {
    if ((position - otherPlanet.position).mag() <= (rad + otherPlanet.rad)) {
      return true;
    }
    return false;
  }

  // absorb with animate according with speed
  void absorb(Planet& otherPlanet) {
    float absorbtionSpeed = speed + otherPlanet.speed;
    Vec3f diff = (rad - otherPlanet.rad);
    float deltaVolume = diff.mag()*(volume+otherPlanet.volume)*absorbtionSpeed;
    volume += deltaVolume;
    otherPlanet.volume -= deltaVolume;
  }

  //update rad with new volume
  void updateRadius() {
    rad = pow((volume * 3 / 4 / 3.14), 1.0 / 3);
    if (rad > 0) {
      mesh.reset();
      addSphere(mesh, rad);
      mesh.generateNormals();
    }
  }
};

struct mePlanet : Planet {
  mePlanet() : Planet() {
    velocity.zero();
    position = (Vec3f(0, 0, -200)).normalize(placeholderSize);
    speed = (0.01);
    acceleration = Vec3f(-0.02, -0.02, -0.02);
    rad = sphereRadius;
    volume = 3.14 * 4 / 3 * (rad) * (rad) * (rad);
    addSphere(mesh, rad);
    mesh.generateNormals();

    c = HSV(200.0 / 360.0f, 1, 1);
  }

  //used for moving
  void update(Planet my) {
    updateRadius();
    Quatf q;
    q.fromAxisAngle(speed, velocity);
    speed *= 0.97;
    q.normalize();
    position = q.rotate(position);
  }

  void clicked(){
    volume -= 50;
    updateRadius();
  }

};

struct enPlanet : Planet {
  enPlanet() : Planet() {
    velocity = r();
    position = (r() * placeholderSize).normalize(placeholderSize);
    speed = rnd::uniform(0.006);
    rad = (rnd::uniformS() * 5) + sphereRadius;
    rad += 2;
    volume = 3.14 * 4 / 3 * (rad) * (rad) * (rad);
    addSphere(mesh, rad);
    mesh.generateNormals();
  }

  void update(Planet my) {
    updateRadius();
    Quatf q;
    q.fromAxisAngle(speed, velocity);
    q.normalize();
    position = q.rotate(position);
    updateColor(my);
  }

};

