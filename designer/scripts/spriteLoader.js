//this is written with just regular webserial API calls!
//due to conflicts with the p5 webserial library getting it to clear out the read buffer
const commands = {
    WEBSERIAL_SPRITE_UPLOAD : 0x01,
    TAMO_HELLO : 0x02,
    REQUEST_NEXT_SPRITE_PACKET : 0x03,
    SET_IDENTITY : 0x04,
    TAMO_DISCONNECT : 0x05,
    SET_MODE : 0x06,
}

//constants that are stored in EEPROM, define which identity Tamo has when it boots
const identities = {
    TAMO : 0,
    NO_IDENTITY : 255,
    PORCINI : 1,
    BUG : 2,
    BOTO : 3,
    CUSTOM_SPRITE : 4
};

const modes = {
    NORMAL_MODE: 0,
    SLIDESHOW_MODE: 1
}

let uploadIdentity = identities.CUSTOM_SPRITE;
let uploadMode = modes.NORMAL_MODE;

//digests everything in the input buffer
async function clearReadBuffer(port) {
    const reader = port.readable.getReader();
    const timeout = 50;
    
    try {
        while (true) {
            const timeoutPromise = new Promise(resolve => 
                setTimeout(() => resolve({ value: null, done: true }), timeout)
            );
            const { value, done } = await Promise.race([
                reader.read(),
                timeoutPromise
            ]);
            
            if (done || value === null) break;
            console.log("Discarded:", value); // remove once confident
        }
    } finally {
        reader.releaseLock();
    }
}

//waits until a certain number of bytes have been read
async function readBytes(port,numberOfBytes){
    const result = new Uint8Array(numberOfBytes);
    console.log("waiting for  "+numberOfBytes+" bytes...")
    let offset = 0;
    const reader = port.readable.getReader();
    while(offset < numberOfBytes){
        const {value,done} = await reader.read();
        if(done)
            break;
        result.set(value.slice(0,numberOfBytes),offset);
        offset += value.length;
    }
    reader.releaseLock();
    console.log(result);
    return result;
}

async function transmitDataInPackets(data,port){
    // write the sprite size
    console.log(`sending data size (${data.byteLength}) as: ${data.byteLength>>8},${data.byteLength&255}`);
    writer = port.writable.getWriter();
    await writer.write(new Uint8Array([data.byteLength>>8,data.byteLength&255]));
    writer.releaseLock();

    let writeLocation = 0;
    let dotCounter = 0;
    while(true){
        //read back tamo's response
        value = await readBytes(port,2);

        console.log("received:");
        console.log(value);

        if(value[0] == commands.REQUEST_NEXT_SPRITE_PACKET){

            const packetSize = value[1];
            console.log("tamo asked for "+packetSize.toString()+" bytes...");
            
            const packet = data.slice(writeLocation,writeLocation+packetSize);

            console.log("sending: ");
            console.log(packet);

            // send the packet
            writer = port.writable.getWriter();
            writer.write(packet);
            writer.releaseLock();

            writeLocation += packetSize;
            
            //update the progress bar
            dotCounter++;
            dotCounter%=3;
            updateUploadProgressBar(writeLocation/data.byteLength*100,dotCounter);

            if(writeLocation >= data.byteLength)
                return true;

        }
        else if(value[0] == commands.TAMO_DISCONNECT){
            console.log("tamo disconnected! closing port...");
            return false;
        }
        else{
            console.log("error! unrecognized command:");
            console.log(value[0]);
            return false;
        }
    }
}

function setUploadIdentity(event){
    uploadIdentity = event.target.value;
    let src;
    if(uploadIdentity == 'TAMO')
        src = "designer/images/preview_sprites/tamo.gif";
    else if(uploadIdentity == 'PORCINI')
            src = "designer/images/preview_sprites/porcini.gif";
    else if(uploadIdentity == 'BUG')
            src = "designer/images/preview_sprites/bug.gif";
    else if(uploadIdentity == 'BOTO')
            src = "designer/images/preview_sprites/boto.gif";
    else if(uploadIdentity == 'CUSTOM_SPRITE')
            src = "designer/images/preview_sprites/custom.gif";
    else if(uploadIdentity == 'NO_IDENTITY')
            src = "designer/images/preview_sprites/egg.gif";
    document.getElementById("identity_preview_image").src = src;
}


function setUploadMode(event){
    uploadMode = event.target.value;
    let src;
    if(uploadIdentity == 'NORMAL_MODE')
        src = "designer/images/preview_sprites/tamo.gif";
    else if(uploadIdentity == 'SLIDESHOW_MODE')
            src = "designer/images/preview_sprites/porcini.gif";
    // document.getElementById("identity_preview_image").src = src;
}

async function writeTamoIdentity(){
//open the port
    let port;
    try{
        port = await navigator.serial.requestPort();
    }
    catch(error){
        console.log(error);
        return;
    }


    await port.open({ baudRate: 115200 });
    //clear buffer
    await clearReadBuffer(port);

    //say hello to tamo
    console.log("setting identity!");
    let writer = port.writable.getWriter();
    await writer.write(new Uint8Array([commands.SET_IDENTITY,identities[uploadIdentity]]));
    writer.releaseLock();
    console.log("closing serial port");
    await port.close();
}

async function uploadSpriteData(){
    //open the port
    let port;
    try{
        port = await navigator.serial.requestPort();
    }
    catch(error){
        console.log(error);
        return;
    }

    await port.open({ baudRate: 115200 });
    //clear buffer
    await clearReadBuffer(port);

    //say hello to tamo
    console.log("hello tamo!");
    let writer = port.writable.getWriter();
    await writer.write(new Uint8Array([commands.WEBSERIAL_SPRITE_UPLOAD]));
    writer.releaseLock();

    // await new Promise(resolve => setTimeout(resolve, 1000)); // give ATtiny time to respond

    console.log("getting tamo response...")
    let value = await readBytes(port,1);

    // console.log(commands.TAMO_HELLO);
    if(value[0] == commands.TAMO_HELLO){
        console.log(value);
        console.log("tamo says hello!");

        //quickly filter sprites to only send the relevant 5
        const organizedSprites = [];
        presetSpriteNames.map((name,nameIndex) => {
            let found = false;
            for(let sprite of sprites){
                if(sprite.fileName.includes(name)){
                    found = true;
                    organizedSprites[nameIndex] = sprite;
                    break;
                }
            }
            //if there isn't one, pass undefined (packSpritesIntoByteArray will substitute it for spriteNotFound)
            if(!found)
                organizedSprites[nameIndex] = undefined;
        })
        console.log(organizedSprites);
        const spriteData = packSpritesIntoByteArray(organizedSprites);
        const success = await transmitDataInPackets(spriteData,port);
        if(success){
            console.log("wrote all data to tamo!");
        }
        else{
            console.log("error!");
        }
        console.log("closing serial port");
        await port.close();
    }
    else{
        console.log("tamo didn't say hey! Received:");
        console.log(...value);
        console.log("closing serial port");
        await port.close();
    }
}