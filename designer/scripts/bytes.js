
function convertFrameToByteArray(sprite,frame,padToLength = 0){
    const pixelArray = [...frame.data];
    let byteArray;

    if(settings.bytePackingFormat == 'horizontal'){
      //move down the image, grabbing 8px at a time along the row and padd the end with zeroes
      byteArray = new Uint8Array(Math.max(pixelArray.length/8,padToLength));
      let count = 0;
      for(let y = 0; y<sprite.height; y++){
        for(let x = 0; x<sprite.width; x+=8){
          let newByte = 0;
          for(let i = 0; i<8; i++){
            let bitVal;
            if(x+i > sprite.width)
              bitVal = 0;
            else bitVal = pixelArray[x+i+y*(sprite.width)];
            newByte |= bitVal<<(7-i);
          }
          byteArray[count++] = newByte;
        }
      }
    }
    else{
      //Do a pass L-->R, taking one-byte deep (8px) vertical slices. Then do another pass, going another layer deep.
      //This is to pass the data to tamo as "pages", which is how the OLED driver works

      //normalize the image to be a multiple of 8 tall
      if(sprite.height%8){
        const padHeight = 8-sprite.height%8;
        for(let i = 0; i<padHeight; i++){
          for(let j = 0; j<sprite.width; j++){
            pixelArray[pixelArray.length] = 0;
          }
        }
      }
      byteArray = new Uint8Array(Math.max(pixelArray.length/8,padToLength));
      let count = 0;
      for(let bite = 0; bite<Math.ceil(sprite.height/8); bite++){
        for(let x = 0; x<sprite.width; x++){
          let newByte = 0;
          for(let i = 0; i<8; i++){
            newByte |= pixelArray[i*sprite.width+x+bite*sprite.width*8]<<i;
          }
          byteArray[count++] = newByte;
        }
      }
    }

    return byteArray;
}

const spriteNotFound = new Uint8Array([
  219, 0, 121, 129, 248, 1, 241, 8, 9, 241, 0, 121, 129, 248, 1, 109, 182, 128, 0, 128, 159, 0, 143, 144, 16, 143, 128, 0, 128, 159, 0, 219,
  109, 1, 124, 129, 253, 0, 249, 5, 4, 249, 1, 124, 129, 253, 0, 219, 219, 0, 128, 128, 63, 128, 159, 32, 160, 159, 0, 128, 128, 63, 128, 182
]);

function packSpritesIntoByteArray(arrayOfSprites,padEachFrameToLength){
  const bytes = [];
  console.log(arrayOfSprites);
  for(let sprite of arrayOfSprites){
    if(sprite == undefined){
      bytes.push(spriteNotFound);
      continue;
    }
    else{
      for(let frame of sprite.frames){
        bytes.push(convertFrameToByteArray(sprite,frame,padEachFrameToLength));
      }
    }
  }
  console.log(bytes);
  let dataLength = 0;
  for(let data of bytes){
    dataLength += data.byteLength;
  }

  const totalBytes = new Uint8Array(dataLength);
  let position = 0;
  for(let data = 0; data<bytes.length; data++){
    totalBytes.set(bytes[data],position);
    position += bytes[data].byteLength;
  }
  console.log(totalBytes);
  return totalBytes;
}

function getByteArrayText(){
  const sprite = sprites[currentSprite];
  const byteArray = convertSpriteToByteArray(sprite);
  let outputString = `//${sprite.fileName}_${sprite.currentFrame}: ${sprite.width}x${sprite.height}\n\rconst unsigned char ${sprite.fileName}[${byteArray.length}] = {\n\t`;
  for(let byte = 0; byte<byteArray.length;byte++){
    outputString+='0x'+byteArray[byte].toString(16).padStart(2,'0')+', ';
    if(((byte%16) == 15) && byte != byteArray.length-1){
      outputString += "\n\t";
    }
  }
  outputString = outputString.slice(0,-2);
  outputString += '\n};\n';
  
  return outputString;
}