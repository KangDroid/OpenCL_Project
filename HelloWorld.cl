__kernel void hello_kernel(__global const char *a,
						__global char *result)
{
    int gid = get_global_id(0);

    result[gid] = a[gid] - 2;
}
