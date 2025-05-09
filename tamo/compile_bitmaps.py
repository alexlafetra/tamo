Import("env")
import os
import sys
import struct

# largely taken from:
# https://github.com/sparkfun/BMPtoArray/blob/master/bmp2array.py

# file the bitmaps will be printed to
output_file = "include/bitmaps.cpp"

# folder the bitmaps are stored in
bmp_directory = "bitmaps"
directory = os.fsencode(bmp_directory)

# iterate over each file in that folder and check if it's a bitmap
for file in os.listdir(directory):
    filename = os.fsdecode(file)
    print(filename)
    # if it is a bitmap, decode it into bytes
    if filename.endswith(".bmp"):
        # get the file bytes
        inputfile = open(bmp_directory+"/"+filename,"rb")
        contents = bytearray(inputfile.read())
        inputfile.close()

        # get the image size
        data = [contents[2],contents[3],contents[4],contents[5]]
        filesize = struct.unpack("I",bytearray(data))
        print("filesize:"+str(filesize[0]))

        #Get the header offset amount
        data = [contents[10], contents[11], contents[12], contents[13]]
        offset = struct.unpack("I", bytearray(data))

        #Make a string to hold the output of our script
        arraySize = int((filesize[0] - offset[0]) / 2)
        print(arraySize)

        # get the image dimensions
        width = contents[18] | (contents[19]<<8)
        height = int(arraySize/width/2)
        print("width:"+str(width))
        print("height:"+str(height))

        outputString = "//"+filename[0:-4]+": "+str(width)+"x"+str(height)+'\r'
        outputString += "const unsigned char " + filename[0:-4] + "[" + str(int(arraySize/height)) + "] PROGMEM = {" + '\r'

        #Start coverting spots to values
        #Start at the offset and go to the end of the file
        for i in range(offset[0], filesize[0]-15, 16):
            color = 0
            print(contents[i])
            for j in range(0,16):
                if(contents[i+j] == 255):
                    color |= 1<<j

            #Add this value to the string
            outputString += hex(color) + ", "
            
        #Once we've reached the end of our input string, pull the last two
        #characters off (the last comma and space) since we don't need
        #them. Top it off with a closing bracket and a semicolon.
        outputString = outputString[:-2]
        outputString += "};"

        #Write the output string to our output file
        outfile = open(output_file,"w")
        outfile.write(outputString)
        outfile.close()
