#include "allocore/io/al_App.hpp"
using namespace al;
using namespace std;

struct MyApp : App, osc::PacketHandler {
  MyApp() {
    initWindow();
    initAudio();
    // I listen on 60777
    oscRecv().open(60777, "", 0.016, Socket::UDP);
    oscRecv().handler(*this);
    oscRecv().start();
    // you better be listening on 60777
    oscSend().open(60777, "192.168.1.123", 0.016, Socket::UDP);
    addSphere(them);
  }
  Mesh them;
  Pose other;
  void onAnimate(double dt) {
    oscSend().send("/xyz", nav().pos().x, nav().pos().y, nav().pos().z);
    oscSend().send("/xyzw", nav().quat().x, nav().quat().y, nav().quat().z,
                   nav().quat().w);
  }
  void onMessage(osc::Message& m) {
    if (m.addressPattern() == "/xyz") {
      double x,y,z;
      m >> x;
      m >> y;
      m >> z;
      Vec3f o(x,y,z);
      other.pos(o);
    } else if (m.addressPattern() == "/xyzw") {
      double x,y,z,w;
      m >> x;
      m >> y;
      m >> z;
      m >> w;
      Quatf q(x,y,z,w);
      other.quat(q);
    } else
      m.print();
  }

  void onDraw(Graphics& g) {
    g.translate(other.pos());
    g.rotate(other.quat());
    g.draw(them);
  }
};

int main() { MyApp().start(); }