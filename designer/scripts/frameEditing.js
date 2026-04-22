function rotateFrame(){
    pushUndoState();
    const sprite = sprites[currentSprite];
    const frame = sprite.frames[sprite.currentFrame];
    const newFrame = PixelFrame(sprite.width,sprite.height,0);
    for(let x = 0; x<sprite.width; x++){
        for(let y = 0; y<sprite.height; y++){
            newFrame.setPixel(x,y,frame.getPixel(y,x));
        }
    }
    sprite.frames[sprite.currentFrame] = newFrame;
    sprite.frames[sprite.currentFrame].mirror('horizontal');
    updateCanvas();
}

function clearFrame(){
  pushUndoState();
  sprites[currentSprite].frames[sprites[currentSprite].currentFrame] = PixelFrame(sprites[currentSprite].width,sprites[currentSprite].height,0);
  updateCanvas();
}

function invertFrame(){
  pushUndoState();
  const sprite = sprites[currentSprite];
  sprite.frames[sprite.currentFrame].invert();
  updateCanvas();
}
function mirrorHorizontally(){
  pushUndoState();
  const sprite = sprites[currentSprite];
  sprite.frames[sprite.currentFrame].mirror('horizontal');
  updateCanvas();
}
function mirrorVertically(){
  pushUndoState();
  const sprite = sprites[currentSprite];
  sprite.frames[sprite.currentFrame].mirror('vertical');
  updateCanvas();
}

function addNewFrame(){
    pushUndoState();
    const sprite = sprites[currentSprite];
    sprite.frames = [...sprite.frames,PixelFrame(sprite.width, sprite.height, 0)];
    sprite.currentFrame = sprite.frames.length-1;
    //trigger rerender to remake previews
    updateFrames();
}

function moveCurrentFrameBack(){
    const index = sprites[currentSprite].currentFrame;
    if(index){
        pushUndoState();
        [sprites[currentSprite].frames[index],sprites[currentSprite].frames[index-1]] = [sprites[currentSprite].frames[index-1],sprites[currentSprite].frames[index]];
        sprites[currentSprite].currentFrame = index-1;
        updateFrames();
    }
}
function moveCurrentFrameForward(){
    const index = sprites[currentSprite].currentFrame;
    if(index < sprites[currentSprite].frames.length - 1){
        pushUndoState();
        [sprites[currentSprite].frames[index],sprites[currentSprite].frames[index+1]] = [sprites[currentSprite].frames[index+1],sprites[currentSprite].frames[index]];
        sprites[currentSprite].currentFrame = index+1;
        updateFrames();
    }
}
function duplicateCurrentFrame(){
    pushUndoState();
    const sprite = sprites[currentSprite];
    const newFrame = PixelFrame(sprite.width, sprite.height, 0);
    
    //copy over all the data (can't copy object ref)
    for(let i = 0; i<newFrame.data.length; i++){
      newFrame.data[i] = sprite.frames[sprite.currentFrame].data[i];
    }
    const newFrames = [...sprite.frames];
    newFrames.splice(sprite.currentFrame,0,newFrame);
    sprite.currentFrame = sprite.currentFrame+1;
    sprite.frames = newFrames;
    //trigger rerender to recreate previews
    updateFrames();

}
function copyFrameToNextFrame(){
    const sprite = sprites[currentSprite];
    if(sprite.currentFrame < sprite.frames.length){
        pushUndoState();
        const newFrame = PixelFrame(sprite.width, sprite.height, sprite.frames[sprite.currentFrame].data);
        sprite.currentFrame = sprite.currentFrame+1;
        sprite.frames[sprite.currentFrame] = newFrame;
        //trigger rerender to recreate previews
        updateFrames();
    }
}
function deleteCurrentFrame(){
    pushUndoState();
    const sprite = sprites[currentSprite];
    //if there's >1 frame
    if(sprite.frames.length>1){
        const newFrames = sprite.frames.toSpliced(sprite.currentFrame,1);
        sprite.frames = newFrames;
        sprite.currentFrame = Math.min(sprite.currentFrame,sprite.frames.length-1);
        //trigger rerender to recreate previews
        updateFrames();
    }
}
function reverseFrames(){
    pushUndoState();
    sprites[currentSprite].frames.reverse();
    updateFrames();
}