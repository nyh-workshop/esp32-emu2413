#ifndef PLAYTUNE_H
#define PLAYTUNE_H

#include "freertos/FreeRTOS.h"

class Miditones {
    public:
    Miditones();
    void updateNote();
    inline SemaphoreHandle_t getSemaphore() { return xSemaphore; }
    inline QueueHandle_t getQueue() { return mdtQueue; }

    private:
    void initTimer0();    
    static unsigned int timePlay;
    static unsigned int timePlayCount;
    static unsigned int songIndex;
    static float speed;
    static bool isPlaying;
    
    friend void IRAM_ATTR timer_group0_isr(void *para);

    static SemaphoreHandle_t xSemaphore;
    static QueueHandle_t mdtQueue;
};

extern uint16_t midiTo2413[128];



#endif