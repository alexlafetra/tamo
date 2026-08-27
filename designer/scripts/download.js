function saveCurrentFrame(){
    window.CanvasToBMP.toBlob(document.getElementById('main_canvas'),(blob) => {
        const filename = sprites[currentSprite].fileName+'_'+(sprites[currentSprite].currentFrame)+'.bmp';
        const a = document.createElement('a');
        a.href = URL.createObjectURL(blob);
        a.download = filename;
        a.click();
        a.remove();
    });
}

function exportCompleteSpritesheet(){
    const timing = settings.spritesheetExportDirection; //which way time moves

    const dims = {
        w : sprites[currentSprite].width,
        h : sprites[currentSprite].height,
        totalW : sprites[currentSprite].width * sprites.length,
        totalH : 0
    };
    for(let sprite of sprites){
        if((sprite.frames.length * dims.h) > dims.totalH)
            dims.totalH = sprite.frames.length * dims.h;
    }
    const tempCanvas = document.createElement('canvas');
    tempCanvas.width = timing=='vertical'?dims.totalW:dims.totalH;
    tempCanvas.height = timing=='vertical'?dims.totalH:dims.totalW;
    const ctx = tempCanvas.getContext('2d');

    for(let i = 0; i<sprites.length; i++){
        for(let f = 0; f<sprites[i].frames.length; f++){
            renderFrame(ctx,sprites[i].frames[f],settings.outputColors,{x:dims.w*(timing=='vertical'?i:f),y:dims.h*(timing=='vertical'?f:i)});
        }
    }
    window.CanvasToBMP.toBlob(tempCanvas,(blob) => {
        const a = document.createElement('a');
        a.href = URL.createObjectURL(blob);
        a.download = 'spritesheet.bmp';
        a.click();
        a.remove();
        tempCanvas.remove();
    });
}

async function zipAllFrames(){
    //create a new JSZip object
    const zip = new JSZip();

    const tempCanvas = document.createElement('canvas');
    const fileName = spriteName;

    //iterate over each sprite
    sprites.map(async (sprite,spriteIndex) => {
        //& iterate over each frame in the sprite
        await sprite.frames.map(async (frame,frameIndex) => {
            //resize canvas to fit frame
            tempCanvas.width = frame.width;
            tempCanvas.height = frame.height;
            //draw frame to the canvas
            renderFrame(tempCanvas.getContext('2d'),frame,settings.outputColors);
            //compile a .bmp file
            await window.CanvasToBMP.toBlob(tempCanvas,(blob) => {
                const filename = sprite.fileName+'_'+(frameIndex+1)+'.bmp';
                zip.file(filename,blob);
            });

        });
    });

    tempCanvas.remove();

    zip.generateAsync({type : 'blob' }).then((content) => {
        const a = document.createElement('a');
        a.href = URL.createObjectURL(content);
        a.download = fileName+'.zip';
        a.click();
        a.remove();
    });
}

async function saveGIF(){
    const sprite = sprites[currentSprite];
    const canvases = [];
    //draw frames to canvases
    sprite.frames.map((frame,index)=>{
        const tempCanvas = document.createElement('canvas');
        tempCanvas.width = sprite.width;
        tempCanvas.height = sprite.height;
        renderFrame(tempCanvas.getContext('2d'),frame,settings.outputColors);
        canvases.push(tempCanvas);
    });
    await canvasesToGif(canvases, settings.frameSpeed,);
};

async function canvasesToGif(canvases, msDelay){
    const props = {
        workerScript: 'designer/scripts/lib/gif.worker.js',
        workers: 2,
        quality: 10,
        width: canvases[0].width,
        height: canvases[0].height,
    }
    if(settings.outputColors.foregroundColor == 'transparent'){
        props.transparent = settings.outputColors.backgroundColor == '#ffffff'?'0x000000':'0xffffff';//set 'transparent' to whichever foreground is not
    }
    else if(settings.outputColors.backgroundColor == 'transparent'){
        props.transparent = settings.outputColors.foregroundColor == '#ffffff'?'0x000000':'0xffffff';//set 'transparent' to whichever background is not
    }

    const gif = new GIF(props);
    for(let frame of canvases){
        gif.addFrame(frame,{delay:msDelay});
    }
    gif.on('finished',function(blob){

        const url = URL.createObjectURL(blob);
        // download it, display it, whatever
        const a = document.createElement('a');
        a.href = url;
        a.download = `${sprites[currentSprite].fileName}.gif`;
        a.click();
        for(let canvas of canvases){
            canvas.remove();
        }
        
    });
    gif.render();
}