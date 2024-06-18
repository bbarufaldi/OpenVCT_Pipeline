#Compiling RayTracing Code
echo "Compiling RayTracing Code..."
make -f OpenVCT/raytracing/Makefile

# echo "Testing OpenCL..."
# gcc host.c -o host -I./usr/include ../usr/lib/x86_64-linux-gnu/libOpenCL.so
# ./host
