#include <iostream>
#include <fstream>
#include <limits>
#include <vector>

void compress_write(int* box, int nx, int ny, int nz, const char* filename) {
  std::ofstream of(filename, std::ios_base::binary);
  of.write((const char*)&nx, sizeof(int));
  of.write((const char*)&ny, sizeof(int));
  of.write((const char*)&nz, sizeof(int));
  uint count=1;
  int value = box[0];
  int item=0;
  for(size_t i=1; i<nx*ny*nz; i++) {
    if(box[i] == box[i-1] && count != std::numeric_limits<uint>::max())
      count++;
    else {
      of.write((const char*)&value, sizeof(int));
      of.write((const char*)&count, sizeof(uint));
      count=1;
      value = box[i];
    }
  }
  of.write((const char*)&value, sizeof(int));
  of.write((const char*)&count, sizeof(uint));
  of.close();
}

std::vector<int> compress_read(const char* filename, int* nx, int* ny, int* nz) {
  std::ifstream inf(filename, std::ios_base::binary);
  inf.read((char*)nx,sizeof(int));
  inf.read((char*)ny,sizeof(int));
  inf.read((char*)nz,sizeof(int));
  const size_t sz = *nx * *ny * *nz;
  std::vector<int> output(sz);
  uint count=0;
  int value=0;
  size_t pos=0;
  int item=0;
  while(true) {
    if(inf.read((char*)&value, sizeof(int)).eof())
      break;
    inf.read((char*)&count, sizeof(uint));
    for(uint i=0;i<count;i++,pos++) {
      if(pos<sz)
        output[pos] = value;
    }
  }
  if(pos!=sz)
    std::cerr << "compress data read failure pos=" << pos << ", sz=" << sz << "\n";
  inf.close();
  return output;
}
