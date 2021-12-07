#include "driver/adc_common.h"
#include "esp_adc_cal.h"
#include "esp_log.h"
#include <math.h>

static const char *TAG = "Thermistor";

//ADC Channels
#if CONFIG_IDF_TARGET_ESP32
// ADC1 channel 6 is GPIO34
#define TERMISTOR_CHANNEL          ADC1_CHANNEL_6
//static const char *TAG_CH[2][10] = {{"ADC1_CH6"}, {"ADC2_CH0"}};
#else
#define TERMISTOR_CHANNEL          ADC1_CHANNEL_2
//static const char *TAG_CH[2][10] = {{"ADC1_CH2"}, {"ADC2_CH0"}};
#endif

//ADC Attenuation
#define ADC_ATTEN           ADC_ATTEN_DB_11

//ADC Calibration
#if CONFIG_IDF_TARGET_ESP32
#define ADC_CALI_SCHEME     ESP_ADC_CAL_VAL_EFUSE_VREF
#elif CONFIG_IDF_TARGET_ESP32S2
#define ADC_CALI_SCHEME     ESP_ADC_CAL_VAL_EFUSE_TP
#elif CONFIG_IDF_TARGET_ESP32C3
#define ADC_CALI_SCHEME     ESP_ADC_CAL_VAL_EFUSE_TP
#elif CONFIG_IDF_TARGET_ESP32S3
#define ADC_CALI_SCHEME     ESP_ADC_CAL_VAL_EFUSE_TP_FIT
#endif


static esp_adc_cal_characteristics_t adc1_chars;

static esp_err_t adc_calibration_init(bool* cali_enable) {
    esp_err_t err;

    err = esp_adc_cal_check_efuse(ADC_CALI_SCHEME);
    if (err == ESP_ERR_NOT_SUPPORTED) {
        ESP_LOGW(TAG, "Calibration scheme not supported, skip software calibration");
    } else if (err == ESP_ERR_INVALID_VERSION) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else if (err == ESP_OK) {
        *cali_enable = true;
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);
    } else {
        ESP_LOGE(TAG, "Invalid arg");
    }

    return err;
}
double read_adc(bool cali_enable) {
        int adc_raw = adc1_get_raw(TERMISTOR_CHANNEL);
        //ESP_LOGI(TAG_CH[0][0], "raw  data: %d", adc_raw[0][0]);
        uint32_t voltage = 0;
        if (cali_enable) {
            voltage = esp_adc_cal_raw_to_voltage(adc_raw, &adc1_chars);
            //ESP_LOGI(TAG_CH[0][0], "cali data: %d mV", voltage);
        }

        double R1 = 10000.0;   // voltage divider resistor value
        double Beta = 3950.0;  // Beta value
        double To = 298.15;    // Temperature in Kelvin for 25 degree Celsius
        double Ro = 10000.0;   // Resistance of Thermistor at 25 degree Celsius
        double adcMax = 4095.0; // ADC resolution 12-bit (0-4095)
        double Vs = 3.3;    
        double vout, Rt = 0;
        double T, Tc, Tf = 0;
        vout = adc_raw * Vs/adcMax;
        Rt = R1 * vout / (Vs - vout);
        T = 1/(1/To + log(Rt/Ro)/Beta);    // Temperature in Kelvin
        Tc = T - 273.15;                   // Celsius
        Tf = Tc * 9 / 5 + 32;              // Fahrenheit

        ESP_LOGI("temp", "%f f voltage %d adc %d channel %d", Tf, voltage, adc_raw, TERMISTOR_CHANNEL);
        return Tf;
}
esp_err_t setup_adc(bool* cali_enable) {
    esp_err_t err = adc1_config_width(ADC_WIDTH_BIT_DEFAULT);
    if (err != ESP_OK) {
	return err;
    }
    err = adc1_config_channel_atten(TERMISTOR_CHANNEL, ADC_ATTEN);
    if (err != ESP_OK) {
	return err;
    }

    ESP_LOGI(TAG, "start ADC");
    err = adc_calibration_init(cali_enable);
    return err;
}
