#include "MicroBit.h"
#include "Adafruit_ST7735.h"

MicroBit uBit;

// // Define the MICROBIT EDGE CONNECTOR pins where the display is connected...
#define LCD_PIN_CS      2
#define LCD_PIN_DC      1
#define LCD_PIN_RST     0
#define LCD_PIN_MOSI    15
#define LCD_PIN_MISO    14
#define LCD_PIN_SCLK    13

#define MAP_WIDTH   	24
#define MAP_HEIGHT  	24
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   160

#define RED     0xFC00
#define GREEN   0x001F
#define BLUE    0x03E0
#define WHITE	0xFFFF
#define YELLOW	0xFFE0
#define BLACK 	0x0000

int worldMap[MAP_WIDTH][MAP_HEIGHT]=
{
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
	{1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1},
	{1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

void setColor(ManagedBuffer buf, uint16_t value, int offset)
{
    uint16_t *p = (uint16_t *) &buf[offset];

	// draw up to specific point
    while(p < (uint16_t *) &buf[buf.length()]) {
        *p = value;
        p++;
    }
}

void verticalLine(ManagedBuffer buf, int x, int drawStart, int drawEnd, int color)
{
    uint16_t *p = (uint16_t *) &buf[0] + (x * SCREEN_WIDTH + drawStart);

    while (drawStart <= drawEnd) {
        *p = color;
        p++;
        drawStart++;
    }
}

bool isButtonPressedA = false;
bool isButtonPressedB = false;

void onButtonA(MicroBitEvent) {
    isButtonPressedA = true;
}

void onButtonB(MicroBitEvent) {
    isButtonPressedB = true;
}

int main()
{
	uBit.init();
	uBit.sleep(500);

	ManagedBuffer img(SCREEN_WIDTH * SCREEN_HEIGHT * 2);
    Adafruit_ST7735 *lcd = new Adafruit_ST7735(LCD_PIN_CS, LCD_PIN_DC, LCD_PIN_RST,
							LCD_PIN_MOSI, LCD_PIN_MISO, LCD_PIN_SCLK);
    lcd->initR(INITR_GREENTAB);

    double posX = 22, posY = 12;      // Initial starting positions
    double dirX = -1, dirY = 0;    	  // Initial direction vector
    double planeX = 0, planeY = 0.66; // 2D Raycaster of camera plane

	uint64_t startTime, endTime;

	double cameraX;	// X-coordinate in camera space
	double rayDirX, rayDirY;
	double sideDistX, sideDistY;
	double deltaX, deltaY;
	double moveSpeed, rotSpeed;
	double perpWallDist;
	double frameTime;

	int mapX, mapY;
	int stepX, stepY;
	int hit, side;
	int w = SCREEN_HEIGHT;
	int h = SCREEN_WIDTH;
	int lineHeight;
	int drawStart, drawEnd;
	int color = 0;
	int prevColor = 0;

    // Register the button A press event handler
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_CLICK, onButtonA);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_BUTTON_EVT_CLICK, onButtonB);

	while(1) {
		startTime = system_timer_current_time(); // Time at start of the loop
        // Clear the screen at the start of each frame
        for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT * 2; i += 2) {
            img[i] = BLACK & 0xFF;
            img[i + 1] = (BLACK >> 8) & 0xFF;
        }
        if (isButtonPressedA) {
			if(worldMap[int(posX + dirX * moveSpeed)][int(posY)] == false) posX += dirX * moveSpeed;
			if(worldMap[int(posX)][int(posY + dirY * moveSpeed)] == false) posY += dirY * moveSpeed;
			isButtonPressedA = false;
        }

		for (int x = 0; x < w; x++) {
			cameraX = (2 * x) / (double)w - 1;
			rayDirX = dirX + (planeX * cameraX);
			rayDirY = dirY + (planeY * cameraX);

			mapX = int(posX);
			mapY = int(posY);

			deltaX = (rayDirX == 0) ? 1e30 : abs(1 / rayDirX);
			deltaY = (rayDirY == 0) ? 1e30 : abs(1 / rayDirY);

			hit = 0;

			if (rayDirX < 0) {
				stepX = -1;
				sideDistX = (posX - mapX) * deltaX;
			} else {
				stepX = 1;
				sideDistX = (mapX + 1.0 - posX) * deltaX;
			}
			if (rayDirY < 0) {
				stepY = -1;
				sideDistY = (posY - mapY) * deltaY;
			} else {
				stepY = 1;
				sideDistY = (mapY + 1.0 - posY) * deltaY;
			}

			// Perform DDA
			while (hit == 0) {
				if (sideDistX < sideDistY) {
					sideDistX += deltaX;
					mapX += stepX;
					side = 0;
				} else {
					sideDistY += deltaY;
					mapY += stepY;
					side = 1;
				}
				if (worldMap[mapX][mapY] > 0)
					hit = 1;
			}

			perpWallDist = (side == 0) ? (sideDistX - deltaX) :
							(sideDistY - deltaY);
			lineHeight = (int)(h / perpWallDist);

			drawStart = -(lineHeight / 2) + (h / 2);
			if (drawStart < 0) drawStart = 0;
			drawEnd = (lineHeight / 2) + (h / 2);
			if (drawEnd >= h) drawEnd = h - 1;

			switch (worldMap[mapX][mapY]) {
				case 1:  color = RED;     break;
				case 2:  color = GREEN;	  break;
				case 3:  color = BLUE;	  break;
				case 4:  color = WHITE;	  break;
				default: color = YELLOW;  break;
			}
			if (side == 1) {
				int red = ((color & 0xF800) >> 1) | 0x0800;
				int green = ((color & 0x07E0) >> 1) | 0x0040;
				int blue = ((color & 0x001F) >> 1) | 0x0001;

				color = (red & 0xF800) | (green & 0x07E0) | (blue & 0x001F);
			}

			// anti-aliasing
			// if (x > 0) { // Skip averaging for the first column
			// 	int avgRed = ((color & 0xF800) + (prevColor & 0xF800)) >> 1;
			// 	int avgGreen = ((color & 0x07E0) + (prevColor & 0x07E0)) >> 1;
			// 	int avgBlue = ((color & 0x001F) + (prevColor & 0x001F)) >> 1;

			// 	int avgColor = (avgRed & 0xF800) | (avgGreen & 0x07E0) | (avgBlue & 0x001F);

			// 	// Draw vertical line with the averaged color
			// 	verticalLine(img, x, drawStart, drawEnd, avgColor);
			// } else {
			// 	// Draw vertical line with the original color for the first column
				verticalLine(img, x, drawStart, drawEnd, color);
			// }

			// Update the previous color
			prevColor = color;
		}
		endTime = system_timer_current_time();
		// get Ticks here
		frameTime = (endTime - startTime) / 1000.0;

		moveSpeed = frameTime * 15.0; //the constant value is in squares/second
    	rotSpeed = frameTime * 9.0; //the constant value is in radians/second

		lcd->sendData(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, img.getBytes());
		if (isButtonPressedB) {
			//both camera direction and camera plane must be rotated
			double oldDirX = dirX;
			dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
			dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
			double oldPlaneX = planeX;
			planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
			planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
			isButtonPressedB = false;
		}
		// uBit.sleep(50);
	}

}