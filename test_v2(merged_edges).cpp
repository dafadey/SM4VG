#include "png_reader.h"
#include "voxelated_meshing.h"

//2d test for simple bitmap but with merged edges

int main(int argc, char* argv[])
{
  image* img = read_png_file("sampledata/example.png");
  
  std::cout << "img w=" << img->w << ", img h=" << img->h << '\n';
  
  if(img->w != img->h) {
    std::cerr << "ERROR: img->w != img->h\n";
    return -1;
  }
  
  BitmapMesher::side = img->w;
  
  std::vector<int> mats(BitmapMesher::side * BitmapMesher::side);
  std::map<color, int> matmap;
  int currentmat=-1;
  for(int i=0; i<BitmapMesher::side; i++) {
    for(int j=0; j<BitmapMesher::side; j++) {
      const color& c = img->get(j,i);
      if(matmap.find(c) == matmap.end())
        matmap[c] = ++currentmat;
      mats[BitmapMesher::mid(j,i)] = matmap[c];
      std::cout << mats[BitmapMesher::mid(j,i)] << ' '; 
    }
    std::cout << '\n';
  }
  std::cout << '\n';
  
  auto faces = BitmapMesher::make(mats, true);
  
  for(auto& f : faces)
    std::cout << '\t' << f.mat << ' ' << f.area() << " holes count: " << f.holes.size() << '\n';

  std::cout << "all done\n";

  return 0;
}
