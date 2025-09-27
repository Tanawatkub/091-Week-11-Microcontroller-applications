#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define LDR_CHANNEL ADC1_CHANNEL_7  // GPIO35
#define DEFAULT_VREF    1100        
#define NO_OF_SAMPLES   64          

static esp_adc_cal_characteristics_t *adc_chars;

void app_main(void)
{
    // กำหนด ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(LDR_CHANNEL, ADC_ATTEN_DB_11);

    // ปรับเทียบ ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11,
                             ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);

    while (1) {
        uint32_t adc_reading = 0;
        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            adc_reading += adc1_get_raw((adc1_channel_t)LDR_CHANNEL);
        }
        adc_reading /= NO_OF_SAMPLES;

        // แปลงค่า
        uint32_t voltage_mv = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        float voltage = voltage_mv / 1000.0;
        float lightLevel = (adc_reading / 4095.0) * 100.0;

        // กำหนดสถานะแสง
        const char* lightStatus;
        if (lightLevel < 20) {
            lightStatus = "มืด";
        } else if (lightLevel < 50) {
            lightStatus = "แสงน้อย";
        } else if (lightLevel < 80) {
            lightStatus = "แสงปานกลาง";
        } else {
            lightStatus = "แสงจ้า";
        }

        // แสดงผลให้อ่านง่าย
        printf("ADC: %-4d | Voltage: %.2f V | Light: %5.1f%% | Status: %s\n",
               adc_reading, voltage, lightLevel, lightStatus);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
