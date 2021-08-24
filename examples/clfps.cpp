#ifdef _WIN32
#warning Cannot compile for Windows
int main() {return 1;}
#else
/*
	OneLoneCoder.com - Command Line First Person Shooter (FPS) Engine
	"Why were games not done like this is 1990?" - @Javidx9

	License
	~~~~~~~
	Copyright (C) 2018  Javidx9
	This program comes with ABSOLUTELY NO WARRANTY.
	This is free software, and you are welcome to redistribute it
	under certain conditions; See license for details. 
	Original works located at:
	https://www.github.com/onelonecoder
	https://www.onelonecoder.com
	https://www.youtube.com/javidx9

	GNU GPLv3
	https://github.com/OneLoneCoder/videos/blob/master/LICENSE

	From Javidx9 :)
	~~~~~~~~~~~~~~~
	Hello! Ultimately I don't care what you use this for. It's intended to be 
	educational, and perhaps to the oddly minded - a little bit of fun. 
	Please hack this, change it and use it in any way you see fit. You acknowledge 
	that I am not responsible for anything bad that happens as a result of 
	your actions. However this code is protected by GNU GPLv3, see the license in the
	github repo. This means you must attribute me if you use it. You can view this
	license here: https://github.com/OneLoneCoder/videos/blob/master/LICENSE
	Cheers!

	Background
	~~~~~~~~~~
	Whilst waiting for TheMexicanRunner to start the finale of his NesMania project,
	his Twitch stream had a counter counting down for a couple of hours until it started.
	With some time on my hands, I thought it might be fun to see what the graphical
	capabilities of the console are. Turns out, not very much, but hey, it's nice to think
	Wolfenstein could have existed a few years earlier, and in just ~200 lines of code.

	IMPORTANT!!!!
	~~~~~~~~~~~~~
	READ ME BEFORE RUNNING!!! This program expects the console dimensions to be set to 
	120 Columns by 40 Rows. I recommend a small font "Consolas" at size 16. You can do this
	by running the program, and right clicking on the console title bar, and specifying 
	the properties. You can also choose to default to them in the future.
	
	Controls: A = Turn Left, D = Turn Right, W = Walk Forwards, S = Walk Backwards

	Future Modifications
	~~~~~~~~~~~~~~~~~~~~
	1) Shade block segments based on angle from player, i.e. less light reflected off
	walls at side of player. Walls straight on are brightest.
	2) Find an interesting and optimised ray-tracing method. I'm sure one must exist
	to more optimally search the map space
	3) Add bullets!
	4) Add bad guys!

	Author
	~~~~~~
	Twitter: @javidx9
	Blog: www.onelonecoder.com

	Video:
	~~~~~~	
	https://youtu.be/xW8skO7MFYw

	Last Updated: 27/02/2017
*/

#include <algorithm>
#include <chrono>
#include <iostream>
#include <utility>
#include <vector>
using namespace std;

#include <stdio.h>
//#include <windows.h>
#include <X11/Xlib.h>
#include <math.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <wchar.h>
#include <blockpix.h>

int nScreenWidth = 160; // Console Screen Size X (columns)
int nScreenHeight = 40; // Console Screen Size Y (rows)
int nMapWidth = 16;     // World Dimensions
int nMapHeight = 16;

float fPlayerX = 14.7f; // Player Start Position
float fPlayerY = 5.09f;
float fPlayerA = 0.0f;        // Player Start Rotation
float fFOV = 3.14159f / 4.0f; // Field of View
float fDepth = 16.0f;         // Maximum rendering distance
float fSpeed = 5.0f;          // Walking Speed

struct termios tu_term, tu_old_term;
bool tu_textlock = false;

int kbhit()
{
    static const int STDIN = 0;
    static bool initialized = false;
    if (!initialized)
    {
        struct termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = true;
    }
    int bytes;
    ioctl(STDIN, FIONREAD, &bytes);
    return bytes;
}

void tu_lockTerm()
{
    if (!tu_textlock)
    {
        tcgetattr(0, &tu_term);
        tcgetattr(0, &tu_old_term);
        tu_term.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(0, TCSANOW, &tu_term);
        tu_textlock = true;
    }
}

void tu_unlockTerm()
{
    if (tu_textlock)
    {
        tcsetattr(0, TCSANOW, &tu_old_term);
        tu_textlock = false;
    }
}

Display *g_pDisplay = XOpenDisplay(getenv("DISPLAY"));

bool GetKeyState(KeySym keySym)
{
    if (g_pDisplay == NULL)
    {
        return false;
    }

    char szKey[32];
    int iKeyCodeToFind = XKeysymToKeycode(g_pDisplay, keySym);

    XQueryKeymap(g_pDisplay, szKey);

    return szKey[iKeyCodeToFind / 8] & (1 << (iKeyCodeToFind % 8));
}

void cleanExit(int sig)
{
    (void)sig;
    tu_unlockTerm();
    bp_quit();
    exit(0);
}

int main()
{
    bp_init();
    signal(SIGINT, cleanExit);

    nScreenWidth = bp_width;
    nScreenHeight = bp_height;

    tu_lockTerm();

    // Create Screen Buffer
    //wchar_t *screen = new wchar_t[nScreenWidth*nScreenHeight];

    //wchar_t *buf = new wchar_t[256];

    setlocale(LC_CTYPE, "");

    //HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    //SetConsoleActiveScreenBuffer(hConsole);
    //uint32_t dwBytesWritten = 0;

    // Create Map of world space # = wall block, . = space
    wstring map;
    map += L"#########.......";
    map += L"#...............";
    map += L"#.......########";
    map += L"#..............#";
    map += L"#......##......#";
    map += L"#......##......#";
    map += L"#..............#";
    map += L"###............#";
    map += L"##.............#";
    map += L"#......####..###";
    map += L"#......#.......#";
    map += L"#......#.......#";
    map += L"#..............#";
    map += L"#......#########";
    map += L"#..............#";
    map += L"################";

    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();

    bool firstrender = true;

    while (1)
    {
        // We'll need time differential per frame to calculate modification
        // to movement speeds, to ensure consistant movement, as ray-tracing
        // is non-deterministic
        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();

        bool render = firstrender;
        firstrender = false;
        // Handle CCW Rotation
        if (GetKeyState((unsigned short)'A'))
        {
            render = true;
            fPlayerA -= (fSpeed * 0.75f) * fElapsedTime;
        }

        // Handle CW Rotation
        if (GetKeyState((unsigned short)'D'))
        {
            render = true;
            fPlayerA += (fSpeed * 0.75f) * fElapsedTime;
        }

        // Handle Forwards movement & collision
        if (GetKeyState((unsigned short)'W'))
        {
            render = true;
            fPlayerX += sinf(fPlayerA) * fSpeed * fElapsedTime;
            ;
            fPlayerY += cosf(fPlayerA) * fSpeed * fElapsedTime;
            ;
            if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
            {
                fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;
                ;
                fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;
                ;
            }
        }

        // Handle backwards movement & collision
        if (GetKeyState((unsigned short)'S'))
        {
            render = true;
            fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;
            ;
            fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;
            ;
            if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
            {
                fPlayerX += sinf(fPlayerA) * fSpeed * fElapsedTime;
                ;
                fPlayerY += cosf(fPlayerA) * fSpeed * fElapsedTime;
                ;
            }
        }

        for (int x = 0; x < nScreenWidth; x++)
        {
            // For each column, calculate the projected ray angle into world space
            float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

            // Find distance to wall
            float fStepSize = 0.1f;       // Increment size for ray casting, decrease to increase
            float fDistanceToWall = 0.0f; //                                      resolution

            bool bHitWall = false;  // Set when ray hits wall block
            bool bBoundary = false; // Set when ray hits boundary between two wall blocks

            float fEyeX = sinf(fRayAngle); // Unit vector for ray in player space
            float fEyeY = cosf(fRayAngle);

            // Incrementally cast ray from player, along ray angle, testing for
            // intersection with a block
            while (!bHitWall && fDistanceToWall < fDepth)
            {
                fDistanceToWall += fStepSize;
                int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
                int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

                // Test if ray is out of bounds
                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
                {
                    bHitWall = true; // Just set distance to maximum depth
                    fDistanceToWall = fDepth;
                }
                else
                {
                    // Ray is inbounds so test to see if the ray cell is a wall block
                    if (map.c_str()[nTestX * nMapWidth + nTestY] == '#')
                    {
                        // Ray has hit wall
                        bHitWall = true;

                        // To highlight tile boundaries, cast a ray from each corner
                        // of the tile, to the player. The more coincident this ray
                        // is to the rendering ray, the closer we are to a tile
                        // boundary, which we'll shade to add detail to the walls
                        vector<pair<float, float>> p;

                        // Test each corner of hit tile, storing the distance from
                        // the player, and the calculated dot product of the two rays
                        for (int tx = 0; tx < 2; tx++)
                            for (int ty = 0; ty < 2; ty++)
                            {
                                // Angle of corner to eye
                                float vy = (float)nTestY + ty - fPlayerY;
                                float vx = (float)nTestX + tx - fPlayerX;
                                float d = sqrt(vx * vx + vy * vy);
                                float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
                                p.push_back(make_pair(d, dot));
                            }

                        // Sort Pairs from closest to farthest
                        sort(p.begin(), p.end(), [](const pair<float, float> &left, const pair<float, float> &right)
                             { return left.first < right.first; });

                        // First two/three are closest (we will never see all four)
                        float fBound = 0.001;
                        if (acos(p.at(0).second) < fBound)
                            bBoundary = true;
                        if (acos(p.at(1).second) < fBound)
                            bBoundary = true;
                        if (acos(p.at(2).second) < fBound)
                            bBoundary = true;
                    }
                }
            }

            // Calculate distance to ceiling and floor
            int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
            int nFloor = nScreenHeight - nCeiling;

            // Shader walls based on distance
            uint8_t nShade = 7;
            if (fDistanceToWall <= fDepth / 4.0f)
                nShade = 255; // Very close
            else if (fDistanceToWall < fDepth / 3.0f)
                nShade = 191;
            else if (fDistanceToWall < fDepth / 2.0f)
                nShade = 127;
            else if (fDistanceToWall < fDepth)
                nShade = 63;
            else
                nShade = 15; // Too far away

            if (bBoundary)
                nShade = 0; // Black it out

            for (int y = 0; y < nScreenHeight; y++)
            {
                // Each Row
                if (y <= nCeiling)
                    bp_set(x, y, 0);
                else if (y > nCeiling && y <= nFloor)
                    bp_set(x, y, bp_color(nShade, nShade, nShade));
                else // Floor
                {
                    // Shade floor based on distance
                    float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
                    if (b < 0.25)
                        nShade = 191;
                    else if (b < 0.5)
                        nShade = 143;
                    else if (b < 0.75)
                        nShade = 95;
                    else if (b < 0.9)
                        nShade = 47;
                    else
                        nShade = 15;
                    //screen[y*nScreenWidth + x] = nShade;
                    bp_set(x, y, bp_color(nShade, nShade, nShade));
                }
            }
        }

        // Display Stats

        //swprintf(buf, 256, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", fPlayerX, fPlayerY, fPlayerA, 1.0f/fElapsedTime);

        //wcsncpy(screen, buf, wcslen(buf));

        // Display Map
        /*for (int nx = 0; nx < nMapWidth; nx++)
			for (int ny = 0; ny < nMapWidth; ny++)
			{
				screen[(ny+1)*nScreenWidth + nx] = map[ny * nMapWidth + nx];
			}
		screen[((int)fPlayerX+1) * nScreenWidth + (int)fPlayerY] = 'P';*/

        // Display Frame
        //screen[nScreenWidth * nScreenHeight - 1] = '\0';
        /*
		fputs("\e[H", stdout);
		fflush(stdout);
		//dwBytesWritten = write(1, screen, nScreenWidth * nScreenHeight);
        FILE* ret = freopen(NULL, "w", stdout);
        (void)ret;
		fputws(screen, stdout);
		fflush(stdout);
        ret = freopen(NULL, "w", stdout);
        (void)ret;
        */
	//pluto_write_out();
        if (render)
	{
        bp_render();  
   	}
    }

    return 0;
}

// That's It!! - Jx9
#endif
