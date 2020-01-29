#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "driver/i2s.h"
#include "driver/timer.h"
#include "esp_task_wdt.h"

#include "emu2413.h"
#include "playtune.h"

#define MSX_CLK 3579545
#define SAMPLERATE 44100
#define BUFFER_SIZE 512  // Buffer size in samples!

esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}

TaskHandle_t nTask1 = NULL, nTask2 = NULL;
int16_t audio_buffer[BUFFER_SIZE];
OPLL *opll;

inline void WORD(char *buf, uint32_t data) {
  buf[0] = data & 0xff;
  buf[1] = (data & 0xff00) >> 8;
}

void ym2413task(void *pvParameters)
{
    uint32_t bytesWritten = 0;
    //uint32_t timeTakenBefore = 0;
    //uint32_t timeTakenAfter = 0;

    while (1)
    {
        //timeTakenBefore = esp_timer_get_time();
        for (auto i = 0; i < BUFFER_SIZE/2; i++) {
            audio_buffer[i*2] = OPLL_calc(opll);          
            audio_buffer[i*2 + 1] = audio_buffer[i*2];
        }
        //timeTakenAfter = esp_timer_get_time();
        //printf("time taken: %d\n", (timeTakenAfter-timeTakenBefore));
        i2s_write(I2S_NUM_0, audio_buffer, BUFFER_SIZE*2, &bytesWritten, portMAX_DELAY);
    }
}

void playTask(void* pvParameters) {
    Miditones mdt;

    printf("Starting playTask...\n");
    printf("playTask running on core: %d\n", xPortGetCoreID());
    
    while (1)
    {
        ulTaskNotifyTake( pdTRUE, portMAX_DELAY );
        mdt.updateNote();
    }
}

extern "C" void app_main(void)
{
    nvs_flash_init();
    static const i2s_config_t i2s_config = {
         .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
         .sample_rate = 44100,
         .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
         .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
         .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
         .intr_alloc_flags = 0, // default interrupt priority
         .dma_buf_count = 8,
         .dma_buf_len = BUFFER_SIZE*2,
         .use_apll = false,
         .tx_desc_auto_clear = true
    };

    static const i2s_pin_config_t pin_config = {
        .bck_io_num = 25,
        .ws_io_num = 27,
        .data_out_num = 26,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);   //install and start i2s driver
    i2s_set_pin(I2S_NUM_0, &pin_config);

    opll = OPLL_new(MSX_CLK, SAMPLERATE);

    OPLL_reset(opll);

    // Set five channels with maximum volume and with instrument Guitar.
    OPLL_writeReg(opll, 0x30, 0x20);
    OPLL_writeReg(opll, 0x31, 0x20); 
    OPLL_writeReg(opll, 0x32, 0x20);
    OPLL_writeReg(opll, 0x33, 0x20);
    OPLL_writeReg(opll, 0x34, 0x20);
        
    xTaskCreate(&ym2413task, "ym2413task", 4096, NULL, 10, &nTask1);
    xTaskCreatePinnedToCore(&playTask, "playtask", 4096, NULL, 1, &nTask2, 0);

    while (true) {
          vTaskDelay(500 / portTICK_RATE_MS);
    }
}
