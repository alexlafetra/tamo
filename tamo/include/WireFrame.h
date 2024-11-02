// const uint8_t orthoMatrix[2][3] = {{1,0,0},{0,1,0}};
// typedef unsigned char uint8_t;
// typedef unsigned short int uint16_t;

class Vertex{
  public:
  float x = 0;
  float y = 0;
  float z = 0;
  Vertex();
  Vertex(float,float,float);
  void rotate(float, uint8_t);
  void coordTransform(float transformer[3][3]);
};

Vertex::Vertex(){}
Vertex::Vertex(float x1, float y1, float z1){
  x = x1;
  y = y1;
  z = z1;
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
        break;
      default:
        break;
    }
  }
    x = x1;
    y = y1;
    z = z1;
}

//rotates a vertex around x (0) y (1) or z (2) axes
void Vertex::rotate(float angle, uint8_t axis){
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
    default:
      return;
  }
  coordTransform(rotationMatrix);
}

class WireFrame{
  public:
  //array of verts
  Vertex* verts  = nullptr;
  uint8_t numberOfVertices = 0;
  //array of vertex pairs [0,1] to form edges
  uint8_t** edges = nullptr;
  uint8_t numberOfEdges = 0;
  uint8_t xPos = 0;
  uint8_t yPos = 0;
  float scale = 1.0;
  WireFrame(){};
  WireFrame(uint8_t vertCount, Vertex* vertArray, uint8_t edgeCount, uint8_t edgeArray [][2]){
    numberOfVertices = vertCount;
    numberOfEdges = edgeCount;

    verts = new Vertex [numberOfVertices];
    for(uint8_t i = 0; i<numberOfVertices; i++){
      verts[i] = vertArray[i];
    }
    edges = new uint8_t* [numberOfEdges];
    for(uint8_t i = 0; i<numberOfEdges; i++){
      edges[i] = new uint8_t [2];
      edges[i] = edgeArray[i];
    }
  }
  //destructor
  // ~WireFrame(){
    // for(uint16_t e = 0; e<numberOfEdges; e++){
    //   delete [] edges[e];
    // }
    // delete [] edges;
    // delete [] verts;
  // }

  //copy constructor
  WireFrame(WireFrame& wf){
    numberOfVertices = wf.numberOfVertices;
    numberOfEdges = wf.numberOfEdges;

    verts = new Vertex [numberOfVertices];
    for(uint8_t i = 0; i<numberOfVertices; i++){
      verts[i] = wf.verts[i];
    }
    edges = new uint8_t* [numberOfEdges];
    for(uint8_t i = 0; i<numberOfEdges; i++){
      edges[i] = new uint8_t [2];
      edges[i][0] = wf.edges[i][0];
      edges[i][1] = wf.edges[i][1];
    }

    xPos = wf.xPos;
    yPos = wf.yPos;
    scale = wf.scale;
  }
  WireFrame& operator= (const WireFrame& wf){

    if(this == &wf)
      return *this;
    if(verts)
      delete [] verts;
    if(edges){
      for(uint8_t e = 0; e<numberOfEdges; e++){
        delete [] edges[e];
      }
      delete [] edges;
    }

    numberOfVertices = wf.numberOfVertices;
    numberOfEdges = wf.numberOfEdges;

    verts = new Vertex [numberOfVertices];
    for(uint8_t i = 0; i<numberOfVertices; i++){
      verts[i] = wf.verts[i];
    }
    edges = new uint8_t* [numberOfEdges];
    for(uint8_t i = 0; i<numberOfEdges; i++){
      edges[i] = new uint8_t [2];
      edges[i][0] = wf.edges[i][0];
      edges[i][1] = wf.edges[i][1];
    }

    xPos = wf.xPos;
    yPos = wf.yPos;
    scale = wf.scale;

    return *this;
  }

  void rotate(float angle, uint8_t axis){
    for(uint8_t vertex = 0; vertex<numberOfVertices; vertex++){
      verts[vertex].rotate(angle,axis);
    }
  }
};