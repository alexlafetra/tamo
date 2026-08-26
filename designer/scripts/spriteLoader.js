//this is written with just regular webserial API calls!
//due to conflicts with the p5 webserial library getting it to clear out the read buffer
const commands = {
    WEBSERIAL_SPRITE_UPLOAD : 0x01,
    TAMO_HELLO : 0x02,
    REQUEST_NEXT_DATA_PACKET : 0x03,
    SET_IDENTITY : 0x04,
    TAMO_DISCONNECT : 0x05,
    SET_MODE : 0x07,
    WEBSERIAL_SLIDESHOW_UPLOAD : 0x08,
    INITIATE_SPRITE_SWAP : 0x09,
    SPRITE_SWAP_READY : 0x0A,
    EXPORT_SPRITE : 0x0D,
    EXPORT_SPRITE_READY : 0x0E
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

const TAMO_SERIAL_BUFFER_SIZE = 64;

let uploadIdentity = identities.CUSTOM_SPRITE;
let uploadMode = modes.NORMAL_MODE;


//looks like this always closes the port early/cuts off some final bytes when its
//run the first time on a page...
async function emulateSpriteSwap(leader){

    //array to hold the received data
    const newSpriteData = new Uint8Array(320);
    //array holding the current data
    const currentSpriteData = getCompressedSpriteData();

    let otherSpriteID = identities.NO_IDENTITY;
    const thisSpriteID = identities.CUSTOM_SPRITE;

    let writer;

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


    if(leader){
        console.log("initializing sprite swap!");
        writer = port.writable.getWriter();
        await writer.write(new Uint8Array([commands.INITIATE_SPRITE_SWAP]));
        writer.releaseLock();
        //wait for response from tamo
        const response = await readBytes(port,2);
        if(response[0] != commands.SPRITE_SWAP_READY){
            console.log("other tamo wasn't ready!");
            return;
        }
        //store other id
        otherSpriteID = response[1];
    }
    else{
        const startCommand = await readBytes(port,1,20000);
        if(startCommand[0] != commands.INITIATE_SPRITE_SWAP)
            return;
    }

    //tell other tamo you're ready, and your identity
    writer = port.writable.getWriter();
    await writer.write(new Uint8Array([commands.SPRITE_SWAP_READY,identities.CUSTOM_SPRITE]));
    writer.releaseLock();

    if(!leader){
        const response = await readBytes(port,2);
        if(response[0] != commands.SPRITE_SWAP_READY){
            console.log("other tamo wasn't ready!");
            return;
        }
        //store other id
        otherSpriteID = response[1];
    }

    let matchingIdentity = "NO_IDENTITY";
    for(let id in identities){
        if(identities[id] == otherSpriteID)
            matchingIdentity = id;
    }
    console.log(`connected tamo is: ${matchingIdentity}`);

    //ask for the first packet
    if(leader){
        writer = port.writable.getWriter();
        await writer.write(new Uint8Array([commands.REQUEST_NEXT_DATA_PACKET,TAMO_SERIAL_BUFFER_SIZE]));
        writer.releaseLock();
    }


    console.log("beginning swap!");
    let read_location = 0;

    const receiveSpritePacket = async () => {
        //clear buffer
        await clearReadBuffer(port);
        //request data from follower
        console.log("requesting data packet...");
        writer = port.writable.getWriter();
        await writer.write(new Uint8Array([commands.REQUEST_NEXT_DATA_PACKET,TAMO_SERIAL_BUFFER_SIZE]));
        writer.releaseLock();

        //read in new data
        const data = await readBytes(port,TAMO_SERIAL_BUFFER_SIZE);
        // console.log("received data packet:",data);
        console.log("received data packet!",data);
        return data;
    }
    const sendSpritePacket = async () => {
        console.log("waiting for data packet request...");
        const response = await readBytes(port,2);
        if(response[0] != commands.REQUEST_NEXT_DATA_PACKET)
            return response[0];
        const sentData = currentSpriteData.slice(read_location,read_location+response[1]);
        console.log("sending data packet!",sentData);
        
        // writer = port.writable.getWriter();
        // await writer.write(sentData);
        // writer.releaseLock();
        const writer = port.writable.getWriter();
        try {
            await writer.write(sentData);
        } finally {
            writer.releaseLock();
        }
        await port.writable.close();
        return response[1];
    }

    while(read_location < 320){

        if(leader){
            let data;
            let sentBytes = 0;
            if(otherSpriteID == identities.CUSTOM_SPRITE){
                data = await receiveSpritePacket();
            }
            if(thisSpriteID == identities.CUSTOM_SPRITE){
                sentBytes = await sendSpritePacket();
                if(sentBytes == commands.TAMO_DISCONNECT){
                    console.log("tamo disconnecting!");
                    break;
                }
            }
            if(otherSpriteID == identities.CUSTOM_SPRITE){
                //save to array
                newSpriteData.set(data,read_location);
            }
            read_location += sentBytes;
        }
        else{
            let sentBytes = 0;
            if(thisSpriteID == identities.CUSTOM_SPRITE){
                sentBytes = await sendSpritePacket();
                if(sentBytes == commands.TAMO_DISCONNECT){
                    console.log("tamo disconnecting!");
                    break;
                }
            }
            if(otherSpriteID == identities.CUSTOM_SPRITE){
                let data = await receiveSpritePacket();
                newSpriteData.set(data,read_location);
            }
            read_location += sentBytes;
        }

        updateTransmissionProgressBar(read_location/320*100,"swapping","swapped!");
    }

    updateTransmissionProgressBar(100,"swapping","swapped!");

    //tell tamo you're done
    // writer = port.writable.getWriter();
    // try {
    //     await writer.write([commands.TAMO_DISCONNECT]);
    // } finally {
    //     writer.releaseLock();
    // }
    await port.writable.close();

    //close the port
    await port.close();

    //parse the received bytes!
    if(otherSpriteID == identities.CUSTOM_SPRITE){
        convertByteArrayToSprite(newSpriteData);
    }
}

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


async function readBytes(port, numberOfBytes, timeout = 1000) {
    const result = new Uint8Array(numberOfBytes);
    let offset = 0;
    //get a reader
    const reader = port.readable.getReader();

    //try/catch block to help handle serial timeouts
    try {
        while (offset < numberOfBytes) {
            //store ref to timeout so it can be cancelled later
            let timeoutRef;
            //second try catch block
            try {
                //promise.race() to only execute what happens first
                const { value, done } = await Promise.race([
                    //start the read from the serial port
                    reader.read(),
                    //timeout
                    new Promise((_, reject) => {
                        timeoutRef = setTimeout(() => {
                            reject(new Error("serial connection timed out with no response! closing port"));
                        }, timeout);
                    })
                ]);
                //clear the timeout if you make it to this point (meaning the read happened)
                clearTimeout(timeoutRef);

                if (done)
                    break;

                const remaining = numberOfBytes - offset;
                const chunk = (value.length > remaining)?value.slice(0, remaining):value;

                result.set(chunk, offset);
                offset += chunk.length;

            }
            catch (error) {
                //if the read timed out, gracefully (lol) close the port
                await reader.cancel();
                reader.releaseLock();
                await port.close();
                throw error;
            }
        }

    }
    finally {
        reader.releaseLock();
    }
    return result;
}

async function transmitDataInPackets(data,port){
    // write the sprite size
    console.log(`sending data size (${data.byteLength}) as: ${data.byteLength>>8},${data.byteLength&255}`);
    writer = port.writable.getWriter();
    await writer.write(new Uint8Array([data.byteLength>>8,data.byteLength&255]));
    writer.releaseLock();

    let writeLocation = 0;
    while(true){
        //read back tamo's response
        const command = await readBytes(port,2);

        console.log("received:");
        console.log(command[0]);

        //send the next data packet
        if(command[0] == commands.REQUEST_NEXT_DATA_PACKET){
            console.log('getting packet size...');
            packetSize = command[1];
            console.log("tamo asked for "+packetSize.toString()+" bytes...");

            if(writeLocation >= data.byteLength){
                console.log("uh oh! tamo is requesting data but there's none left");
                continue;
            }
            
            const packet = data.slice(writeLocation,writeLocation+packetSize);

            console.log("sending: ");
            console.log(packet);

            // send the packet
            writer = port.writable.getWriter();
            await writer.write(packet);
            writer.releaseLock();

            writeLocation += packetSize;
            
            //update the progress bar
            updateTransmissionProgressBar(writeLocation/data.byteLength*100,"uploading","sent sprites!");
        }
        //transmission done! safe to close the port now
        else if(command[0] == commands.TAMO_DISCONNECT){
            console.log("tamo disconnected! closing port...");
            return true;
        }
        else{
            console.log("error! unrecognized command:");
            console.log(command[0]);
            return false;
        }
    }
    return true;
}

function setUploadIdentity(val){
    uploadIdentity = val;
    writeTamoIdentity();
}

function setUploadMode(event){
    uploadMode = event.target.value;
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

async function writeTamoMode(which){
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
    const modeValues = {
        slideshow:modes.SLIDESHOW_MODE,
        sprite:modes.NORMAL_MODE
    };

    //clear buffer
    await clearReadBuffer(port);

    //say hello to tamo
    console.log("setting mode!");
    let writer = port.writable.getWriter();
    await writer.write(new Uint8Array([commands.SET_MODE,modeValues[which]]));
    writer.releaseLock();
    console.log("closing serial port");
    await port.close();
}

function uploadData(){
    if(settings.type == 'sprite')
        uploadSpriteData();
    else if(settings.type == 'slideshow')
        uploadSlideshowData();

}

//organizes sprites into the correct order for sending to tamo, then
//converts them to a byte array
function getCompressedSpriteData(){
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
    return packSpritesIntoByteArray(organizedSprites,32);
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

    console.log("getting tamo response...")
    let value = await readBytes(port,1);

    if(value[0] == commands.TAMO_HELLO){
        console.log(value);
        console.log("tamo says hello!");

        const spriteData = getCompressedSpriteData();
        const success = await transmitDataInPackets(spriteData,port);
        if(success){
            console.log("wrote all data to tamo!");
        }
        else{
            console.log("error!");
        }
        console.log("closing serial port");

        //important! this caused a nasty bug by closing the port before tamo had read in all the data.
        //make sure you don't do this before getting an "OK" from the attiny
        await port.close();
    }
    else{
        console.log("tamo didn't say hey! Received:");
        console.log(...value);
        console.log("closing serial port");
        await port.close();
    }
}

async function uploadSlideshowData(){
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
    await writer.write(new Uint8Array([commands.WEBSERIAL_SLIDESHOW_UPLOAD,sprites[0].frames.length,settings.slideshowSpeed,settings.slideshowSleepTime,settings.slideshowBlinkInterval]));
    writer.releaseLock();

    console.log("getting tamo response...")
    let value = await readBytes(port,1);

    // console.log(commands.TAMO_HELLO);
    if(value[0] == commands.TAMO_HELLO){
        console.log("tamo says hello!");

        const spriteData = packSpritesIntoByteArray(sprites,256);
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

async function downloadSpriteData(){
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
    let writer = port.writable.getWriter();
    await writer.write(new Uint8Array([commands.EXPORT_SPRITE]));
    writer.releaseLock();

    console.log("getting tamo response...")
    let value = await readBytes(port,1);
    console.log(value);

    if(value[0] == commands.EXPORT_SPRITE_READY){

        const data = new Uint8Array(320);
        for(let i = 0; i<320; i+=64){
            //request another data packet
            let writer = port.writable.getWriter();
            await writer.write(new Uint8Array([commands.REQUEST_NEXT_DATA_PACKET,TAMO_SERIAL_BUFFER_SIZE]));
            writer.releaseLock();

            //receive data and store in array
            const chunk = await readBytes(port,TAMO_SERIAL_BUFFER_SIZE);
            data.set(chunk,i);

            updateTransmissionProgressBar(i/320*100,"downloading","downloaded!");
        }

        updateTransmissionProgressBar(100,"downloading","downloaded!");

        //tell tamo you're done
        writer = port.writable.getWriter();
        await writer.write(new Uint8Array([commands.TAMO_DISCONNECT]));
        writer.releaseLock();
        await port.close();

        //parse the received bytes!
        convertByteArrayToSprite(data);
    }
    else{
        console.log("tamo didn't say hey! Received:");
        console.log(...value);
        console.log("closing serial port");
        await port.close();
    }
}