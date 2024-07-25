#include "gpio.h"
#include "usart.h"
#include "tim.h"

typedef uint32_t u32;

extern "C" {
    void _write(void) {}
    void _read(void) {}
    void SystemClock_Config(void);
}

struct pin_t {
    GPIO_TypeDef *bank;
    uint16_t pin;
};

enum mode_t {
    MOTOR_STOPPED,
    MOTOR_ACCELERATING,
    MOTOR_DECELERATING,
    MOTOR_RUNNING
};

struct motor_t {
    pin_t a{}, b{}, c{}, d{};
    u32 step = 0; // current position, tick % 4 gets you the current stepConfig index
    u32 velocity = 0; // in ticks per 10 seconds
    u32 setpoint = 0;
};

struct character_t {
    int id{};
    motor_t motor;
    pin_t hall{};
    /* extra data */
};

bool stepConfig[4][4] {
    { 1, 0, 0, 1 },
    { 0, 0, 1, 1 },
    { 0, 1, 1, 0 },
    { 1, 1, 0, 0 } };

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

void step_motor(motor_t *motor) {
    motor->step++;
    int index = motor->step % 4;
    set_pin(&motor->a, stepConfig[index][0]);
    set_pin(&motor->b, stepConfig[index][1]);
    set_pin(&motor->c, stepConfig[index][2]);
    set_pin(&motor->d, stepConfig[index][3]);
}

u32 max(u32 a, u32 b) {
    return a > b ? a : b;
}
u32 min(u32 a, u32 b) {
    return a > b ? b : a;
}
u32 constrain(u32 lower, u32 upper, u32 value) {
    if(value < lower) return lower;
    if(value > upper) return upper;
    return value;
}


u32 deltaStepsGivenAccAndSpeed(u32 acc, u32 speed) {
    u32 deltaChunks = speed / acc;
    u32 deltaStepsToAcc = acc * deltaChunks * deltaChunks / 2;
    return deltaStepsToAcc;
}

extern "C"
int main() {
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_TIM1_Init();

    HAL_TIM_Base_Start(&htim1);

//    motor_profile_t profile {
//            .max_speed = 650, // steps per second
//            .acc = 200, // steps/sec/ms
//            .dec = 1000, // steps/sec/ms
//    };
//    calculate_dec_all(profile);

    motor_t m = characters[2].motor;
    m.step = 0;
    m.velocity = 0;
    m.setpoint = 2048;

    u32 CHUNKS_PER_SECOND = 10;
    u32 MAX_SPEED_PER_SEC = 700;     // steps per second

    u32 MAX_SPEED = MAX_SPEED_PER_SEC / CHUNKS_PER_SECOND; // steps per chunk max
    u32 ACC = MAX_SPEED / 3; // takes 5 chunks to accelerate
    u32 DCC = MAX_SPEED / 3; // takes 5 chunks to decelerate


    m.velocity = 500 / CHUNKS_PER_SECOND;

    while(true) {
        if(m.setpoint == m.step) {
            HAL_Delay(1000 * 1);
            m.setpoint = 2048;
            m.step = 0;
            continue; // TODO
        }
        htim1.Instance->CNT = 0;
        /* going to split time into 100ms chunks
         * where we step the stepper 'velocity / 10' times, evenly distributed
         */
        u32 steps = m.velocity;
        u32 time_delta_us = 1000 * 1000 / CHUNKS_PER_SECOND / steps; // each timer tick is 100uS
        for(u32 i = 0; i < steps; i++) {
            htim1.Instance->CNT = 0;
            while(htim1.Instance->CNT < time_delta_us) {}
            if(m.step < m.setpoint) { // if I screw up, don't overdo it
                step_motor(&m);
            }
        }

        if(m.setpoint == m.step) {
            continue; // busy wait for now
        }
        u32 left_to_go = m.setpoint - m.step;
        if (left_to_go < deltaStepsGivenAccAndSpeed(DCC, m.velocity)) {
            // start decelerating
            if(m.velocity > DCC) {
                m.velocity = m.velocity - DCC;
            } else {
                // I think this shouldn't happen, but we'll just let it finish out if it does?
            }
        } else {
            m.velocity = m.velocity + ACC;
            if (m.velocity > MAX_SPEED) m.velocity = MAX_SPEED;
        }


    }

}
