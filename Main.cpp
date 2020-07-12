#include "OpenCLInitHelper.h"

char* fileToArray(string file_destination, int& length_return) {
    ifstream ifs(file_destination);
    ostringstream oss;
    oss << ifs.rdbuf();

    string srcStdStr = oss.str();

    length_return = srcStdStr.length();
    cout << srcStdStr.length() << endl;
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
    char* source_array = fileToArray("test.mp4", length_source);
    char* result_array = new char[length_source];
    int to_cut = 8192*4;
    int iter_count = length_source / to_cut;
    int iter_remainder = length_source % to_cut;
    if (iter_remainder > 1) {
        iter_count++;
    }
    cout << iter_count << endl;
    for (int i = 0; i < iter_count; i++) {
        int length_to_send = to_cut;
        char* actual_array = source_array + (i * to_cut);

        if (i == iter_count - 1) {
            length_to_send = iter_remainder;
        }
        if (!init_helper.CreateMemObjects(context, memObjects, actual_array, length_to_send))
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

        size_t globalWorkSize[1] = { length_to_send };
        size_t localWorkSize[1] = { 1 };

        // Queue the kernel up for execution across the array
        errNum = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL,
                                        globalWorkSize, localWorkSize,
                                        0, NULL, NULL);
        if (errNum != CL_SUCCESS)
        {
            std::cerr << "Error queuing kernel for execution." << std::endl;
            cerr << errNum << endl;
            init_helper.Cleanup(context, commandQueue, program, kernel, memObjects);
            return 1;
        }

        // Read the output buffer back to the Host
        errNum = clEnqueueReadBuffer(commandQueue, memObjects[1], CL_TRUE,
                                    0, length_to_send * sizeof(char), result_array,
                                    0, NULL, NULL);
        if (errNum != CL_SUCCESS)
        {
            std::cerr << "Error reading result buffer." << std::endl;
            init_helper.Cleanup(context, commandQueue, program, kernel, memObjects);
            return 1;
        }

        // Output the result buffer
        ofstream outfile("enc_test.mp4", ios::app);
        for (int i = 0; i < length_to_send; i++)
        {
            outfile << result_array[i];
        }
        outfile.close();
        //std::cout << "Executed program succesfully." << std::endl;
    }

    init_helper.Cleanup(context, commandQueue, program, kernel, memObjects);
    delete[] source_array;
    delete[] result_array;

    return 0;
}