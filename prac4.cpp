#include "mbed.h"
#include "GYRO_DISCO_F429ZI.h"
#include "LCD_DISCO_F429ZI.h"
#include "stm32f429i_discovery_lcd.h"

GYRO_DISCO_F429ZI gyro;
LCD_DISCO_F429ZI lcd;

DigitalOut led1(LED1);

bool isGesture(float angle) {
    return (angle >= 45.0f || angle <= -45.0f);
}

void displayAngle(float angleX, float angleY, float angleZ) {
    uint8_t buffer[100];
    sprintf((char*)buffer, "X: %.1f, Y: %.1f, Z: %.1f\n", angleX, angleY, angleZ);
    lcd.DisplayStringAt(100, 100, buffer, CENTER_MODE);
}


int main()
{
    float GyroBuffer[3];
    float zero_level[3] = {0, 0, 0};
    uint8_t buffer[100];
    lcd.Init();
    lcd.DisplayStringAt(10, 10, (uint8_t*)"Gyroscope_started", CENTER_MODE);
    uint8_t tmpreg = 0;
    GYRO_IO_Read(&tmpreg,L3GD20_CTRL_REG4_ADDR,1);
    switch(tmpreg & L3GD20_FULLSCALE_SELECTION)
    {
        case L3GD20_FULLSCALE_250:
            lcd.DisplayStringAt(20, 20, (uint8_t*)"250", CENTER_MODE);
        break;
    
        case L3GD20_FULLSCALE_500:
            lcd.DisplayStringAt(20, 20, (uint8_t*)"500", CENTER_MODE);
        break;
    
        case L3GD20_FULLSCALE_2000:
            lcd.DisplayStringAt(20, 20, (uint8_t*)"2000", CENTER_MODE);
        break;
  }
    GYRO_IO_Read(&tmpreg, L3GD20_CTRL_REG1_ADDR, 1);
    switch(tmpreg >> 6) {
        case 0:
            lcd.DisplayStringAt(20, 40, (uint8_t*)"95", CENTER_MODE);
            break;
        case 1:
            lcd.DisplayStringAt(20, 40, (uint8_t*)"190", CENTER_MODE);
            break;
        case 2:
            lcd.DisplayStringAt(20, 40, (uint8_t*)"380", CENTER_MODE);
            break;
        case 3:
            lcd.DisplayStringAt(20, 40, (uint8_t*)"760", CENTER_MODE);
            break;
    }

    for (int i = 0; i < 50; i++) {
        gyro.GetXYZ(GyroBuffer);
        zero_level[0] += GyroBuffer[0] / 17.5f;
        zero_level[1] += GyroBuffer[1] / 17.5f;
        zero_level[2] += GyroBuffer[2] / 17.5f;

        wait_ms(10);
    }
    for (int i = 0; i < 3; i++)
        zero_level[i] /= 50;
    
    float noise_level = 0.0;
    for (int i = 0; i < 50; i++) {
        gyro.GetXYZ(GyroBuffer);
        if ((GyroBuffer[0] / 17.5f - zero_level[0]) > noise_level) {
            noise_level = (GyroBuffer[0] / 17.5f - zero_level[0]);
        } 
        else if ((GyroBuffer[0] / 17.5f - zero_level[0]) < -noise_level) {
            noise_level = -(GyroBuffer[0] / 17.5f - zero_level[0]);
        }
        wait_ms(10);
    }
    sprintf((char*)buffer, "Noise = %.2f\n", noise_level);
    lcd.DisplayStringAt(20, 70, buffer, CENTER_MODE);
    
    
    float angleX = 0.0, angleY = 0.0, angleZ = 0.0;
    float prev_valueX = 0.0, prev_valueY = 0.0, prev_valueZ = 0.0;

    while(1) {
        wait(0.02);
        gyro.GetXYZ(GyroBuffer);
        
        // Обробка осі X
        float valueX = GyroBuffer[0] / 17.5f - zero_level[0];
        if (valueX >= noise_level || valueX <= -noise_level) {
            angleX += (valueX + prev_valueX) / 2 * 0.0175 * 0.02f / 0.825;
        }
        prev_valueX = valueX;

        // Обробка осі Y
        float valueY = GyroBuffer[1] / 17.5f - zero_level[1];
        if (valueY >= noise_level || valueY <= -noise_level) {
            angleY += (valueY + prev_valueY) / 2 * 0.0175 * 0.02f / 0.825;
        }
        prev_valueY = valueY;

        // Обробка осі Z
        float valueZ = GyroBuffer[2] / 17.5f - zero_level[2];
        if (valueZ >= noise_level || valueZ <= -noise_level) {
            angleZ += (valueZ + prev_valueZ) / 2 * 0.0175 * 0.02f / 0.825;
        }
        prev_valueZ = valueZ;

        // Розпізнавання та відображення знайдених жестів
        if (isGesture(angleX)) {
            lcd.DisplayStringAt(20, 120, (uint8_t*)"X detected!", CENTER_MODE);
        }

        if (isGesture(angleY)) {
            lcd.DisplayStringAt(120, 120, (uint8_t*)"Y detected!", CENTER_MODE);
        }

        if (isGesture(angleZ)) {
            lcd.DisplayStringAt(220, 120, (uint8_t*)"Z detected!", CENTER_MODE);
        }

        // Відображення значень кутів на екрані
        displayAngle(angleX, angleY, angleZ);
    }
}

