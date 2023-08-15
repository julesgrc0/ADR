import os
import sys
import pycdlib



def main(args: list[str]) -> int:
    if len(args) < 1:
        print(args)
        return 1
    
    inpath = args[0]
    filename = os.path.basename(inpath)

    if not os.path.exists(inpath):
        print("no found")
        return 1
    
    outpath = os.path.dirname(inpath) + "\\pADR.iso"
    print(f"{inpath} => {outpath}")

    iso = pycdlib.PyCdlib()
    iso.new(joliet=3)
    
    try:
        iso.add_file(inpath, joliet_path=f'/{filename};1')
        iso.write(outpath)
    except Exception as e:
        print(e)
        return 1
    iso.close()
    return 0

if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))