#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_log.h"

// กำหนด pin ที่ใช้
#define POTENTIOMETER_CHANNEL ADC1_CHANNEL_6  // GPIO34 (ADC1_CH6)
#define DEFAULT_VREF    1100        // ใช้ adc2_vref_to_gpio() เพื่อให้ได้ค่าประมาณที่ดีกว่า
#define NO_OF_SAMPLES   64          // การสุ่มสัญญาณหลายครั้ง

static const char *TAG = "ADC_POT";
static esp_adc_cal_characteristics_t *adc_chars;

void app_main(void)
{
    // กำหนดค่า ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(POTENTIOMETER_CHANNEL, ADC_ATTEN_DB_11);

    // ปรับเทียบ ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);

    printf("ตำแหน่ง Potentiometer\tค่า ADC\tแรงดัน (V)\tเปอร์เซ็นต์ (%%)\n");
    printf("-------------------------------------------------------------\n");

    while (1) {
        uint32_t adc_reading = 0;

        // การสุ่มสัญญาณหลายครั้ง
        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            adc_reading += adc1_get_raw((adc1_channel_t)POTENTIOMETER_CHANNEL);
        }
        adc_reading /= NO_OF_SAMPLES;

        // แปลง adc_reading เป็นแรงดันในหน่วย mV
        uint32_t voltage_mv = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        float voltage = voltage_mv / 1000.0;

        // แปลงเป็นเปอร์เซ็นต์
        float percentage = (adc_reading / 4095.0) * 100.0;

        // แสดงผลเป็นตาราง (ใส่ตำแหน่งปัจจุบันเป็น "อ่านค่า")
        printf("อ่านค่า\t\t\t%d\t%.2f\t\t%.1f%%\n", adc_reading, voltage, percentage);

        vTaskDelay(pdMS_TO_TICKS(1000));  // หน่วงเวลา 1 วินาที
    }
}
