#pragma once
#include <array>
#include <cmath>
#include <algorithm>
#include <iostream>

typedef std::array<double,3> v3d;

v3d operator+(const v3d& v1, const v3d& v2) {
  return v3d{v1[0]+v2[0], v1[1]+v2[1], v1[2]+v2[2]};
}

v3d operator-(const v3d& v1, const v3d& v2) {
  return v3d{v1[0]-v2[0], v1[1]-v2[1], v1[2]-v2[2]};
}

double operator*(const v3d& v1, const v3d& v2) {
  return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

double abs(const v3d& v) {
  return std::sqrt(v*v);
}

v3d hprod(const v3d& v1, const v3d& v2) {
  return v3d{v1[0]*v2[0], v1[1]*v2[1], v1[2]*v2[2]};
}

std::ostream& operator<<(std::ostream& s, const v3d& v) {
  return s << "(" << v[0] << ", " << v[1] << ", " << v[2] << ")";
}

struct Geo {
  virtual bool inside(const v3d&) {
    return false;
  }
  int layer{};
  int mat{};
  virtual void printMe(std::ostream& s) const;
};

std::ostream& operator<<(std::ostream& s, const Geo& g) {
  return s << g.mat << ":" << g.layer << " " << "empty geo";
}

void Geo::printMe(std::ostream& s) const {
  s << *this;
}

struct Sphere : public Geo {
  Sphere(const v3d& c_, double r_) : c{c_}, r{r_} {} 
  virtual bool inside(const v3d& p) override {
    return abs(p - c) < r;
  }
  virtual void printMe(std::ostream& s) const override;
  v3d c;
  double r;
};

std::ostream& operator<<(std::ostream& s, const Sphere& sp) {
  return s << sp.mat << ":" << sp.layer << " " << sp.c << " " << sp.r;
}

void Sphere::printMe(std::ostream& s) const {
  s << *this;
}

struct Box : public Geo {
  Box(const v3d& v1_, const v3d& v2_) {
    for(int i=0;i<3;i++) {
      v1[i] = std::min(v1_[i],v2_[i]);
      v2[i] = std::max(v1_[i],v2_[i]);
    }
  }

  virtual void printMe(std::ostream& s) const override;
  
  virtual bool inside(const v3d& p) override {
    auto h = hprod(v1-p, v2-p);
    return h[0] < .0 && h[1] < .0 && h[2] < .0;
  }
  
  v3d v1;
  v3d v2;
};

std::ostream& operator<<(std::ostream& s, const Box& b) {
  return s << b.mat << ":" << b.layer << " " << b.v1 << "-" << b.v2;
}

void Box::printMe(std::ostream& s) const {
  s << *this;
}

struct Scene : public std::vector<Geo*> {
  int getMat(const v3d& v) {
    std::vector<std::array<int, 2>> stack; //layer, mat
    for(auto& g : *this) {
      if(g->inside(v))
        stack.emplace_back(std::array<int, 2>{g->layer, g->mat});
    }
    if(stack.size() == 0) {
      //std::cout << 'x' << ' ';
      return -1;
    }
    std::sort(stack.begin(), stack.end(), [](const std::array<int, 2>& i1, const std::array<int, 2>& i2) -> bool {return i1[0] < i2[0];});
    //std::cout << stack[0][1] << ' ';
    return stack[0][1]; 
  }
};

std::ostream& operator<<(std::ostream& s, const Scene& sc) {
  for(auto& g : sc) {
    s << '\t';
    g->printMe(s);
    s << '\n';
  }
  return s;
}
