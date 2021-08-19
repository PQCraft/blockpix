#include <blockpix.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int maxi = 50; // Number of iter

typedef struct HsvColor
{
    unsigned char h;
    unsigned char s;
    unsigned char v;
} HsvColor;

uint32_t HsvToRgb(HsvColor hsv)
{
    unsigned char region, remainder, p, q, t;
    
    uint8_t r, g, b;

    if (hsv.s == 0)
    {
        r = hsv.v;
        g = hsv.v;
        b = hsv.v;
        return r << 16 | g << 8 | b;
    }

    region = hsv.h / 43;
    remainder = (hsv.h - (region * 43)) * 6;

    p = (hsv.v * (255 - hsv.s)) >> 8;
    q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
    t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
    case 0:
        r = hsv.v;
        g = t;
        b = p;
        break;
    case 1:
        r = q;
        g = hsv.v;
        b = p;
        break;
    case 2:
        r = p;
        g = hsv.v;
        b = t;
        break;
    case 3:
        r = p;
        g = q;
        b = hsv.v;
        break;
    case 4:
        r = t;
        g = p;
        b = hsv.v;
        break;
    default:
        r = hsv.v;
        g = p;
        b = q;
        break;
    }

    return r << 16 | g << 8 | b;
}

void create_mandelbrot()
{
    for (int row = 0; row < bp_height; row++)
    {
        for (int col = 0; col < bp_width; col++)
        {
            double c_re = (col - bp_width / 2) * 4.0 / bp_width;
            double c_im = (row - bp_height / 2) * 4.0 / bp_width;
            double x = 0, y = 0;
            int iter = 0;

            while (x * x + y * y < 4 && iter < maxi)
            {
                double x_new = x * x - y * y + c_re;
                y = 2 * x * y + c_im;
                x = x_new;
                iter++;
            }
            if (iter < maxi)
            {
                uint32_t tmp = HsvToRgb((HsvColor){(int)(255 * iter / maxi), 255, 255});
                bp_set(col, row, tmp);
            }
        }
    }
}

int main(int argc, char **argv)
{
    if (argc > 2)
        return 1;
    if (argc == 2)
    {
        maxi = atoi(argv[1]);
    }

    srand(clock());

    bp_init();

    create_mandelbrot();
    bp_render();

    bp_quit();
}
