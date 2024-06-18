#Compiling RayTracing Code
echo "Compiling RayTracing..."
make -f OpenVCT/raytracing/Makefile

# echo "Testing RayTracing..."
# cd OpenVCT/raytracing
# ./XPLProjectionSim_GPU_docker -xml_input ./Projection.xml

# echo "Testing OpenCL..."
# gcc host.c -o host -I./usr/include ../usr/lib/x86_64-linux-gnu/libOpenCL.so
# ./host
