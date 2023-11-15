#include "mbed.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"


#include "stlogo.h"

#define IMAGE_WIDTH 200
#define IMAGE_HEIGHT 200

int main()
{
    TS_StateTypeDef TS_State;
    uint16_t x, y;
    uint8_t text[30];
    uint8_t status;
    uint8_t idx;
    uint8_t cleared = 0;
    uint8_t prev_nb_touches = 0;

    BSP_LCD_Init();
    BSP_LCD_LayerDefaultInit(LTDC_ACTIVE_LAYER, LCD_FB_START_ADDRESS);
    BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER);

    BSP_LCD_DisplayStringAt(0, LINE(5), (uint8_t *)"TOUCHSCREEN DEMO", CENTER_MODE);
    HAL_Delay(1000);

    status = BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
    if (status != TS_OK) {
        BSP_LCD_Clear(LCD_COLOR_RED);
        BSP_LCD_SetBackColor(LCD_COLOR_RED);
        BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
        BSP_LCD_DisplayStringAt(0, LINE(5), (uint8_t *)"TOUCHSCREEN INIT FAIL", CENTER_MODE);
    } else {
        BSP_LCD_Clear(LCD_COLOR_GREEN);
        BSP_LCD_SetBackColor(LCD_COLOR_GREEN);
        BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
        BSP_LCD_DisplayStringAt(0, LINE(5), (uint8_t *)"TOUCHSCREEN INIT OK", CENTER_MODE);
    }

    HAL_Delay(1000);
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

    uint16_t imageX = BSP_LCD_GetXSize() / 2 - IMAGE_WIDTH / 2;
    uint16_t imageY = BSP_LCD_GetYSize() / 2 - IMAGE_HEIGHT / 2;

    

    while(1) {

        BSP_TS_GetState(&TS_State);
        if (TS_State.touchDetected) {
            // Clear lines corresponding to old touches coordinates
            if (TS_State.touchDetected == 1) {
                if (prev_nb_touches == 0) {
                    x = TS_State.touchX[0];
                    y = TS_State.touchY[0];
                } else {
                    imageX += TS_State.touchX[0] - x;
                    imageY += TS_State.touchY[0] - y;
                    
                    if (imageX < 0) imageX = 0;
                    if (imageX > BSP_LCD_GetXSize() - IMAGE_WIDTH) imageX = BSP_LCD_GetXSize() - IMAGE_WIDTH;
                    if (imageY < 0) imageY = 0;
                    if (imageY > BSP_LCD_GetYSize() - IMAGE_HEIGHT) imageY = BSP_LCD_GetYSize() - IMAGE_HEIGHT;

                    x = TS_State.touchX[0];
                    y = TS_State.touchY[0];
                }

            }
            prev_nb_touches = TS_State.touchDetected;

        
            BSP_LCD_Clear(LCD_COLOR_BLUE);
            sprintf((char*)text, "Touches: %d", TS_State.touchDetected);
            BSP_LCD_DisplayStringAt(0, LINE(0), (uint8_t *)&text, LEFT_MODE);
            
            BSP_LCD_DrawBitmap(imageX, imageY, (uint8_t *)stlogo);

            
        } else {
            if (!cleared) {
                BSP_LCD_Clear(LCD_COLOR_BLUE);
                sprintf((char*)text, "Touches: 0");
                BSP_LCD_DisplayStringAt(0, LINE(0), (uint8_t *)&text, LEFT_MODE);
                cleared = 1;
            }
        }
    }
}
