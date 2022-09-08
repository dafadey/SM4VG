#define MESHING_SRC
#include "voxelated_meshing.h"
#include <array>

namespace BitmapMesher {

int mid(int x, int y) {
    return x + y * side;
}

int vid(int x, int y) {
    return x + y * (side+1);
}

int getx(int vid) {
  return vid % (side+1);
}

int gety(int vid) {
  return vid / (side+1);
}

bool operator==(const vertex& v1, const vertex& v2) {
  return v1.x == v2.x && v1.y == v2.y;
}

std::vector<edge> get_edges(const std::vector<int>& mats) {
  
  std::vector<edge> edges;
  //std::cout << "get_edges: side=" << side << '\n';
  // make horizontal merged edges
  for(int i=0;i<side+1;i++) {
    int jstart{-1};
    int m1p{-1};
    int m2p{-1};
    int m1pa{-1};
    int m2pa{-1};
    for(int j=0; j<side; j++) {
      int m1 = i - 1 >= 0 ? mats[mid(j, i - 1)] : -1 /*NOTE: pixel from neighboring box should be here*/;
      int m2 = i < side ? mats[mid(j, i)] : -1 /*NOTE: pixel from neighboring box should be here*/;
      int m1a = i - 1 >= 0 ? ((mats[mid(j, i - 1)] & 0xff) == (mats[mid(j, i - 1)] >> 8) ? -1 /*set void for 'alternated' pixel since top and bottom are the same*/: mats[mid(j, i - 1)]): -1 /*NOTE: pixel/void from neighboring box should be here*/;
      int m2a = i < side ? ((mats[mid(j, i)] & 0xff) == (mats[mid(j, i)] >> 8) ? -1/*set void for 'alternated' pixel since top and bottom are the same*/ : mats[mid(j, i)]) : -1 /*NOTE: pixel/void from neighboring box should be here*/;
      
      /*
      //this is wrong, uncomment to check is test for gaps work
      m1=m1a;
      m2=m2a;
      */
      
      if(m1p != m1 || m2p != m2) { // stop if materials changed in alowed area OR in VOID* area. it is important to keep that nodes!
        if(jstart != -1) {
          edges.emplace_back(edge(jstart,i,j,i,m1pa,m2pa)); // horizontal
          jstart = -1;
        }
      }
      
      if(jstart == -1 && m1 != m2 && !(m1a==-1 && m2a==-1)) // start new edge only if material changed but we are not in void* area
        jstart = j;
      
      m1p = m1;
      m2p = m2;
      m1pa = m1a;
      m2pa = m2a;
    }
    if(jstart != -1)
      edges.emplace_back(edge(jstart,i,side,i,m1pa,m2pa));
  }

  // make vertical merged edges
  for(int j=0;j<side+1;j++) {
    int istart{-1};
    int m1p{-1};
    int m2p{-1};
    int m1pa{-1};
    int m2pa{-1};
    for(int i=0; i<side; i++) {
      int m1 = j - 1 >= 0 ? mats[mid(j - 1, i)] : -1/*NOTE: pixel from neighboring box should be here*/;
      int m2 = j < side ? mats[mid(j, i)] : -1/*NOTE: pixel from neighboring box should be here*/;

      int m1a = j - 1 >= 0 ? ((mats[mid(j - 1, i)] & 0xff) == (mats[mid(j - 1, i)] >> 8) ? -1/*set void for 'alternated' pixel since top and bottom are the same*/ : mats[mid(j - 1, i)]): -1/*NOTE: pixel/void from neighboring box should be here*/;
      int m2a = j < side ? ((mats[mid(j, i)] & 0xff) == (mats[mid(j, i)] >> 8) ? -1/*set void for 'alternated' pixel since top and bottom are the same*/ : mats[mid(j, i)]) : -1/*NOTE: pixel/void from neighboring box should be here*/;

      /*
      //this is wrong, uncomment to check is test for gaps work
      m1=m1a;
      m2=m2a;
      */
      
      if(m1p != m1 || m2p != m2) { // stop if materials changed in alowed area OR in VOID* area. it is important to keep that nodes!
        if(istart != -1) {
          edges.emplace_back(edge(j,istart,j,i,m1pa,m2pa)); // vertical
          istart = -1;
        }
      }
      
      if(istart == -1 && m1 != m2 && !(m1a==-1 && m2a==-1)) // start new edge only if material changed but we are not in void* area
        istart = i;
      
      m1p = m1;
      m2p = m2;
      m1pa = m1a;
      m2pa = m2a;
    }
    if(istart != -1)
      edges.emplace_back(edge(j,istart,j,side,m1pa,m2pa));
  }
  return edges;
}
//*void area - an area where nothing really exists or where we have same materials in top (mat & 0xff) and bottom (mat >> 8) layers

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

std::vector<sharednode> get_shared_nodes(const std::vector<edge>& edges) {
  std::vector<sharednode> snds;
  snds.resize((side+1) * (side+1), sharednode{-1,-1,-1,-1, 0});
  //std::cout << "start\n" << std::flush;
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
  //std::cout << "end\n" << std::flush;
  return snds;
}

void getLoop(loop* cur_face, int eid, int cur_mat, const std::vector<int> mats, std::vector<edge>& edges, const std::vector<sharednode>& nodes, int winding) {
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
  for(const auto& merged_edge : *cur_face) {
    
    int xp = std::min(merged_edge.v1.x, merged_edge.v2.x);
    int yp = std::min(merged_edge.v1.y, merged_edge.v2.y);
    for(int y=yp; y <= std::max(merged_edge.v1.y, merged_edge.v2.y); y++) {
      for(int x=xp; x <= std::max(merged_edge.v1.x, merged_edge.v2.x); x++) {
        if(x!=xp || y!=yp) {
          edge e(x,y,xp,yp,merged_edge.m1,merged_edge.m2);
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
        xp=x;
      }
      yp=y;
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
  
  for(auto e : holes) {
    const sharednode& n1 = nodes[vid(e.v1.x, e.v1.y)];
    const sharednode& n2 = nodes[vid(e.v2.x, e.v2.y)];
    for(int i=0; i<n1[4]; i++)
        edges[n1[i]].m1 = n1[i] != -1 ? cur_mat : edges[n1[i]].m1;
    for(int i=0; i<n2[4]; i++)
        edges[n2[i]].m1 = n2[i] != -1 ? cur_mat : edges[n2[i]].m1;
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

std::vector<face> get_faces(const std::vector<int> mats, std::vector<edge>& edges, const std::vector<sharednode>& nodes, int winding) {
  has_holes = false;
  
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
    
    if(cur_face->area()<0) {
      faces.pop_back();
      has_holes = true;
    }
  }
  return faces; 
}

std::vector<vertex> loopToVerts(const loop& f){
  std::vector<vertex> verts;
  if(f.size() < 4)
    return verts;
  for(int i=0; i<f.size(); i++)
    verts.emplace_back(f[i].v1 == f[(i+1) % f.size()].v1 || f[i].v1 == f[(i+1) % f.size()].v2 ? f[i].v1 : f[i].v2);
  return verts;
}

std::vector<loop> faceToLoops(const face& f) {
  std::vector<loop> result;
/*
  std::vector<std::vector<vertex>> loopsv;
  for(auto& h : holes)
    loopsv.emplace_back(loopToVerts(h));
  auto outerv = loopToVerts(f);
  
  for(auto& l1 : loopsv) {
    for(auto& l2 : loopsv) {
      if(l1==l2)
        continue;
      //find closest edges
      int d = 1000000;
      vertex v1,v2;
      for(auto& _v1 : l1) {
        for(auto& _v2 : l2) {
          if(abs(_v1-_v2) < d) {
            d = abs(_v1-_v2);
            v1 = _v1;
            v2 = _v2;
          }
        }
      }
    }
  }
*/
  return result;
}

std::vector<face> make(std::vector<int> mats, bool dump) {
  auto edges = get_edges(mats);

  //std::cout << "number of edges:" << edges.size() << '\n' << std::flush;
  
  auto nodes = get_shared_nodes(edges);

  std::vector<face> faces = get_faces(mats, edges, nodes);

  // make holes will reuse edges and temproray apply material to holes edges.
  // those materials will be immediatelly wiped out by getLoop.
  if(has_holes) {
    for(auto& f : faces)
      makeHoles(&f, mats, edges, nodes);
  }

  if(dump) {
    std::cout << "!!!!!!!!!!!!!!!!1\n";
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
  
  //dump holes
    std::ofstream ofh("holes.dat");
    ofh << side << '\n';

    for(auto mat : mats)
      ofh << mat << '\n';

    int hole_edges_size=0;
    for(auto f : faces) {
      for(auto h : f.holes)
        hole_edges_size += h.size();
    }
    ofh << hole_edges_size << '\n';
    for(auto f : faces) {
      for(auto h : f.holes) {
        for(auto e : h)
          ofh << e.v1.x << ' ' << e.v1.y << ' ' << e.v2.x << ' ' << e.v2.y << ' ' << e.m1 << ' ' << e.m2 << '\n';
      }
    }
    ofh.close();
    
  //dump faces
    std::ofstream off("faces.dat");
    off << faces.size() << '\n';
    for(auto f : faces) {
      auto verts = loopToVerts(f);
      off << f.mat << ' ' << verts.size() << ' ' << f.holes.size() << '\n';
      for(auto& v : verts)
        off << v.x << ' ' << v.y << '\n';
      for(auto& hl : f.holes) {
        auto hverts = loopToVerts(hl);
        off << hverts.size() << '\n';
        for(auto& v : hverts)
          off << v.x << ' ' << v.y << '\n';
      }
    }
    off.close();
    
  }

  return faces;
}

} // namespace BitmapMesher

std::ostream& operator<<(std::ostream& os, const BitmapMesher::vertex& v) {
  return os << '(' << v.x << ", " << v.y << ')';
}

BitmapMesher::vertex operator+(const BitmapMesher::vertex& v1, const BitmapMesher::vertex& v2) {
  return BitmapMesher::vertex(v1.x+v2.x,v1.y+v2.y);
}
BitmapMesher::vertex operator-(const BitmapMesher::vertex& v1, const BitmapMesher::vertex& v2) {
  return BitmapMesher::vertex(v1.x-v2.x,v1.y-v2.y);
}
int abs(const BitmapMesher::vertex& v1) {
  return std::abs(v1.x)+std::abs(v1.y);
}
