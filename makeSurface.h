#pragma once
#include <map>
#include <array>

#include "searchGrid.h" // template for faster search

//smple structures for mapping and adjacency
typedef std::array<int,3> vi3d;

vi3d operator+(const vi3d& v1, const vi3d& v2);

vi3d operator-(const vi3d& v1, const vi3d& v2);

struct n3d : public vi3d {
  n3d(int x, int y, int z);
  std::vector<int> edges; //edge ids contating this node
};

typedef std::array<int,2> e3d; // vertex id, vertex id

typedef std::vector<int> loop3d;

struct f3d : public loop3d { // vertex id, vertex id, vertex id ...
  f3d(const loop3d& in) : loop3d(in), holes() {}
  std::vector<loop3d> holes; // same thing: vector of loops consisting of vertex id-s
};

typedef std::vector<int> body; // face id-s starting from 1 (negative sign means opposite direction of normal vector)


struct surfaceMesh {
  std::vector<n3d> nodesCollection; // all shared nodes
  std::vector<e3d> edgesCollection; // all shared edges
  std::vector<f3d> facesCollection; // all shared faces
  //NOTE: all that stuff is needed since faces are refering edges which in turn are refering nodes 
  std::map<int, body> bodiesCollection; // all bodies
  idGrid<vi3d> idGrd;
  
  void save(const char*);
  void saveBlender(const char*);
  int countGaps(); // verification
  void Euler(const body& b, int& V, int& E, int& F);
};

surfaceMesh makeSurface(int* box, int sx, int sy, int sz);

surfaceMesh makeSurfaceSimple(int* box, int sx, int sy, int sz);
