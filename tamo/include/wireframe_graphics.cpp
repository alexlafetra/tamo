#include "WireFrame.h"
#include "fbo.h"

FrameBuffer fbo(18,16);

const Vertex verts[9] = {
  //outline
  Vertex(-2.5,-1.5,0),Vertex(2.5,-1.5,0),Vertex(2.5,1.5,0),Vertex(-2.5,1.5,0),
  //triangle tip
  Vertex(-1,0,0),
  //stripes
  Vertex(-1.25,-0.25,0),Vertex(2.5,-0.25,0),Vertex(-1.25,0.25,0),Vertex(2.5,0.25,0)
};

const uint8_t edges[8][2] = {
  //rect
  {0,1},{1,2},{2,3},{3,0},
  //triangle
  {0,4},{4,3},
  //stripes
  {5,6},{7,8}
};

WireFrame flag(9,verts,8,edges);

void initWireFrame(){
  flag.scale = 3.0;
  flag.xPos = 7;
  flag.yPos = 8;
  flag.rotate(15,0);
}