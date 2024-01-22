#!/usr/bin/env python3

import sys

'''
    LORA PACKAGE FORMAT
    -------------------

    A buffer of 19 bits is used to store the LoRa package.
    This buffer contains the following fields (format: [begin bit, end bit]):

    - [0, 7]. 8 reserved for ID. 
    - [8, 15]. 8 bits reserved for counter. 
    - [16, 18]. 3 bits reserved for NN classification. 

    Example: 00000000 00000100 110 means:
    - ID: 0.
    - Counter: 4.
    - NN classification: true for class 1, true for class 2, false for class 3.
'''

def dissassemble_lora_package(package: list):
    '''
    Function: dissassemble_lora_package
    Brief: Dissassembles a LoRa package into its fields.
    Args:
        package (list): LoRa package to be dissassembled.
    Returns:
        - (str): ESP-ID.
        - (int): Year.
        - (int): Month.
        - (int): Day.
        - (int): Hour.
        - (int): Minute.
        - (int): Second.
        - (int): Neural Network Output.
        - (int): Checksum.
    '''

    # Dissassemble package.
    esp_id = ''.join(chr(package[i]) for i in range(6))

    year = package[6] << 4 | package[7] >> 4
    month = package[7] & 0x0f
    day = (package[8] & 0xf8) >> 3
    hour = ((package[8] & 0x07) << 2) | ((package[9] & 0xc0) >> 6)
    minute = package[9] & 0x3f 
    second = (package[10] & 0xfc) >> 2
    output = package[10] & 0x03
    checksum = package[11]

    return esp_id, year, month, day, hour, minute, second, output, checksum


# direct execution: testing

if __name__ == '__main__':
    parameters = [int(sys.argv[i]) for i in range(1, len(sys.argv))]

    esp_id, year, month, day, hour, minute, second, output, checksum = dissassemble_lora_package(parameters)

    print('ESP-ID: {}'.format(esp_id))
    print('Year: {}'.format(year))
    print('Month: {}'.format(month))
    print('Day: {}'.format(day))
    print('Hour: {}'.format(hour))
    print('Minute: {}'.format(minute))
    print('Second: {}'.format(second))
    print('Output: {}'.format(output))
    print('Checksum: {}'.format(checksum))

