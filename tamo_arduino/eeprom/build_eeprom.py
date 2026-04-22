import os
Import("env")

def generate_eep_file(source, target, env):

    cmd = f'avr-objcopy -I binary -O ihex {source} {target}'
    print("\033[33mBuilding EEPROM...\033[0m")
    fileSize = os.path.getsize(source)
    print(f"\033[34mWriting {fileSize} byte(s) into {target}!\033[0m")
    env.Execute(cmd)
    print("\033[32mDone.\033[0m")

generate_eep_file("eeprom/eeprom_data.bin","eeprom/eeprom_data.eep",env)
