#pragma once
#include "png.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <map>

struct color
{
  color() : red(0.), green(0.), blue(0.) {}
  color(double r, double g, double b) : red(r), green(g), blue(b) {}
  double red;
  double green;
  double blue;
  double distance(color c)
  {
    return std::sqrt(std::pow(this->red - c.red,2)
              + std::pow(this->green - c.green,2)
              + std::pow(this->blue - c.blue,2));
  }
};

bool operator<(const color& c1, const color& c2) {
  if(c1.red!=c2.red)
    return c1.red < c2.red;

  if(c1.green!=c2.green)
    return c1.green < c2.green;
    
  if(c1.blue!=c2.blue)
    return c1.blue < c2.blue;
  
  return false;
}

std::ostream& operator<<(std::ostream& o, const color& c) {
  return o << '(' << c.red << ", " << c.green << ", " << c.blue << ')';
}

struct image
{
  int w;
  int h;
  color* data;
  color get(int x, int y)
  {
    return data[x*h+y];
  }
  void set(color c, int x, int y)
  {
     data[x*h+y]=c;
  }
  image() {}
  image(int _w, int _h): w(_w), h(_h)
  {
    std::cout << "initialized image " << w << "x" << h << std::endl;
    data=new color[w*h];
  }
  ~image()
  {
    delete[] data;
  }
};

image* read_png_file(const char* file_name)
{
  std::cout << "start reading file " << file_name << std::endl;
  int x, y;

  int width, height;
  png_byte color_type;
  png_byte bit_depth;

  png_structp png_ptr;
  png_infop info_ptr;
  int number_of_passes;
  png_bytep * row_pointers;

  unsigned char header[8];    // 8 is the maximum size that can be checked

  std::cout << "open file and test for it being a png" << std::endl;
  FILE *fp = fopen(file_name, "rb");
  if (!fp)
          std::cerr << "[read_png_file] File " << file_name << " could not be opened for reading" << std::endl;
  fread(header, 1, 8, fp);
  if (png_sig_cmp(header, 0, 8))
         std::cerr << "[read_png_file] File " << file_name << " is not recognized as a PNG file" << std::endl;


  std::cout << "reding header done" << std::endl;
  std::cout << "initialize stuff" << std::endl;
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (!png_ptr)
          std::cerr << "[read_png_file] png_create_read_struct failed" << std::endl;

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
          std::cerr << "[read_png_file] png_create_info_struct failed" << std::endl;

  if (setjmp(png_jmpbuf(png_ptr)))
          std::cerr << "[read_png_file] Error during init_io" << std::endl;

  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, 8);

  png_read_info(png_ptr, info_ptr);

  width = png_get_image_width(png_ptr, info_ptr);
  height = png_get_image_height(png_ptr, info_ptr);
  color_type = png_get_color_type(png_ptr, info_ptr);
  bit_depth = png_get_bit_depth(png_ptr, info_ptr);

  number_of_passes = png_set_interlace_handling(png_ptr);
  png_read_update_info(png_ptr, info_ptr);


  std::cout << "read file" << std::endl;
  if (setjmp(png_jmpbuf(png_ptr)))
          std::cerr << "[read_png_file] Error during read_image" << std::endl;

  row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
  for (y=0; y<height; y++)
          row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));

  png_read_image(png_ptr, row_pointers);

  fclose(fp);
  std::cout << "reading done" << std::endl;
  image* img=new image(width,height);
  for(int j=0;j<height;j++)
  {
    for(int i=0;i<width;i++)
    {
      color c;
      c.red = double(row_pointers[j][i*4])/255.0;
      c.green = double(row_pointers[j][i*4+1])/255.0;
      c.blue = double(row_pointers[j][i*4+2])/255.0;
      img->set(c,i,j);
    }
  }
  return img;
}

std::vector<int> getPatternFromPng(const char* filename) {
  image* img = read_png_file(filename);
  
  std::cout << "img w=" << img->w << ", img h=" << img->h << '\n';

  std::vector<int> mats;
  
  if(img->w != img->h) {
    std::cerr << "ERROR: img->w != img->h\n";
    return mats;
  }
  
  int side=img->w;
  
  mats.resize(side * side);
  std::map<color, int> matmap;
  int currentmat=-1;
  for(int i=0; i<side; i++) {
    for(int j=0; j<side; j++) {
      const color& c = img->get(j,i);
      //std::cout << c;
      if(matmap.find(c) == matmap.end())
        matmap[c] = ++currentmat;
      mats[j*side+i] = matmap[c];
      std::cout << mats[j*side+i] << ' '; 
      //std::cout << currentmat << ':' << matmap[c] << ' ';
    }
    std::cout << '\n';
  }
  std::cout << '\n';
  return mats;
}
