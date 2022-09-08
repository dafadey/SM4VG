import patterns;
unitsize(1cm);

currentpen = linewidth(0.03cm);

pen[] colors={red, green, blue, cyan, yellow, purple, gray};

file fin=input("holes.dat").word();

int[] raw = fin;

write("raw.size=", raw.length);
add("crosshatch", crosshatch(1mm));
int offset = 0;
int side = raw[offset];
offset +=1;

int mat_offset = offset;

write("side=", side);

for(int j=0;j<side;++j)
{
  for(int i=0;i<side;++i) {
    int material = raw[i+j*side+offset];
    if(material >= 0)
      fill(shift(i,j)*unitsquare, colors[material % colors.length]);
    else
      fill(shift(i,j)*unitsquare, pattern("crosshatch"));
  }
}

offset += side*side;

int ecount = raw[offset];
offset +=1; 

int edges_offset = offset;

for(int eid=0;eid<ecount; ++eid)
{
  
  if(raw[offset+eid*6]+4 != -1) {
    pair v1 = (raw[offset+eid*6],raw[offset+eid*6+1]);
    pair v2 = (raw[offset+eid*6+2],raw[offset+eid*6+3]);
    draw((v1--v2));
  }
}
