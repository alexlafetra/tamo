
let undoBuffer = [];
let redoBuffer = [];

function pushUndoState() {
  //if the buffer gets long enough, start removing early entries
  if (undoBuffer.length > 200) {
    undoBuffer.shift();
  }
  const spritesJSON = JSON.stringify(sprites);
  const state = {
    spritesJSON: spritesJSON,
    currentSprite: currentSprite,
    selectionBox: {
      started: selectionBox.started,
      active: selectionBox.active,
      startCoord: { ...selectionBox.startCoord },
      endCoord: { ...selectionBox.endCoord },
    }
  }
  undoBuffer.push(state);
  // pixelSaveState.current = PixelFrame(spritesRef.current[currentSpriteRef.current].width,spritesRef.current[currentSpriteRef.current].height,spritesRef.current[currentSpriteRef.current].frames[spritesRef.current[currentSpriteRef.current].currentFrame].data);
  //adding to the undo buffer resets the redo buffer
  redoBuffer = [];
  localStorage.setItem(settings.type, spritesJSON);
}

function parseAppStateJSON(jsonString){
  const restoredSprites = JSON.parse(jsonString);
  // console.log(restoredSprites);
  //rebuild sprites object from json
  const hydrated = restoredSprites.map(raw => {
    const restoredSprite = Sprite();
    restoredSprite.width = raw.width;
    restoredSprite.height = raw.height;
    restoredSprite.fileName = raw.fileName;
    restoredSprite.currentFrame = raw.currentFrame;
    restoredSprite.frames = raw.frames.map(frame => {
      const newFrame = PixelFrame(frame.width, frame.height, frame.data);
      return newFrame;
    });
    return restoredSprite;
  })
  return [...hydrated];
}

function restoreState(state){
  sprites = parseAppStateJSON(state.spritesJSON);
  currentSprite = state.currentSprite;

  selectionBox.startCoord = { ...state.selectionBox.startCoord };
  selectionBox.endCoord = { ...state.selectionBox.endCoord };
  selectionBox.started = state.selectionBox.started;
  selectionBox.active = state.selectionBox.active;
  selectionBox.updateCSS();
  // localStorage.setItem(settings.type, state.spritesJSON);
  updateFramePreviews();
}

function undo() {
  if (undoBuffer.length === 0)
    return;
  const previousState = undoBuffer.pop();
  const state = {
    spritesJSON: JSON.stringify(sprites),
    currentSprite: currentSprite,
    selectionBox: {
      started: selectionBox.started,
      active: selectionBox.active,
      startCoord: { ...selectionBox.startCoord },
      endCoord: { ...selectionBox.endCoord },
    }
  };
  redoBuffer.push(state);
  restoreState(previousState);
  localStorage.setItem(settings.type, state.spritesJSON);
}

function redo() {
  if (redoBuffer.length === 0)
    return;
  const nextState = redoBuffer.pop();
  const state = {
    spritesJSON: JSON.stringify(sprites),
    currentSprite: currentSprite,
    selectionBox: {
      started: selectionBox.started,
      active: selectionBox.active,
      startCoord: { ...selectionBox.startCoord },
      endCoord: { ...selectionBox.endCoord },
    }
  };
  undoBuffer.push(state);
  restoreState(nextState);
}
