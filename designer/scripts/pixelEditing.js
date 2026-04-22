
let offscreenBuffer = PixelFrame(16, 16, 0);
const line = {
  started: false,
  ended: false,
  start: { x: 0, y: 0 },
  end: { x: 0, y: 0 }
}
const copyBuffer = {
  pixels : undefined,
  bounds : {
    start : {x:0,y:0},
    end : {x:0,y:0}
  }
};

function copy(cut = false){
  if(selectionBox.active){
    if(cut)
      pushUndoState();
    const sprite = sprites[currentSprite];
    copyBuffer.bounds = {
      start: { x: Math.min(selectionBox.startCoord.x, selectionBox.endCoord.x), y: Math.min(selectionBox.startCoord.y, selectionBox.endCoord.y) },
      end: { x: Math.max(selectionBox.startCoord.x, selectionBox.endCoord.x), y: Math.max(selectionBox.startCoord.y, selectionBox.endCoord.y) }
    };
    copyBuffer.pixels = PixelFrame(selectionBox.getWidth(),selectionBox.getHeight(),0);
    for(let x = 0; x<copyBuffer.pixels.width; x++){
      for(let y = 0; y<copyBuffer.pixels.height; y++){
        copyBuffer.pixels.setPixel(x,y,sprite.frames[sprite.currentFrame].getPixel(x + copyBuffer.bounds.start.x, y + copyBuffer.bounds.start.y));
        if(cut){
          sprite.frames[sprite.currentFrame].setPixel(x + copyBuffer.bounds.start.x, y + copyBuffer.bounds.start.y,0);
        }
      }
    }
    if(cut)
      updateCanvas();
  }
}

function cut(){
  copy(true);
}

function paste(){
  if(copyBuffer.pixels && currentMouseCoords){

    const sprite = sprites[currentSprite];
    pushUndoState();
    for(let x = 0; x<copyBuffer.pixels.width; x++){
      for(let y = 0; y<copyBuffer.pixels.height; y++){
        const pixelVal = copyBuffer.pixels.getPixel(x,y) || sprite.frames[sprite.currentFrame].getPixel(x +currentMouseCoords.x, y + currentMouseCoords.y);
        sprite.frames[sprite.currentFrame].setPixel(x + currentMouseCoords.x, y + currentMouseCoords.y,pixelVal);
      }
    }
    const w = Math.abs(copyBuffer.bounds.start.x - copyBuffer.bounds.end.x);
    const h = Math.abs(copyBuffer.bounds.start.y - copyBuffer.bounds.end.y);
    //set selection box
    selectionBox.active = true;
    selectionBox.started = true;
    selectionBox.startCoord = {
      x:currentMouseCoords.x,
      y:currentMouseCoords.y,
    };
    selectionBox.endCoord = {
      x:currentMouseCoords.x + w,
      y:currentMouseCoords.y + h,
    };
    selectionBox.updateCSS();
    updateCanvas();
  }
}

const selectionBox = {
  startCoord:{x:0,y:0},
  endCoord:{x:0,y:0},
  started:false,
  active:false,
  start:function(coords){
    this.active = false;
    this.started = true;
    this.startCoord = {x:coords.x_rounded,y:coords.y_rounded};
    this.endCoord = {x:coords.x_rounded,y:coords.y_rounded};
    this.updateCSS();
  },
  update:function(coords){
    this.endCoord = {x:coords.x_rounded,y:coords.y_rounded};
    this.updateCSS();
  },
  end:function(coords){
    //if an area with a dimension less than 1 was selected, cancel the box
    if (Math.abs(coords.x_rounded - this.startCoord.x) < 1 || Math.abs(coords.y_rounded - this.startCoord.y) < 1) {
      this.active = false;
      this.started = false;
    }
    else {
      this.active = true;
      this.started = false;
      this.endCoord = { x: coords.x_rounded, y: coords.y_rounded };
    }
    this.updateCSS();
  },
  selectAll:function(){
    this.active = true;
    this.started = false;
    this.startCoord = {x:0,y:0};
    this.endCoord = {x:sprites[currentSprite].width,y:sprites[currentSprite].height};
    this.updateCSS();
  },
  getWidth:function(){
      return Math.abs(this.startCoord.x-this.endCoord.x);
  },
  getHeight:function(){
      return Math.abs(this.startCoord.y-this.endCoord.y);
  },
  getOffsetLeft:function(){
      return Math.min(this.startCoord.x,this.endCoord.x);
  },
  getOffsetTop:function(){
      return Math.min(this.startCoord.y,this.endCoord.y);
  },
  updateCSS:function(){
    document.documentElement.style.setProperty('--selection-box-visibility', (this.started||this.active)?'visible':'hidden');
    document.documentElement.style.setProperty('--selection-box-width', `${this.getWidth()}px`);
    document.documentElement.style.setProperty('--selection-box-height', `${this.getHeight()}px`);
    document.documentElement.style.setProperty('--selection-box-start-x', `${this.getOffsetLeft()}px`);
    document.documentElement.style.setProperty('--selection-box-start-y', `${this.getOffsetTop()}px`); 
  }
};

const move = {
  started : false,
  fromSelectionBox : false,
  //these are the main canvas coords you started moving from
  startingBounds : {
    start : {x : 0, y: 0},
    end : {x : 0, y: 0},
  },
  translatedBounds : {
    start : {x : 0, y: 0},
    end : {x : 0, y: 0},
  },
  offsetFromMouse : {
    x : 0,
    y : 0
  },
  moveBuffer : undefined,
  width:function(){
    return (this.startingBounds.end.x - this.startingBounds.start.x);
  },
  height:function(){
    return (this.startingBounds.end.y - this.startingBounds.start.y);
  },
  start : function(coords){
    pushUndoState();
    const sprite = sprites[currentSprite];
    this.started = true;
    //if there's a selectionbox, use that as the startingBounds
    if(selectionBox.active){
      this.fromSelectionBox = true;
      this.startingBounds = {
        start: { x: Math.min(selectionBox.startCoord.x, selectionBox.endCoord.x), y: Math.min(selectionBox.startCoord.y, selectionBox.endCoord.y) },
        end: { x: Math.max(selectionBox.startCoord.x, selectionBox.endCoord.x), y: Math.max(selectionBox.startCoord.y, selectionBox.endCoord.y) }
      };
    }
    else{
      this.startingBounds = {
        start: {x: 0, y: 0},
        end: { x: sprites[currentSprite].width, y: sprites[currentSprite].height }
      };
    }

    this.offsetFromMouse = {
      x : this.startingBounds.start.x - coords.x_rounded,
      y : this.startingBounds.start.y - coords.y_rounded
    };

    this.translatedBounds = {
      start : {...this.startingBounds.start},
      end : {...this.startingBounds.end},
    };

    this.moveBuffer = PixelFrame(Math.abs(this.startingBounds.start.x - this.startingBounds.end.x),Math.abs(this.startingBounds.start.y-this.startingBounds.end.y),0);
    offscreenBuffer = PixelFrame(sprite.width, sprite.height, sprite.frames[sprite.currentFrame].data);
    //copy area into offscreen buffer
    for(let x = 0; x<this.moveBuffer.width; x++){
      for(let y = 0; y<this.moveBuffer.height; y++){
        this.moveBuffer.setPixel(x,y,sprite.frames[sprite.currentFrame].getPixel(x + this.startingBounds.start.x, y+ this.startingBounds.start.y));
        //clear out area to be moved from offscreen buffer
        offscreenBuffer.setPixel(x + this.startingBounds.start.x, y + this.startingBounds.start.y,0);
      }
    }
  },
  update:function(coords){

    const sprite = sprites[currentSprite];

    this.translatedBounds = {
      start : {x:coords.x_rounded + this.offsetFromMouse.x ,y:coords.y_rounded + this.offsetFromMouse.y},
      end : {x:coords.x_rounded + this.width() + this.offsetFromMouse.x,y:coords.y_rounded + this.height() + this.offsetFromMouse.y},
    };

    //reset current frame to the offscreen buffer
    sprite.frames[sprite.currentFrame] = PixelFrame(sprite.width,sprite.height,offscreenBuffer.data);
    //clear out main canvas and overlay this canvas
    for(let x = 0; x<this.moveBuffer.width; x++){
      for(let y = 0; y<this.moveBuffer.height; y++){
        //overwrite with new
        const pixelVal = this.moveBuffer.getPixel(x,y) || sprite.frames[sprite.currentFrame].getPixel(x + this.translatedBounds.start.x, y + this.translatedBounds.start.y);
        sprite.frames[sprite.currentFrame].setPixel(x + this.translatedBounds.start.x, y + this.translatedBounds.start.y, pixelVal);
      }
    }
    if(this.fromSelectionBox){
      selectionBox.startCoord = {x:this.translatedBounds.start.x,y:this.translatedBounds.start.y};
      selectionBox.endCoord =   {x:this.translatedBounds.end.x,y:this.translatedBounds.end.y};
      selectionBox.updateCSS();
    }
    updateCanvas();
  },
  end:function(){
    this.started = false;
  }
};

function handleMouseEnter(e){
  setMouseCoordDisplay(e);
  document.documentElement.style.setProperty('--cursor-visibility', 'visible');
}

function handleMouseOut(e) {
  setTooltip(settings.currentTool);
  currentMouseCoords = undefined;
  document.documentElement.style.setProperty('--cursor-visibility', 'hidden');
  switch(settings.currentTool){
    case 'move':
      move.end();
      break;
  }
}

function startLine(coords){
  const sprite = sprites[currentSprite];
  pushUndoState();
  line.start = { ...coords };
  //make a backup of the line
  offscreenBuffer = PixelFrame(sprite.width, sprite.height, sprite.frames[sprite.currentFrame].data);
  line.started = true;
}
function continueLine(coords){
  const sprite = sprites[currentSprite];
  sprite.frames[sprite.currentFrame] = PixelFrame(offscreenBuffer.width, offscreenBuffer.height, offscreenBuffer.data);
  sprite.frames[sprite.currentFrame].drawLine(line.start.x, line.start.y, coords.x, coords.y, settings.currentColor);
  updateCanvas();
}
function endLine(coords){
  const sprite = sprites[currentSprite];
  sprite.frames[sprite.currentFrame] = PixelFrame(offscreenBuffer.width, offscreenBuffer.height, offscreenBuffer.data);
  sprite.frames[sprite.currentFrame].drawLine(line.start.x, line.start.y, coords.x, coords.y, settings.currentColor);
  line.started = false;
  updateCanvas();
}
function cancelLine(){
  const sprite = sprites[currentSprite];
  sprite.frames[sprite.currentFrame] = PixelFrame(offscreenBuffer.width, offscreenBuffer.height, offscreenBuffer.data);
  line.started = false;
  updateCanvas();
}

function handleMouseDown(e) {
  const coords = getClickCoords(e);
  e.preventDefault();
  const sprite = sprites[currentSprite];
  switch (settings.currentTool) {
    case 'pixel': {
      pushUndoState();
      const newFrames = sprite.frames;
      newFrames[sprite.currentFrame].setPixel(coords.x, coords.y, settings.currentColor);
      updateCanvas();
      break;
    }
    case 'fill': {
      pushUndoState();
      const newFrames = sprite.frames;
      newFrames[sprite.currentFrame].fill(coords.x, coords.y, settings.currentColor);
      updateCanvas();
      break;
    }
    case 'line':
      //if you haven't started drawing a line yet
      if (!line.started) {
        startLine(coords);
      }
      break;
    case 'move':
      if (!move.started) {
        move.start(coords);
      }
      break;
    case 'select':
      selectionBox.start(coords);
      break;
  }
}

function handleMouseMove(e) {
  const coords = getClickCoords(e);
  currentMouseCoords = {...coords};
  setMouseCoordDisplay(e);
  e.preventDefault();
  //detect if the mouse button is held down (necessary for dragging)
  if (e.buttons || (e.type == "touchmove")) {
    if (e.type == "touchmove" && e.touches.length > 1) {
      return;
    }
    const sprite = sprites[currentSprite];
    switch (settings.currentTool) {
      case 'pixel':
        sprite.frames[sprite.currentFrame].setPixel(coords.x, coords.y, settings.currentColor);
        updateCanvas();
        break;
      case 'line':
        //if you've already started a line, draw it
        if (line.started) {
          continueLine(coords);
        }
        else{
          startLine(coords);
        }
        break;
      case 'fill': {
        sprite.frames[sprite.currentFrame].fill(coords.x, coords.y, settings.currentColor);
        updateCanvas();
      }
        break;
      case 'move':
        if (move.started) {
          move.update(coords);
        }
        break;
      case 'select':
        if(selectionBox.started) {
          selectionBox.update(coords);
        }
        else   
          selectionBox.start(coords);
        break;
    }
  }
}

function handleMouseUp(e) {
  const coords = getClickCoords(e);
  const sprite = sprites[currentSprite];
  switch (settings.currentTool) {
    case 'line':
      if(line.started){
        endLine(coords);
      }
      break;
    case 'move':
      if(move.started){
        move.end();
      }
      break;
    case 'select':
      if (selectionBox.started && !selectionBox.active) {
        selectionBox.end(coords);
      }
      break;
  }
}