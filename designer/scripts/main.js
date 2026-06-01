
let sprites = [
  Sprite('idle', 16, 16),
  // Sprite('happy'),
  // Sprite('angry'),
  // Sprite('sad'),
  // Sprite('eating')
];


const settings = {
  canvasScale : 15,
  currentTool: 'pixel',
  currentColor: 1,
  lineStarted: false,
  moveStarted: false,
  overlayGhosting: true,
  showGrid:true,
  foregroundColor: '#ffffff',
  backgroundColor: '#000000',
  frameSpeed: 500,
  maxCanvasDimension: 128,
  useAlphaAsBackground: false, //treat empty areas as though they're a color
  resizeCanvasToImage: false,
  createSpritesByFileName: true,
  showGrid: true,
  maxFrames: 2,
  outputColors : {
    foregroundColor:'#ffffff',
    backgroundColor:'#000000'
  },
  type : 'sprite'
};


function loadDefaultSpritesFromJSON(){
  //rebuild sprites object from json
  sprites = [];
  defaultSprites.map(raw => {
    const sprite = Sprite();
    sprite.width = raw.width;
    sprite.height = raw.height;
    sprite.fileName = raw.fileName;
    sprite.currentFrame = raw.currentFrame;
    sprite.frames = [];
    raw.frames.map(frame => {
      sprite.frames.push(PixelFrame(frame.width,frame.height,frame.data));
    });
    sprites.push(sprite);
  });
}

function loadApp(){
  //grabbing some visual settings from url
  const urlParams = new URLSearchParams(window.location.search);//uses the ? string following the url
  if(urlParams.has('sprite')){
    settings.type = 'sprite';
  }
  else if(urlParams.has('slideshow')){
    settings.type = 'slideshow';
    setImageUploadType('image');
  }
  let spritesJSON = localStorage.getItem(settings.type);
  if(spritesJSON != undefined){
    sprites = parseAppStateJSON(spritesJSON);
  }
  else{
    loadTemplate(templates[settings.type]);
  }
  reloadSpritePreviews();
  updateFramePreviews();
  setTool('pixel');
}

//resets the full app
function resetSprites(){
  pushUndoState();
  sprites = [Sprite('idle', 16, 16)];
  currentSprite = 0;
  reloadSpritePreviews();
  updateFramePreviews();
}

let presetSpriteNames = [
  'idle',
  'happy',
  'mad',
  'sad',
  'eating',
  // 'misc.'
];

const templates = {
  sprite: {
    defaultName: 'tamo',
    presetSpriteNames: [
      'idle',
      'happy',
      'mad',
      'sad',
      'eating',
      'misc.'
    ],
    width: 16,
    height: 16,
    frames: 2,
    maxFrames: 2
  },
  thought: {
    defaultName: 'love',
    presetSpriteNames: [
      'thinking',
    ],
    width: 7,
    height: 7,
    frames: 2,
    maxFrames: 3
  },
  food: {
    defaultName: 'apple',
    presetSpriteNames: [
      'food'
    ],
    width: 16,
    height: 13,
    frames: 5,
    maxFrames: 5
  },
  slideshow : {
    defaultName : 'slideshow',
    presetSpriteNames : [
      'slideshow'
    ],
    width:64,
    height:32,
    frames: 2,
    maxFrames : 10,
    defaultCanvasScale : 6
  }
}

let currentMouseCoords = {
  x : 0,
  y : 0
};

let currentSprite = 0;
let timeoutID = undefined;
let spriteName = "new sprite";
let settingsShown = false;
let resizeDimensions = {
  width : sprites[currentSprite].width,
  height : sprites[currentSprite].height,
  alignment : 'center',
}


const imageUploadSettings = {
  type : 'sprite', //'sprite' or 'image'
  fit : 'width',
  render : 'atkinson',
  brightness : 1.0,
  dataURL : null
}


function hideResizePreview(){
  document.documentElement.style.setProperty('--resize-box-visibility','hidden');
}
function showResizePreview(){
  document.documentElement.style.setProperty('--resize-box-visibility','visible');
  updateResizePreview();
}
function updateResizePreview(){
  let x,y;
  if(resizeDimensions.alignment == 'center'){
    x = Math.round((sprites[currentSprite].width - resizeDimensions.width)/2);
    y = Math.round((sprites[currentSprite].height - resizeDimensions.height)/2);
  }
  else if(resizeDimensions.alignment == 'top left'){
    x = 0;
    y = 0;
  }
  document.documentElement.style.setProperty('--resize-box-width', `${resizeDimensions.width}px`);
  document.documentElement.style.setProperty('--resize-box-height', `${resizeDimensions.height}px`);
  document.documentElement.style.setProperty('--resize-box-start-x', `${x}px`);
  document.documentElement.style.setProperty('--resize-box-start-y', `${y}px`); 
}

function updateResizeSliders(){
  document.getElementById("width_input").value = sprites[currentSprite].width;
  document.getElementById("height_input").value = sprites[currentSprite].height;
  updateResizePreview();
}

function setResizeDimensionWidth(e){
  const val = e.target.value;
  if(val == undefined || val == null)
    val = 0;
  resizeDimensions.width = val;
  updateResizePreview();
}
function setResizeDimensionHeight(e){
  const val = e.target.value;
  if(val == undefined || val == null)
    val = 0;
  resizeDimensions.height = val;
  updateResizePreview();
}
function setResizeAlignment(e){
  resizeDimensions.alignment = e.target.value;
  updateResizePreview();
}
function resizeAllSprites(){
  for(let sprite of sprites){
    sprite.resize(Math.max(resizeDimensions.width,1),Math.max(resizeDimensions.height,1),resizeDimensions.alignment);
  }
  updateResizeSliders();
  reloadFramePreviews();
  reloadSpritePreviews();
  updateCanvas();
}

function setImageUploadTypeEvent(e){
  e.stopImmediatePropagation();
  e.preventDefault();

  setImageUploadType(e.target.innerText);
}

function setImageUploadType(val){
  const old = document.getElementById(`${imageUploadSettings.type}_upload_type_button`);
  imageUploadSettings.type = val;
  const newButton = document.getElementById(`${imageUploadSettings.type}_upload_type_button`);
  old.style.color = null;
  old.style.backgroundColor = null;
  newButton.style.color = 'yellow';
  newButton.style.backgroundColor = 'var(--button-highlight-color)';
}

function setUploadFit(fit){
  const old = document.getElementById(`upload_fit_${imageUploadSettings.fit}`);
  old.style.backgroundColor = null;
  old.style.color = null;
  imageUploadSettings.fit = fit;
  const newElement = document.getElementById(`upload_fit_${imageUploadSettings.fit}`);
  newElement.style.backgroundColor = 'var(--button-highlight-color)';
  newElement.style.color = "yellow";
  renderPreviewImage();
}

function setRenderAlgorithm(algo){
  const old = document.getElementById(`${imageUploadSettings.render}_algorithm_button`);
  old.style.background = null;
  old.style.color = null
  imageUploadSettings.render = algo;
  const newButton = document.getElementById(`${imageUploadSettings.render}_algorithm_button`);
  newButton.style.background = 'blue';
  newButton.style.color = 'yellow';
  renderPreviewImage();
}
function setRenderBrightness(event){
  imageUploadSettings.brightness = parseFloat(event.target.value);
  renderPreviewImage();
}

function updateUploadProgressBar(percent,dotCounter){
  let text;
  let color = 'var(--button-highlight-color)';
  if(percent>=100){
    text = ' sent sprites!';
    color = 'green';
  }
  else{
    text = ' uploading';
    for(let i = 0; i<Math.round(percent/(100/3)); i++){
      text += '.';
    }
  }
  document.getElementById("upload_progress_bar").innerText = text;
  document.documentElement.style.setProperty('--progress-bar-display','block');
  document.documentElement.style.setProperty('--progress-bar-color', color);
  document.documentElement.style.setProperty('--upload-progress', `${percent}%`);
}

function loadTemplate(template) {
  pushUndoState();
  spriteName = template.defaultName;
  document.getElementById("sprite_name_static").innerHTML = spriteName;
  presetSpriteNames = [...template.presetSpriteNames];
  settings.maxFrames = template.maxFrames;
  sprites = [
    Sprite(template.presetSpriteNames[0], template.width, template.height)
  ];
  //add in frames to match the amount the template needs
  for (let i = sprites[0].frames.length; i < template.frames; i++) {
    sprites[0].frames.push(PixelFrame(template.width, template.height, 0));
  }
  document.documentElement.style.setProperty('--sprite-width', `${template.width}px`);
  document.documentElement.style.setProperty('--sprite-height', `${template.height}px`);
  document.documentElement.style.setProperty('--background-width', `${100 / template.width}%`);
  document.documentElement.style.setProperty('--background-height', `${100 / template.height}%`);
  if(template.defaultCanvasScale){
    document.documentElement.style.setProperty('--canvas-scale', `${template.defaultCanvasScale}`);
    settings.canvasScale = template.defaultCanvasScale;
  }
  reloadSpritePreviews();
  updateFramePreviews();
}

function loadSelectedTemplate(event) {
  loadTemplate(templates[event.target.value]);
}


function setAnimationSpeed(event) {
  //set new speed
  settings.frameSpeed = parseInt(event.target.value);
  document.getElementById("frame_speed_label").innerText = `${settings.frameSpeed}ms`;
  //clear old timeout ID, if there was one then the sequence was playing
  if (timeoutID) {
    window.clearTimeout(timeoutID);
    timeoutID = window.setTimeout(playNextFrame, settings.frameSpeed);
  }
}

function setCanvasScale(event){
  document.documentElement.style.setProperty('--canvas-scale', event.target.value);
  settings.canvasScale = parseFloat(event.target.value);
}

function toggleExtraSettingsVisibility(domElement){
  settingsShown = !settingsShown;
  document.documentElement.style.setProperty('--extra-settings-display', settingsShown?'flex':'none');
  if(domElement){
    domElement.style.backgroundColor = settingsShown?"var(--button-highlight-color)":null;
    domElement.style.color = settingsShown?"yellow":null;
  }
}

function toggleGridVisibility(domElement) {
  settings.showGrid = !settings.showGrid;
  settings.showGrid?showGrid(domElement):hideGrid(domElement);
}

function showGrid(domElement){
  if(domElement){
    domElement.innerText = "hide grid";
    domElement.style.backgroundColor = "var(--button-highlight-color)";
    domElement.style.color = "yellow";
  }
  document.documentElement.style.setProperty('--grid-visibility', 'visible');
}

function hideGrid(domElement){
  if(domElement){
    domElement.innerText = "show grid";
    domElement.style.backgroundColor = null;
    domElement.style.color = null;
  }
  document.documentElement.style.setProperty('--grid-visibility', 'hidden');
}

function togglePreviousFrameOverlay(domElement) {
  settings.overlayGhosting = !settings.overlayGhosting;
  if (settings.overlayGhosting) {
    domElement.innerText = "disable overlay";
    domElement.style.background = "var(--button-highlight-color)";
    domElement.style.color = "yellow";
  }
  else {
    domElement.innerText = "enable overlay";
    domElement.style.background = null;
    domElement.style.color = null;
  }
  updateCanvas(false);
}

function editName() {
  //remove static name
  document.getElementById("sprite_name_static").remove();
  const nameArea = document.getElementById("sprite_name_container");
  //add textarea name
  const newTextArea = document.createElement('textarea');
  newTextArea.id = "sprite_name_textarea";
  newTextArea.autofocus = true;
  newTextArea.value = spriteName;
  newTextArea.addEventListener('input', (e) => {
    spriteName = e.target.value;
  });
  //finish edit
  newTextArea.addEventListener('blur', finishEditingName);
  newTextArea.addEventListener('keydown', (e) => {
    if (e.key === "Enter" && !e.shiftKey) {
      e.preventDefault();
      finishEditingName();
    }
  });
  nameArea.insertAdjacentElement('afterbegin',newTextArea);
}

function finishEditingName() {
  const textArea = document.getElementById("sprite_name_textarea");
  textArea.removeEventListener('blur', finishEditingName);//prevent this from firing again when remove() is called
  textArea.remove();
  const staticText = document.createElement('div');
  staticText.id = "sprite_name_static";
  staticText.innerText = spriteName;
  staticText.addEventListener('dblclick', editName);
  const nameArea = document.getElementById("sprite_name_container");
  nameArea.insertAdjacentElement('afterbegin',staticText);
}

function getClickCoords(e) {
  const dims = e.target.getBoundingClientRect();
  const sprite = sprites[currentSprite];
  let clickCoords;
  if (e.type == 'touchmove' || e.type == 'touchstart') {
    clickCoords = {
      x: e.touches[0].clientX - dims.left,
      y: e.touches[0].clientY - dims.top
    };
  }
  else if(e.type == 'touchend' || e.type == 'touchcancel'){
    clickCoords = {
      x: e.changedTouches[0].clientX - dims.left,
      y: e.changedTouches[0].clientY - dims.top
    };
  }
  else {
    clickCoords = {
      x: e.clientX - dims.left,
      y: e.clientY - dims.top
    };
  }
  //px per char
  const pixelDims = {
    width: dims.width / sprite.width,
    height: dims.height / sprite.height,
  };

  return { x: Math.trunc(clickCoords.x / pixelDims.width), x_rounded: Math.round(clickCoords.x / pixelDims.width), y: Math.trunc(clickCoords.y / pixelDims.height), y_rounded: Math.round(clickCoords.y / pixelDims.height) };
}

function handleKeyDown(e) {
  if ((e.target === document.body)) {
    const sprite = sprites[currentSprite];
    switch (e.key) {
      case 'a':
      case 'A':
        if (e.metaKey || e.ctrlKey) {
          e.preventDefault();
          selectionBox.selectAll();
        }
        break;
      case 'X':
      case 'x':
        if (e.metaKey || e.ctrlKey) {
          copyBuffer.cut();
        }
        break;
      case 'C':
      case 'c':
        if (e.metaKey || e.ctrlKey) {
          copyBuffer.copy();
        }
        else{
          setTool('circle');
        }
        break;
      case 'Z':
      case 'z':
        if (e.metaKey || e.ctrlKey) {
          e.preventDefault();
          e.stopPropagation();
          if (e.shiftKey) {
            redo();
          }
          else {
            undo();
          }
        }
        break;
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        const newVal = parseInt(e.key) - 1;
        if (newVal < sprite.frames.length) {
          sprite.currentFrame = newVal;
          updateFramePreviews();
        }
        break;
      case '+':
      case '=':
        addNewFrame();
        break;
      case '-':
        deleteCurrentFrame();
        break;
      case 'p':
      case 'P':
        setTool('pixel');
        break;
      case 'r':
      case 'R':
        setTool('rectangle');
        break;
      case 's':
      case 'S':
        setTool('select');
        break;
      case 'l':
      case 'L':
        setTool('line');
        break;
      case 'f':
      case 'F':
        setTool('fill');
        break;
      case 'm':
      case 'M':
        setTool('move');
        break;
      case 'v':
      case 'V':
        if (e.metaKey || e.ctrlKey) {
          copyBuffer.paste(e);
          return;
        }
      case 'ArrowLeft':
        e.preventDefault();
        e.stopImmediatePropagation();
        sprite.previousFrame();
        updateFramePreviews();
        break;
      case 'ArrowRight':
        e.preventDefault();
        e.stopImmediatePropagation();
        sprite.nextFrame();
        updateFramePreviews();
        break;
      case 'ArrowUp':
        e.preventDefault();
        e.stopImmediatePropagation();
        if(currentSprite)
          currentSprite--;
        else
          currentSprite = sprites.length-1;
        updateFramePreviews();
        reloadSpritePreviews();
        break;
      case 'ArrowDown':
        e.preventDefault();
        e.stopImmediatePropagation();
        if(currentSprite < sprites.length - 1)
          currentSprite++;
        else
          currentSprite = 0;
        updateFramePreviews();
        reloadSpritePreviews();
        break;
      case ' ':
        e.preventDefault();
        e.stopImmediatePropagation();
        togglePlayback();
        break;
      case 'Shift':
        toggleColor();
        break;
    }
  }
}

function handleKeyUp(e) {
  if ((e.target === document.body)) {
    switch (e.key) {
      case 'Shift':
        toggleColor();
        break;
    }
  }
}

function setOutputForeground(domElement){
  const colorName = domElement.innerText;

  const currentButton = document.getElementById(`foreground_${settings.outputColors.foregroundColor == '#000000'?'black':(settings.outputColors.foregroundColor == '#ffffff')?'white':settings.outputColors.foregroundColor}`);
  switch(currentButton.innerText){
    case 'transparent':
    case 'white':
      currentButton.style.backgroundColor = null;
      currentButton.style.color = null;
      break;
    case 'black':
      currentButton.style.backgroundColor = 'black';
      currentButton.style.color = 'white';
      break;
  }
  switch(colorName){
    case 'white':
      settings.outputColors.foregroundColor = '#ffffff';
      break;
    case 'black':
      settings.outputColors.foregroundColor = '#000000';
      break;
    case 'transparent':
      settings.outputColors.foregroundColor = 'transparent';
      break;
  }
  domElement.style.backgroundColor = 'var(--button-highlight-color)';
  domElement.style.color = 'yellow';
}

function setOutputBackground(domElement){
  const colorName = domElement.innerText;

  const currentButton = document.getElementById(`background_${settings.outputColors.backgroundColor == '#000000'?'black':(settings.outputColors.backgroundColor == '#ffffff')?'white':settings.outputColors.backgroundColor}`);
  switch(currentButton.innerText){
    case 'transparent':
    case 'white':
      currentButton.style.backgroundColor = null;
      currentButton.style.color = null;
      break;
    case 'black':
      currentButton.style.backgroundColor = 'black';
      currentButton.style.color = 'white';
      break;
  }
  switch(colorName){
    case 'white':
      settings.outputColors.backgroundColor = '#ffffff';
      break;
    case 'black':
      settings.outputColors.backgroundColor = '#000000';
      break;
    case 'transparent':
      settings.outputColors.backgroundColor = 'transparent';
      break;
  }
  domElement.style.backgroundColor = 'var(--button-highlight-color)';
  domElement.style.color = 'yellow';
}

function setTooltip(text) {
  document.getElementById("tooltip_text").innerText = text;
}
function setTool(tool, domElement) {
  // if(tool == settings.currentTool)
  //   return;
  //logic to cancel active tools
  switch(settings.currentTool){
    case 'line':
      if(line.started){
        cancelLine();
      }
      break;
    case 'move':
      break;
  }
  settings.currentTool = tool;
  let otherToolButtons = document.getElementsByClassName('tool_button');
  for (let tool of otherToolButtons) {
    tool.style.backgroundColor = '';
  }
  if (!domElement)
    domElement = document.getElementById(`${tool}_tool_button`);
  domElement.style.backgroundColor = 'var(--button-highlight-color)';
}

function setMouseCoordDisplay(e) {
  let coords = getClickCoords(e);
  document.documentElement.style.setProperty('--cursor-x', `${coords.x}px`);
  document.documentElement.style.setProperty('--cursor-y', `${coords.y}px`);
  setTooltip(`{${coords.x},${coords.y}}`);
}

function blendColor(c1, c2) {
  let newBytes = '#';
  for (let byte = 0; byte < 3; byte++) {
    const b1 = c1.substring(byte + 1, byte + 3);
    const b2 = c2.substring(byte + 1, byte + 3);
    newBytes += (Math.round((parseInt(b1, 16) + parseInt(b2, 16)) / 2)).toString(16).padStart(2, '0');
  }
  return newBytes;
}

function playNextFrame() {
  sprites[currentSprite].nextFrame();
  updateFramePreviews();
  timeoutID = window.setTimeout(playNextFrame, settings.frameSpeed);
}
const backupSettings = {
  showGrid:settings.showGrid,
  overlayGhosting:settings.overlayGhosting
};
function togglePlayback() {
  if (settings.playing) {
    settings.playing = false;
    window.clearTimeout(timeoutID);
    timeoutID = undefined;
    settings.overlayGhosting = backupSettings.overlayGhosting;
    backupSettings.showGrid?showGrid():hideGrid();
    updateCanvas(false);
  }
  else {
    settings.playing = true;
    backupSettings.overlayGhosting = settings.overlayGhosting;
    backupSettings.showGrid = settings.showGrid;
    hideGrid();
    settings.overlayGhosting = false;
    playNextFrame();
  }
}
function toggleColor(domElement) {
  if (!domElement)
    domElement = document.getElementById('color_toggle_button');
  settings.currentColor = 1 - settings.currentColor;
  domElement.style.backgroundColor = settings.currentColor ? 'white' : 'black';
}
function deleteSprite(index){
  if(sprites.length <= 1)
    return;
  pushUndoState();
  sprites.splice(index,1);
  if(currentSprite >= sprites.length){
    currentSprite = sprites.length - 1;
  }
  else if(currentSprite > index){
    currentSprite--;
  }
  reloadSpritePreviews();
  updateFramePreviews();
}

function createNewSprite(title, setToCurrent = true) {
  pushUndoState();
  const newSprite = Sprite(title);
  for(let i = 0; i<sprites[currentSprite].frames.length; i++){
    newSprite.frames[i] = PixelFrame(sprites[currentSprite].width,sprites[currentSprite].height,sprites[currentSprite].frames[i].data);
  }
  sprites.push(newSprite);
  if (setToCurrent) {
    currentSprite = sprites.length - 1;
    reloadSpritePreviews();
    updateFramePreviews();
  }
  else {
    reloadSpritePreviews();
  }
}

//draws the current sprite to the main canvas, and updates the preview for the current frame
function updateCanvas(updatePreview = true) {
  //if you're editing the first frame, update the sprite preview for this sprite to match
  if (sprites[currentSprite].currentFrame == 0) {
    renderFrame(document.getElementById(`sprite_preview_${currentSprite}`).getContext('2d'), sprites[currentSprite].frames[0]);
  }
  const canvas = document.getElementById("main_canvas");
  if (!canvas)
    return;

  const sprite = sprites[currentSprite];
  document.documentElement.style.setProperty('--sprite-width', `${sprite.width}px`);
  document.documentElement.style.setProperty('--sprite-height', `${sprite.height}px`);
  document.documentElement.style.setProperty('--background-width', `${100 / sprite.width}%`);
  document.documentElement.style.setProperty('--background-height', `${100 / sprite.height}%`);
                
  //figure out the last frame, to draw ghosting
  let previousFrame = undefined;
  if (sprite.currentFrame > 0)
    previousFrame = sprite.currentFrame - 1;
  else if (sprite.frames.length > 1)
    previousFrame = sprite.frames.length - 1;

  //set canvas dims (these aren't the visual size of the canvas)
  canvas.width = sprite.width;
  canvas.height = sprite.height;

  //get drawing context
  const context = canvas.getContext("2d");
  //draw over each pixel
  for (let x = 0; x < sprite.width; x++) {
    for (let y = 0; y < sprite.height; y++) {
      //if there's a pixel there, draw foreground color
      if (sprite.frames[sprite.currentFrame].getPixel(x, y)) {
        context.fillStyle = settings.foregroundColor;
      }
      else {
        //if there's a no pixel, but one on a previous frame, ghost it
        if (previousFrame !== undefined && settings.overlayGhosting && sprite.frames[previousFrame].getPixel(x, y)) {
          context.fillStyle = blendColor(settings.foregroundColor, settings.backgroundColor);
        }
        else {
          context.fillStyle = settings.backgroundColor
        }
      }
      context.fillRect(x, y, 1, 1);
    }
  }
  if(updatePreview)
    updateActivePreview(sprite.currentFrame);
}

function renderFrame(context, frame, palette = {foregroundColor : settings.foregroundColor,backgroundColor : settings.backgroundColor}, offset = {x:0,y:0}) {
  //draw over each pixel
  for (let x = 0; x < frame.width; x++) {
    for (let y = 0; y < frame.height; y++) {
      context.fillStyle = frame.getPixel(x, y) ? (palette.foregroundColor ==  'transparent'?(palette.backgroundColor == '#000000'?'#ffffff':'#000000'):palette.foregroundColor):(palette.backgroundColor ==  'transparent'?(palette.foregroundColor == '#000000'?'#ffffff':'#000000'):palette.backgroundColor);
      context.fillRect(x+offset.x, y+offset.y, 1, 1);
    }
  }
}

function updateActivePreview(index) {
  const canv = document.getElementById(`frame_${index}_preview`);
  renderFrame(canv.getContext('2d'),sprites[currentSprite].frames[index]);
}

let draggedFrame = null;
function getDragAfterElement(container,e){
  const draggableElements = [...container.querySelectorAll('.preview_canvas:not(.dragging)')];
  return draggableElements.reduce((closest,child) => {
    const box = child.getBoundingClientRect();
    const offsetX = e.clientX-box.left-box.width/2;
    const offsetY = e.clientY-box.top-box.height;
    if(offsetX < 0  && offsetY < 0 && (offsetX > closest.offset)){
      return {offset:offsetX,element:child};
    }
    else return closest;
  },{offset:Number.NEGATIVE_INFINITY}).element;
}

function handleFrameHolderDragOver(e,frameHolder){
  e.preventDefault();
  const afterElement = getDragAfterElement(frameHolder,e);
  //if it's the last element
  if(afterElement === null || afterElement === undefined){
    frameHolder.appendChild(draggedFrame.element);      
    draggedFrame.newIndex = sprites[currentSprite].frames.length-1;
  }
  else{
    frameHolder.insertBefore(draggedFrame.element,afterElement);
    if(afterElement.key > draggedFrame.oldIndex)
      draggedFrame.newIndex = afterElement.key-1;
    else
      draggedFrame.newIndex = afterElement.key;
  }
};


//recreates preview canvases
function reloadFramePreviews() {

  const frameHolder = document.getElementById("preview_gallery_holder");

  //doing some bounds checking on the preview dimensions so they don't get huge or tiny
  const maxPreviewDim = 32;
  const aspectRatio = sprites[currentSprite].height / sprites[currentSprite].width;
  let scaledWidth, scaledHeight;
  if (aspectRatio > 1) {
    scaledHeight = maxPreviewDim;
    scaledWidth = scaledHeight / aspectRatio;
  }
  else {
    scaledWidth = maxPreviewDim;
    scaledHeight = scaledWidth * aspectRatio;
  }

  const frames = [];
  for (let f = 0; f < sprites[currentSprite].frames.length; f++) {
    //create preview canvas
    let newCanvas = document.createElement('canvas');
    newCanvas.width = sprites[currentSprite].width;
    newCanvas.height = sprites[currentSprite].height;

    //styling canvas
    newCanvas.className = (f == sprites[currentSprite].currentFrame) ? 'active_canvas preview_canvas' : 'preview_canvas';
    newCanvas.id = `frame_${f}_preview`;
    newCanvas.style.borderColor = (f == sprites[currentSprite].currentFrame) ? 'var(--button-highlight-color)' : null;
    newCanvas.style.width = scaledWidth + 'px';
    newCanvas.style.height = scaledHeight + 'px';
    newCanvas.draggable = true;
    newCanvas.key = f;

    newCanvas.addEventListener('click', () => { sprites[currentSprite].currentFrame = f; updateFramePreviews(); })
    newCanvas.addEventListener('mouseenter', ()=>{setTooltip(`frame ${f+1}`)});

    //adding drag listeners
    //method from: https://www.youtube.com/watch?v=OWARn8lQbVE
    newCanvas.addEventListener('dragstart', (event)=>{
      draggedFrame = {
        element:newCanvas,
        oldIndex:f,
        newIndex:null
      };
      newCanvas.classList.add('dragging');
      document.getElementById('new_canvas_button').style.display = 'none';
    });
    newCanvas.addEventListener('dragend', (e)=>{

      //reorder frames
      const sprite = sprites[currentSprite];
      //remove item from old loc
      const [targetFrame] = sprite.frames.splice(draggedFrame.oldIndex,1);
      //put it in new loc
      sprite.frames.splice(draggedFrame.newIndex,0,targetFrame);
      
      //check and see if you need to change the currentFrame val to stay consistent
      if(draggedFrame.oldIndex < sprite.currentFrame && draggedFrame.newIndex >= sprite.currentFrame)
        sprite.currentFrame--;
      else if(draggedFrame.oldIndex > sprite.currentFrame && draggedFrame.newIndex <= sprite.currentFrame)
        sprite.currentFrame++;
      else if(draggedFrame.oldIndex == sprite.currentFrame)
        sprite.currentFrame = draggedFrame.newIndex;

      draggedFrame = null;
      updateCanvas(false);//a ~little~ redundant...u don't always need to update depending on which frames are moved
      reloadFramePreviews();
    });

    //draw frame to canvas
    renderFrame(newCanvas.getContext('2d'),sprites[currentSprite].frames[f]);

    //add it to the list of children
    frames.push(newCanvas);
  }
  // new frame button
  const newCanvButton = document.createElement('div');
  newCanvButton.addEventListener('click', addNewFrame);
  newCanvButton.addEventListener('mouseenter', ()=>{setTooltip('new frame')});
  const newImg = document.createElement('img');
  newImg.src = "designer/images/icons/plus_icon.gif";
  newCanvButton.appendChild(newImg);
  newCanvButton.className = "button";
  newCanvButton.id = "new_canvas_button";
  frames.push(newCanvButton)

  frameHolder.replaceChildren(...frames);
}

//reloads frame previews AND sets the frame counter text
//AND updates the main canvas, so that you don't need to call "updateCanvas()" (which in turn updates the active frame preview...this is kinda convoluted but prevents double-updating the active frame preview)
function updateFramePreviews() {
  reloadFramePreviews();
  updateCanvas(false);
  document.getElementById('frame_counter_label').innerText = `Frame -- ${sprites[currentSprite].currentFrame + 1} / ${sprites[currentSprite].frames.length}`
}

function createSpritePreview(domElement,index){
  domElement.className = "sprite_preview_holder";
  const canv = document.createElement('canvas');
  canv.id = `sprite_preview_${index}`;
  canv.className = "sprite_preview";
  canv.width = sprites[index].width;
  canv.height = sprites[index].height;

  const maxPreviewDim = 16;
  const aspectRatio = sprites[currentSprite].height / sprites[currentSprite].width;
  let scaledWidth, scaledHeight;
  if (aspectRatio > 1) {
    scaledHeight = maxPreviewDim;
    scaledWidth = scaledHeight / aspectRatio;
  }
  else {
    scaledWidth = maxPreviewDim;
    scaledHeight = scaledWidth * aspectRatio;
  }

  canv.style.width = `${scaledWidth}px`;
  canv.style.height = `${scaledHeight}px`;
  domElement.addEventListener('click', () => {
    currentSprite = index;
    reloadSpritePreviews();
    updateFramePreviews();
  })
  domElement.addEventListener('mouseenter', () => {
    setTooltip(`${sprites[index].fileName} sprite`)
  });

  renderFrame(canv.getContext('2d'),sprites[index].frames[0]);
  domElement.appendChild(canv);
}

function createBlankPreview(domElement,name){
  domElement.className = "sprite_blank_preview_holder";
  const blank = document.createElement('div');
  const blankImg = document.createElement('img');
  blankImg.src = "designer/images/icons/plus_icon.gif";
  blank.appendChild(blankImg);
  blank.className = "new_sprite_button";
  blank.addEventListener('click', () => createNewSprite(name,true));
  blank.addEventListener('mouseenter', () => setTooltip(`create ${name} sprite`));
  domElement.appendChild(blank);
}

function spriteWithName(name){
  for(let s  = 0; s<sprites.length; s++){
    if(sprites[s] && sprites[s].fileName == name)
      return {index:s,sprite:sprites[s]};
  }
  return undefined;
}

//clears out and rebuilds the "sprite slots" previews
function reloadSpritePreviews() {
  //clear out old previews
  const previewHolder = document.getElementById('sprite_previews');
  previewHolder.textContent = '';

  //draw sprites you already have
  for(let i = 0; i<sprites.length; i++){
    const slot = document.createElement('div');
    if (i == currentSprite) {
      slot.style.background = 'var(--button-highlight-color)';
      slot.style.color = 'white';
    }

    //if there's an existing sprite for this slot, draw it
    createSpritePreview(slot,i);

    const text = document.createElement('div');
    text.innerText = sprites[i].fileName;
    slot.appendChild(text);
    const deleteButton = document.createElement('img');
    deleteButton.className = "tool_icon delete_sprite_button";
    deleteButton.style.paddingRight = '5px';
    deleteButton.src = "designer/images/icons/clear_icon.gif";
    deleteButton.addEventListener('click', (event) => {
      event.stopImmediatePropagation();
      deleteSprite(i);
    });
    deleteButton.addEventListener('mouseenter', () => setTooltip(`delete ${sprites[i].fileName} sprite`));
    slot.appendChild(deleteButton);
    previewHolder.appendChild(slot);
  }
  //draw empty slots for sprites that are still needed
  for (let i = 0; i < presetSpriteNames.length; i++) {
    const slot = document.createElement('div');
    //if there's an existing sprite for this slot, draw it
    const matchingSprite = spriteWithName(presetSpriteNames[i]);
    if (matchingSprite) {
      continue;
    }
    //if not, draw a blank area
    else {
      createBlankPreview(slot,presetSpriteNames[i]);
    }

    const text = document.createElement('div');
    text.innerText = presetSpriteNames[i];
    slot.appendChild(text);
    previewHolder.appendChild(slot);
  }
}

function factoryResetSprites() {
  alert("WARNING: you are about to reset all artwork on Tamo! You will lose any custom sprites.")
}

function checkCanvasOverflowBounds(){
  const element = document.getElementById('canvas_container_container');
  const margin = 8;
  const active = '3px dashed var(--button-highlight-color)';
  const inactive = '3px dashed transparent';
  document.documentElement.style.setProperty('--canvas-border-left',(element.scrollLeft > margin)?active:inactive);
  document.documentElement.style.setProperty('--canvas-border-right',((element.clientWidth) < (element.scrollWidth - element.scrollLeft - margin))?active:inactive);
  document.documentElement.style.setProperty('--canvas-border-top',(element.scrollTop > margin)?active:inactive);
  document.documentElement.style.setProperty('--canvas-border-bottom',((element.clientHeight) < (element.scrollHeight - element.scrollTop - margin))?active:inactive);


}

window.addEventListener("keydown", handleKeyDown);
window.addEventListener("keyup", handleKeyUp);
loadApp();