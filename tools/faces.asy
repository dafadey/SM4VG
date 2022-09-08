unitsize(1cm);

pen[] colors={red, green, blue, cyan, yellow, purple, gray};

file fin=input("faces.dat").word();

int[] raw = fin;

currentpen = linewidth(0.03cm);

write("raw.size=", raw.length);
int offset=0;

while(true) {
  int mat=raw[offset];
  offset+=1;
  int sz = raw[offset];
  offset+=1;
  guide g;
  for(int i=0;i<sz;++i) {
    int x1=raw[offset+i*2];
    int y1=raw[offset+i*2+1];
    g = g--(x1,y1);
  }
  g = g -- cycle;
  fill(g, colors[mat % colors.length]);
  offset+=sz*2;
  if(offset == raw.length)
    break;
}
