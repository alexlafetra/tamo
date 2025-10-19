import os
import math

# Code help from:
# https://github.com/sparkfun/BMPtoArray/blob/master/bmp2array.py

# file the bitmaps will be printed to
output_file = "include/bitmaps.h"

# folder the bitmaps are stored in
bmp_directory = "bitmaps"

# okay, got it. The way image2cpp works is it does a pass L-->R, taking one-byte deep (8px) vertical slices. Then it does another pass, going another layer deep. Annoying.

# not using a flipped array here
def pixelArrayToByteArray(pixelArray,w,h):

    # array storing the packed bits
    bArray = []

    # if height isn't a multiple of 8, pad out bottom of array
    if h%8 :
        # how many blank arrays to add
        padHeight = 8-h%8

        # for each row needed, add in a blank row
        for i in range(0,padHeight,1):
            for j in range(0,w,1):
                pixelArray.append(0x00)
    
    # chunks of 8
    # each 'bite' is a pass, where you grab the top 8 bytes from the array at each column
    for bite in range(0,math.ceil(h/8),1):
        # move over each column
        for x in range(0,w,1):
            newByte = 0x00
            # dive down into each column
            for i in range(0,8,1):
                newByte |= pixelArray[i*w+x+bite*w*8]<<i
            bArray.append(newByte)

    return bArray

def compileBitmap(file,filename):

    # if it is a bitmap, decode it into bytes
    if file.endswith(".bmp"):

        # get the file bytes
        inputfile = open(file,"rb")
        contents = bytearray(inputfile.read())
        inputfile.close()

        # start of pixel data
        pixelArrayOffset = int.from_bytes(contents[10:13], byteorder = 'little', signed = False)
        
        # height is a signed integer
        imageHeight = abs(int.from_bytes(contents[22:23], byteorder = 'little', signed = True))
        imageWidth = int.from_bytes(contents[18:21], byteorder = 'little', signed = False)
        # bitDepth = int.from_bytes(contents[28:30], byteorder = 'little', signed = False)

        # 4 bytes per pixel
        pixelRowSize = math.ceil(4*imageWidth)
        pixelArraySize = pixelRowSize * imageHeight

        # turn this into 1 byte per pixel
        pixelArray = []
        for pixel in range(0,pixelArraySize,4):
            pixelArray.append(contents[pixelArrayOffset+pixel] & 0x01)

        cByteData = pixelArrayToByteArray(pixelArray,imageWidth,imageHeight)

        # add in variable title
        outputString = "//"+filename[0:-4]+": "+str(imageWidth)+"x"+str(imageHeight)+'\r'
        outputString += "const unsigned char " + filename[0:-4] + "[" + str(len(cByteData)) + "] PROGMEM = {\n\t"

        # formatting bytes
        for byte in range(0,len(cByteData),1):
            outputString += str(hex(cByteData[byte]))+", "
            if (byte%16 == 15) & (byte != (len(cByteData)-1)):
                outputString += "\n\t"
        
        #Once we've reached the end of our input string, pull the last two
        #characters off (the last comma and space) since we don't need
        #them. Top it off with a closing bracket and a semicolon.
        outputString = outputString[:-2]
        outputString += "\n};\n"

        #Write the output string to our output file
        outfile = open(output_file,"a")
        outfile.write(outputString)
        outfile.close()

def compileBitmaps(directory):
    # iterate over each file in that folder and check if it's a bitmap
    for file in os.listdir(directory):
        if os.path.isdir(directory+"/"+os.fsdecode(file)):

            # write a header for this new directory
            outfile = open(output_file,"a")
            outfile.write("\n//------------------------------------\n/* "+os.fsdecode(file)+" */\n//------------------------------------\n\n")
            outfile.close()

            # run fn recursively for the new file
            directoryPath = directory+"/"+os.fsdecode(file)
            # green font
            print(f"\033[34mCompiling {directoryPath}\033[0m")

            compileBitmaps(directoryPath)
        else:
            compileBitmap(directory+'/'+file,file)
    
print("\033[33mCompiling bitmap images inside \'/bitmaps\' to byte arrays...\033[0m")

#Write a header, and erase previous contents
outfile = open(output_file,"w")
outfile.write("/* BITMAPS\ncompiled with:\npython3 compile_bitmaps.py\n*/\n\n")
outfile.close()

compileBitmaps(bmp_directory)

print("\033[32mDone! Bitmaps written to include/bitmaps.h\033[0m")
