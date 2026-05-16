

//threshold, bayer, floyd-steinberg, and atkinson
//taken from https://beyondloom.com/blog/dither.html
function atkinson(pixels, w, threshold) {
    const e = Array(2 * w).fill(0), m = [0, 1, w - 2, w - 1, w, 2 * w - 1]
    return pixels.map(x => {
        const pix = x + (e.push(0), e.shift()), col = pix > threshold, err = (pix - col) / 8
        m.forEach(x => e[x] += err)
        return col ? 255 : 0;
    })
}
//from chatGPT :( i caved, i didn't wanna write a convolver
function generateBayerMatrix(k) {
    // Base case for 1×1 matrix
    if (k === 0) {
        return [[0]];
    }
    // Recursively build the matrix of size 2^(k-1)
    const M = generateBayerMatrix(k - 1);
    const n = 1 << k;        // 2^k
    const half = n >> 1;     // 2^(k-1)
    const out = Array.from({ length: n }, () => Array(n).fill(0));
    for (let y = 0; y < half; y++) {
        for (let x = 0; x < half; x++) {
            const v = M[y][x];
            out[y][x] = 4 * v;       // top-left
            out[y][x + half] = 4 * v + 2;   // top-right
            out[y + half][x] = 4 * v + 3;   // bottom-left
            out[y + half][x + half] = 4 * v + 1; // bottom-right
        }
    }
    return out;
}
function bayer(pixels, width, threshold) {
    const matrixLevel = 4;
    const M = generateBayerMatrix(matrixLevel);
    const size = 1 << matrixLevel;
    const maxVal = size * size - 1;
    const out = new Uint8Array(pixels.length);
    for (let y = 0; y < pixels.length / width; y++) {
        for (let x = 0; x < width; x++) {
            const i = y * width + x;
            const v = pixels[i] - threshold; // grayscale 0–1
            const t = M[y % size][x % size] / maxVal;
            out[i] = v < t ? 0 : 255;
        }
    }
    return out;
}
function threshold(pixels, w, thresh) {
    const out = [];
    for (let px of pixels) {
        out.push(px > thresh ? 255 : 0);
    }
    return out;
}


function renderPreviewImage() {
    if (!imageUploadSettings.dataURL)
        return;

    //get the dither algorithm
    const ditherAlgorithms = {
        threshold: threshold,
        bayer: bayer,
        atkinson: atkinson,
    };
    const ditherAlgorithm = ditherAlgorithms[imageUploadSettings.render];

    //draw image to canvas element
    const canvElement = document.getElementById('image_preview');
    canvElement.style.display = "block";
    const img = new Image();
    img.onload = function(){
        //resize canvas appropriately
        const imageAspectRatio = img.height/img.width;
        if(imageUploadSettings.fit == 'width'){
            canvElement.width = sprites[currentSprite].width;
            canvElement.height = canvElement.width * imageAspectRatio;
        }
        else{
            canvElement.height = sprites[currentSprite].height;
            canvElement.width = canvElement.height / imageAspectRatio;
        }
        let htmlWidth,htmlHeight;
        const maxDim = 150;
        if(canvElement.height > canvElement.width){
            htmlHeight = maxDim;
            htmlWidth = maxDim * canvElement.width/canvElement.height;
        }
        else{
            htmlWidth = maxDim;
            htmlHeight = maxDim * canvElement.height/canvElement.width;
        }
        canvElement.style.width = `${htmlWidth}px`;
        canvElement.style.height = `${htmlHeight}px`;
        
        const ctx = canvElement.getContext('2d');
        ctx.drawImage(img,0,0,canvElement.width,canvElement.height);
        //get the pixels that we just drew to this canvas
        const imageData = ctx.getImageData(0,0,canvElement.width, canvElement.height);
        let outputPixels = [];
        // convert color to BW
        for (let byte = 0; byte < imageData.data.length; byte += 4) {
            const val = (255.0 - imageData.data[byte + 3]) + (imageData.data[byte + 3] / 255.0) * ((imageData.data[byte] + imageData.data[byte + 1] + imageData.data[byte + 2]) / 3.0);
            outputPixels.push(val / 255.0);
        }

        //load dithered pixels back into the image
        outputPixels = ditherAlgorithm(outputPixels, canvElement.width, 1.0 - imageUploadSettings.brightness);
        for (let p = 0; p < imageData.data.length; p += 4) {
            imageData.data[p] = outputPixels[p / 4];
            imageData.data[p + 1] = outputPixels[p / 4];
            imageData.data[p + 2] = outputPixels[p / 4];
            imageData.data[p + 3] = 255;
            // imageData.data[p + 3] = outputPixels[p / 4] ? 0 : 255;
        }
        ctx.putImageData(imageData, 0, 0);
    }
    img.src = imageUploadSettings.dataURL;
}

async function copyPreviewToCanvas(){
    pushUndoState();
    await new Promise((resolve)=>{
        const canvElement = document.getElementById('image_preview');
        canvElement.style.display = "none";
        drawDataURLToCanvas(canvElement.toDataURL(), sprites[currentSprite], 0, sprites[currentSprite].currentFrame,resolve);
    });
    document.getElementById("uploaded_image_settings").style.display = "none";
    reloadFramePreviews();
    reloadSpritePreviews();
    updateCanvas();
}
