#include <am.h>
#include <klib-macros.h>
#include <time.h>
#include <stdlib.h>

#define W 400 //y
#define H 300 //x
static uint32_t buffer[400 * 300];

//设置渐变步数
int K = 200;
//设置时间间隔;
int delay = 20;

uint32_t colors[]=
{
    0x000000, 0xff0000, 
    0x00ff00, 0x0000ff, 
    0xffff00, 0xff00ff, 
    0x00ffff, 0xffffff
};

void draw(uint32_t color) 
{
    int w = io_read(AM_GPU_CONFIG).width;
    int h = io_read(AM_GPU_CONFIG).height;

    for(int i = 0; i < w * h; ++i)
        buffer[i] = color;     

    io_write(AM_GPU_FBDRAW,0,0,buffer,W,H,true);
}
void key()
{
    AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
    if(ev.keycode != AM_KEY_NONE)//按了键盘
    {
        if(ev.keydown)
        {
            if(ev.keycode == AM_KEY_ESCAPE)
                exit(0);
            else delay = 2; //加速
        }
        else delay = 20;
    }
}
int main()
{
    ioe_init(); // initialization for GUI
    srand(time(NULL));
    //设置第一个颜色
    uint32_t now = colors[0];

    while (1) 
    {
        //0xff00ff
        //生成随机选择的颜色的下标
        int idx = rand() % (sizeof(colors) / sizeof(colors[0]));
        //取出得到的颜色
        uint32_t tmp = colors[idx];

        //计算刚开始的RGB分量
        int R0 = (now >> 16) & 0xFF;
        int G0 = (now >> 8) & 0xFF;
        int B0 = now & 0xFF;
        //计算目标颜色的RGB分量
        int RK = (tmp >> 16) & 0xFF;
        int GK = (tmp >> 8) & 0xFF;
        int BK = tmp & 0xFF;
        for(int i = 0;i <= K; ++i)
        {
			key();
            int R1 = R0 + (RK - R0) * i / K;
            int G1 = G0 + (GK - G0) * i / K;
            int B1 = B0 + (BK - B0) * i / K;

            uint32_t color = (R1 << 16) | (G1 << 8) | B1;
            draw(color);
			long start = io_read(AM_TIMER_UPTIME).us;
            while ((io_read(AM_TIMER_UPTIME).us - start) < delay * 1000);
//			delay += 2;
        }
		now = tmp;
    }
    return 0;
}
