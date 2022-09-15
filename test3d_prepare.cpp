#include <string>

#include "png_reader.h"
#include "makeSurface.h"

#include "geo.h" // preicates for circles and boxes to fill voxelmap

#include "compress_util.h"

#ifndef TEST3
#define RANDOM_TEST
#endif

int main(int argc, char* argv[])
{
  auto urand=[]() {
    return double(rand())/double(RAND_MAX);
  };
  //create testing scene with geometry predicates
  Scene scene;
  #ifdef TEST3
  Sphere s1(v3d{0.57,0.53,0.65}, 0.5);
  s1.layer = 3;
  s1.mat = 0;
  scene.emplace_back(&s1);
  Sphere s2(v3d{0.07,0.2,0.08}, 0.47);
  s2.layer = 1;
  s2.mat = 2;
  scene.emplace_back(&s2);
  Box b1(v3d{0.3,0.3,0.3}, v3d{1.,1.,1.});
  b1.layer = 2;
  b1.mat = 3;
  scene.emplace_back(&b1);
  #endif
  #ifdef RANDOM_TEST
  for(int i=0;i<33;i++) {
    if(urand()>0.5) {
      double r = urand();
      Sphere* s = new Sphere(v3d{urand(),urand(),urand()}, r);
      s->layer = int(7./r);
      s->mat = rand() % 3;
      scene.emplace_back(s);
    } else{
      double dx = urand();
      double dy = urand();
      double dz = urand();
      double x0 = urand();
      double y0 = urand();
      double z0 = urand();
      Box* b = new Box(v3d{x0, y0, z0}, v3d{x0+dx, y0+dy, z0+dz});
      b->layer = int(7./std::sqrt(dx*dx+dy*dy+dz*dz));
      b->mat = rand() % 3;
      scene.emplace_back(b);
    }
  }
  #endif

  std::cout << "scene:\n" << scene << '\n';

  //voxelate geometry
  int sz = argc > 1 ? atoi(argv[1]) : 16;
  std::vector<int> box(sz * sz * sz, -1);
  
  for(int k=0;k<sz;k++) {
    for(int j=0;j<sz;j++) {
      for(int i=0;i<sz;i++)
        box[i+j*sz+k*sz*sz] = scene.getMat(v3d{(double(i)+.5)/double(sz), (double(j)+.5)/double(sz), (double(k)+.5)/double(sz)});
    }
  }
  std::cout << "created voxelated geometry\n";

  std::string filename = "geometry.cdat";
  if(argc>2)
    filename = std::string(argv[2]);

  compress_write(box.data(), sz, sz, sz, filename.c_str());
  std::cout << "saved geometry\n";
  return 0;
}
