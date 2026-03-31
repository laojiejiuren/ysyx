#include <am.h>
#include <klib-macros.h>

#define W 400
#define H 300

void draw(uint32_t color) 
{
    int w = io_read(AM_GPU_CONFIG).width;
    int h = io_read(AM_GPU_CONFIG).height;

    static uint32_t buffer[400 * 300];

    for(int i=0; i < w * h; ++i)
        buffer[i] = color;

    io_write(AM_GPU_FBDRAW,0,0,buffer,w,h,true);
}

int main()
{
    ioe_init(); // initialization for GUI
    while (1) 
    {
        draw(0x00000000);
    }
    return 0;
}

