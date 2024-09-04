#include "face_recognition.h"
#include "./BSP/LCD/lcd.h"



void frec_get_image_data(uint16_t *dbuf,uint8_t xoff,uint8_t yoff,uint8_t xsize,uint8_t width)
{
	int8_t w, h; 
	uint8_t height=width*4/3;
	float scale=(float)xsize/width;
	for(h=0;h<height;h++)
	{
		for(w=0;w<width;w++)
		{
			dbuf[h*width+w]=lcd_read_point(xoff+w*scale,yoff+h*scale);			
 		}
	}
}



