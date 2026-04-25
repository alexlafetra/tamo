
  let undoBuffer = [];
  let redoBuffer = [];

  function pushUndoState(){
    //if the buffer gets long enough, start removing early entries
    if(undoBuffer.length > 200){
      undoBuffer.shift();
    }
    undoBuffer.push({
      spritesJSON:JSON.stringify(sprites),
      currentSprite:currentSprite,
      selectionBox:{
        started : selectionBox.started,
        active : selectionBox.active,
        startCoord:{...selectionBox.startCoord},
        endCoord:{...selectionBox.endCoord},
      }
    });
    // pixelSaveState.current = PixelFrame(spritesRef.current[currentSpriteRef.current].width,spritesRef.current[currentSpriteRef.current].height,spritesRef.current[currentSpriteRef.current].frames[spritesRef.current[currentSpriteRef.current].currentFrame].data);
    //adding to the undo buffer resets the redo buffer
    redoBuffer = [];
  }

  function restoreState(state){
    const restoredSprites = JSON.parse(state.spritesJSON);
    //rebuild sprites object from json
    const hydrated = restoredSprites.map(raw => {
      const restoredSprite = Sprite();
      restoredSprite.width = raw.width;
      restoredSprite.height = raw.height;
      restoredSprite.fileName = raw.fileName;
      restoredSprite.currentFrame = raw.currentFrame;
      restoredSprite.frames = raw.frames.map(frame => {
        const newFrame = PixelFrame(frame.width,frame.height,frame.data);
        return newFrame;
      });
      return restoredSprite;
    }) 
    sprites = [...hydrated];
    currentSprite = state.currentSprite;

    selectionBox.startCoord = {...state.selectionBox.startCoord};
    selectionBox.endCoord = {...state.selectionBox.endCoord};
    selectionBox.started = state.selectionBox.started;
    selectionBox.active = state.selectionBox.active;
    selectionBox.updateCSS();
    updateFramePreviews();
  }

  function undo(){
    if(undoBuffer.length === 0)
      return;
    const previousState = undoBuffer.pop();
    redoBuffer.push({
      spritesJSON:JSON.stringify(sprites),
      currentSprite:currentSprite,
      selectionBox:{
        started : selectionBox.started,
        active : selectionBox.active,
        startCoord:{...selectionBox.startCoord},
        endCoord:{...selectionBox.endCoord},
      }
    });
    restoreState(previousState);
  }

  function redo(){
    if(redoBuffer.length === 0)
      return;
    const nextState = redoBuffer.pop();
    undoBuffer.push({
      spritesJSON:JSON.stringify(sprites),
      currentSprite:currentSprite,
      selectionBox:{
        started : selectionBox.started,
        active : selectionBox.active,
        startCoord:{...selectionBox.startCoord},
        endCoord:{...selectionBox.endCoord},
      }
    });
    restoreState(nextState);
  }
