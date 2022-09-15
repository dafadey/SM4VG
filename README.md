# SM4VG
SM4VG stands for Surface Mesher for Voxelated Geomerty
this software is intended to create *minimalist surface meshes from voxelated geometries
*in this context it means that flat Manhattan faces should have minimal number of nodes and be non further mergiable

Build and run prerequisites:

Windows:
MinGW with libpng-1.6.35 library for reading testing png bitmap patterns (most probably available in Cygwin). Asymptote for plotting debugging data into eps or pdf files. Blender to visualize mesh.
After installing and setting up MinGW use Linux instructions below.

Linux:
g++ -O3 makeSurface.cpp voxelated_meshing.cpp 'test_v1(simple_edges).cpp' -lpng -o test1
g++ -O3 makeSurface.cpp voxelated_meshing.cpp 'test_v2(merged_edges).cpp' -lpng -o test2
g++ -O3 makeSurface.cpp voxelated_meshing.cpp 'test_v1(simple_edges).cpp' -lpng -o test3
g++ -O3 makeSurface.cpp voxelated_meshing.cpp test3d_16x16x16.cpp -lpng -o test3d_16x16x16
g++ -O3 makeSurface.cpp voxelated_meshing.cpp test3d_prepare.cpp -lpng -o test3d_prepare
g++ -O3 makeSurface.cpp voxelated_meshing.cpp test3d.cpp -lpng -o test3d

Tests:

Simplest algorithm is implemented in test_v1(simple_edges).cpp, this source is self-consistent. dov1.cmd builds and runs test. It uses example.png as input testing bitmap. Output are dump.dat which contains bitmap, and nodemap. If some edges are not processed it will appear in final dump.dat. returning from while::get_faces(…) will result in processing just the first face (also comment main::makeHoles() call). See details in documentation_v1(simple_edges).pdf. Another output is holes.dat which contains holes contours. Another output is face.dat which contains faces, they are drawn by corresponding asy script, faces.pdf can be studied in inkscape, there it can be verified that holes are properly processed.

Next version is test_v2(merged_edges).cpp it uses API from voxelated_meshing.h. voxelated_meshing.cpp implements same algorithms as ones used in test_v1(simple_edges).cpp except for get_edges() procedure. It is quite tricky and creates merged faces and described in details in interface.pdf. It is ready for interfaces – i.e. two bitmap patterns. This test uses same single example.png bitmap though. Dov2.cmd builds and runs the test. This test has the same output, using inkscape for faces.pdf it can be verified that edges are merged.
test_v3(interface).cpp implements interface case and uses iA.png and iB.png, executed but dov3.cmd and have the same output.

test3d_16x16x16.cpp uses geo.h to build geometry predicates (two spheres and one box in this case), it creates faces for each interface for each direction (x,y and z) and stores results per material. Simple structures vi3d, n3d, e3d and f3d structures are used to connect mesh graph. Very basic linear search algorithms are used in getNodeId and getEdgeId to connect all together. Output of this test is surf.dat file which is used bu mesh.blend (it contains pythion scripts that uses blender python API ‘bpy’ to create blender structure from surf.dat). After surf.dat is generated one can run blender open mesh.blend and run script (tight click in scripting pane).

test3d_16x16x16.cpp also verifies that mesh has no gaps. It checks only node to edge gaps, faces are not tested but by design there should not be one.

output formats:
surf_Blender.dat - contains vertices and plygonal faces without holes acceptable by script in tool/mesh.blend.
surf_simple.dat - same blender oriented output but for raw voxels (only interface faces are drawn).
the above formats are per body so each body has own vertices and own faces, vertices are not shared between bodies.
refer to tools/mesh.blend to see how the format is interpreted.

surf.dat - format with shared vertices and shared faces. faces may be with holes.
format is as follows:
  number of verts
  x0 y0 z0
  x1 y1 z1
  x2 y2 z2
  ...
  number of faces
  number of holes
  v0 v1 v2 v3 v4 v5
  v110 v111 v112 v113
  v211 v221 v232 v233
  ...
  number of holes
  v13 v11 v12 v33 v41 v52
  v73 v72 v71 v69
  v37 v39 v144 v169
  ...
  body id
  number of faces
  f0 f1 f2 f3 ...
  body id
  number of faces
  f13 f11 f12 -f3 ... // - means for this body face f3 has inverted normal 

test3d and test3d_prepare

test3d_prepare creates testing voxelated geometry written in compressed format (nx,ny,nz,voxel0,count,voxel1,count...voxellast,count). test3d_prepare accepts input nx=ny=nz (i.e. resolution) and output file name. so sample usage is as follows:
./test3d_prepare 32 test32.cdat
test3d_prepare may be compiled with -DTEST3 to switch from random geometry to a predefined scene.
test3d reads voxelated geometry written in compressed format and creates surfaces (same way as test3d_16x16x16 does)
sample usage is:
./test3d test32.cdat

FOR THOSE WHO LASILY SCRROLLED UP TO HERE:
./build
./test3d_prepare 64 test64.cdat
#now your sample voxel data is in test64.cdat
./test3d test64.dat
#now your shared surface is in surf.dat
#blender friendly siurface is in surf_Blender.dat
#tha latter can be visualized with tools/mesh.blend which contains py script to create blendee scene.
