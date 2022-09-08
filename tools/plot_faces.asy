unitsize(1cm);

currentpen = linewidth(0.03cm);

pen[] colors={red, green, blue, cyan, yellow, purple, gray};

file fin=input("faces.dat").word();

int[] raw = fin;

write("raw.size=", raw.length);
int offset = 0;
int nfaces = raw[offset];
offset +=1;

for(int i=0; i<nfaces;++i) {
  int mat = raw[offset];
  offset += 1;
  int nverts = raw[offset];
  offset += 1;
  int nholes = raw[offset];
  offset += 1;
  path[] p;
  guide gf;
  for(int j=0;j<nverts;++j) {
    gf = gf--(raw[offset], raw[offset+1]);
    offset+=2;
  }
  gf = gf -- cycle;
  p = p^^gf;
  for(int k=0;k<nholes;++k) {
    int nhverts = raw[offset];
    offset += 1;
    path gh;
    for(int j=0;j<nhverts;++j) {
      gh = gh--(raw[offset], raw[offset+1]);
      offset+=2;
    }
    gh = gh -- cycle;
    p = p ^^ gh;
  }
  fill(p,colors[mat % colors.length]);
}
