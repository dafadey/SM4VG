unitsize(1cm);

pen[] colors={red, green, blue, cyan, yellow, purple, gray};

file fin=input("dump_front.dat").word();

int[] raw = fin;

write("raw.size=", raw.length);

int offset = 0;
int side = raw[offset];
offset +=1;

int mat_offset = offset;

write("side=", side);

for(int j=0;j<side;++j)
{
  for(int i=0;i<side;++i)
    fill(shift(i,j)*unitsquare, colors[raw[i+j*side+offset]]);
}
