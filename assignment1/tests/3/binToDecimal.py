import struct
import sys

array = []
filename=sys.argv[1]

with open(filename,'br') as f:
    tmp = f.read(4)
    while (tmp != b''):
        array.append(struct.unpack('<i', tmp)[0])
        tmp = f.read(4)

print(array)

