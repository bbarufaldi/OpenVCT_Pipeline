import subprocess
import os

# List of directory paths to create
directories = [
    "OpenVCT/anatomy/vctx",
    "OpenVCT/anatomy/xml",
    "OpenVCT/deform/vctx",
    "OpenVCT/deform/xml",
    "OpenVCT/inserter/vctx",
    "OpenVCT/inserter/xml",
    "OpenVCT/noise/proj",
    "OpenVCT/raytracing/xml",
    "OpenVCT/raytracing/proj"
]

def compile_code(command):
    try:
        # Run the 'make' command with the specified Makefile

        result = subprocess.run(
            ['make', '-f', command], 
            check=True, 
            stdout=subprocess.PIPE, 
            stderr=subprocess.PIPE
        )
        
        # Print the standard output and error
        print("Compilation Output:\n", result.stdout.decode('utf-8'))
        print("Compilation Errors (if any):\n", result.stderr.decode('utf-8'))
        
    except subprocess.CalledProcessError as e:
        print(f"An error occurred during compilation: {e}")
        print("Error Output:\n", e.stderr.decode('utf-8'))

if __name__ == "__main__":
    
    # Compile C++ version of code
    print("Compiling Anatomy Code...")
    compile_code("OpenVCT/anatomy/Makefile")
    
    print("Compiling RayTracing Code...")
    compile_code("OpenVCT/raytracing/Makefile")

    # Create directories to save xml and outputs
    for directory in directories:
        os.makedirs(directory, exist_ok=True)

    # Compile Reconstruction
    os.chdir('OpenVCT/reconstruction/pydbt')
    
    print("Compiling Reconstruction Code...")
    subprocess.call(["python3", "./setup.py", "build_ext", "--inplace"])

