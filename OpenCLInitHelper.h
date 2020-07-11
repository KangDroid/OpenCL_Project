#include <iostream>
#include <fstream>
#include <sstream>

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

using namespace std;

class OpenCLInitHelper {
private:
    const int ARRAY_SIZE = 1000;
public:
    ///
    //  Create an OpenCL context on the first available platform using
    //  either a GPU or CPU depending on what is available.
    //
    cl_context CreateContext();

    ///
    //  Create a command queue on the first device available on the
    //  context
    //
    cl_command_queue CreateCommandQueue(cl_context context, cl_device_id *device);

    ///
    //  Create an OpenCL program from the kernel source file
    //
    cl_program CreateProgram(cl_context context, cl_device_id device, const char* fileName);

    ///
    //  Create memory objects used as the arguments to the kernel
    //  The kernel takes three arguments: result (output), a (input),
    //  and b (input)
    //
    bool CreateMemObjects(cl_context context, cl_mem memObjects[3], float *a, float *b);

    ///
    //  Cleanup any created OpenCL resources
    //
    void Cleanup(cl_context context, cl_command_queue commandQueue, cl_program program, cl_kernel kernel, cl_mem memObjects[3]);
};