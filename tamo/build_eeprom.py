import os

def generate_eep_file(source, target, env):
    bin_file = "eeprom_data.bin"
    eep_file = "eeprom_data.eep"

    cmd = f'avr-objcopy -I binary -O ihex {bin_file} {eep_file}'
    print(f"[INFO] Generating EEPROM: {cmd}")
    env.Execute(cmd)
