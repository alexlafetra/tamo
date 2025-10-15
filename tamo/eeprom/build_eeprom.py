import os
Import("env")

def generate_eep_file(source, target, env):

    cmd = f'avr-objcopy -I binary -O ihex {source} {target}'
    print(f"Building EEPROM...")
    fileSize = os.path.getsize(source)
    print(f"Writing {fileSize} byte(s) into {target}!")
    env.Execute(cmd)
    print('Done.')

generate_eep_file("eeprom/eeprom_data.bin","eeprom/eeprom_data.eep",env)
