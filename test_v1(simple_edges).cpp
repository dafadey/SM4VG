#include "png_reader.h"
#include <array>

//this is a 2d algorithm to get faces with holes from bitmap patterns

//global variables:
int side; // length of rectangular bitmap along each side
std::vector<int> markers; // used for front advance algorithm that eats neighboring pixels of the same color(material)
std::vector<int> vedge_markers; // used to mark used VERTICAL (v) edges for makeHoles algorithm
std::vector<int> hedge_markers; // used to mark used HORIZONTAL (h) edges for makeHoles algorithm

struct vertex {
  vertex(int x0, int y0) : x(x0), y(y0) {}
  int x,y;
  
  int id() const {
    return x + y * (side+1);
  }
};

bool operator==(const vertex& v1, const vertex& v2) {
  return v1.x == v2.x && v1.y == v2.y;
}

std::ostream& operator<<(std::ostream& os, const vertex& v) {
  return os << '(' << v.x << ", " << v.y << ')';
}

int mid(int x, int y) {
    return x + y * side;
}

//vertex id, note that number of veritices is side+1 by side+1
int vid(int x, int y) {
    return x + y * (side+1);
}

//backward conversion for vertex from id to coords
int getx(int vid) {
  return vid % (side+1);
}

int gety(int vid) {
  return vid / (side+1);
}

struct edge {
  edge(int x0, int y0, int x1, int y1, int m1_, int m2_) : v1(x0, y0), v2(x1, y1), m1(m1_), m2(m2_) {}
  vertex v1,v2;
  int m1,m2;
};

//simple algo to get all edges with O(side*side) complexity ~O(n^2)
//edges are collected in vector
std::vector<edge> get_edges(const std::vector<int>& mats) {
  std::vector<edge> edges;
  for(int i=0;i<side;i++) {
    edges.emplace_back(edge(0,i,0,i+1,mats[mid(0,i)],-1));
    for(int j=1;j<side;j++) {
      int m1 = mats[mid(j - 1,i)];
      int m2 = mats[mid(j,i)];
      if(m1 != m2)
        edges.emplace_back(edge(j,i,j,i+1,m1,m2));
    }
    edges.emplace_back(edge(side,i,side,i+1, mats[mid(side-1,i)], -1));
  }
  
  for(int j=0;j<side;j++) {
    edges.emplace_back(edge(j,0,j+1,0,mats[mid(j,0)],-1));
    for(int i=1;i<side;i++) {
      int m1 = mats[mid(j,i - 1)];
      int m2 = mats[mid(j,i)];
      if(m1 != m2)
        edges.emplace_back(edge(j,i,j+1,i,m1,m2));
    }
    edges.emplace_back(edge(j,side,j+1,side,mats[mid(j,side-1)],-1));
  }
  
  return edges;
}

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
  int area() const;
};

//holes have negative area. this is used to distiguish between holes and outer contours of faces
int face::area() const {
  int xp = 0;
  int yp = 0;
  int area = 0;
  for(int eid = 0; eid < size() + 1; eid++) {
    const auto& e = (*this)[eid % size()];
    const auto& e_n = (*this)[(eid + 1) % size()];
    int nid = (e.v1.id() == e_n.v1.id() || e.v1.id() == e_n.v2.id()) ? e.v1.id() : e.v2.id();
    int x = getx(nid); 
    int y = gety(nid); 
    if(eid > 0)
      area += (x-xp) * y;
    xp = x;
    yp = y;
  }
  return area;
}

//creates side+1 by side+1 mapping consisting of sharednode to map from vertex to edges
//complexity is O((side+1)*(side+1)) ~O(n^2) because mapping have to be reset prior to any modification
//this complexity can be improved by using non fixed id-s counted from any arbitratu number, when maximum number reaches intmax is should be reset but it is very rare case
std::vector<sharednode> get_shared_nodes(const std::vector<edge>& edges) {
  std::vector<sharednode> snds;
  snds.resize((side+1) * (side+1), sharednode{-1,-1,-1,-1, 0});
  std::cout << "start\n" << std::flush;
  for(int e_id = 0; e_id < edges.size(); e_id++) {
    const auto& e = edges[e_id];
    const int id1 = e.v1.id();
    if(id1 >= snds.size())
      std::cerr << "error: id1=" << id1 << '\n';
    snds[id1][snds[id1][4]++] = e_id;
    const int id2 = e.v2.id();
    if(id2 >= snds.size())
      std::cerr << "error: id2=" << id2 << '\n';
    snds[id2][snds[id2][4]++] = e_id;
  }
  std::cout << "end\n" << std::flush;
  return snds;
}

//given some shared edge and target mateterial the routine creates loop (with either positive or negative(hole) area).
//during this process nodes are modified for each node edge_id of passed edge is set to -1.
//complexity is O(n_edges_per_face) ~O(n)
void getLoop(loop* cur_face, int eid, int cur_mat, const std::vector<int> mats, std::vector<edge>& edges, const std::vector<sharednode>& nodes, int winding=1) {
  int prev_eid = eid;
  while(cur_mat >= 0) {
    auto& e = edges[eid];
    cur_face->emplace_back(e);
    e.m1 = e.m1 == cur_mat ? -1 : e.m1;
    e.m2 = e.m2 == cur_mat ? -1 : e.m2;
    
    bool upd = false;
    
    auto move_forward = [&](const int vid) {
      auto& snds = nodes[vid];
      
      for(int ceid=0; ceid < snds[4]; ceid++) {
        const int neweid = snds[ceid];
        if(neweid == eid)
          continue;

        const edge& newe = edges[neweid];

        if(snds[4] == 4) { // force to make a turn in quad node
          int qx = vid % (side + 1);
          int qy = vid / (side + 1);
          int dxe = e.v1.x-qx + e.v2.x-qx;
          int dye = e.v1.y-qy + e.v2.y-qy;
          int dxnewe = newe.v1.x-qx + newe.v2.x-qx;
          int dynewe = newe.v1.y-qy + newe.v2.y-qy;
          if(mats[mid(qx, qy)] == cur_mat) {
            if((dxe + dye) * (dxnewe+dynewe) < 0)
              continue;
          } else if(mats[mid(qx - 1, qy)] == cur_mat) {
            if((dxe + dye) * (dxnewe+dynewe) > 0 && dxe*dynewe-dye*dxnewe != 0)
              continue;
          }
        }

        if(newe.m1 == cur_mat || newe.m2 == cur_mat) {
          if(prev_eid != neweid) {
            prev_eid = eid;
            eid = neweid;
            upd = true;
          }
        }
        if(upd)
          break;
      }
    };
    
    if(winding == 0) {
      move_forward(e.v1.id());
      if(!upd)
        move_forward(e.v2.id());
    } else {
      const auto& v1 = e.v1;
      const auto& v2 = e.v2;
      if(v2.x == v1.x) {
        if((v2.y > v1.y) == ((v2.x < side && mats[v2.x + std::min(v1.y, v2.y) * side] == cur_mat) || (v2.x == side && mats[v2.x - 1 + std::min(v1.y, v2.y) * side] != cur_mat)))
          move_forward(winding > 0 ? v2.id() : v1.id());
        else
          move_forward(winding > 0 ? v1.id() : v2.id());
      }
      if(v2.y == v1.y) {
        if((v2.x < v1.x) == ((v2.y < side && mats[std::min(v2.x, v1.x) + v2.y * side] == cur_mat) || (v2.y == side && mats[std::min(v2.x, v1.x) + (v2.y - 1) * side] != cur_mat)))
          move_forward(winding > 0 ? v2.id() : v1.id());
        else
          move_forward(winding > 0 ? v1.id() : v2.id());
      }
    }
    if(!upd)
      break;
  }
}

//create holes for faces. it is a very complex (theoretically O(n^2) per face) but is advances front of outer pixels for face until it reaches holes.
//vedge_markers and hedge_markers are used for that. for every face this markers are reset.
//after routine reaches holes (i.e. fill nodes map again with hole edges) it calls getLoop in the same manner as it was done before for all loops.
//NOTE: this routine should be called only after all loops were created and nodes mapping become empty. after each call of this routine nodes mapping become empty again (after getLoop).
//this routine should be called only when holes exist
void makeHoles(face* cur_face, const std::vector<int> mats, std::vector<edge>& edges, const std::vector<sharednode>& nodes) {
  
  std::vector<edge> holes;
  
  int cur_mat = cur_face->mat;
  if(markers.size() < side * side)
    markers.resize(side * side);
  for(int i=0;i<side*side; i++)
    markers[i] = 0;
  
  if(vedge_markers.size() < (side+1) * side)
    vedge_markers.resize((side+1) * side);
  for(int i=0;i<(side+1) * side; i++)
    vedge_markers[i] = 0;

  if(hedge_markers.size() < (side+1) * side)
    hedge_markers.resize((side+1) * side);
  for(int i=0;i<(side+1) * side; i++)
    hedge_markers[i] = 0;
  
  std::vector<std::array<int, 2>> front;
  
  //mark pixels
  for(const auto& e : *cur_face) {
    int minx = std::min(e.v1.x, e.v2.x);
    int miny = std::min(e.v1.y, e.v2.y);
    const int addr = minx+miny*side;
    if(minx < side && miny < side && mats[addr] == cur_mat) {
      front.emplace_back(std::array<int,2>{minx, miny}); 
      markers[addr] = 1;
    }
    if(e.v1.x == e.v2.x) { // v edge
      vedge_markers[miny * (side+1) + minx] = 1;
      if(minx > 0 && mats[addr-1] == cur_mat) {
        front.emplace_back(std::array<int,2>{minx-1, miny});
        markers[addr-1] = 1; 
      }
    } else { // h edge
      hedge_markers[miny * side + minx] = 1;
      if(miny > 0 && mats[addr-side] == cur_mat) {
        front.emplace_back(std::array<int,2>{minx, miny-1}); 
        markers[addr-side] = 1; 
      }
    }
  }
  
  //advance pixels while possible
  std::array<std::array<int,2>,4> cross = {{{{1,0}}, {{-1,0}}, {{0,1}}, {{0,-1}}}};
  std::vector<std::array<int,2>> newfront;
  while(true) {
    for(const std::array<int,2>& pixel : front) {
      const int x = pixel[0];
      const int y = pixel[1];
      for(const auto& dir : cross) {
        int xn = x+dir[0];
        int yn = y+dir[1];
        if(xn<0 || xn>=side || yn<0 || yn >= side)
          continue;
        int addrn = mid(xn, yn);
        if(markers[addrn] == 0) {
          if(mats[addrn] == cur_mat) {
            newfront.emplace_back(std::array<int,2>{xn, yn}); 
            markers[addrn] = 1;
          } else {// reached hole boundary
            if(dir[1] == 0) { //horiontal dir -> vetical edge
              int edge_x = dir[0] > 0 ? xn : x;
              int edge_y = y;
              if(vedge_markers[edge_y * (side+1) + edge_x] == 0)
                holes.emplace_back(edge(edge_x, edge_y, edge_x, edge_y+1,-1,cur_mat));
            } else { //vertical dir -> horizontal edge {
              int edge_x = x;
              int edge_y = dir[1] > 0 ? yn : y;
              if(hedge_markers[edge_y * side + edge_x] == 0)
                holes.emplace_back(edge(edge_x, edge_y, edge_x+1, edge_y,-1,cur_mat));
            }
          }
        }
      }
    }
    if(newfront.size()==0)
      break;
    else
      front = newfront;
    newfront.clear();
  }
  
  auto getEdgeIdByNodes = [&](const sharednode& n1, const sharednode& n2) -> int {
    for(int i=0; i<n1[4]; i++) {
      for(int j=0; j<n2[4]; j++) {
        if(n1[i] == n2[j])
          return n1[i];
      }
    }
    return -1;
  };
  
  for(auto e : holes) {
    const int edgeId = getEdgeIdByNodes(nodes[vid(e.v1.x, e.v1.y)], nodes[vid(e.v2.x, e.v2.y)]);
    if(edgeId != -1)
      edges[edgeId].m1 = cur_mat;
  }
  
  //extract hole edges from last front  
  auto getEdgeId = [&]() -> int {
    for(int eid=0; eid < edges.size(); eid++)
    {
      auto& e = edges[eid];
      if(e.m1 != -1)
        return eid;
    }
    return -1;
  };
  
  if(holes.size() == 0)
    return;
  
  while(true) {
    int eid = getEdgeId();
    if(eid == -1)
      break;
      
    int cur_mat = edges[eid].m1;
    
    cur_face->holes.emplace_back(loop());
    loop* cur_hole = &cur_face->holes.back();
    
    getLoop(cur_hole, eid, cur_mat, mats, edges, nodes);
  }
}

// uses the above routines in quite straightforward way to make faces. holes are not done yet.
std::vector<face> get_faces(int mats_count, const std::vector<int> mats, std::vector<edge>& edges, const std::vector<sharednode>& nodes, int winding=1) {
  std::vector<face> faces;
  
  auto getEdgeId = [&]() -> int {
    for(int eid=0; eid < edges.size(); eid++)
    {
      auto& e = edges[eid];
      if(e.m1 != -1 || e.m2 != -1)
        return eid;
    }
    return -1;
  };
  
  while(true) {
    int eid = getEdgeId();
    
    if(eid == -1)
      break;
      
    int cur_mat = edges[eid].m1 == -1 ? edges[eid].m2 : edges[eid].m1;
    
    faces.emplace_back(face(cur_mat));
    face* cur_face = &faces.back();
    
    getLoop(cur_face, eid, cur_mat, mats, edges, nodes, winding);
    
    if(cur_face->area()<0)
      faces.pop_back();
  }
  return faces; 
}

//face consists of edges byt sometimes it needs to be represented as a vector of vertices
std::vector<vertex> loopToVerts(const loop& f){
  std::vector<vertex> verts;
  if(f.size() < 4)
    return verts;
  for(int i=0; i<f.size(); i++)
    verts.emplace_back(f[i].v1 == f[(i+1) % f.size()].v1 || f[i].v1 == f[(i+1) % f.size()].v2 ? f[i].v1 : f[i].v2);
  return verts;
}

int main(int argc, char* argv[])
{
  //read pattern from png file:
  image* img = read_png_file("sampledata/example.png");
  
  std::cout << "img w=" << img->w << ", img h=" << img->h << '\n';
  
  if(img->w != img->h) {
    std::cerr << "ERROR: img->w != img->h\n";
    return -1;
  }
  
  side=img->w;
  
  std::vector<int> mats(side * side);
  std::map<color, int> matmap;
  int currentmat=-1;
  for(int i=0; i<side; i++) {
    for(int j=0; j<side; j++) {
      const color& c = img->get(j,i);
      //std::cout << c;
      if(matmap.find(c) == matmap.end())
        matmap[c] = ++currentmat;
      mats[mid(j,i)] = matmap[c];
      std::cout << mats[mid(j,i)] << ' '; 
      //std::cout << currentmat << ':' << matmap[c] << ' ';
    }
    std::cout << '\n';
  }
  std::cout << '\n';
  
  //create faces with holes:
  
  auto edges = get_edges(mats);

  std::cout << "number of edges:" << edges.size() << '\n' << std::flush;
  
  auto nodes = get_shared_nodes(edges);

  auto faces = get_faces(matmap.size(), mats, edges, nodes);
  
  // make holes will reuse edges and temproray apply material to holes edges.
  // those materials will be immediatelly wiped out by getLoop.
  for(auto& f : faces)
    makeHoles(&f, mats, edges, nodes);
  
  for(auto& f : faces)
    std::cout << '\t' << f.mat << ' ' << f.area() << " holes count: " << f.holes.size() << '\n';

  {
    std::ofstream of("dump.dat");
    of << side << '\n';

    for(auto mat : mats)
      of << mat << '\n';

    of << edges.size() << '\n';
    for(auto e : edges)
      of << e.v1.x << ' ' << e.v1.y << ' ' << e.v2.x << ' ' << e.v2.y << ' ' << e.m1 << ' ' << e.m2 << '\n';
    
    for(auto n : nodes)
      of << n[0] << ' ' << n[1] << ' ' << n[2] << ' ' << n[3] << ' ' << n[4] << '\n';
    
    of.close();
  }
  
  //dump holes
  {
    std::ofstream of("holes.dat");
    of << side << '\n';

    for(auto mat : mats)
      of << mat << '\n';

    int hole_edges_size=0;
    for(auto f : faces) {
      for(auto h : f.holes)
        hole_edges_size += h.size();
    }
    
    of << hole_edges_size << '\n';
    for(auto f : faces) {
      for(auto h : f.holes) {
        for(auto e : h)
          of << e.v1.x << ' ' << e.v1.y << ' ' << e.v2.x << ' ' << e.v2.y << ' ' << e.m1 << ' ' << e.m2 << '\n';
      }
    }
    
    of.close();
  }

  //dump faces
  {
    std::ofstream of("faces.dat");
    of << faces.size() << '\n';
    for(auto f : faces) {
      auto verts = loopToVerts(f);
      of << f.mat << ' ' << verts.size() << ' ' << f.holes.size() << '\n';
      for(auto& v : verts)
        of << v.x << ' ' << v.y << '\n';
      for(auto& hl : f.holes) {
        auto hverts = loopToVerts(hl);
        of << hverts.size() << '\n';
        for(auto& v : hverts)
          of << v.x << ' ' << v.y << '\n';
      }
    }
    of.close();
  }
  
  std::cout << "all done\n";

  return 0;
}
