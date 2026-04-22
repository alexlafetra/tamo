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
                    frame.pixels.map((pixelValue, pixelIndex) => {
                        //get the x,y coords for the PixelFrame (using the gif frame offsets, and accounting for diff dimensions)
                        const x = pixelIndex % frame.dims.width + frame.dims.left;
                        const y = Math.trunc(pixelIndex / frame.dims.width) + frame.dims.top;
                        if(frame.transparentIndex)
                            spriteFrames[frameIndex].setPixel(x, y, (pixelValue == frame.transparentIndex) ? 0 : 1);
                        else
                            spriteFrames[frameIndex].setPixel(x, y, (pixelValue == gif.lsd.backgroundColorIndex) ? 0 : 1);
                    });
                });

                //send frames out via this callback
                frameCallback(spriteFrames);
            }
        }
        reader.readAsArrayBuffer(file);
    // })
}

function drawFileToCanvas(file, sprite, startFrame, index) {
    return new Promise((resolve) => {
        const reader = new FileReader();
        //callback once the file is read
        reader.onload = function () {
            //make an image, draw it to canvas
            const img = new Image();
            img.onload = function () {
                if (settings.resizeCanvasToImage) {
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
                ctx.drawImage(img, 0, 0);

                //make new frames as needed
                while (index >= sprite.frames.length) {
                    sprite.frames.push(PixelFrame(sprite.width, sprite.height, 0));
                }
                //copy canvas data
                sprite.frames[startFrame ? (index + startFrame) : index].copyCanvas(tempCanvas, settings.useAlphaAsBackground);
                tempCanvas.remove();
                resolve();
            }
            img.src = reader.result;
        }
        reader.readAsDataURL(file);
    });
}

// gets images and turns them into anim frames
async function processLoadedFiles(fileList, sprite, startFrame) {
    pushUndoState();
    if (fileList.length === 1) {
        fileList = [fileList[0]];
    }
    const promises = fileList.map((file, index) => {
        //grabbing sprite frames from a gif
        if (file.type === 'image/gif') {
            return gifToSprite(file, (frames) => {
                const sprite = sprites[currentSprite + index];
                sprite.frames = frames;
                sprite.width = frames[0].width;
                sprite.height = frames[0].height;
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
    reloadFramePreviews();
    reloadSpritePreviews();
    updateCanvas();
}

//callback from the <input> element
function loadFiles(files) {
    //parsing files by name
    if (settings.createSpritesByFileName) {
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
                file.spriteName = "misc.";
                unorderedFiles.push(file);
            }
        }
        let newSprites = [];
        for (let fileList of orderedFiles) {
            if (!fileList)
                continue;
            const newSprite = Sprite();
            newSprite.fileName = fileList[0].spriteName;

            processLoadedFiles(fileList, newSprite, (files.length === 1) ? sprites[currentSprite].currentFrame : 0);
            newSprites.push(newSprite);
        }
        //add in the stragglers that didn't match any names
        if (unorderedFiles.length) {
            const miscSprite = Sprite();
            miscSprite.fileName = 'misc.';
            processLoadedFiles(unorderedFiles, miscSprite);
            newSprites.push(miscSprite);
        }
        currentSprite = 0;
        [newSprites, sprites] = [sprites, newSprites];
        sprites[currentSprite].currentFrame = 0;
        //reloads the preview frames
    }
    else {
        //load files like normal, into the current sprite
        processLoadedFiles(files, sprites[currentSprite], (files.length === 1) ? sprites[currentSprite].currentFrame : 0);
        reloadSpritePreviews();
        reloadFramePreviews();
        updateCanvas();
    }
}
function handleFileInput(e){
    const files = e.target.files;
    loadFiles(files);
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
    loadFiles(e.dataTransfer.files);
}