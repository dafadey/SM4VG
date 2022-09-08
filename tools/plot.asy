import patterns;
unitsize(1cm);

pen[] colors={red, green, blue, cyan, yellow, purple, gray};

file fin=input("dump.dat").word();

int[] raw = fin;

currentpen = linewidth(0.03cm);

write("raw.size=", raw.length);

int offset = 0;
int side = raw[offset];
offset +=1;

int mat_offset = offset;

write("side=", side);

add("crosshatch", crosshatch(1mm));

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
  real d=0.05;
  
  if(raw[offset+eid*6]+4 != -1) {
    pair v1 = (raw[offset+eid*6],raw[offset+eid*6+1]);
    pair v2 = (raw[offset+eid*6+2],raw[offset+eid*6+3]);
    
    if(abs((v2 - v1).x) < 0.000001) {
      int xi = round((v1.x + v2.x)*.5);
      int yi = floor((v1.y + v2.y)*.5);
      if(xi < side) {
        if(raw[mat_offset+xi+yi*side] == raw[offset+eid*6+4] || raw[mat_offset+xi+yi*side] == raw[offset+eid*6+5])
          draw(shift(d,0)*(v1--v2));
      }
      if(xi - 1 >= 0) {
        if(raw[mat_offset+xi-1+yi*side] == raw[offset+eid*6+4] || raw[mat_offset+xi-1+yi*side] == raw[offset+eid*6+5])
          draw(shift(-d,0)*(v1--v2));
      }
    }
    
    
    if(abs((v2 - v1).y) < 0.000001) {
      int xi = floor((v1.x + v2.x)*.5);
      int yi = round((v1.y + v2.y)*.5);
      if(yi < side) {
        if(raw[mat_offset+xi+yi*side] == raw[offset+eid*6+4] || raw[mat_offset+xi+yi*side] == raw[offset+eid*6+5])
          draw(shift(0,d)*(v1--v2));
        //if(raw[mat_offset+xi+yi*side] != raw[offset+eid*6+4] && raw[mat_offset+xi+yi*side] != raw[offset+eid*6+5])
        //  write("### ("+string(xi)+", "+string(yi)+") mt: "+string(raw[mat_offset+xi+yi*side])+" ee: "+string(raw[offset+eid*6+4]) + " " + string(raw[offset+eid*6+5]));
      }
      if(yi - 1 >= 0) {
        if(raw[mat_offset+xi+(yi-1)*side] == raw[offset+eid*6+4] || raw[mat_offset+xi+(yi-1)*side] == raw[offset+eid*6+5])
          draw(shift(0,-d)*(v1--v2));
        //if(raw[mat_offset+xi+(yi-1)*side] != raw[offset+eid*6+4] && raw[mat_offset+xi+(yi-1)*side] != raw[offset+eid*6+5])
        //  write("!!! ("+string(xi)+", "+string(yi)+") mt: "+string(raw[mat_offset+xi+(yi-1)*side])+" ee: "+string(raw[offset+eid*6+4]) + " " + string(raw[offset+eid*6+5]));
      }

    }
  }
}

offset += ecount * 6;
write("offset=", offset); 
for(int j=0;j<side+1; ++j)
{
  for(int i=0;i<side+1; ++i)
  {
    for(int k=0; k<raw[offset+(j*(side+1)+i)*5+4];++k)
      fill(shift(i+((k%2)-0.5)/7,j+(floor(k/2)-0.5)/7)*scale(0.0017cm)*shift(-.5,-.5)*unitcircle);
    //  dot((i+((k%2)-0.5)/7,j+(floor(k/2)-0.5)/7));
      
    for(int k=0; k<4; ++k) {
      int eid = raw[offset+(j*(side+1)+i)*5+k];
      if(eid == -1)
        continue;
      pair v1 = (raw[edges_offset+eid*6],raw[edges_offset+eid*6+1]);
      pair v2 = (raw[edges_offset+eid*6+2],raw[edges_offset+eid*6+3]);
      
      //draw((i+((k%2)-0.5)/7,j+(floor(k/2)-0.5)/7) -- .5*(v1+v2));
    }
    
  }
}
