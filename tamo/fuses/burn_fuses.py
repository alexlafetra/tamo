# Notes from figuring this out, for historical safekeeping:

# burning fuses
# rn, the program takes up: 12,032 bytes, so BOOTEND could be twice that and still leave room for 2x sprites?
# BOOTEND should be 94 ==> 0x5E
# APPEND should be 0 ==> 0x00 (the rest of flash)
# rn, sprites take up 2342 byes ==> 2560 to be divisible by 256 ==> 0x0A

# writing fuses:
# pymcuprog write --tool uart --device attiny3217 --uart /dev/cu.usbmodem14101 --clk 230400 --memory fuses --offset 7 --literal 0x00 0x5E 
# reading fuses:
# pymcuprog read --tool uart --device attiny3217 --uart /dev/cu.usbmodem14101 --clk 230400 --memory fuses

# notes on BOOTEND vv
# https://onlinedocs.microchip.com/oxy/GUID-A2109DC3-B5FF-4E1B-BDB5-622C21D35F43-en-US-5/GUID-B4C7F32B-FEF9-4930-B092-2E702953E23E.html
# getting no program running when writing these bytes doe

# OKAY! So fuse 7 is APPEND
# Fuse 8 is BOOTEND (weird that they did it the other way around, idk)

# When BOOTEND is 0x00, the whole flash is BOOT (which is the only way I can get the program to run rn)
# When APPEND is 0x00, everything from BOOT onwards is APPCODE section

# docs say BOOT can write to APPCODE

# okay, so what needs to happen next is:
# i need to specify in the linker file where the bitmaps should go (APPCODE or APPDATA, gotta see what the disadvantages are), and that the code should go in BOOT


Import("env")   # PlatformIO's SCons environment — gives access to build info
import subprocess


# pymcuprog write --tool uart --device attiny3217 --uart /dev/cu.usbmodem14101 --clk 230400 --memory fuses --offset 7 --literal 0x00 0x5E 

def burn_memory_fuses(source,target,env):
    # 'source' = input files (your .elf), 'target' = output, 'env' = build env
    upload_port = env.get("UPLOAD_PORT")
    subprocess.run([
        "pymcuprog",        # calls pymcuprog on your system PATH
        "write",            # subcommand: write data to the device

        "-t", "uart",       # transport: "uart" for a serial UPDI adapter (e.g. CH340, FT232)
                            # use "hid" if you have a PKOB/EDBG/Curiosity board

        "-d", "attiny3217", # target device

        "-u", upload_port,       # your serial port (Windows: "COM3", Mac/Linux: "/dev/ttyUSB0")

        "--clk", "230400",
        "--memory", "fuses",
        "--offset","7",
        "--literal", "0x00", "0x5E"
    ], check=True)
    # check=True means: if pymcuprog exits with an error code, raise an exception
    # and fail the PlatformIO build loudly rather than silently continuing

env.AddPostAction("upload", burn_memory_fuses)
# Registers the function to run AFTER the main flash upload completes
# You could also use AddPreAction if you wanted it to run before
