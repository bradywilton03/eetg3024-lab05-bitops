/*
 * Name: Brady Wilton
 * Date: 02/12/26
 * Program: Lab 5 - Bit Operations / Marquee
 * Description:
 *  This program will visibly cycle through the DELL Keyboard LEDs in one direction.
 *  Notably, if you would like to reverse the direction of the strobe, you may press R to have it run from right to left or L to have it run from left to right.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>
#include <linux/kd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/timeb.h>

#define MODEENABLE    1
#define MODERESTORE   0
#define MSPERSEC      1000L
#define SHIFTLEFT     1
#define SHIFTRIGHT    -1
#define LEDMIN        0x1
#define LEDMAX        0x4
#define LEDMASK       0x7
#define SHIFTDELAYMS  200

static void changeMode(int dir);
static int  kbhit(void);
static void setLeds(int ledBits);

int main(void)
{
    int direction = SHIFTLEFT;
    int led = LEDMIN;       // One hot LED. Ex: 0x4 -> 0x2 -> 0x1.
    int ch;              // Store the character read from the keyboard input.
    long lastMs, nowMs;  // Store the timestamps in total milliseconds.

    /*Struct used to store the current system time (seconds and milliseconds).*/
    struct timeb t;

    /*Initialize the timing reference.*/
    ftime(&t);
    lastMs = (long)t.time * MSPERSEC + (long)t.millitm;

    /*Enable the raw keyboard mode.*/
    changeMode(MODEENABLE);

    /*Set the initial LED state.*/
    setLeds(led & LEDMASK);

    while (1)
    {
        /*If a key is available, read it immediately. Assign actions.*/
        if (kbhit())
        {
            ch = getchar();

            if (ch == 'q' || ch == 'Q')
                break;

            if (ch == 'l' || ch == 'L')
                direction = SHIFTLEFT;

            if (ch == 'r' || ch == 'R')
                direction = SHIFTRIGHT;
        }

        /*For every 200 ms, update the LED output and shift the active bit in the current direction. This includes a wraparound.*/
        ftime(&t);
        nowMs = (long)t.time * MSPERSEC + (long)t.millitm;

        if (nowMs - lastMs >= SHIFTDELAYMS)
        {
            lastMs = nowMs;

            setLeds(led & LEDMASK);

            if (direction == SHIFTLEFT)
            {
                led <<= 1;
                if (led > LEDMAX) led = LEDMIN;
            }
            else
            {
                led >>= 1;
                if (led == 0) led = LEDMAX;
            }
        }
    }

    /*Exit: All LEDs off and restore terminal settings.*/
    setLeds(0);
    changeMode(MODERESTORE);
    return 0;
}

/*Send the specified 3-bit pattern to the keyboard LEDs using ioctl.*/
static void setLeds(int ledBits)
{
    ioctl(STDIN_FILENO, KDSETLED, ledBits);
}

/*Enable or restore raw keyboard mode to allow immediate, non-echoed key input.*/
static void changeMode(int dir)
{
    static struct termios oldt, newt;

    if (dir == MODEENABLE)
    {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    }
    else
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
}

/*Check whether or not a key has been pressed without pausing the program execution.*/
static int kbhit(void)
{
    struct timeval tv;
    fd_set rdfs;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&rdfs);
    FD_SET(STDIN_FILENO, &rdfs);

    select(STDIN_FILENO + 1, &rdfs, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &rdfs);
}
