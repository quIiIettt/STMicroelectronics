//#include "DISCO-F746NG_LCDTS_demo/BSP_DISCO_F746NG/stm32746g_discovery_lcd.h"
#include "mbed.h"
#include "stm32746g_discovery_lcd.h"

int main()
{
    BSP_LCD_Init();
    BSP_LCD_LayerDefaultInit(LTDC_ACTIVE_LAYER, LCD_FB_START_ADDRESS);
    BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER);

    int x = 10;
    int y = 20;
    int width = 70;
    int height = 60;

    int dirX = 1;
    int dirY = 1;

    while (1) {
        BSP_LCD_Clear(LCD_COLOR_BLACK);
        
        BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
        BSP_LCD_FillRect(x, y, width, height);

        if (x + width >= BSP_LCD_GetXSize() || x <= 0) {
            dirX *= -1;
            if (width > 10) {
                width -= 5;
            }
        }

        if (y + height >= BSP_LCD_GetYSize() || y <= 0) {
            dirY *= -1;
            if (height > 10) {
                height -= 5;
            }
        }

        x += width * dirX;
        y += height * dirY;

        uint32_t time = HAL_GetTick();
        while (HAL_GetTick() < time + 20) {

        }

        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_FillRect(x, y, width, height);
 
        if (width == 20 && height == 10) {
            x = 10;
            y = 20;
            width = 70;
            height = 60;

            dirX *= -1;
            dirY *= -1;
        }

        HAL_Delay(1500);
    }
}
