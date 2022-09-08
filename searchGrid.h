#pragma once
#include <vector>
#include <cmath>

template <typename T>
struct idGrid : public std::vector<int> {
  
  void expand(int x, int y, int z) {
    if(x >= nx || y >= ny || z >= nz) {
      int newnx = std::max(nx, int(std::ceil(double(x+1)/double(quanta)))*quanta);
      int newny = std::max(ny, int(std::ceil(double(y+1)/double(quanta)))*quanta);
      int newnz = std::max(nz, int(std::ceil(double(z+1)/double(quanta)))*quanta);
      std::vector<int> newContainer(newnx*newny*newnz, -1);
      for(int k=0; k<nz; k++) {
        for(int j=0; j<ny; j++) {
          for(int i=0; i<nx; i++)
            newContainer[i+j*newnx+k*newnx*newny] = (*this)[i+j*nx+k*nx*ny];
        }
      }
      this->resize(newnx*newny*newnz, -1);
      for(int addr=0; addr < newnx*newny*newnz; addr++)
        (*this)[addr] = newContainer[addr];
      nx = newnx;
      ny = newny;
      nz = newnz;
    }
  }

  void addId(const T& t, int id) {
    expand(t[0],t[1],t[2]);
    (*this)[t[0] + t[1] * nx + t[2] * nx * ny] = id;
  }

  int getId(const T& t) {
    if(t[0] < nx && t[1] < ny && t[2] < nz) 
      return (*this)[t[0] + t[1] * nx + t[2] * nx * ny];
    return -1;
  }  

  void reset() {
    for(int i=0; i<size(); i++)
      (*this)[i] = -1;
  }

  int nx{};
  int ny{};
  int nz{};
  const int quanta = 16;
};
