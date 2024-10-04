import pyopencl as cl
import subprocess

# Get available platforms
platforms = cl.get_platforms()

if not platforms:
    print("Error: No platforms found!")
    exit(-1)

# Iterate through platforms and list devices
for i, platform in enumerate(platforms):
    # Get platform info
    platform_name = platform.get_info(cl.platform_info.NAME)
    platform_version = platform.get_info(cl.platform_info.VERSION)
    platform_vendor = platform.get_info(cl.platform_info.VENDOR)
    print(f"Platform {i + 1}: {platform_name}")
    print(f"  Vendor: {platform_vendor}")
    print(f"  Version: {platform_version}")

    # Get devices for the current platform
    devices = platform.get_devices()

    if not devices:
        print("  Error: No devices found for this platform!")
        continue

    # Iterate through devices and print details
    for j, device in enumerate(devices):
        device_name = device.get_info(cl.device_info.NAME)
        device_type = device.get_info(cl.device_info.TYPE)
        
        print(f"  Device {j + 1}: {device_name}")
        
        # Identify device type
        if device_type == cl.device_type.CPU:
            print("    Type: CPU")
        elif device_type == cl.device_type.GPU:
            print("    Type: GPU")
        elif device_type == cl.device_type.ACCELERATOR:
            print("    Type: Accelerator")
        else:
            print("    Type: Other/Unknown")

    # Optionally, select the first device for context creation
    dev = devices[0]
    context = cl.Context([dev])
    print(f"\nContext created with device: {dev.get_info(cl.device_info.NAME)}")

# Run nvidia-smi command to check for GPUs
try:
    result = subprocess.run(['nvidia-smi'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if result.returncode == 0:
        print("nvidia-smi output:\n", result.stdout)
    else:
        print("No GPUs found or nvidia-smi command failed.")
except FileNotFoundError:
    print("nvidia-smi not found. Are NVIDIA drivers installed?")