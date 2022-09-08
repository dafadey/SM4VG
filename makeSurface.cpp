#include "makeSurface.h"
#include "voxelated_meshing.h"

#include <set>
#include <map>
#include <array>

vi3d operator+(const vi3d& v1, const vi3d& v2) {
  return vi3d{v1[0]+v2[0], v1[1]+v2[1], v1[2]+v2[2]};
}

vi3d operator-(const vi3d& v1, const vi3d& v2) {
  return vi3d{v1[0]-v2[0], v1[1]-v2[1], v1[2]-v2[2]};
}

n3d::n3d(int x, int y, int z) : vi3d{ x,y,z }, edges() {}


surfaceMesh makeSurface(int* box, int nx, int ny, int nz) {

  surfaceMesh sm;

  int sz = std::max(std::max(nx, ny), nz);
  //mesh layers:
  BitmapMesher::side = sz;
  
  std::vector<n3d>& nodesCollection = sm.nodesCollection;
  std::vector<e3d>& edgesCollection = sm.edgesCollection;
  std::vector<f3d>& facesCollection = sm.facesCollection;
  //NOTE: all that stuff is needed since faces are refering edges which in turn are refering nodes 
  std::map<int, body>& bodiesCollection = sm.bodiesCollection;
  
  idGrid<vi3d>& idGrd  = sm.idGrd;
  
  //pair of lambdas using mappings to get node and edge id-s
  auto getNodeId=[&](const vi3d& v)->int {
    auto& nodes = nodesCollection;
    auto& idgrid = idGrd;
    
    int id = idgrid.getId(v);
    if(id == -1) {
      id = nodes.size();
      idgrid.addId(v,id);
      nodes.emplace_back(n3d(v[0], v[1], v[2]));
    }
    return id;
  };
  
  auto getEdgeId=[&](int n1id, int n2id)->int {
    auto& nodes = nodesCollection;
    auto& edges = edgesCollection;
    n3d& n1 = nodes[n1id];
    n3d& n2 = nodes[n2id];
    for(auto& eid1 : n1.edges) {
      for(auto& eid2 : n2.edges) {
        if(eid1 == eid2)
          return eid1;
      }
    }
    e3d e{n1id, n2id};
    int eid = edges.size();
    edges.emplace_back(e);
    n1.edges.emplace_back(eid);
    n2.edges.emplace_back(eid);
    return eid;
  };
  
  std::vector<int> mats(sz * sz);
  // z(horizontal i.e. with normal vecror along z) layers:
  for (int addr = 0; addr < sz * sz; addr++)
    mats[addr] = -1;
  for(int i=0;i<nz+1;i++) {
    for (int yi = 0; yi < ny; yi++) {
      for (int xi = 0; xi < nx; xi++) {
        int addr3d = xi + yi * nx;
        int addr = xi + yi * sz;
        int m1 = i < nz ? box[i * nx * ny + addr3d] : -1;
        int m2 = i > 0 ? box[(i - 1) * nx * ny + addr3d] : -1;
        mats[addr] = (m1 + 1) + ((m2 + 1) << 8);
      }
    }
    
    auto faces2d = BitmapMesher::make(mats);
    for(auto& f : faces2d) {
      if(f.holes.size())
        std::cout << "holes count " << f.holes.size() << '\n';

      auto makeF3=[&](BitmapMesher::loop& f)->loop3d {
        loop3d f3;
        std::vector<int> edloop;
        for(auto& e : f) {
          int n1id = getNodeId(vi3d{e.v1.x, e.v1.y, i});
          int n2id = getNodeId(vi3d{e.v2.x, e.v2.y, i});
          int eid = getEdgeId(n1id, n2id);
          edloop.emplace_back(eid);
        }
        auto& edges = edgesCollection;
        for(int i=0; i<edloop.size(); i++) {
          e3d& e1 = edges[edloop[i]];
          e3d& e2 = edges[edloop[(i+1)%edloop.size()]];
          f3.emplace_back(e1[0] == e2[0] || e1[0] == e2[1] ? e1[0] : e1[1]);
        } 
        return f3;
      };
      
      f3d f3(makeF3(f));
    
      //holes
      for(auto& h : f.holes)
        f3.holes.emplace_back(makeF3(h));
      
      //put face to a collection now since in bodies collection we use indices startting from 1 not from 0
      facesCollection.emplace_back(f3);

      std::array<int,2> matIds = {(f.mat & 0xff) - 1, (f.mat >> 8) - 1};
      for(int ii=0; ii < 2; ii++ ) {
        int mat = matIds[ii];
        if(mat != -1)
          bodiesCollection[mat].emplace_back((ii % 2 == 0 ? 1 : -1) * facesCollection.size());
      }
    }
  }
  // y layers
  for (int addr = 0; addr < sz * sz; addr++)
    mats[addr] = -1;
  for(int i=0;i<ny+1;i++) {
    for(int yi=0; yi<nz; yi++) {
      for(int xi=0; xi<nx; xi++) {
        int addr3d = xi + yi * nx * ny;
        int addr = xi + yi * sz;
        int m1 = i < ny ? box[i*nx+addr3d] : -1;
        int m2 = i > 0 ? box[(i-1)*nx+addr3d] : -1;
        mats[addr] = (m1+1) + ((m2+1) << 8);
      }
    }
    auto faces2d = BitmapMesher::make(mats);
    for(auto& f : faces2d) {
      if(f.holes.size())
        std::cout << "holes count " << f.holes.size() << '\n';

      auto makeF3=[&](BitmapMesher::loop& f)->loop3d {
        loop3d f3;
        std::vector<int> edloop;
        for(auto& e : f) {
          int n1id = getNodeId(vi3d{e.v1.x, i, e.v1.y});
          int n2id = getNodeId(vi3d{e.v2.x, i, e.v2.y});
          int eid = getEdgeId(n1id, n2id);
          edloop.emplace_back(eid);
        }
        auto& edges = edgesCollection;
        for(int i=0; i<edloop.size(); i++) {
          e3d& e1 = edges[edloop[i]];
          e3d& e2 = edges[edloop[(i+1)%edloop.size()]];
          f3.emplace_back(e1[0] == e2[0] || e1[0] == e2[1] ? e1[0] : e1[1]);
        }
        return f3;
      };
      
      f3d f3(makeF3(f));
    
      //holes
      for(auto& h : f.holes)
        f3.holes.emplace_back(makeF3(h));
      
      //put face to a collection now since in bodies collection we use indices startting from 1 not from 0
      facesCollection.emplace_back(f3);

      std::array<int,2> matIds = {(f.mat & 0xff) - 1, (f.mat >> 8) - 1};
      for(int ii=0; ii < 2; ii++ ) {
        int mat = matIds[ii];
        if(mat != -1)
          bodiesCollection[mat].emplace_back((ii % 2 == 0 ? -1 : 1) * facesCollection.size());
      }
    }
  }
  
  // x layers
  for (int addr = 0; addr < sz * sz; addr++)
    mats[addr] = -1;
  for(int i=0;i<nx+1;i++) {
    for(int yi=0; yi<nz; yi++) {
      for(int xi=0; xi<ny; xi++) {
        int addr3d = xi * nx + yi * nx * ny;
        int addr = xi + yi * sz;
        int m1 = i < nx ? box[i+addr3d] : -1;
        int m2 = i > 0 ? box[(i-1)+addr3d] : -1;
        mats[addr] = (m1+1) + ((m2+1) << 8);
      }
    }
    auto faces2d = BitmapMesher::make(mats, i==nx);
    for(auto& f : faces2d) {
      if(f.holes.size())
        std::cout << "holes count " << f.holes.size() << '\n';

        auto makeF3=[&](BitmapMesher::loop& f)->loop3d {
          loop3d f3;
          std::vector<int> edloop;
          for(auto& e : f) {
            int n1id = getNodeId(vi3d{i, e.v1.x, e.v1.y});
            int n2id = getNodeId(vi3d{i, e.v2.x, e.v2.y});
            int eid = getEdgeId(n1id, n2id);
            edloop.emplace_back(eid);
          }
          auto& edges = edgesCollection;
          for(int i=0; i<edloop.size(); i++) {
            e3d& e1 = edges[edloop[i]];
            e3d& e2 = edges[edloop[(i+1)%edloop.size()]];
            f3.emplace_back(e1[0] == e2[0] || e1[0] == e2[1] ? e1[0] : e1[1]);
          } 
          return f3;
        };
        
        f3d f3(makeF3(f));

        //holes
        for(auto& h : f.holes)
          f3.holes.emplace_back(makeF3(h));

      //put face to a collection now since in bodies collection we use indices startting from 1 not from 0
      facesCollection.emplace_back(f3);

      std::array<int,2> matIds = {(f.mat & 0xff) - 1, (f.mat >> 8) - 1};
      for(int ii=0; ii < 2; ii++ ) {
        int mat = matIds[ii];
        if(mat != -1)
          bodiesCollection[mat].emplace_back((ii % 2 == 0 ? 1 : -1) * facesCollection.size());
      }
    }
  }
  return sm;
}


void surfaceMesh::save(const char* filename) {
  std::ofstream of(filename);
  of << nodesCollection.size() << '\n';
  for(auto& n : nodesCollection) 
    of << n[0] << ' ' << n[1] << ' ' << n[2] << '\n';
  of << facesCollection.size() << '\n';
  for(auto& f : facesCollection) { 
    of << f.holes.size() << '\n';
    for(int i=0;i<f.size();i++)
      of << f[i] << (i == f.size() - 1 ? '\n' : ' ');
    for(int hid=0; hid < f.holes.size(); hid++) {
      for(int hvid=0; hvid<f.holes[hid].size(); hvid++)
        of << f.holes[hid][hvid] << (hvid == f.holes[hid].size() - 1 ? '\n' : ' ');
    }
  }
  
  for(auto& it : bodiesCollection) {
    int mat = it.first;
    auto& bodies = it.second;
    of << mat << '\n';
    of << bodies.size() << '\n';
    for(int i=0;i<bodies.size();i++)
      of << bodies[i] << (i == bodies.size() - 1 ? '\n' : ' ');
  }
  of.close();
}

  //TESTING-------TESTING-------TESTING-------TESTING-------TESTING-------TESTING-------
  //find gaps
  //here we introduce simple edge that consists of two vertices.
  //then we will collect all vertices and all that simple edges in a container and try to find if any vertex appears exactly on the edge but does not conicide with any edge vertex
int surfaceMesh::countGaps() {
  typedef std::array<vi3d,2> se;
  auto& nodes = nodesCollection;
  std::vector<se> alledges;
  for(auto& e : edgesCollection) {
    auto& n1 = nodes[e[0]];
    auto& n2 = nodes[e[1]];
    alledges.emplace_back(se{vi3d{n1[0], n1[1], n1[2]}, vi3d{n2[0], n2[1], n2[2]}});
  }
  
  //find gaps:
  std::cout << "finding gaps:\n";
  int gaps_count=0;
  
  //since coordinates are integer and everything is Manhattan we can avoid tweaking with tolerances
  auto badlyOnEdge=[](const vi3d& v, const se& e) -> bool {
    auto v1 = e[0]-v;
    auto v2 = e[1]-v;
    if(v1[0]*v2[1] == v2[0]*v1[1] && v1[0]*v2[2] == v2[0]*v1[2] && v1[2]*v2[1] == v2[2]*v1[1]) { // collinear
      if(v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2] < 0) // inside edge
        return true;
    }
    return false;
  };
  
  //do everybody with everybody dumb test:
  for(auto& e : alledges) {
    vi3d& ev1 = e[0];
    vi3d& ev2 = e[1];
    int xmin = std::min(ev1[0],ev2[0]); 
    int xmax = std::max(ev1[0],ev2[0]); 
    int ymin = std::min(ev1[1],ev2[1]); 
    int ymax = std::max(ev1[1],ev2[1]); 
    int zmin = std::min(ev1[2],ev2[2]); 
    int zmax = std::max(ev1[2],ev2[2]); 
    for(int k = zmin; k <= zmax; k++) {
      for(int j = ymin; j <= ymax; j++) {
        for(int i = xmin; i <= xmax; i++) {
          int nodeId = idGrd.getId(vi3d{i,j,k});
          if(nodeId == -1)
            continue;
          const auto& v = nodes[nodeId];
          if(badlyOnEdge(v,e)) {
            std::cout << "error: vertex(" << v[0] << ", " << v[1] << ", " << v[2] << ") is on edge(" << e[0][0] << ", " << e[0][1] << ", " << e[0][2] << ")--(" << e[1][0] << ", " << e[1][1] << ", " << e[1][2] << ")\n";
            gaps_count++;
          }
        }
      }
    }
  }
  return gaps_count;
}

surfaceMesh makeSurfaceSimple(int* box, int sx, int sy, int sz) {
  surfaceMesh sm;
  std::vector<n3d>& nodesCollection = sm.nodesCollection; // material id to nodes mapping
  std::vector<e3d>& edgesCollection = sm.edgesCollection; // material id to edges mapping
  std::vector<f3d>& facesCollection = sm.facesCollection; // material id to faces mapping
  //NOTE: all that stuff is needed since faces are refering edges which in turn are refering nodes 
  std::map<int, body>& bodiesCollection = sm.bodiesCollection;
  
  idGrid<vi3d>& idGridsCollection = sm.idGrd;
  
  auto getNodeId = [&](const vi3d& v)->int {
    auto& nodes = nodesCollection;
    auto& idgrid = idGridsCollection;

    int id = idgrid.getId(v);
    if (id == -1) {
      id = nodes.size();
      idgrid.addId(v, id);
      nodes.emplace_back(n3d(v[0], v[1], v[2]));
    }
    return id;
  };

  auto getEdgeId = [&](int n1id, int n2id)->int {
    auto& nodes = nodesCollection;
    auto& edges = edgesCollection;
    n3d& n1 = nodes[n1id];
    n3d& n2 = nodes[n2id];
    for (auto& eid1 : n1.edges) {
      for (auto& eid2 : n2.edges) {
        if (eid1 == eid2)
          return eid1;
      }
    }
    e3d e{ n1id, n2id };
    int eid = edges.size();
    edges.emplace_back(e);
    n1.edges.emplace_back(eid);
    n2.edges.emplace_back(eid);
    return eid;
  };
  

  const std::array<std::array<int,3>,6> nbdirs = {{{{-1,0,0}},
                                                   {{1,0,0}},
                                                   {{0,-1,0}},
                                                   {{0,1,0}},
                                                   {{0,0,-1}},
                                                   {{0,0,1}}}};
  const std::array<std::array<int, 3>, 8> nodesmap = {{ {{0,0,0}}, {{1,0,0}}, {{1,1,0}}, {{0,1,0}}, {{0,0,1}}, {{1,0,1}}, {{1,1,1}}, {{0,1,1}} }};
  
  const std::array<std::array<int, 2>, 12> edgesmap = {{ {{0,1}}, {{1,2}}, {{3,2}}, {{0,3}}, {{4,5}}, {{5,6}}, {{7,6}}, {{4,7}}, {{0,4}}, {{1,5}}, {{2,6}}, {{3,7}} }};
//                                                    0 x+      1 y+     2 x-    3 y-     4 x+      5 y+     6 x-    7 y-      8 z+     9 z+    10 z+     11 z+
//  const std::array<std::array<int, 4>, 6> faces = {{ {{3,0,4,7}} /*x-*/, {{1,2,6,5}} /*x+*/, {{0,1,5,4}} /*y-*/, {{2,3,6,7}} /*y+*/, {{0,1,2,3}} /*z-*/, {{4,5,6,7}} /*z+*/}};

  const std::array<std::array<int, 4>, 6> facesmap = {{ {{3,8,7,11}} /*x-*/, {{1,10,5,9}} /*x+*/, {{0,9,4,8}} /*y-*/, {{2,11,6,10}} /*y+*/, {{0,3,2,1}} /*z-*/, {{4,5,6,7}} /*z+*/}};

  for (int k = 0; k < sz; k++) {
    for (int j = 0; j < sy; j++) {
      for (int i = 0; i < sx; i++) {
        int mat = box[i + j * sx + k * sx * sy];
        if(mat == -1)
          continue;
        for(int nb = 0; nb < 6; nb++) {
          int nbi = i + nbdirs[nb][0];
          int nbj = j + nbdirs[nb][1];
          int nbk = k + nbdirs[nb][2];
          int nbmat = (nbi >= 0 && nbi < sx && nbj >= 0 && nbj < sy && nbk >= 0 && nbk < sz) ? box[nbi + nbj * sx + nbk * sx * sy] : -1;
          if(nbmat == mat)
            continue;
          //std::cout << "adding " << i << ' ' << j << ' ' << k << '\n';
          std::array<int,4> edloop;
          for (int eid = 0; eid < 4; eid++) {
            int nid1 = edgesmap[facesmap[nb][eid]][0];
            int nid2 = edgesmap[facesmap[nb][eid]][1];
            int gn1id = getNodeId(vi3d{ i + nodesmap[nid1][0], j + nodesmap[nid1][1], k + nodesmap[nid1][2] });
            int gn2id = getNodeId(vi3d{ i + nodesmap[nid2][0], j + nodesmap[nid2][1], k + nodesmap[nid2][2] });
            int geid = getEdgeId(gn1id, gn2id);
            edloop[eid] = geid;
          }
          
          loop3d f3;
          auto& edges = edgesCollection;
          for (int i = 0; i < edloop.size(); i++) {
            e3d& e1 = edges[edloop[i]];
            e3d& e2 = edges[edloop[(i + 1) % edloop.size()]];
            f3.emplace_back(e1[0] == e2[0] || e1[0] == e2[1] ? e1[0] : e1[1]);
          }

          f3d ff3(f3);
          facesCollection.emplace_back(f3);
          bodiesCollection[mat].emplace_back(facesCollection.size());
        }
      }
    }
  }
  return sm;
}

void surfaceMesh::saveBlender(const char* filename) {
  std::ofstream of(filename);
  of << bodiesCollection.size() << '\n';
  idGrid<vi3d> searchGrid;
  for(auto& it : bodiesCollection) {
    std::vector<vi3d> vertices;
    std::vector<f3d> faces;
    
    searchGrid.reset();
    
    auto addNode = [&](int nid) -> int {
      auto& nd = nodesCollection[nid];
      vi3d v{nd[0], nd[1], nd[2]};
      int id = searchGrid.getId(v);
      if(id == -1) {
        id = vertices.size();
        searchGrid.addId(v,id);
        vertices.emplace_back(v);
      }
      return id;
    };
    
    for(int fid_ : it.second) {
      int fid = std::abs(fid_) - 1;
      loop3d local_f_;
      const f3d& f = facesCollection[fid];
      for(int i=0; i< f.size(); i++) {
        int nid = fid_ > 0 ? f[i] : f[f.size() - 1 - i];
        local_f_.emplace_back(addNode(nid));
      }
      f3d local_f(local_f_);
      for(auto& h : f.holes) {
        loop3d local_h;
        for(int ii=0; ii < h.size(); ii++) {
          int hnid = fid_ > 0 ? h[ii] : h[h.size() - 1 - ii];
          local_h.emplace_back(addNode(hnid));
        }
        local_f.holes.emplace_back(local_h);
      }
      faces.emplace_back(local_f);
    }
    of << it.first << '\n';
    of << vertices.size() << '\n';
    for(auto& v : vertices)
      of << v[0] << ' ' << v[1] << ' ' << v[2] << '\n';
    of << faces.size() << '\n';
    for(auto& f : faces) {
      of << f.holes.size() << '\n';
      for(int i=0;i<f.size();i++)
        of << f[i] << (i == f.size() - 1 ? '\n' : ' ');
      for(int hid=0; hid < f.holes.size(); hid++) {
        for(int hvid=0; hvid<f.holes[hid].size(); hvid++)
          of << f.holes[hid][hvid] << (hvid == f.holes[hid].size() - 1 ? '\n' : ' ');
      }
    }
  }
  of.close();
}

void surfaceMesh::Euler(const body& b, int& V, int& E, int& F) {
  F=0;
  E=0;
  V=0;
  std::set<int> vertices;
  std::set<std::array<int,2>> edges;
  idGrid<vi3d> searchGrid;

  auto count = [&](loop3d& f) {
    for(int v_id : f) {
      n3d& nd = nodesCollection[v_id];
      vertices.insert(v_id);
    }
    for(int local_v_id=0; local_v_id < f.size(); local_v_id++) {
      int local_v_id_next = (local_v_id + 1) % f.size(); 
      /*
      n3d& nd1 = nodesCollection[f[local_v_id]];
      n3d& nd2 = nodesCollection[f[local_v_id_next]];
      for(int eid1 : nd1.edges) {
        for(int eid2 : nd2.edges) {
          if(eid1==eid2)
            edges.insert(eid1);
        }
      }
      */
      edges.insert(std::array<int,2>{std::min(f[local_v_id], f[local_v_id_next]), std::max(f[local_v_id], f[local_v_id_next])});
    }
  };

  for(int f_id : b) {
    F++;
    f3d& f = facesCollection[std::abs(f_id) - 1];
    count(f);
    for(auto& h : f.holes) {
      F--;
      count(h);
    }
  }
  V = vertices.size();
  E = edges.size();
}

