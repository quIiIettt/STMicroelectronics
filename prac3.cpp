#include "mbed.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
#include "Monster1.h"
#include "upbutton.h"
#include "downbutton.h"

#define LCD_FB1_START_ADDRESS (LCD_FB_START_ADDRESS + (480 * 272) * 4)
#define LCD_MEM_ADDR (LCD_FB1_START_ADDRESS + 4 * 480 * 272)

TIM_HandleTypeDef htim3;
DMA2D_HandleTypeDef hdma2d;

uint32_t ramBuffer[ (192 * 32 * 2) / 4];

int y_offset = 0;
int x_offset = 0;
int old_x_offset = 0, old_y_offset = 0;

extern "C" void Init_TimerSensor(void) {
    uint32_t prescalerValue = 0;
    
    __HAL_RCC_TIM3_CLK_ENABLE();

    prescalerValue = (uint32_t)(SystemCoreClock / 2 / 10000)-1;
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = prescalerValue;
    htim3.Init.Period = 100 - 1;
    htim3.Init.ClockDivision = 0;
    htim3.Init.RepetitionCounter = 0;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    HAL_TIM_Base_Init(&htim3);
    
    HAL_NVIC_SetPriority(TIM3_IRQn, 3, 0);

    HAL_NVIC_EnableIRQ(TIM3_IRQn);
    
    HAL_TIM_Base_Start_IT(&htim3);
}

extern "C" void TIM3_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim3);
}

extern "C" void EXTI15_10_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(TS_INT_PIN);
}

void DMA2D_Config(void);
void FillRectWithColor(uint32_t color, uint32_t dst_addr, int x, int y, int width, int height);
void MoveTileToMemory(uint32_t source_addr, uint32_t dest_addr, int width, int height);

int main()
{
    HAL_Init();
    
    TS_StateTypeDef TS_State;
    uint16_t x, y;
    uint8_t text[30];
    uint8_t status;
    uint8_t idx;
    uint8_t cleared = 0;
    uint8_t prev_nb_touches = 0;

    BSP_LCD_Init();
    BSP_LCD_LayerDefaultInit(0, LCD_FB1_START_ADDRESS); // init background layer address
    
    BSP_LCD_LayerDefaultInit(LTDC_ACTIVE_LAYER, LCD_FB_START_ADDRESS);
    BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER);
    BSP_LCD_SetColorKeying(1, 0xffffffff);

    BSP_LCD_DisplayStringAt(0, LINE(5), (uint8_t *)"TOUCHSCREEN DEMO", CENTER_MODE);
    HAL_Delay(1000);

    status = BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
    if (status != TS_OK) {
        BSP_LCD_Clear(LCD_COLOR_RED);
        BSP_LCD_SetBackColor(LCD_COLOR_RED);
        BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
        BSP_LCD_DisplayStringAt(0, LINE(5), (uint8_t *)"TOUCHSCREEN INIT FAIL", CENTER_MODE);
    } else {
        //BSP_LCD_Clear(LCD_COLOR_GREEN);
        BSP_LCD_SetBackColor(LCD_COLOR_GREEN);
        BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
        //BSP_LCD_DisplayStringAt(0, LINE(5), (uint8_t *)"TOUCHSCREEN INIT OK", CENTER_MODE);
    }
    
    if (BSP_TS_ITConfig() != TS_OK) {
        BSP_LCD_Clear(LCD_COLOR_RED);
        BSP_LCD_SetBackColor(LCD_COLOR_RED);
        BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
        BSP_LCD_DisplayStringAt(0, LINE(5), (uint8_t *)"TOUCHSCREEN INTERRUPT FAIL", CENTER_MODE);
    }

    HAL_Delay(1000);
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    
    //Init_TimerSensor();
    
    // load image into memory
    DMA2D_Config();


    HAL_DMA2D_Start(&hdma2d, (uint32_t)&pic_Dude_Monster_Walk_6_bmp, (uint32_t)&ramBuffer, 192, 32);
    while(HAL_DMA2D_PollForTransfer(&hdma2d, 1000) != HAL_OK);
    
    HAL_DMA2D_Start(&hdma2d, (uint32_t)&down_button_image_array, LCD_MEM_ADDR, 64, 64);
    while(HAL_DMA2D_PollForTransfer(&hdma2d, 1000) != HAL_OK);
    HAL_DMA2D_Start(&hdma2d, (uint32_t)&up_button_image_array, LCD_MEM_ADDR + 64*64*2, 64, 64);
    while(HAL_DMA2D_PollForTransfer(&hdma2d, 1000) != HAL_OK);
    
    FillRectWithColor(0xff00ff00, LCD_FB_START_ADDRESS, 0, 0, 480, 272);
    HAL_DMA2D_Start(&hdma2d, 0xff00ff00, LCD_FB1_START_ADDRESS, 480, 272);
    while(HAL_DMA2D_PollForTransfer(&hdma2d, 1000) != HAL_OK);

    
    hdma2d.Init.Mode = DMA2D_M2M_PFC;
    hdma2d.Init.ColorMode = DMA2D_ARGB8888;
    hdma2d.Init.OutputOffset = 480 - 64;
    hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    hdma2d.LayerCfg[1].InputAlpha = 0xff;
    hdma2d.LayerCfg[1].InputColorMode = CM_RGB565;
    hdma2d.LayerCfg[1].InputOffset = 0;
    HAL_DMA2D_Init(&hdma2d);
    HAL_DMA2D_ConfigLayer(&hdma2d, 1);
    HAL_DMA2D_Start(&hdma2d, LCD_MEM_ADDR, LCD_FB_START_ADDRESS + 240 * 4 + 480 * 200 * 4, 64, 64);
    while(HAL_DMA2D_PollForTransfer(&hdma2d, 1000) != HAL_OK);
    HAL_DMA2D_Start(&hdma2d, LCD_MEM_ADDR + 64*64*2, LCD_FB_START_ADDRESS + 240 * 4 + 480 * 136 * 4, 64, 64);
    while(HAL_DMA2D_PollForTransfer(&hdma2d, 1000) != HAL_OK);
    
    int counter = 0;
    int xpos_offset = 0;
    int dir = 0;

    while(1) {
        HAL_Delay(50);
        FillRectWithColor(0xff00ff00, LCD_FB_START_ADDRESS, 0, 0, 480, 100);
        
        hdma2d.Init.Mode = DMA2D_M2M_PFC;
        hdma2d.Init.ColorMode = DMA2D_ARGB8888;
        hdma2d.Init.OutputOffset = 480 - 32;
        hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
        hdma2d.LayerCfg[1].InputAlpha = 0xff;
        hdma2d.LayerCfg[1].InputColorMode = CM_RGB565;
        hdma2d.LayerCfg[1].InputOffset = 192 - 32;
        HAL_DMA2D_Init(&hdma2d);
        HAL_DMA2D_ConfigLayer(&hdma2d, 1);
        //HAL_DMA2D_Start(&hdma2d, (uint32_t)&ramBuffer, LCD_FB_START_ADDRESS + offset + xpos_offset, 150, 150);
        HAL_DMA2D_Start(&hdma2d, ((uint32_t)&ramBuffer) + counter * 64, LCD_FB_START_ADDRESS + xpos_offset * 4 + y_offset * 480 * 4, 32, 32);
        while(HAL_DMA2D_PollForTransfer(&hdma2d, 1000) != HAL_OK);
        if (dir == 0) {
            old_x_offset = xpos_offset;
            xpos_offset++;
            if (xpos_offset == 480-32) dir = 1;
        }
        else {
            old_x_offset = xpos_offset;
            xpos_offset--;
            if (xpos_offset == 0) dir = 0;
        }
        counter++;
        if (counter == 6)
            counter = 0;
        
        /*BSP_TS_GetState(&TS_State);
        if (TS_State.touchDetected) {
            // Clear lines corresponding to old touches coordinates
            if (TS_State.touchDetected < prev_nb_touches) {
                for (idx = (TS_State.touchDetected + 1); idx <= 5; idx++) {
                    BSP_LCD_ClearStringLine(idx);
                }
            }
            prev_nb_touches = TS_State.touchDetected;

            cleared = 0;

            sprintf((char*)text, "Touches: %d", TS_State.touchDetected);
            BSP_LCD_DisplayStringAt(0, LINE(0), (uint8_t *)&text, LEFT_MODE);

            for (idx = 0; idx < TS_State.touchDetected; idx++) {
                x = TS_State.touchX[idx];
                y = TS_State.touchY[idx];
                sprintf((char*)text, "Touch %d: x=%d y=%d    ", idx+1, x, y);
                BSP_LCD_DisplayStringAt(0, LINE(idx+1), (uint8_t *)&text, LEFT_MODE);
            }

            BSP_LCD_DrawPixel(TS_State.touchX[0], TS_State.touchY[0], LCD_COLOR_ORANGE);
        } else {
            if (!cleared) {
                BSP_LCD_Clear(LCD_COLOR_BLUE);
                sprintf((char*)text, "Touches: 0");
                BSP_LCD_DisplayStringAt(0, LINE(0), (uint8_t *)&text, LEFT_MODE);
                cleared = 1;
            }
        }*/
    }
}

extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) { 
    TS_StateTypeDef ts;
    BSP_TS_GetState(&ts);
    if (ts.touchDetected) {
        int x = ts.touchX[0];
        int y = ts.touchY[0];
        if (x > 240 && y > 136 && x < 304 && y < 200 && y_offset > 0) {
            y_offset -= 2;
        }
        else if (x > 240 && y > 200 && x < 304 && y < 264 && y_offset < 72) {
            y_offset += 2;
        }
    }
}

extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    TS_StateTypeDef ts;
    BSP_TS_GetState(&ts);
    if (ts.touchDetected) {
        int x = ts.touchX[0];
        int y = ts.touchY[0];
        if (x > 240 && y > 136 && x < 304 && y < 200 && y_offset > 0) {
            old_y_offset = y_offset;
            y_offset -= 2;
        }
        else if (x > 240 && y > 200 && x < 304 && y < 264 && y_offset < 72) {
            old_y_offset = y_offset;
            y_offset += 2;
        }
        else if (x > 0 && x > 64 && y < 0 && y < 272 && x_offset < 240) {
            old_x_offset = x_offset;
            x_offset += 2;
        }
        else if (x > 416 && x > 480 && y < 0 && y < 272 && x_offset < 304) {
            old_x_offset = x_offset;
            x_offset += 2;
        }
    }
}




void DMA2D_Config(void) {
    hdma2d.Instance = DMA2D;
    hdma2d.Init.Mode = DMA2D_M2M;
    hdma2d.Init.OutputOffset = 0;
    hdma2d.Init.ColorMode = DMA2D_RGB565;

    hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    hdma2d.LayerCfg[1].InputAlpha = 0xff;
    hdma2d.LayerCfg[1].InputOffset = 0;
    hdma2d.LayerCfg[1].InputColorMode = CM_RGB565;

    HAL_DMA2D_Init(&hdma2d);
    HAL_DMA2D_ConfigLayer(&hdma2d, 1);
}

void FillRectWithColor(uint32_t color, uint32_t dst_addr, int x, int y, int width, int height) {
    hdma2d.Init.Mode = DMA2D_R2M;
    hdma2d.Init.ColorMode = DMA2D_ARGB8888;
    hdma2d.Init.OutputOffset = 480 - width;
    HAL_DMA2D_Init(&hdma2d);
    HAL_DMA2D_Start(&hdma2d, color, dst_addr + x * 4 + y * 480 * 4, width, height);
    while(HAL_DMA2D_PollForTransfer(&hdma2d, 1000) != HAL_OK);
}

void MoveTileToMemory(uint32_t source_addr, uint32_t dest_addr, int width, int height) {
    for (int y = 0; y < 272; y++) {
        for (int x = 0; x < 480; x += width) {
        HAL_DMA2D_Start(&hdma2d, source_addr, dest_addr + x * 4 + y * 480 * 4, width, height);
        while (HAL_DMA2D_PollForTransfer(&hdma2d, 1000) != HAL_OK);
        }
    }
}


