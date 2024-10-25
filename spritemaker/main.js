class Sprite{
    constructor(){
        this.width = 16;
        this.height = 16;
        this.pixelData = [];
        for(let i = 0; i<16; i++){
            let row = [];
            for(let j = 0; j<16; j++){
                row[j] = 0;
            }
            this.pixelData[i] = row;
        }
        this.lastPixelClicked = {x:0,y:0};
    }
    renderBig(alpha,grid){
        stroke(0);
        if(grid)
            strokeWeight(1);
        else
            noStroke();
        this.render(0,0,dim,dim,alpha);
    }
    renderSmall(xLoc){
        this.render(xLoc,dim,dim/8,dim/8);
    }
    render(x,y,w,h,alpha){
        const scaleX = w/16;
        const scaleY = h/16;
        for(let i = 0; i<16; i++){
            for(let j = 0; j<16; j++){
                if(this.pixelData[i][j])
                    fill(255,255,255,alpha);
                else
                    continue;
                rect(x+i*scaleX,y+j*scaleY,scaleX,scaleY);
            }
        }
    }
    click(){
        const loc = this.getMouseLocation();
        if(loc.x == this.lastPixelClicked.x && loc.y == this.lastPixelClicked.y)
            return;
        this.pixelData[loc.x][loc.y] = !this.pixelData[loc.x][loc.y];
        this.lastPixelClicked.x = loc.x;
        this.lastPixelClicked.y = loc.y;
    }
    getMouseLocation(){
        const scale = dim/16;
        const xLoc = int(mouseX/scale);
        const yLoc = int(mouseY/scale);
        return {x:xLoc,y:yLoc};
    }
    getBytes(){
        const bytes = new Uint8Array(32);
        for(let i = 0; i < (16*16); i++){
            const whichByte = int(i/8);
            const whichBit = i%8;
            const x = i%16;
            const y = int(i/16);

            bytes[whichByte] = bytes[whichByte] | (this.pixelData[x][y] << whichBit);
        }
        return bytes;
    }
    saveImage(title){
        let img = createImage(16,16);
        img.loadPixels();
        for(let i = 0; i<16; i++){
            for(let j = 0; j<16; j++){
                img.set(i,j,this.pixelData[i][j]?[255,255,255,255]:[0,0,0,0]);
            }
        }
        img.updatePixels();
        img.save(mainTitle+'_frame'+title+'.bmp', 'bmp');
    }
};

class SpriteSheet{
    constructor(){
        this.activeSpriteID = 0;
        this.sprites = [new Sprite()];
        this.playing = false;
        this.ghosting = true;
        this.grid = true;
    }
    render(){
        background(100,255,255);
        fill(0);
        noStroke();
        rect(0,0,dim,dim);
        this.sprites[this.activeSpriteID].renderBig(255,this.grid);
        if(this.ghosting && this.sprites.length > 1){
            let prev = this.activeSpriteID;
            if(this.activeSpriteID)
                prev = this.activeSpriteID-1;
            else
                prev = this.sprites.length-1;
            this.sprites[prev].renderBig(100,this.grid);
        }
        for(let i = 0; i<this.sprites.length; i++){
            fill(0);
            noStroke();
            rect(i*dim/8,dim,dim/8,dim/8);
            this.sprites[i].renderSmall(i*mainCanvas.width/8);
            if(i == this.activeSpriteID){
                stroke(255,0,0);
                strokeWeight(2);
                noFill();
                rect(i*dim/8,dim,dim/8,dim/8);
            }
        }
    }
    copyPixels(from,to){
        for(let i = 0; i<16; i++){
            for(let j = 0; j<16; j++){
                this.sprites[to].pixelData[i][j] = this.sprites[from].pixelData[i][j];
            }
        }
    }
    getBytes(){
        //big string
        let str = "";
        let frameNumber = 0;
        for(let s of this.sprites){
            let littleStr = "const unsigned char "+mainTitle+"_frame"+frameNumber+" [] PROGMEM = {";
            const arr = s.getBytes();
            for(let n = 0; n<arr.length; n++){
                littleStr+=arr[n];
                if(n != arr.length-1)
                    littleStr+=", ";
            }
            littleStr+="};"
            frameNumber++;
            str+=littleStr+"\n";
        }
        str += "const unsigned char* "+mainTitle+"["+frameNumber+"] = {";
        for(let i = 0; i<frameNumber; i++){
            str+= mainTitle+"_frame"+i;
            if(i!=frameNumber-1)
                str+=",";
        }
        str+="};";
        return str;
    }
}

let spriteSheet;
let mainTitle = "anim";
let mainCanvas;
let resetButton,clearButton,addToListButton,playButton,speedSlider,toggleGhostingButton,dupeButton,getBytesButton,toggleGridButton;
let saveImageButton,saveAllButton,deleteButton;
const dim = 400;

function mousePressed(){
    if(mouseY>mainCanvas.height)
        return;
    if(mouseY<=dim)
        spriteSheet.sprites[spriteSheet.activeSpriteID].click();
    else{
        let id = int(mouseX/(dim/8));
        if(id<spriteSheet.sprites.length)
            spriteSheet.activeSpriteID = id;
    }
}
function mouseDragged(){
    spriteSheet.sprites[spriteSheet.activeSpriteID].click();
}

function clearCurrentCanvas(){
    spriteSheet.sprites[spriteSheet.activeSpriteID] = new Sprite();
}
function resetAll(){
    spriteSheet.activeSpriteID = 0;
    spriteSheet.sprites = [new Sprite()];
}

function setup(){
    mainCanvas = createCanvas(dim,dim+dim/8);
    background(100,255,255);
    spriteSheet = new SpriteSheet();
    clearButton = createButton('Clear');
    clearButton.mousePressed(clearCurrentCanvas);
    resetButton = createButton('Reset All');
    resetButton.mousePressed(resetAll);
    addToListButton = createButton('Add');
    addToListButton.mousePressed(() => {
        spriteSheet.activeSpriteID = spriteSheet.sprites.length;
        spriteSheet.sprites[spriteSheet.activeSpriteID] = new Sprite();
    });
    saveImageButton = createButton('Detele');
    saveImageButton.mousePressed(() => {
        spriteSheet.sprites[spriteSheet.activeSpriteID].saveImage(spriteSheet.activeSpriteID);
    });
    dupeButton = createButton('Duplicate');
    dupeButton.mousePressed(() => {
        spriteSheet.activeSpriteID++;
        spriteSheet.sprites[spriteSheet.activeSpriteID] = new Sprite();
        spriteSheet.copyPixels(spriteSheet.activeSpriteID-1,spriteSheet.activeSpriteID);
    });
    playButton = createButton('Play');
    playButton.mousePressed(() => {
        spriteSheet.playing = !spriteSheet.playing;
        playButton.html(spriteSheet.playing?"Stop":"Play");
    });
    speedSlider = createSlider(1, 50, 25, 1);
    toggleGhostingButton = createButton('Ghosting: On');
    toggleGhostingButton.mousePressed(() => {
        spriteSheet.ghosting = !spriteSheet.ghosting;
        toggleGhostingButton.html('Ghosting: '+(spriteSheet.ghosting?"On":"Off"));
    });
    toggleGridButton = createButton('Grid: On');
    toggleGridButton.mousePressed(() => {
        spriteSheet.grid = !spriteSheet.grid;
        toggleGridButton.html('Grid: '+(spriteSheet.grid?"On":"Off"));
    });
    getBytesButton = createButton('Get Bytes');
    getBytesButton.mousePressed(() => {
        const bytes = spriteSheet.getBytes();
        document.getElementById("outputText").innerHTML = bytes;
    });
    saveImageButton = createButton('Save Image');
    saveImageButton.mousePressed(() => {
        spriteSheet.sprites[spriteSheet.activeSpriteID].saveImage(spriteSheet.activeSpriteID);
    });


}
function draw(){
    spriteSheet.render();
    if(spriteSheet.playing && !(frameCount%speedSlider.value())){
        spriteSheet.activeSpriteID++;
        spriteSheet.activeSpriteID%=spriteSheet.sprites.length;
    }
}