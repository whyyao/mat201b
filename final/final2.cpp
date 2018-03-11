#include "allocore/io/al_App.hpp"
#include "Cuttlebone/Cuttlebone.hpp"
#include <cmath> 
#include "planet.cpp"
using namespace al;
using namespace std;


Mesh sphere;  

// helper to make random

string fullPathOrDie(string fileName, string whereToLook = ".") {
  SearchPaths searchPaths;
  searchPaths.addSearchPath(whereToLook);
  string filePath = searchPaths.find(fileName).filepath();
  assert(filePath != "");
  return filePath;
}


struct MyApp : App {

  // State* state = new State;
  //cuttlebone::Maker<State> maker;

  Material material;
  Light light;

  vector<Planet> planets;
  Planet special;
  Planet myPlanet;
  bool simulate = true;
  Mesh bgMesh;
  Texture bgTexture;

  MyApp(){
    addSphereWithTexcoords(bgMesh);
 // load image into texture print out error and exit if failure
    Image image;
    if (image.load(fullPathOrDie("cell.jpg"))) {
      cout << "Read image from "<< endl;
    } else {
      cout << "Failed to read image from " <<  "!!!" << endl;
      exit(-1);
    }

    bgTexture.allocate(image.array());
   
    light.pos(0, 0, 0);         
    nav().pos(0, 0, 70);        
    lens().far(400);             

    planets.resize(particleCount);
    myPlanet.setMe();

    background(Color(0.07));
    initWindow();
    initAudio();
  }

  void onAnimate(double dt) {

      if (!simulate)
      // skip the rest of this function
      return;

    // for (auto& b: planets) {
    //   b.update(myPlanet);
    // }
    // myPlanet.update(myPlanet);

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

    

    for(int i = 0; i<planets.size(); i++){
      if(myPlanet.ifCollide(planets[i])){
         if(myPlanet.volume>planets[i].volume){
              myPlanet.absorb(planets[i]);
              deletingPlanet.push_back(i);
            }else{
              simulate = false;
            }
      }
    }

    for(auto index: deletingPlanet){
      planets.erase(planets.begin()+index);
    }

    static double t = 0;
    static int which = 0;
    t += dt;
    if (t > 3) {
      t -= 3;
      which = rnd::uniform(particleCount);
    }


    Quatf p(planets[which].position), q(special.position);
    p.normalize();
    q.normalize();
    q.slerpTo(p, 0.03);
    special.position.x = q.x;
    special.position.y = q.y;
    special.position.z = q.z;
    special.position.normalize(23);

    for (auto& p : planets) p.update(myPlanet);
    myPlanet.update(myPlanet);
    //maker.set(*state);
    // for (unsigned i = 0; i < boids.size(); i++)
    //    state->position[i] = boids[i].location;
  }

  void onDraw(Graphics& g) {
    material();
    light();
    //bgTexture.quad(g);

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
      //myPlanet.velocity.z += 1;
      myPlanet.velocity.x += 2;
      break;
    case 'l':
      //myPlanet.velocity.z -= 1;
      myPlanet.velocity.x -= 2;
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
