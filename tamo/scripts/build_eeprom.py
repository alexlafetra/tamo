# heavily stolen from claude :(

Import("env")   # PlatformIO's SCons environment — gives access to build info
import subprocess

def write_eeprom_via_pymcuprog(source, target, env):
    # 'source' = input files (your .elf), 'target' = output, 'env' = build env
    # These are SCons args — we don't use them here, but the signature is required

    upload_port = env.get("UPLOAD_PORT")
    subprocess.run([
        "pymcuprog",        # calls pymcuprog on your system PATH
        "write",            # subcommand: write data to the device

        "-t", "uart",       # transport: "uart" for a serial UPDI adapter (e.g. CH340, FT232)
                            # use "hid" if you have a PKOB/EDBG/Curiosity board

        "-d", "attiny3217", # target device

        "-u", upload_port,       # your serial port (Windows: "COM3", Mac/Linux: "/dev/ttyUSB0")

        "-b", "115200",     # baud rate — 115200 is a safe default for UPDI

        "-f", "scripts/eeprom_data.bin",  # path to your .bin or .eep, relative to project root

        "-m", "eeprom",     # memory region to write to — IMPORTANT!
                            # without this, pymcuprog defaults to flash
                            # other options: "flash", "fuses", "userrow"

        "--verify"          # reads back after writing and confirms it matches
                            # optional but strongly recommended
    ], check=True)
    # check=True means: if pymcuprog exits with an error code, raise an exception
    # and fail the PlatformIO build loudly rather than silently continuing

env.AddPostAction("upload", write_eeprom_via_pymcuprog)
# Registers the function to run AFTER the main flash upload completes
# You could also use AddPreAction if you wanted it to run before

def build_eeprom_binary(output,data):
    outputFile = open(output, "wb")
    outputFile.write(bytes(data))
    outputFile.close()

params = [
    0,      #MODE -- normal tamo mode
    255,    #IDENTITY -- unset (random identity/egg)
    255,    #HEALTH_MSB
    255,    #HEALTH_LSB
    0,      #STATUS
    16,     #SLIDESHOW_FRAMECOUNT
    1,      #SLIDESHOW_SPEED
    1,      #SLIDESHOW_SLEEP
    0       #SLIDESHOW_BLINK
]

build_eeprom_binary("scripts/eeprom_data.bin",params)
