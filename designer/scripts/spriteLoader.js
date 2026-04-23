//this is written with just regular webserial API calls
//due to conflicts with the p5 webserial library getting it to clear out the read buffer
//and wait for a specific number of bytes to be received
const commands = {
    WEBSERIAL_HELLO : 0x67,
    TAMO_HELLO : 0x68,
    REQUEST_NEXT_SPRITE_PACKET : 0x69,
    SET_IDENTITY : 0x6A,
    TAMO_DISCONNECT : 0x6B
}

async function clearReadBuffer(port) {
    const tempReader = port.readable.getReader();
    try {
        while (true) {
            const { value, done } = await Promise.race([
                tempReader.read(),
                new Promise(resolve => setTimeout(() => resolve({ value: null, done: true }), 50))
            ]);
            if (done) break;
        }
    } finally {
        tempReader.releaseLock();
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
    return result;
}

async function transmitDataInPackets(data){
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
    await writer.write(new Uint8Array([commands.WEBSERIAL_HELLO]));
    writer.releaseLock();

    console.log("getting tamo response...")
    let value = await readBytes(port,1);
    console.log(value);
    console.log(commands.TAMO_HELLO);
    if(value[0] == commands.TAMO_HELLO){

        console.log("tamo says hello!");

        //quickly filter sprites to only send the relevant 5
        const organizedSprites = [];
        presetSpriteNames.map((name,nameIndex) => {
            let found = false;
            for(let sprite of sprites){
                if(sprite.fileName.contains(name)){
                    found = true;
                    organizedSprites[nameIndex] = sprite;
                    break;
                }
            }
            //if there isn't one, pass undefined (packSpritesIntoByteArray will substitute it for spriteNotFound)
            if(!found)
                organizedSprites[nameIndex] = undefined;
        })
        const spriteData = packSpritesIntoByteArray(organizedSprites);
        const success = await transmitDataInPackets(spriteData);
        await port.close();
    }
    console.log("port closed");
}