#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    double r;
    double g;
    double b;
} rgb;

typedef struct {
    double h;
    double s;
    double v;
} hsv;

hsv rgb2hsv(rgb in) {
    hsv out;
    double min, max, delta;

    min = in.r < in.g ? in.r : in.g;
    min = min < in.b ? min : in.b;

    max = in.r > in.g ? in.r : in.g;
    max = max > in.b ? max : in.b;

    out.v = max; // v
    delta = max - min;
    if (delta < 0.00001) {
        out.s = 0;
        out.h = 0; // undefined, maybe nan?
        return out;
    }
    if (max > 0.0) { // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max); // s
    } else {
        // if max is 0, then r = g = b = 0
        // s = 0, h is undefined
        out.s = 0.0;
        out.h = NAN; // its now undefined
        return out;
    }
    if (in.r >= max) // > is bogus, just keeps compilor happy
        out.h = (in.g - in.b) / delta; // between yellow & magenta
    else if (in.g >= max)
        out.h = 2.0 + (in.b - in.r) / delta; // between cyan & yellow
    else
        out.h = 4.0 + (in.r - in.g) / delta; // between magenta & cyan

    out.h *= 60.0; // degrees

    if (out.h < 0.0)
        out.h += 360.0;

    return out;
}

rgb hsv2rgb(hsv in) {
    double hh, p, q, t, ff;
    long i;
    rgb out;

    if (in.s <= 0.0) { // < is bogus, just shuts up warnings
        out.r = in.v;
        out.g = in.v;
        out.b = in.v;
        return out;
    }
    hh = in.h;
    if (hh >= 360.0)
        hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch (i) {
        case 0:
            out.r = in.v;
            out.g = t;
            out.b = p;
            break;
        case 1:
            out.r = q;
            out.g = in.v;
            out.b = p;
            break;
        case 2:
            out.r = p;
            out.g = in.v;
            out.b = t;
            break;

        case 3:
            out.r = p;
            out.g = q;
            out.b = in.v;
            break;
        case 4:
            out.r = t;
            out.g = p;
            out.b = in.v;
            break;
        case 5:
        default:
            out.r = in.v;
            out.g = p;
            out.b = q;
            break;
    }
    return out;
}

void print_rgb(rgb i) {
    printf("#%x%x%x\n", (int)(i.r * 255), (int)(i.g * 255), (int)(i.b * 255));
}
void print_hsv(hsv i) {
    printf("HSV %lf %lf %lf\n", i.h, i.s, i.v);
}

void print_help() {
    //TODO
}

unsigned char reverse(unsigned char b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

rgb hash(const char * str) {
    //inspired by djb2
    unsigned char c = 0;
    uint8_t r = 251;
    uint8_t g = 241;
    uint8_t b = 239;

    while ((c = *str++)) {
        c = reverse(c);
        r = ((g << 5) + g) ^ c;
        g = ((b << 5) + b) ^ c;
        b = ((r << 5) + r) ^ c;
    }

    rgb col;

    col.r = r / 255.0;
    col.g = g / 255.0;
    col.b = b / 255.0;

    return col;
}

uint8_t uint8str(char * str) {
    uint8_t r;
    char * endptr;
    {
        errno = 0;
        r = (short)strtol(str, &endptr, 0);
        if (errno != 0) {
            perror("strtol");
            exit(EXIT_FAILURE);
        }
        if (endptr == str) {
            fprintf(stderr, "No digits were found\n");
            exit(EXIT_FAILURE);
        }
    }
    return r;
}

int main(int argc, char ** argv) {
    double saturation = 0.6;
    double value = 0.7;
    rgb start;

    start.r = 50 / 255.0;
    start.g = 150 / 255.0;
    start.b = 200 / 255.0;

    bool use_auto = 0;
    const char * text = "";
    {
        int c;
        while (1) {
            int option_index = 0;
            static struct option long_options[] = {{"saturation", required_argument, 0, 's'},
                {"value", required_argument, 0, 'v'}, {"red", required_argument, 0, 'r'},
                {"green", required_argument, 0, 'g'}, {"blue", required_argument, 0, 'b'},
                {"text", required_argument, 0, 't'}, {"auto", no_argument, 0, 'a'}, {0, 0, 0, 0}};
            c = getopt_long(argc, argv, "s:v:r:g:b:t:a", long_options, &option_index);
            if (c == -1) {
                break;
            }
            switch (c) {
                case 's':
                    saturation = uint8str(optarg) / 255.0;
                    break;
                case 'v':
                    value = uint8str(optarg) / 255.0;
                    break;
                case 't':
                    text = optarg;
                    break;
                case 'r':
                    start.r = uint8str(optarg) / 255.0;
                    break;
                case 'g':
                    start.g = uint8str(optarg) / 255.0;
                    break;
                case 'b':
                    start.b = uint8str(optarg) / 255.0;
                    break;
                case 'a':
                    use_auto = 1;
                    break;
                default:
                    print_help();
                    exit(EXIT_FAILURE);
            }
        }
    }

    if (use_auto) {
        start = hash(text);
    }

    hsv initial = rgb2hsv(start);

    initial.s = initial.s * (1.0 - saturation) + saturation;
    initial.v = initial.v * (1.0 - value) + value;
    hsv og = initial;

    const size_t textlen = strlen(text);

    //print_rgb(start);
    //print_rgb(end);
    //print_rgb(text);

    for (size_t j = 0; j < textlen; ++j) {
        hsv tmp = og;
        tmp.h = fmod(tmp.h + (30 * (double)j / textlen), 360);
        rgb start = hsv2rgb(tmp);
        const int tr = (int)(255 * (start.r));
        const int tg = (int)(255 * (start.g));
        const int tb = (int)(255 * (start.b));

        hsv tmpt = og;
        tmpt.h = fmod(tmpt.h + (180 + 30 * (double)j / textlen), 360);
        rgb tex = hsv2rgb(tmpt);
        const int mr = (int)(255 * (tex.r));
        const int mg = (int)(255 * (tex.g));
        const int mb = (int)(255 * (tex.b));
        printf("\033[48;2;%d;%d;%dm\033[38;2;%d;%d;%dm%c", tr, tg, tb, mr, mg, mb, text[j]);
    }
    printf("\033[0m");
}
