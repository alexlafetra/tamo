const { parseGIF, decompressFrames } = gifuct;

function gifToSprite(file, frameCallback) {
    // return new Promise((resove) => {
        const reader = new FileReader();

        reader.onload = function () {
            //get array buffer from the file reader
            const arrayBuffer = reader.result;
            //array that's gonna hold the PixelFrame objects
            const spriteFrames = [];

            //idk but recommended by the docs: https://github.com/matt-way/gifuct-js
            if (arrayBuffer) {
                //build gif object
                const gif = parseGIF(arrayBuffer);

                //get frame data (pass false bc we don't need to turn it into colors for drawing)
                const gifFrames = decompressFrames(gif, false);

                //loop over each frame, build & fill a new PixelFrame object with its data
                gifFrames.map((frame, frameIndex) => {
                    spriteFrames.push(PixelFrame(gif.lsd.width, gif.lsd.height, 0));
                    //this is inefficient, but consistent
                    let containsTransparentPixels = false;
                    //check for any transparent pixels!
                    frame.pixels.map((pixelValue, pixelIndex) => {
                        if(pixelValue == frame.transparentIndex){
                            containsTransparentPixels = true;
                            return;
                        }
                    });
                    frame.pixels.map((pixelValue, pixelIndex) => {
                        //get the x,y coords for the PixelFrame (using the gif frame offsets, and accounting for diff dimensions)
                        const x = pixelIndex % frame.dims.width + frame.dims.left;
                        const y = Math.trunc(pixelIndex / frame.dims.width) + frame.dims.top;
                        //if this frame contains any transparent pixels, those should be black and everything else should be white
                        if(containsTransparentPixels){
                            spriteFrames[frameIndex].setPixel(x, y, (pixelValue == frame.transparentIndex) ? 0 : 1);
                        }
                        //if this pixel matches the designated 'background color' (not sure when .gif's even use that)
                        //or if it's brightness > 0, the pixel should be white
                        else{
                            spriteFrames[frameIndex].setPixel(x, y, (pixelValue == gif.lsd.backgroundColorIndex) ? 0 : (((frame.colorTable[pixelValue][0]+frame.colorTable[pixelValue][1]+frame.colorTable[pixelValue][2])) > 0));
                        }
                    });
                });

                //send frames out via this callback
                frameCallback(spriteFrames);
            }
        }
        reader.readAsArrayBuffer(file);
    // })
}

function drawDataURLToCanvas(url,sprite,startFrame,index,resolve){
    //make an image, draw it to canvas
    const img = new Image();
    img.onload = function () {
        if (settings.automaticallyResizeCanvasToImage) {
            const aspectRatio = img.width / img.height;
            if (img.width > img.height) {
                if (img.width > settings.maxCanvasDimension) {
                    img.width = settings.maxCanvasDimension;
                    img.height = img.width / aspectRatio;
                }
            }
            else if (img.height >= img.width) {
                if (img.height > settings.maxCanvasDimension) {
                    img.height = settings.maxCanvasDimension;
                    img.width = img.height * aspectRatio;
                }
            }
            sprite.resize(img.width, img.height);
            const newScale = Math.min(Math.trunc(350 / img.width), 12);
            settings.canvasScale = newScale;
        }
        // draw image to main canvas
        const tempCanvas = document.createElement('canvas');
        tempCanvas.width = sprite.width;
        tempCanvas.height = sprite.height;
        const ctx = tempCanvas.getContext('2d');
        let offsetX = 0,offsetY = 0;
        if(imageUploadSettings.align == 'center'){
            offsetX = (tempCanvas.width - img.width)/2;
            offsetY = (tempCanvas.height - img.height)/2;
        }
        ctx.drawImage(img, offsetX, offsetY,);

        //make new frames as needed
        while (index >= sprite.frames.length) {
            sprite.frames.push(PixelFrame(sprite.width, sprite.height, 0));
        }
        //copy canvas data
        sprite.frames[startFrame ? (index + startFrame) : index].copyCanvas(tempCanvas, settings.useAlphaAsBackground);
        tempCanvas.remove();
        resolve();
    }
    img.src = url;
}

function drawFileToCanvas(file, sprite, startFrame, index) {
    return new Promise((resolve) => {
        const reader = new FileReader();
        //callback once the file is read
        reader.onload = function () {
            drawDataURLToCanvas(reader.result,sprite,startFrame,index,resolve);
        }
        reader.readAsDataURL(file);
    });
}

// gets images and turns them into anim frames
async function processSpriteFiles(fileList, sprite, startFrame) {
    pushUndoState();
    if (fileList.length === 1) {
        fileList = [fileList[0]];
    }
    console.log(fileList);
    const promises = fileList.map((file, index) => {
        //grabbing sprite frames from a gif
        if (file.type == 'image/gif') {
            return gifToSprite(file, (frames) => {
                const sprite = sprites[currentSprite + index];
                // const originalFramecount = frames.length;
                for(let i = 0; i<frames.length; i++){
                    if((i+sprite.currentFrame)<sprite.frames.length)
                        sprite.frames[i+sprite.currentFrame] = frames[i];
                    else
                        sprite.frames.push(frames[i]);
                }
                resizeDimensions.width = frames[0].width;
                resizeDimensions.height = frames[0].height;
                resizeAllSprites();
                updateResizeSliders();
                reloadFramePreviews();
                reloadSpritePreviews();
                updateCanvas();
            });
        }
        //grabbing sprites/frames from a folder
        else {
            return drawFileToCanvas(file, sprite, startFrame, index);
        }
    });

    await Promise.all(promises);
    updateResizeSliders();
    reloadFramePreviews();
    reloadSpritePreviews();
    updateCanvas();
}

//draws a gif frame from the gifuct decompressFrames() function
//to a canvas, then converts it to a dataurl
function gifFrameToDataURL(frame){
    console.log(frame);
    const tempCanvas = document.createElement('canvas');
    tempCanvas.width = frame.dims.width;
    tempCanvas.height = frame.dims.height;


    const ctx = tempCanvas.getContext('2d');
    const imageData = ctx.createImageData(tempCanvas.width,tempCanvas.height);
    // set the patch data as an override
    imageData.data.set(frame.patch)

    // draw the patch back over the canvas
    ctx.putImageData(imageData, 0, 0)

    const dataURL = tempCanvas.toDataURL();
    tempCanvas.remove();
    return dataURL;
}

//for dithering/processing normal images
async function handleImageUpload(event){
    let fileList = event.target.files;
    const sprite = sprites[currentSprite];
    const startFrame = sprite.currentFrame;

    pushUndoState();
    if (fileList.length === 1) {
        fileList = [fileList[0]];
    }
    const promises = (fileList.map((file, index) => {
        return new Promise((resolve) => {

            //skip if it's a gif
            if (file.type == 'image/gif') {
                const reader = new FileReader();
                reader.onload = function () {
                    //get array buffer from the file reader
                    const arrayBuffer = reader.result;

                    //idk but recommended by the docs: https://github.com/matt-way/gifuct-js
                    if (arrayBuffer) {
                        //build gif object
                        const gif = parseGIF(arrayBuffer);

                        //get frame data (pass false bc we don't need to turn it into colors for drawing)
                        const gifFrames = decompressFrames(gif, true);
                        imageUploadSettings.isGif = true;
                        imageUploadSettings.gifFrames = gifFrames;
                        imageUploadSettings.dataURL = gifFrameToDataURL(gifFrames[0]);
                        renderPreviewImage();
                    }
                    resolve();
                }
                reader.readAsArrayBuffer(file);

            }
            else{
                const reader = new FileReader();
                //callback once the file is read
                reader.onload = function () {
                    imageUploadSettings.isGif = false;
                    imageUploadSettings.gifFrames = null;
                    imageUploadSettings.dataURL = reader.result;
                    renderPreviewImage();
                    resolve();
                }
                reader.readAsDataURL(file);
            }
        })
    }));
    await Promise.all(promises);
    document.getElementById("uploaded_image_settings").style.display = "flex";
    updateResizeSliders();
    reloadFramePreviews();
    reloadSpritePreviews();
    updateCanvas();
}

async function handleSpriteUpload(event){
    const files = event.target.files;
    //parsing files by name
    if (settings.automaticallyProcessSprites && files.length > 1) {
        const filesByName = [];
        let similarFiles = [];

        //creature/action name
        //this'll grab the creature name from the file name, or name the sprite the full image name if it's not a prenamed file
        spriteName = files[0].name.split('_')[0];
        if (spriteName.endsWith('.png'))
            spriteName = spriteName.substring(0, -4);

        document.getElementById("sprite_name_static").innerText = spriteName;
        const orderedFiles = [];
        const unorderedFiles = [];
        for (let file of files) {
            let foundMatchingName = false;
            for (let i = 0; i < presetSpriteNames.length; i++) {
                if (file.name.includes(presetSpriteNames[i])) {
                    file.spriteName = presetSpriteNames[i];
                    //add to that file list (it's another frame belonging to the same sprite)
                    if (orderedFiles[i])
                        orderedFiles[i].push(file);
                    //or create a new file list for it at that sprite location
                    else
                        orderedFiles[i] = [file];
                    foundMatchingName = true;
                    break;
                }
            }
            if (!foundMatchingName) {
                file.spriteName = spriteName;
                unorderedFiles.push(file);
            }
        }
        let newSprites = [];
        for (let fileList of orderedFiles) {
            if (!fileList)
                continue;
            const newSprite = Sprite();
            newSprite.fileName = fileList[0].spriteName;

            processSpriteFiles(fileList, newSprite, (files.length === 1) ? sprites[currentSprite].currentFrame : 0);
            newSprites.push(newSprite);
        }
        //add in the stragglers that didn't match any names
        if (unorderedFiles.length) {
            const miscSprite = Sprite();
            miscSprite.fileName = 'misc.';
            processSpriteFiles(unorderedFiles, miscSprite);
            newSprites.push(miscSprite);
        }
        currentSprite = 0;
        [newSprites, sprites] = [sprites, newSprites];
        sprites[currentSprite].currentFrame = 0;
        //reloads the preview frames
    }
    else {
        const fileList = [];
        for(let file of files){
            fileList.push(file);
        }
        //load files like normal, into the current sprite
        processSpriteFiles(fileList, sprites[currentSprite], sprites[currentSprite].currentFrame);
    }

}

function handleDragOver(e){
    e.preventDefault();
    document.documentElement.style.setProperty('--canvas-filter', "brightness(0.4)");
}
function handleDragLeave(e){
    document.documentElement.style.setProperty('--canvas-filter', "none");
}
function handleDrop(e){
    document.documentElement.style.setProperty('--canvas-filter', "none");
    e.preventDefault();
    e.stopPropagation();
    const dummyEvent = {
        target : {files : e.dataTransfer.files}
    };
    handleSpriteUpload(dummyEvent);
}