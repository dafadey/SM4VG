#pragma once
#include <iostream>
#include <fstream>
#include <cmath>
#include <limits>
#include <map>
#include <vector>

#ifdef MESHING_SRC
#define EXT
#else
#define EXT extern
#endif

//this is a 2d algorithm to get faces with holes from bitmap patterns

namespace BitmapMesher {
  //global variables:
  EXT int side; // length of rectangular bitmap along each side

  EXT std::vector<int> markers; // used for front advance algorithm that eats neighboring pixels of the same color(material)
  EXT std::vector<int> vedge_markers; // used to mark used VERTICAL (v) edges for makeHoles algorithm
  EXT std::vector<int> hedge_markers; // used to mark used HORIZONTAL (h) edges for makeHoles algorithm

  EXT bool has_holes;

  struct vertex {
    vertex(int x0, int y0) : x(x0), y(y0) {}
    int x,y;
    
    int id() const {
      return x + y * (side+1);
    }
  };
  
  vertex operator+(const vertex& v1, const vertex& v2);
  vertex operator-(const vertex& v1, const vertex& v2);
  int abs(const vertex& v1);
  
  bool operator==(const vertex& v1, const vertex& v2);

  int mid(int x, int y);

  //vertex id, note that number of veritices is side+1 by side+1
  int vid(int x, int y);

  //backward conversion for vertex from id to coords
  int getx(int vid);
  int gety(int vid);

  struct edge {
    edge(int x0, int y0, int x1, int y1, int m1_, int m2_) : v1(x0, y0), v2(x1, y1), m1(m1_), m2(m2_) {}
    vertex v1,v2;
    int m1,m2;
  };

  //this routine creates merged faces instead of edges with leght=1
  //with O(side*side) complexity ~O(n^2)
  //edges are collected in vector
  std::vector<edge> get_edges(const std::vector<int>& mats);

  //this is needed for mapping from vertes to adjacent faces
  //since we are working with shared(between faces/pixels) edges only 4 edges can contact a vertex
  typedef std::array<int, 5> sharednode; // edge id, edge id, edge id, edge id, count

  //loop has no holes. simplest structure for contour
  typedef std::vector<edge> loop;

  //face with material and holes
  struct face : public loop {
    face(int mat_) : mat(mat_) {}
    std::vector<loop> holes;
    int mat;
    int area() const; //holes have negative area. this is used to distiguish between holes and outer contours of faces
  };

  std::vector<vertex> loopToVerts(const loop& f);

  //creates side+1 by side+1 mapping consisting of sharednode to map from vertex to edges
  //complexity is O((side+1)*(side+1)) ~O(n^2) because mapping have to be reset prior to any modification
  //this complexity can be improved by using non fixed id-s counted from any arbitratu number, when maximum number reaches intmax is should be reset but it is very rare case
  std::vector<sharednode> get_shared_nodes(const std::vector<edge>& edges);

  //given some shared edge and target mateterial the routine creates loop (with either positive or negative(hole) area).
  //during this process nodes are modified for each node edge_id of passed edge is set to -1.
  //complexity is O(n_edges_per_face) ~O(n)
  void getLoop(loop* cur_face, int eid, int cur_mat, const std::vector<int> mats, std::vector<edge>& edges, const std::vector<sharednode>& nodes, int winding=1);

  //create holes for faces. it is a very complex (theoretically O(n^2) per face) but is advances front of outer pixels for face until it reaches holes.
  //vedge_markers and hedge_markers are used for that. for every face this markers are reset.
  //after routine reaches holes (i.e. fill nodes map again with hole edges) it calls getLoop in the same manner as it was done before for all loops.
  //NOTE: this routine should be called only after all loops were created and nodes mapping become empty. after each call of this routine nodes mapping become empty again (after getLoop).
  //this routine should be called only when holes exist
  void makeHoles(face* cur_face, const std::vector<int> mats, std::vector<edge>& edges, const std::vector<sharednode>& nodes);

  //uses the above routines in quite straightforward way to make faces. holes are not done yet.
  std::vector<face> get_faces(const std::vector<int> mats, std::vector<edge>& edges, const std::vector<sharednode>& nodes, int winding=1);
  
  //uses all the above algorithms to make faces with holes
  std::vector<face> make(std::vector<int> mats, bool dump=false);

}

std::ostream& operator<<(std::ostream& os, const BitmapMesher::vertex& v);
