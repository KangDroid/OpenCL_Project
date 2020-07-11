#include "OpenCLInitHelper.h"

char* fileToArray(string file_destination, int& length_return) {
    ifstream ifs(file_destination);
    ostringstream oss;
    oss << ifs.rdbuf();

    string srcStdStr = oss.str();

    length_return = srcStdStr.length();
    char* array_dyn = new char[length_return];
    for (int i = 0; i < length_return; i++) {
        array_dyn[i] = srcStdStr.at(i);
    }

    return array_dyn;
}

int main(void) {
    OpenCLInitHelper init_helper;

    // OpenCL variables
    cl_context context = 0;
    cl_command_queue commandQueue = 0;
    cl_program program = 0;
    cl_device_id device = 0;
    cl_kernel kernel = 0;
    cl_mem memObjects[2] = { 0, 0};
    cl_int errNum;

    // Create an OpenCL context on first available platform
    context = init_helper.CreateContext();
    if (context == NULL)
    {
        std::cerr << "Failed to create OpenCL context." << std::endl;
        return 1;
    }

    // Create a command-queue on the first device available
    // on the created context
    commandQueue = init_helper.CreateCommandQueue(context, &device);
    if (commandQueue == NULL)
    {
        init_helper.Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    // Create OpenCL program from HelloWorld.cl kernel source
    program = init_helper.CreateProgram(context, device, "HelloWorld.cl");
    if (program == NULL)
    {
        init_helper.Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    // Create OpenCL kernel
    kernel = clCreateKernel(program, "hello_kernel", NULL);
    if (kernel == NULL)
    {
        std::cerr << "Failed to create kernel" << std::endl;
        init_helper.Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    // Create memory objects that will be used as arguments to
    // kernel.  First create host memory arrays that will be
    // used to store the arguments to the kernel
    int length_source;
    char* source_array = fileToArray("test.txt", length_source);
    char* result_array = new char[length_source];

    if (!init_helper.CreateMemObjects(context, memObjects, source_array, length_source))
    {
        init_helper.Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    // Set the kernel arguments (result, a, b)
    errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &memObjects[0]);
    errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &memObjects[1]);
    if (errNum != CL_SUCCESS)
    {
        std::cerr << "Error setting kernel arguments." << std::endl;
        init_helper.Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    size_t globalWorkSize[1] = { length_source };
    size_t localWorkSize[1] = { 1 };

    // Queue the kernel up for execution across the array
    errNum = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL,
                                    globalWorkSize, localWorkSize,
                                    0, NULL, NULL);
    if (errNum != CL_SUCCESS)
    {
        std::cerr << "Error queuing kernel for execution." << std::endl;
        init_helper.Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    // Read the output buffer back to the Host
    errNum = clEnqueueReadBuffer(commandQueue, memObjects[1], CL_TRUE,
                                 0, length_source * sizeof(char), result_array,
                                 0, NULL, NULL);
    if (errNum != CL_SUCCESS)
    {
        std::cerr << "Error reading result buffer." << std::endl;
        init_helper.Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    // Output the result buffer
    for (int i = 0; i < length_source; i++)
    {
        std::cout << result_array[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "Executed program succesfully." << std::endl;
    init_helper.Cleanup(context, commandQueue, program, kernel, memObjects);
    delete[] source_array;
    delete[] result_array;

    return 0;
}