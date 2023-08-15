import base64
import sys
import os

def b64(inpath: str) -> None:
    key = "=ADR=".encode("utf-8")
    outpath = os.path.dirname(inpath) + "\\ADR.b64"
    print(f"{inpath} => {outpath}")
    
    with open(inpath,"rb") as file:
        data = base64.encodebytes(file.read())
        file.close()

    with open(outpath, "wb") as file:
        file.write(key)
        file.write(data)
        file.close()

def bin(inpath: str) -> None:
    key = ord('A')
    outpath = os.path.dirname(inpath) + "\\ADR.bin"
    print(f"{inpath} => {outpath}")

    with open(inpath,"rb") as file:
        encoded = bytearray()
        for c in file.read():
            encoded.append(c ^ key)

    with open(outpath, "wb") as file:
        file.write(encoded)
        file.close()

def xor_str(data: str, key: str):
    encoded = ''.join([chr(ord(char) ^ ord(key)) for char in data])
    for char in encoded:
        hex_value = hex(ord(char))[2:].zfill(2)
        print(f"0x{hex_value}", end=', ')

def main(args: list[str]) -> int:
    if len(args) < 1:
        return 1
    
    inpath = sys.argv[1]
    if not os.path.exists(inpath):
        xor_str(inpath, 'A')
        return 1
    
    # b64(inpath)
    bin(inpath)

    return 0

if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))