
let zip = new JSZip();

function downloadZip(fileName){
    zip.generateAsync({type : 'blob' }).then((content) => {
        const a = document.createElement('a');
        a.href = URL.createObjectURL(content);
        a.download = fileName+'.zip';
        a.click();
        a.remove();
    });
}


function downloadAllFramesAsBMPs(){
    //create a new JSZip object
    zip = new JSZip();

    const tempCanvas = document.createElement('canvas');
    const fileName = spriteName;
    //call this function recursively, when each file is zipped it zips the next one!
    const addFilesToZip = (spriteIndex,frameIndex) => {
      const sprite = sprites[currentSprite];
      tempCanvas.width = sprite.width;
      tempCanvas.height = sprite.height;
      renderFrame(tempCanvas.getContext('2d'),sprite,sprite.frames[frameIndex]);
      window.CanvasToBMP.toBlob(tempCanvas,(blob) => {
        const filename = spriteName+'_'+sprite.fileName+'_'+(frameIndex+1)+'.bmp';
        zip.file(filename,blob);
        if(frameIndex < sprite.frames.length-1){
          addFilesToZip(spriteIndex,frameIndex+1);
        }
        else if(spriteIndex < (sprites.length-1)){
          addFilesToZip(spriteIndex+1,0);
        }
        else{
          downloadZip(fileName);
          tempCanvas.remove();
        }
      });
    }
    addFilesToZip(0,0);
}

async function saveGIF(){
    const sprite = sprites[currentSprite];
    const canvases = [];
    //draw frames to canvases
    sprite.frames.map((frame,index)=>{
        const tempCanvas = document.createElement('canvas');
        tempCanvas.width = sprite.width;
        tempCanvas.height = sprite.height;
        renderFrame(tempCanvas.getContext('2d'),sprite,frame,{foregroundColor:'#ffffff',backgroundColor:'#000000'});
        // renderFrame(tempCanvas.getContext('2d'),sprite,frame,{foregroundColor:'#000000',backgroundColor:'#ffffff'});
        canvases.push(tempCanvas);
    });
    await canvasesToGif(canvases, settings.frameSpeed,);
};

async function canvasesToGif(canvases, msDelay){
    const gif = new GIF({
        workerScript: 'designer/scripts/lib/gif.worker.js',
        workers: 2,
        quality: 10,
        width: canvases[0].width,
        height: canvases[0].height,
        // transparent: '0xffffff',
        transparent:'0x000000'
        // background: '0x0000ff'
    });
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