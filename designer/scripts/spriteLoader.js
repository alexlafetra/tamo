const commands = {
    WEBSERIAL_HELLO : 0x67,
    TAMO_HELLO : 0x68,
    REQUEST_NEXT_SPRITE_PACKET : 0x69,
    SET_IDENTITY : 0x6A,
    TAMO_DISCONNECT : 0x6B
}

class SerialInterface{
    constructor(){
        this.connected = false;
        this.port = createSerial();
        this.options = {
            baudRate: 9600,
            bufferSize: 255
        };
    }
    async connect(){
        await this.port.open(this.options.baudRate);
        if(this.port.opened()){
            this.connected = true;
            console.log(this.port.port.getInfo());
            document.getElementById("serial_connect_button").innerText = "disconnect from tamo";
        }
    };
    disconnect(){
        if(this.port.opened()){
            this.port.close();
            document.getElementById("serial_connect_button").innerText = "connect to tamo";
        }
        this.connected = false;
    }
    write(val){
        this.port.write(val);
    }
    read(size){
        return this.port.read(size);
    }
    read(){
        return this.port.read();
    }
}

const interface = new SerialInterface();

async function toggleSerialConnection(){
    if(interface.connected)
        interface.disconnect();
    else
        await interface.connect();
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
    // console.log("received:");
    // console.log(result);
    reader.releaseLock();
    return result;
}

async function uploadSpriteData(){


    //open the port
    const port = await navigator.serial.requestPort();
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
        // const spriteData = convertSpriteToByteArray(sprites[0]);
        const spriteData = packAllSpritesIntoByteArray();
        let sending = true;

        // write the sprite size
        console.log("sending sprite size...");
        console.log(spriteData.byteLength);
        writer = port.writable.getWriter();
        await writer.write(new Uint8Array([spriteData.byteLength>>8,spriteData.byteLength&255]));
        writer.releaseLock();

        let writeLocation = 0;
        while(sending){
            //read back tamo's response
            value = await readBytes(port,2);

            console.log("received:");
            console.log(value);
            if(value[0] == commands.REQUEST_NEXT_SPRITE_PACKET){
                const packetSize = value[1];
                console.log("tamo asked for "+packetSize.toString()+" bytes...");
                
                const data = spriteData.slice(writeLocation,writeLocation+packetSize);

                console.log("sending: ");
                console.log(data);

                // write the sprite size
                writer = port.writable.getWriter();
                writer.write(data);
                writer.releaseLock();

                writeLocation += packetSize;
                if(writeLocation >= spriteData.byteLength)
                    break;

            }
            else if(value[0] == commands.TAMO_DISCONNECT){
                console.log("tamo disconnected! closing port...");
                await port.close();
                sending = false;

            }
        }
    }
    console.log("port closed");
}