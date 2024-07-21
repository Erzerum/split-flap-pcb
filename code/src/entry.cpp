#include "gpio.h"
#include "usart.h"

extern "C" {
    void _write(void) {}
    void _read(void) {}
    void SystemClock_Config(void);
}

struct pin_t {
    GPIO_TypeDef *bank;
    uint16_t pin;
};


struct motor_t {
    pin_t a, b, c, d;
    int tick = 0; // current position, tick % 4 gets you the current stepConfig index
};

struct character_t {
    int id; 
    motor_t motor;
    pin_t hall; 
    /* extra data */
};

bool stepConfig[4][4] {
    { 1, 1, 0, 0 }, 
    { 0, 1, 1, 0 }, 
    { 0, 0, 1, 1 }, 
    { 1, 0, 1, 0 } };

#define PIN(BANK, NUMBER) { GPIO##BANK, GPIO_PIN_##NUMBER }
#define P(BANK, NUMBER) PIN(BANK, NUMBER)

character_t characters[6]{
    { 1, { P(B, 15), P(B, 14), P(B, 13), P(B, 12) }, P(A,  8) },
    { 2, { P(B,  0), P(A,  7), P(A,  6), P(A,  5) }, P(B,  1) },
    { 3, { P(A,  3), P(A,  2), P(A,  1), P(A,  0) }, P(A,  4) },
    { 4, { P(F,  0), P(C, 15), P(C, 14), P(C, 13) }, P(F,  1) },
    { 5, { P(B,  6), P(B,  5), P(B,  4), P(B,  3) }, P(B,  7) },
    { 6, { P(F,  7), P(F,  6), P(A, 12), P(A, 11) }, P(A, 15) }
};

#undef P

void set_pin(pin_t *pin, bool on) {
    HAL_GPIO_WritePin(pin->bank, pin->pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void tick_motor(motor_t *motor) {
    motor->tick++;
    int index = motor->tick % 4;
    set_pin(&motor->a, stepConfig[index][0]);
    set_pin(&motor->b, stepConfig[index][1]);
    set_pin(&motor->c, stepConfig[index][2]);
    set_pin(&motor->d, stepConfig[index][3]);
}

extern "C"
int main() {
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    // MX_USART1_UART_Init(); // we don't need this until we do comms

    while(1) { 
        HAL_Delay(10);
        for(int i = 0; i < 6; i++) { 
            tick_motor(&characters->motor);
        }
    }
}
