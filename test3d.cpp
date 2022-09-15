#include "png_reader.h"
#include "makeSurface.h"

#include "geo.h" // preicates for circles and boxes to fill voxelmap

#include "compress_util.h"

int main(int argc, char* argv[])
{

  if(argc!=2) {
    std::cout << "please specify input filename\n";
    return -1;
  }
  
  int nx,ny,nz;
  std::vector<int> box=compress_read(argv[1],&nx,&ny,&nz);
  std::cout << "read voxelated geometry of size " << nx << 'x' << ny << 'x' << nz << "\n";


  surfaceMesh sm = makeSurface(box.data(), nx, ny, nz);
  
  std::cout << "saving raw format\n";
  sm.save("surf.dat");

  std::cout << "saving Blender format\n";
  sm.saveBlender("surf_Blender.dat");

  #ifdef CHECK
  //testing (expecting 0 gaps):
  std::cout << "found " << sm.countGaps() << " gaps\n";

  surfaceMesh sms = makeSurfaceSimple(box.data(), nx, ny, nz);
  
  sms.saveBlender("surf_simple.dat");

  std::cout << "Euler's formula check (V + F = E + 2)\n";
  for (auto& it : sm.bodiesCollection) {
    int V = 0;
    int E = 0;
    int F = 0;
    sm.Euler(it.second, V, E, F);
    std::cout << it.first << " V(" << V << ") + F(" << F << ") =(" << V + F << ") " << (V+F == E+2 ? "==" : "=/=") << " (" << E + 2 << ") E(" << E << ") + 2\n";
  }
  #endif
  return 0;
}
