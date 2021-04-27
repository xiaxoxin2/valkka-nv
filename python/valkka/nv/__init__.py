from .valkka_nv import * 
from .version import checkVersion, getVersion

cuda_ok = True

checkVersion()

cuda_ok = NVcuInit()
if not cuda_ok:
    print("WARNING: valkka.nv could not be initialized:\
there's something wrong with your CUDA installation")
