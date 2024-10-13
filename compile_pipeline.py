import subprocess

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
    
    #Compile C++ version of code
    print("Compiling Anatomy Code...")
    compile_code("OpenVCT/anatomy/Makefile")
    
    print("Compiling RayTracing Code...")
    compile_code("OpenVCT/raytracing/Makefile")
