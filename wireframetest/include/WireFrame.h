const uint8_t orthoMatrix[2][3] = {{1,0,0},{0,1,0}};

class Vertex{
  public:
  float x;
  float y;
  float z;
  Vertex();
  Vertex(float,float,float);
  void render(uint8_t, uint8_t, float);
  void render(uint8_t, uint8_t, float, uint8_t);
  void render(uint8_t, uint8_t, float, uint8_t, bool);
  void render(uint8_t, uint8_t,float, uint8_t, String);
  void rotate(float, uint8_t);
  void rotateAroundPoint(Vertex, float, uint8_t);
  void coordTransform(float transformer[3][3]);
  void moveTowardsPoint(Vertex v,float amount);
};

Vertex::Vertex(){
  x = 0;
  y = 0;
  z = 0;
}
Vertex::Vertex(float x1, float y1, float z1){
  x = x1;
  y = y1;
  z = z1;
}
void Vertex::rotateAroundPoint(Vertex v, float amt, uint8_t axis){
  x -= v.x;
  y -= v.y;
  z -= v.z;
  rotate(amt,axis);
  x += v.x;
  y += v.y;
  z += v.z;
}

void setPixel(uint8_t x, uint8_t y, uint16_t c){
  oled.setCursor(x,y);
	oled.startData();
  oled.sendData(0b10000000);
  oled.endData();
}

void Vertex::render(uint8_t xOffset,uint8_t yOffset, float scale){
  // display.fillCircle(x*scale+xOffset,y*scale+yOffset,2,SSD1306_WHITE);
}
void Vertex::render(uint8_t xOffset,uint8_t yOffset, float scale, uint8_t size){
  // if(size)
  //   display.fillCircle(x*scale+xOffset,y*scale+yOffset,size,SSD1306_WHITE);
  // else
  //   display.drawPixel(x*scale+xOffset,y*scale+yOffset,SSD1306_WHITE);
}

void Vertex::render(uint8_t xOffset,uint8_t yOffset, float scale, uint8_t size, bool full){
  // if(size){
  //   if(full)
  //     display.fillCircle(x*scale+xOffset,y*scale+yOffset,size,SSD1306_WHITE);
  //   else
  //     display.drawCircle(x*scale+xOffset,y*scale+yOffset,size,SSD1306_WHITE);
  // }
  // else{
    // display.drawPixel(x*scale+xOffset,y*scale+yOffset,1);
    setPixel(x*scale+xOffset,y*scale+yOffset,1);
  // }
}
void Vertex::render(uint8_t xOffset,uint8_t yOffset, float scale, uint8_t size, String text){
  // if(size)
  //   display.fillCircle(x*scale+xOffset,y*scale+yOffset,size,SSD1306_WHITE);
  // else
  //   display.drawPixel(x*scale+xOffset,y*scale+yOffset,SSD1306_WHITE);
}

void Vertex::moveTowardsPoint(Vertex v,float amount){
  x += (v.x-x)*amount;
  y += (v.y-y)*amount;
  z += (v.z-z)*amount;
}

//multiplies a vertex by a transformation matrix
void Vertex::coordTransform(float transformer[3][3]){
  float x1 = 0, y1 = 0, z1 = 0;
  //multiplying columns
  for(uint8_t a = 0; a<3; a++){
    float temp = transformer[a][0] * x + transformer[a][1] * y + transformer[a][2] * z;
    switch(a){
      case 0:
        x1 = temp;
        break;
      case 1:
        y1 = temp;
        break;
      case 2:
        z1 = temp;
    }
  }
    x = x1;
    y = y1;
    z = z1;
}

//rotates a vertex around x (0) y (1) or z (2) axes
void Vertex::rotate(float angle, uint8_t axis){
  // Vector<Vector<float>> rotationMatrix;
  float rotationMatrix[3][3];
  //convert to radians
  angle = angle*(PI/180.0);
  switch(axis){
    //x
    case 0:
      rotationMatrix[0][0] = 1;
      rotationMatrix[0][1] = 0;
      rotationMatrix[0][2] = 0;

      rotationMatrix[1][0] = 0;
      rotationMatrix[1][1] = cos(angle);
      rotationMatrix[1][2] = -sin(angle);

      rotationMatrix[2][0] = 0;
      rotationMatrix[2][1] = sin(angle);
      rotationMatrix[2][2] = cos(angle);
      break;
    //y
    case 1:
      rotationMatrix[0][0] = cos(angle);
      rotationMatrix[0][1] = 0;
      rotationMatrix[0][2] = sin(angle);

      rotationMatrix[1][0] = 0;
      rotationMatrix[1][1] = 1;
      rotationMatrix[1][2] = 0;

      rotationMatrix[2][0] = -sin(angle);
      rotationMatrix[2][1] = 0;
      rotationMatrix[2][2] = cos(angle);
      break;
    //z
    case 2:
      rotationMatrix[0][0] = cos(angle);
      rotationMatrix[0][1] = -sin(angle);
      rotationMatrix[0][2] = 0;

      rotationMatrix[1][0] = sin(angle);
      rotationMatrix[1][1] = cos(angle);
      rotationMatrix[1][2] = 0;

      rotationMatrix[2][0] = 0;
      rotationMatrix[2][1] = 0;
      rotationMatrix[2][2] = 1;
      break;
  }
  coordTransform(rotationMatrix);
}

//shorthand constructor
Vertex V(float x, float y){
  return Vertex(x,y,0);
}
Vertex V(float x, float y, float z){
  return Vertex(x,y,z);
}


// class WireFrame{
//   public:
//   float currentAngle[3];
//   Vector<Vertex> verts;
//   //this should also be a Vector of arrays instead of a vec of vecs
//   //bc each edge will only be between 2 points
//   Vector<Vector<uint16_t>>edges;//should be 16-bit so it can handle more than 256 verts (just in case)
//   Vector<uint16_t> dots;
//   uint8_t xPos = 0;
//   uint8_t yPos = 0;
//   uint8_t dotSize = 0;
//   float scale;
//   bool drawEdges = true;
//   bool drawDots = false;
//   WireFrame(){};
//   WireFrame(Vector<Vertex>);
//   WireFrame(Vector<Vertex>,Vector<Vector<uint8_t>>);
//   WireFrame(Vector<Vertex>,Vector<Vector<uint16_t>>);

//   void render();
//   void view();
//   void renderDie();
//   void renderDotsIfInFrontOf(float zCutoff);
//   void rotate(float,uint8_t);
//   void rotateRelativeTo(Vertex point, float amount,uint8_t axis);
//   void scaleRelativeTo(Vertex point, float amount);
//   void setRotation(float,uint8_t);
//   bool isFarthestVert(uint8_t);
//   bool isClosestVert(uint8_t);
//   uint8_t getFarthestVert();
//   uint8_t getClosestVert();
//   void reset(uint8_t);
//   void resetExceptFor(uint8_t);
//   void rotateVertRelative(uint8_t,float,uint8_t);
//   void join(WireFrame);
//   void addVerts(Vector<Vertex>);
//   void addEdges(Vector<Vector<uint16_t>>);
//   void move(float,float,float);
//   void moveTowardsPoint(Vertex v,float amount);
//   void scaleAxis(float amount, uint8_t axis);
//   void stitch(WireFrame);
//   void applyScale();
//   void rotateSpecificVertices(Vector<uint16_t> which, Vertex point, float amount, uint8_t axis);
// };
// WireFrame::WireFrame(Vector<Vertex> vertices){
//   verts = vertices;
//   scale = 1;
//   drawEdges = true;
//   drawDots = false;
// }
// WireFrame::WireFrame(Vector<Vertex> vertices,Vector<Vector<uint8_t>> edgeList){
//   verts = vertices;
//   //casting to 16-bit
//   Vector<Vector<uint16_t>> edgeVec16Bit;
//   for(int i = 0; i<edgeList.size(); i++){
//     Vector<uint16_t> tempVec = {uint16_t(edgeList[i][0]),uint16_t(edgeList[i][1])};
//     edgeVec16Bit.push_back(tempVec);
//   }
//   edges = edgeVec16Bit;
//   scale = 1;
//   drawEdges = true;
//   drawDots = false;
// }
// WireFrame::WireFrame(Vector<Vertex> vertices,Vector<Vector<uint16_t>> edgeList){
//   verts = vertices;
//   edges = edgeList;
//   scale = 1;
//   drawEdges = true;
//   drawDots = false;
// }
// void WireFrame::join(WireFrame w){

//   int16_t offset = verts.size();

//   //increment dot and edge indices, then add them to the mesh
//   for(uint16_t e = 0; e<w.edges.size(); e++){
//     edges.push_back({uint16_t(w.edges[e][0]+offset),uint16_t(w.edges[e][1]+offset)});
//   }
//   //adding dots
//   for(uint16_t d = 0; d<w.dots.size(); d++){
//     dots.push_back({uint16_t(w.dots[d]+offset)});
//   }
//   //finally, add verts
//   verts.insert(verts.end(),w.verts.begin(),w.verts.end());
// }

// void WireFrame::stitch(WireFrame w){
//   uint16_t limit = verts.size();
//   if(w.verts.size()<limit){
//     limit = w.verts.size();
//   }
//   uint8_t vertOffset = w.verts.size();
//   for(uint16_t i = 0; i < limit; i++){
//       edges.push_back({static_cast<unsigned short>(i+verts.size()-vertOffset),static_cast<unsigned short>(i+verts.size())});
//   }
//   join(w);
// }

// void WireFrame::rotateSpecificVertices(Vector<uint16_t> which, Vertex point, float amount, uint8_t axis){
//   for(uint8_t v : which){
//     verts[v].rotateAroundPoint(point,amount,axis);
//   }
// }

// void WireFrame::rotateRelativeTo(Vertex point, float amount,uint8_t axis){
//   for(uint16_t i = 0; i<verts.size(); i++){
//     verts[i].rotateAroundPoint(point,amount,axis);
//   }
// }

// void WireFrame::scaleRelativeTo(Vertex point, float amount){
//   for(uint16_t i = 0; i<verts.size(); i++){
//     verts[i].x -= point.x;
//     verts[i].y -= point.y;
//     verts[i].z -= point.z;

//     verts[i].x *= amount;
//     verts[i].y *= amount;
//     verts[i].z *= amount;

//     verts[i].x += point.x;
//     verts[i].y += point.y;
//     verts[i].z += point.z;
//   }
// }


// void WireFrame::move(float x1, float y1, float z1){
//   for(int i = 0; i<verts.size(); i++){
//     verts[i].x += x1;
//     verts[i].y += y1;
//     verts[i].z += z1;
//   }
// }
// void WireFrame::moveTowardsPoint(Vertex v,float amount){
//   for(int i = 0; i<verts.size(); i++){
//     verts[i].moveTowardsPoint(v,amount);
//   }
// }
// void WireFrame::addVerts(Vector<Vertex> v){
//   for(Vertex vert:v){
//       this->verts.push_back(vert);
//   }
// }
// void WireFrame::addEdges(Vector<Vector<uint16_t>> e){
//   for(Vector<uint16_t> edge:e){
//       this->edges.push_back(edge);
//   }
// }

// bool WireFrame::isFarthestVert(uint8_t which){
//   for(uint8_t v =0; v<verts.size(); v++){
//     if(verts[v].z<verts[which].z)
//       return false;
//   }
//   return true;
// }

// bool WireFrame::isClosestVert(uint8_t which){
//   for(uint8_t v = 0; v<verts.size(); v++){
//     //if any coordinate is closer, u know it's not the closest
//     if(verts[v].z>verts[which].z)
//       return false;
//   }
//   return true;
// }
// uint8_t WireFrame::getFarthestVert(){
//   uint8_t farthest = 0;
//   float lowestZ = verts[0].z;
//   for(uint8_t v = 1; v<verts.size(); v++){
//     if(verts[v].z<lowestZ){
//       farthest = v;
//       lowestZ = verts[v].z;
//     }
//   }
//   return farthest;
// }
// uint8_t WireFrame::getClosestVert(){
//   uint8_t closest = 0;
//   float highestZ = verts[0].z;
//   for(uint8_t v = 1; v<verts.size(); v++){
//     if(verts[v].z>highestZ){
//       closest = v;
//       highestZ = verts[v].z;
//     }
//   }
//   return closest;
// }

// void WireFrame::renderDie(){
//   //stores index of the farthest vertex
//   if(drawDots){
//     renderDotsIfInFrontOf(0.0);
//   }
//   if(drawEdges){
//     uint16_t farthestVert;
//     //loop thru each vertex and drW it if it's not the farthest one
//     for(uint16_t vertex = 0; vertex<verts.size(); vertex++){
//       if(isFarthestVert(vertex)){
//         farthestVert = vertex;
//         continue;
//       }
//     }
//     //draw edges
//     for(uint16_t edge = 0; edge<edges.size(); edge++){
//       //checking to see if this very is the farthest
//       //if it is, don't draw any lines
//       if(isFarthestVert(edges[edge][0]) || isFarthestVert(edges[edge][1])){
//         continue;
//       }
//       else
//         display.drawLine(verts[edges[edge][0]].x*scale+xPos,verts[edges[edge][0]].y*scale+yPos,verts[edges[edge][1]].x*scale+xPos,verts[edges[edge][1]].y*scale+yPos,SSD1306_WHITE);
//     }
//   }
// }

// void WireFrame::applyScale(){
//   for(uint16_t i = 0; i<verts.size(); i++){
//     verts[i].x *= scale;
//     verts[i].y *= scale;
//     verts[i].z *= scale;
//   }
//   scale = 1;
// }

// void WireFrame::render(){
//   if(drawDots){
//     for(uint16_t i = 0; i<dots.size(); i++){
//       verts[dots[i]].render(xPos,yPos,scale,dotSize);
//       //for labelling verts
//       // verts[dots[i]].render(xPos,yPos,scale,1,String(i));
//     }
//   }
//   //if it's just edges
//   if(drawEdges){
//     //draw edges
//     for(uint16_t edge = 0; edge<edges.size(); edge++){
//       Vertex v1 = verts[edges[edge][0]];
//       Vertex v2 = verts[edges[edge][1]];
//       display.drawLine(v1.x*scale+xPos,v1.y*scale+yPos,v2.x*scale+xPos,v2.y*scale+yPos,SSD1306_WHITE);
//     }
//   }
// }
// void WireFrame::view(){
//   while(true){
//     rotate(1,0);
//     rotate(0.2,1);
//     rotate(0.8,2);
//     display.clearDisplay();
//     render();
//     display.display();
//   }
// }
// void WireFrame::renderDotsIfInFrontOf(float zCutoff){
//   for(uint8_t i = 0; i<dots.size(); i++){
//     if(verts[dots[i]].z>zCutoff)
//       verts[dots[i]].render(xPos,yPos,scale,1);
//   }
// }

// //roates each vert
// void WireFrame::rotate(float angle, uint8_t axis){
//   for(uint8_t vertex = 0; vertex<verts.size(); vertex++){
//     verts[vertex].rotate(angle,axis);
//   }
//   currentAngle[axis] += angle;
//   // while(abs(currentAngle[axis]) >= 360.0){
//   //   currentAngle[axis] += currentAngle[axis]>0?-360:360;
//   // }
// }
// //resets verts rotation before and after applying the transformation
// void WireFrame::rotateVertRelative(uint8_t which, float angle, uint8_t axis){
//   //reset rotation
//   verts[which].rotate(-currentAngle[0],0);
//   verts[which].rotate(-currentAngle[1],1);
//   verts[which].rotate(-currentAngle[2],2);
  
//   //actual rotation
//   verts[which].rotate(angle,axis);

//   //reset rotation
//   verts[which].rotate(currentAngle[0],0);
//   verts[which].rotate(currentAngle[1],1);
//   verts[which].rotate(currentAngle[2],2);
// }
// //this one resets the rotation of the target axis, THEN applies the rotation
// void WireFrame::setRotation(float angle, uint8_t axis){
//   float oldAngles[3] = {currentAngle[0],currentAngle[1],currentAngle[2]};
//   resetExceptFor(axis);
//   rotate(angle,axis);
//   for(uint8_t i = 0; i<3; i++){
//     if(i != axis)
//       rotate(oldAngles[i],i);
//   }
// }

// //resets the rotation of the WF based on the stored angle
// void WireFrame::reset(uint8_t axis){
//   for(uint8_t vertex = 0; vertex<verts.size(); vertex++){
//     verts[vertex].rotate(-currentAngle[axis],axis);
//   }
// }
// void WireFrame::resetExceptFor(uint8_t exceptFor){
//   for(uint8_t vertex = 0; vertex<verts.size(); vertex++){
//     for(uint8_t axis = 0; axis<3; axis++){
//       if(axis != exceptFor)
//         verts[vertex].rotate(-currentAngle[axis],axis);
//     }
//   }
// }

// void WireFrame::scaleAxis(float amount, uint8_t axis){
//   for(uint8_t vertex = 0; vertex<verts.size(); vertex++){
//     switch(axis){
//       case 0:
//         verts[vertex].x *= amount;
//         break;
//       case 1:
//         verts[vertex].y *= amount;
//         break;
//       case 2:
//         verts[vertex].z *= amount;
//         break;
//     }
//   }
// }