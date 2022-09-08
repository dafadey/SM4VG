#include "png_reader.h"
#include "voxelated_meshing.h"
#include <array>

// 2d test for pair of bitmap representing interface

int main(int argc, char* argv[])
{
  image* imgA = read_png_file("sampledata/iA.png");
  image* imgB = read_png_file("sampledata/iB.png");
  
  std::cout << "img w=" << imgA->w << ", img h=" << imgA->h << '\n';
  
  if(!(imgA->w == imgA->h && imgB->w == imgB->h && imgB->w == imgA->h)) {
    std::cerr << "ERROR: image sizes do not match\n";
    return -1;
  }
  
  BitmapMesher::side = imgA->w;
  
  std::vector<int> mats(BitmapMesher::side * BitmapMesher::side);
  std::map<color, int> matmap;
  int currentmat=-1;
  for(int i=0; i<BitmapMesher::side; i++) {
    for(int j=0; j<BitmapMesher::side; j++) {
      const color& cA = imgA->get(j,i);
      if(matmap.find(cA) == matmap.end())
        matmap[cA] = ++currentmat;
      const color& cB = imgB->get(j,i);
      if(matmap.find(cB) == matmap.end())
        matmap[cB] = ++currentmat;
      int m1 = matmap[cA];
      int m2 = matmap[cB];
      //the trick: packing interface into one bitmap
      //since materials are marked from 0 so we have to add 1 to material id.
      //NOTE: aras where top and bottom materials are the same cannot be marked as void at this stage (see explaination in interface.pdf)
      mats[BitmapMesher::mid(j,i)] = (m1+1) + ((m2+1) << 8);
      if(mats[BitmapMesher::mid(j,i)]==-1)
        std::cout << "x:x";
      else {
        if((mats[BitmapMesher::mid(j,i)] & 0xff) - 1 == -1)
          std::cout << 'x';
        else
          std::cout << m1;
        std::cout << ':';
        if((mats[BitmapMesher::mid(j,i)] >> 8) - 1==-1)
          std::cout << 'x';
        else
          std::cout << m2;
      }
      std::cout << ' ';
    }
    std::cout << '\n';
  }
  std::cout << '\n';
  
  //now using exaclty the same fucntionality as we used for simple bitmap we can get faces
  auto faces = BitmapMesher::make(mats, true);
  
  for(auto& f : faces)
    std::cout << '\t' << f.mat << ' ' << f.area() << " holes count: " << f.holes.size() << '\n';
  
  std::cout << "all done\n";

  return 0;
}
