#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#define LDR_CHANNEL ADC_CHANNEL_7   // GPIO35
#define ADC_UNIT    ADC_UNIT_1

void app_main(void)
{
    // ---------- ADC CONFIG ----------
    adc_oneshot_unit_handle_t adc_handle;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT,
    };
    adc_oneshot_new_unit(&init_config, &adc_handle);

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,   // ใน v6 ใช้ DB_12 แทน DB_11
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    adc_oneshot_config_channel(adc_handle, LDR_CHANNEL, &config);

    // ---------- Calibration ----------
    adc_cali_handle_t cali_handle = NULL;
    bool do_calibration = false;
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    if (adc_cali_create_scheme_line_fitting(&cali_config, &cali_handle) == ESP_OK) {
        do_calibration = true;
    }

    // ---------- Print table header ----------
    printf("=====================================================\n");
    printf("| %-8s | %-10s | %-8s | %-12s |\n",
           "ADC", "Voltage(V)", "Light(%)", "สถานะ");
    printf("=====================================================\n");

    // ---------- Loop ----------
    while (1) {
        int raw = 0;
        int voltage_mv = 0;

        adc_oneshot_read(adc_handle, LDR_CHANNEL, &raw);

        if (do_calibration) {
            adc_cali_raw_to_voltage(cali_handle, raw, &voltage_mv);
        }

        float voltage = voltage_mv / 1000.0f;
        float lightLevel = (raw / 4095.0f) * 100.0f;

        const char* lightStatus;
        if (lightLevel < 20) lightStatus = "มืด";
        else if (lightLevel < 50) lightStatus = "แสงน้อย";
        else if (lightLevel < 80) lightStatus = "แสงปานกลาง";
        else lightStatus = "แสงจ้า";

        printf("| %-8d | %-10.2f | %-8.1f | %-12s |\n",
               raw, voltage, lightLevel, lightStatus);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
