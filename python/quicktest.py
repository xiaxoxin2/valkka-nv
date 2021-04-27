from valkka.nv import cuda_ok, NVgetDevices

print("Cuda status:", cuda_ok)
devices = NVgetDevices()
print(devices)
