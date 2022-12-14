// 3D Filled Vector Graphics
// (c) 2019 Pawel A. Hernik

/*
 Implemented features:
 - optimized rendering without local framebuffer, in STM32 case 1 to 32 lines buffer can be used
 - pattern based background
 - 3D starfield
 - no floating point arithmetic
 - no slow trigonometric functions
 - rotations around X and Y axes
 - simple outside screen culling
 - rasterizer working for all convex polygons
 - backface culling
 - visible faces sorting by Z axis
 - support for quads and triangles
 - optimized structures, saved some RAM and flash
 - added models
 - optimized stats displaying
 - fake light shading
*/

#include "conv565.h"
#include "pat2.h"


int16_t numVerts;
int16_t *verts;
int16_t numPolys;
uint8_t *polys;
uint16_t *polyColors;

 int polyMode;
  int orient;
  int bfCull;


  uint16_t RGBto565(uint8_t r, uint8_t g, uint8_t b) {
  
  return cc85[ r & 0xff ] << 11
			    		| cc86[ g & 0xff ] <<  5
			    		| cc85[ b & 0xff ];

}


  #define SCR_WD 320
#define SCR_HT 240

#define WD_3D 320 
#define HT_3D 240

#define swap(a, b) { int t = a; a = b; b = t; }

// 16 for ST7789 or 32 for ST7735
#define NLINES 16
uint16_t frBuf[SCR_WD*NLINES];
int yFr=0;

// ------------------------------------------------
#define MAXSIN 255
const uint8_t sinTab[91]  = {
0,4,8,13,17,22,26,31,35,39,44,48,53,57,61,65,70,74,78,83,87,91,95,99,103,107,111,115,119,123,
127,131,135,138,142,146,149,153,156,160,163,167,170,173,177,180,183,186,189,192,195,198,200,203,206,208,211,213,216,218,
220,223,225,227,229,231,232,234,236,238,239,241,242,243,245,246,247,248,249,250,251,251,252,253,253,254,254,254,254,254,
255
};

int fastSin(int i)
{
  while(i<0) i+=360;
  while(i>=360) i-=360;
  if(i<90)  return(sinTab[i]); else
  if(i<180) return(sinTab[180-i]); else
  if(i<270) return(-sinTab[i-180]); else
            return(-sinTab[360-i]);
}

int fastCos(int i)
{
  return fastSin(i+90);
}

// ------------------------------------------------

#define COL11 0x07df  // CYAN
#define COL12 0x05b6
#define COL13 0x069a

#define COL21 0xf81f // MAGENTA
#define COL22 0xb016
#define COL23 0xd01a

#define COL31 0xffc0 // YELLOW
#define COL32 0xb5a0
#define COL33 0xd680

#define COL41 0xfca0 // ORANGE
#define COL42 0xb320
#define COL43 0xd460

#define COL51 0x7c0 // GREEN
#define COL52 0x5a0
#define COL53 0x0680

#define COL61 0xffdf // GREY
#define COL62 0xb5b6
#define COL63 0xd69a

#define DRED     0x9000
#define DGREEN   0x04a0
#define DBLUE    0x0012
#define DCYAN    0x04b2
#define DYELLOW  0x94a0
#define DMAGENTA 0x9012

#define BLACK 0x0
#include "got.h"

#include "models3d.h"

// ----------------------------------------------- 
// input arrays


#define MAXVERTS 140
#define MAXPOLYS 240



// output arrays
int16_t transVerts[MAXVERTS*3];
int16_t projVerts[MAXVERTS*2];
uint16_t sortedPolys[MAXPOLYS];
uint16_t normZ[MAXPOLYS];

int rot0 = 0, rot1 = 0;
int numVisible = 0;
int lightShade = 0;

// simple Amiga like blitter implementation
void rasterize(int x0, int y0, int x1, int y1, int16_t *line) 
{
  if((y0<yFr && y1<yFr) || (y0>=yFr+NLINES && y1>=yFr+NLINES)) return; // exit if line outside rasterized area
  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);
  int err2,err = dx-dy;
  int sx = (x0 < x1) ? 1 : -1;
  int sy = (y0 < y1) ? 1 : -1;
  
  while(1)  {
    if(y0>=yFr && y0<yFr+NLINES) {
      if(x0<line[2*(y0-yFr)+0]) line[2*(y0-yFr)+0] = x0>0 ? x0 : 0;
      if(x0>line[2*(y0-yFr)+1]) line[2*(y0-yFr)+1] = x0<WD_3D ? x0 : WD_3D-1;
    }

    if(x0==x1 && y0==y1)  return;
    err2 = err+err;
    if(err2 > -dy) { err -= dy; x0 += sx; }
    if(err2 < dx)  { err += dx; y0 += sy; }
  }
}

void drawQuad( int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint16_t c) 
{
  int x,y;
  int16_t line[NLINES*2];
  for(y=0;y<NLINES;y++) { line[2*y+0] = WD_3D+1; line[2*y+1] = -1; }

  rasterize( x0, y0, x1, y1, line );
  rasterize( x1, y1, x2, y2, line );
  rasterize( x2, y2, x3, y3, line );
  rasterize( x3, y3, x0, y0, line );

  for(y=0;y<NLINES;y++)
    if(line[2*y+1]>line[2*y+0]) for(x=line[2*y+0]; x<=line[2*y+1]; x++) frBuf[SCR_WD*y+x]=c;
    //if(line[2*y+1]>line[2*y]) for(x=line[2*y]; x<=line[2*y+1]; x++) if(c==COL13) frBuf[SCR_WD*y+x]=pat7[((y+yFr)&0x1f)*32 + ((x-line[2*y])&0x1f)]; else frBuf[SCR_WD*y+x]=c;
}

void drawTri( int x0, int y0, int x1, int y1, int x2, int y2, uint16_t c) 
{
  int x,y;
  int16_t line[NLINES*2];
  for(y=0;y<NLINES;y++) { line[2*y+0] = WD_3D+1; line[2*y+1] = -1; }

  rasterize( x0, y0, x1, y1, line );
  rasterize( x1, y1, x2, y2, line );
  rasterize( x2, y2, x0, y0, line );

  for(y=0;y<NLINES;y++)
    if(line[2*y+1]>line[2*y+0]) for(x=line[2*y+0]; x<=line[2*y+1]; x++) frBuf[SCR_WD*y+x]=c;
}

void cullQuads(int16_t *v) 
{
  // backface culling
  numVisible=0;
  int x1,y1,x2,y2,z;
  for(int i=0;i<numPolys;i++) {
    if(bfCull) {
      x1 = v[3*polys[4*i+0]+0]-v[3*polys[4*i+1]+0];
      y1 = v[3*polys[4*i+0]+1]-v[3*polys[4*i+1]+1];
      x2 = v[3*polys[4*i+2]+0]-v[3*polys[4*i+1]+0];
      y2 = v[3*polys[4*i+2]+1]-v[3*polys[4*i+1]+1];
      z = x1*y2-y1*x2;
      normZ[i] = z<0? -z : z;
      if((!orient && z<0) || (orient && z>0)) sortedPolys[numVisible++] = i;
    } else sortedPolys[numVisible++] = i;
    //char txt[30];
    //snprintf(txt,30,"%d z=%6d  dr=%2d r0=%d",i,z,sortedQuads[i],rot[0]);
    //Serial.println(txt);
  }
  
  int i,j,zPoly[numVisible];
  // average Z of the polygon
  for(i=0;i<numVisible;++i) {
    zPoly[i] = 0.0;
    for(j=0;j<4;++j) zPoly[i] += v[3*polys[4*sortedPolys[i]+j]+2];
  }

  // sort by Z
  for(i=0;i<numVisible-1;++i) {
    for(j=i;j<numVisible;++j) {
      if(zPoly[i]<zPoly[j]) {
        swap(zPoly[j],zPoly[i]);
        swap(sortedPolys[j],sortedPolys[i]);
      }
    }
  }
}

void cullTris(int16_t *v) 
{
  // backface culling
  numVisible=0;
  int x1,y1,x2,y2,z;
  for(int i=0;i<numPolys;i++) {
    if(bfCull) {
      x1 = v[3*polys[3*i+0]+0]-v[3*polys[3*i+1]+0];
      y1 = v[3*polys[3*i+0]+1]-v[3*polys[3*i+1]+1];
      x2 = v[3*polys[3*i+2]+0]-v[3*polys[3*i+1]+0];
      y2 = v[3*polys[3*i+2]+1]-v[3*polys[3*i+1]+1];
      z = x1*y2-y1*x2;
      normZ[i] = z<0? -z : z;
      if((!orient && z<0) || (orient && z>0)) sortedPolys[numVisible++] = i;
    } else sortedPolys[numVisible++] = i;
  }

  int i,j,zPoly[numVisible];
  // average Z of the polygon
  for(i=0;i<numVisible;++i) {
    zPoly[i] = 0.0;
    for(j=0;j<3;++j) zPoly[i] += v[3*polys[3*sortedPolys[i]+j]+2];
  }

  // sort by Z
  for(i=0;i<numVisible-1;++i) {
    for(j=i;j<numVisible;++j) {
      if(zPoly[i]<zPoly[j]) {
        swap(zPoly[j],zPoly[i]);
        swap(sortedPolys[j],sortedPolys[i]);
      }
    }
  }
}

void drawQuads(int16_t *v2d) 
{
  int q,v0,v1,v2,v3,c,i;
  for(i=0;i<numVisible;i++) {
    q = sortedPolys[i];
    v0 = polys[4*q+0];
    v1 = polys[4*q+1];
    v2 = polys[4*q+2];
    v3 = polys[4*q+3];
    if(lightShade>0) {
      c = normZ[q]*255/lightShade;
      if(c>255) c=255;
      drawQuad(v2d[2*v0+0],v2d[2*v0+1],  v2d[2*v1+0],v2d[2*v1+1], v2d[2*v2+0],v2d[2*v2+1], v2d[2*v3+0],v2d[2*v3+1], RGBto565(c,c,c/2));
    } else
      drawQuad(v2d[2*v0+0],v2d[2*v0+1],  v2d[2*v1+0],v2d[2*v1+1], v2d[2*v2+0],v2d[2*v2+1], v2d[2*v3+0],v2d[2*v3+1], polyColors[q]);
  }
} 

void drawTris(int16_t *v2d) 
{
  int q,v0,v1,v2,v3,c,i;
  for(i=0;i<numVisible;i++) {
    q = sortedPolys[i];
    v0 = polys[3*q+0];
    v1 = polys[3*q+1];
    v2 = polys[3*q+2];
    if(lightShade>0) {
      c = normZ[q]*255/18000;
      if(c>255) c=255;
      drawTri(v2d[2*v0+0],v2d[2*v0+1],  v2d[2*v1+0],v2d[2*v1+1], v2d[2*v2+0],v2d[2*v2+1], RGBto565(c,c,c/2));
    } else
      drawTri(v2d[2*v0+0],v2d[2*v0+1],  v2d[2*v1+0],v2d[2*v1+1], v2d[2*v2+0],v2d[2*v2+1], polyColors[q]);
  }
} 

// animated checkerboard pattern
void backgroundChecker(int i)
{
  int x,y,xx,yy, xo,yo;
  xo = 25*fastSin(4*i)/256+50;
  yo = 25*fastSin(5*i)/256+50+yFr;

  for(y=0;y<NLINES;y++) {
    yy = (y+yo) % 64;
    for(x=0;x<WD_3D;x++) {
      xx = (x+xo) % 64;
      frBuf[SCR_WD*y+x] = ((xx<32 && yy<32) || (xx>32 && yy>32)) ? RGBto565(40,40,20) : RGBto565(80,80,40);
    }
  }
}

void backgroundPattern(int i, const unsigned short *pat)
{
  int x,y,xp,yp;
  xp = 25*fastSin(4*i)/256+50;  // 256 not MAXSIN=255 to avoid jumping at max sin value
  yp = 25*fastSin(5*i)/256+50+yFr;
  for(y=0;y<NLINES;y++) for(x=0;x<WD_3D;x++) frBuf[SCR_WD*y+x] = 0xf81f;// pat[((y+yp)&0x1f)*32 + ((x+xp)&0x1f)];
}

// ------------------------------------------------

typedef struct  {
  int16_t x,y,z;
  int16_t x2d,y2d, x2dOld,y2dOld;
}Star;

#define NUM_STARS 150
Star stars[NUM_STARS];
int starSpeed = 20;

void initStar(int i)
{
  stars[i].x = 1;// random(-500, 500);
  stars[i].y =1;// random(-500, 500);
  stars[i].z =1;// random(100, 2000);
  // remove stars from the center
  if(stars[i].x<80 && stars[i].x>-80) stars[i].x=80;
  if(stars[i].y<80 && stars[i].y>-80) stars[i].y=80;
}

int16_t rotZ = 1;

void updateStars()
{
  int16_t i,x,y;
  for(i=0; i<NUM_STARS; i++) {
    if(rotZ) {
      x = stars[i].x;
      y = stars[i].y;
      //stars[i].x = (x * fastCos(rotZ) - y * fastSin(rotZ))/MAXSIN;
      //stars[i].y = (y * fastCos(rotZ) + x * fastSin(rotZ))/MAXSIN;
      stars[i].x = (x*254 - y*2)/MAXSIN;
      stars[i].y = (y*254 + x*2)/MAXSIN;
    }

    stars[i].z -= starSpeed;
    stars[i].x2d = WD_3D/2 + 100 * stars[i].x / stars[i].z;
    stars[i].y2d = HT_3D/2 + 100 * stars[i].y / stars[i].z;

    if(stars[i].x2d>WD_3D || stars[i].x2d<0 || stars[i].y2d>HT_3D || stars[i].y2d<0) {
      initStar(i);
      stars[i].x2d = WD_3D/2 + 100 * stars[i].x / stars[i].z;
      stars[i].y2d = HT_3D/2 + 100 * stars[i].y / stars[i].z;
      stars[i].x2dOld = stars[i].x2d;
      stars[i].y2dOld = stars[i].y2d;
    }
  }
}

void initStars()
{
  for(int i=0; i<NUM_STARS; i++) initStar(i);
  updateStars();
  for(int i=0; i<NUM_STARS; i++) {
    stars[i].x2dOld = stars[i].x2d;
    stars[i].y2dOld = stars[i].y2d;
  }
}

void backgroundStars(int f)
{
  int i;
  for(i=0; i<NLINES*WD_3D; i++) frBuf[i] = BLACK;
  for(i=0; i<NUM_STARS; i++) {
    int r = 255-stars[i].z/5;
    //int r = 255-stars[i].z*stars[i].z/15000;
    if(r>255) r=255;
    if(r<40) r=40;
    uint16_t col = RGBto565(r,r,r);
    int x = stars[i].x2d;
    int y = stars[i].y2d - yFr;
    if(x>=0 && x<WD_3D && y>0 && y<NLINES) frBuf[SCR_WD*y+x] = col;
  }
}

// ------------------------------------------------

int t=0;

// mode=0 for quads, mode=1 for tris
void render3D(int mode, uint16_t *buf)
{
  int cos0,sin0,cos1,sin1;
  int i,x0,y0,z0,fac,distToObj;
  int camZ = 200;
  int scaleFactor = HT_3D/4;
  int near = 300;
  
 /* polyMode = 0;
   orient = 0;
   bfCull = 1;
  //lightShade = 1000;
      numVerts  = numVertsMonkey;
      verts     = (int16_t*)vertsMonkey;
      numPolys  = numTrisMonkey;
      polys     = (uint8_t*)trisMonkey;
      //polyColors = (uint16_t*)colsMonkey;
      lightShade = 20000;
      bfCull    = 1;
      orient    = 1;
      polyMode  = 1;
*/
      orient = 1;
      bfCull = 1;


      numVerts  = numgotverts;
      verts     = (int16_t*)vertgot;

      if(mode == 1) {
      numPolys  = numgottris;
      polys     = (uint8_t*)trigot;
      }

      else {
      numPolys  = numgotquads;
      polys     = (uint8_t*)quadgot;

      }
    //  polyColors = (uint16_t*)colsgot;
   //   bfCull    = 1;
      lightShade = 14000;
    //  bfCull    = 0;
      

    rot1 = 180; //upright
   // rot0 = 90;


  if(t++>360) t-=360;
  distToObj = 150 + 300*fastSin(3*t)/MAXSIN;
  cos0 = fastCos(rot0);
  sin0 = fastSin(rot0);
  cos1 = fastCos(rot1);
  sin1 = fastSin(rot1);

  for(i=0;i<numVerts;i++) {
    x0 = verts[3*i+0];
    y0 = verts[3*i+1];
    z0 = verts[3*i+2];
    //snprintf(txt,30,"[%d] %d %d %d",i,x0,y0,z0); Serial.println(txt);
    transVerts[3*i+0] = (cos0*x0 + sin0*z0)/MAXSIN;
    transVerts[3*i+1] = (cos1*y0 + (cos0*sin1*z0-sin0*sin1*x0)/MAXSIN)/MAXSIN;
    transVerts[3*i+2] = camZ + ((cos0*cos1*z0-sin0*cos1*x0)/MAXSIN - sin1*y0)/MAXSIN;

    fac = scaleFactor * near / (transVerts[3*i+2]+near+distToObj);

    projVerts[2*i+0] = (100*WD_3D/2 + fac*transVerts[3*i+0] + 100/2)/100;
    projVerts[2*i+1] = (100*HT_3D/2 + fac*transVerts[3*i+1] + 100/2)/100;
    //snprintf(txt,30,"[%d] %d %d",i,projVerts[2*i+0],projVerts[2*i+1]); Serial.println(txt);
  }

 // if(bgMode==3) updateStars();
  mode ? cullTris(transVerts) : cullQuads(transVerts);

  for(i=0;i<HT_3D;i+=NLINES) {
    yFr = i;
    backgroundPattern(t,pat2);// else
    //if(bgMode==1) backgroundPattern(t,pat8); else
  //  if(bgMode==2) backgroundPattern(t,pat7); else
  //  if(bgMode==3) backgroundStars(t); else
    //if(bgMode==4) backgroundChecker(t);
    mode ? drawTris(projVerts) : drawQuads(projVerts);

            for(int j = 0; j < NLINES; ++j)
      for(int k = 0; k < SCR_WD; ++k)
        buf[((i+j)*SCR_WD)+k] = frBuf[(j*SCR_WD)+k] == 0xf81f ? buf[((i+j)*SCR_WD)+k] : frBuf[(j*SCR_WD)+k];
   // lcd.drawImage(0,yFr,SCR_WD,NLINES,frBuf);
  }

  rot0 += 2;
  rot1 += 4;
  if(rot0>360) rot0-=360;
  if(rot1>360) rot1-=360;
} 

