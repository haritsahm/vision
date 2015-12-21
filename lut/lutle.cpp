#include <cstdio>

struct ColourData
{
    float grminv, grmaxv, brminv, brmaxv;
    int yminv, ymaxv, uminv, umaxv, vminv, vmaxv,coloursch;
};

typedef unsigned char uchar;

inline int doCheck3C(int a, int b, int c, int mina, int maxa, int minb, int maxb, int minc, int maxc)
{
	if((a<=maxa)&&(a>=mina)&&(b<=maxb)&&(b>=minb)&&(c<=maxc)&&(c>=minc))
	{
		return 1; 
	}
	else
		return 0;
}

uchar* marker_create_lut(ColourData &c)
{
    uchar* lut = new unsigned char [256*256*256];//[16777216];	//256*256*256

    float r,g,b;
    float Reds, Greens, Blues, Ys, Crs, Cbs;
    int value;


    for(r=0.0; r<256.0; r++)
    for(g=0.0; g<256.0; g++)
    for(b=0.0; b<256.0; b++)
    {
       Ys = 0.299*r + 0.587*g + 0.114*b;
       Crs = (r-Ys)*0.713 + 128.0;
       Cbs = (b-Ys)*0.564 + 128.0;

       Reds = r/255.0;
       Greens = g/255.0;
       Blues = b/255.0;

       if(c.coloursch==0)
       {
           if(doCheck3C(Ys, Crs, Cbs, c.yminv, c.ymaxv, c.uminv, c.umaxv, c.vminv, c.vmaxv))
           {
                lut[(uchar)r + (((uchar)g)<<8) + (((uchar)b)<<16)] = 255;
           }
           else
           {
                lut[(uchar)r + (((uchar)g)<<8) + (((uchar)b)<<16)] = 0;
           }
       }
       else
       {
            if((Greens<c.grmaxv*Reds)&&(Greens>c.grminv*Reds)&&(Blues<c.brmaxv*Reds)&&(Blues>c.brminv*Reds))
            {
                 lut[(uchar)r + (((uchar)g)<<8) + (((uchar)b)<<16)] = 255;
            }
            else
            {
                 lut[(uchar)r + (((uchar)g)<<8) + (((uchar)b)<<16)] = 0;
            }
       }

    }
    return lut;
}

int main(int argc, char const *argv[])
{
	ColourData c;
	c.coloursch = 0;
	printf("Ymin: ");
	scanf("%d", &c.yminv);
	printf("Ymax: ");
	scanf("%d", &c.ymaxv);
	printf("Umin: ");
	scanf("%d", &c.uminv);
	printf("Umax: ");
	scanf("%d", &c.umaxv);
	printf("Vmin: ");
	scanf("%d", &c.vminv);
	printf("Vmax: ");
	scanf("%d", &c.vmaxv);
  char name[100];
  printf("LUT Name: ");
  scanf("%s", name);
	uchar* mylut = marker_create_lut(c);
	FILE* fp;
	fp=fopen(name,"wb");
  fwrite(mylut,sizeof(uchar),256*256*256,fp);
  fclose(fp);
	return 0;
}