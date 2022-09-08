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
g++ -O3 makeSurface.cpp voxelated_meshing.cpp test3d_16x16x16.cpp -lpng -o test3d

Tests:

Simplest algorithm is implemented in test_v1(simple_edges).cpp, this source is self-consistent. dov1.cmd builds and runs test. It uses example.png as input testing bitmap. Output are dump.dat which contains bitmap, and nodemap. If some edges are not processed it will appear in final dump.dat. returning from while::get_faces(…) will result in processing just the first face (also comment main::makeHoles() call). See details in documentation_v1(simple_edges).pdf. Another output is holes.dat which contains holes contours. Another output is face.dat which contains faces, they are drawn by corresponding asy script, faces.pdf can be studied in inkscape, there it can be verified that holes are properly processed.

Next version is test_v2(merged_edges).cpp it uses API from voxelated_meshing.h. voxelated_meshing.cpp implements same algorithms as ones used in test_v1(simple_edges).cpp except for get_edges() procedure. It is quite tricky and creates merged faces and described in details in interface.pdf. It is ready for interfaces – i.e. two bitmap patterns. This test uses same single example.png bitmap though. Dov2.cmd builds and runs the test. This test has the same output, using inkscape for faces.pdf it can be verified that edges are merged.
test_v3(interface).cpp implements interface case and uses iA.png and iB.png, executed but dov3.cmd and have the same output.

test3d_16x16x16.cpp uses geo.h to build geometry predicates (two spheres and one box in this case), it creates faces for each interface for each direction (x,y and z) and stores results per material. Simple structures vi3d, n3d, e3d and f3d structures are used to connect mesh graph. Very basic linear search algorithms are used in getNodeId and getEdgeId to connect all together. Output of this test is surf.dat file which is used bu mesh.blend (it contains pythion scripts that uses blender python API ‘bpy’ to create blender structure from surf.dat). After surf.dat is generated one can run blender open mesh.blend and run script (tight click in scripting pane).

test3d_16x16x16.cpp also verifies that mesh has no gaps. It checks only node to edge gaps, faces are not tested but by design there should not be one.
