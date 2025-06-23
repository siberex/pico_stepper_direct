/**
 * pico_stepper_direct
 *
 * Author: Stephen Jingle <sib.li>
 * Created: 21 Jun 2025
 */

#include "stepper.h"

#include <cmath>

Stepper::Stepper() : Stepper(0) {}
Stepper::Stepper(const unsigned int positiveA) : Stepper(positiveA, positiveA + 1, positiveA + 2, positiveA + 3) {}
Stepper::Stepper(const unsigned int positiveA, const unsigned int negativeA, const unsigned int positiveB,
                 const unsigned int negativeB) :
    m_GpioPositiveA(positiveA), m_GpioNegativeA(negativeA), m_GpioPositiveB(positiveB), m_GpioNegativeB(negativeB) {
    initGpio();
}

Stepper::~Stepper() {
    off();

    gpio_deinit(m_GpioPositiveA);
    gpio_deinit(m_GpioNegativeA);
    gpio_deinit(m_GpioPositiveB);
    gpio_deinit(m_GpioNegativeB);
}

void Stepper::halfStep(const int steps) const {

    static int step_position = 0;

    for (int i = 0; i < abs(steps); i++) {
        setCoilA(k_SequenceHalfStep[step_position][0]);
        setCoilB(k_SequenceHalfStep[step_position][1]);

        if (steps > 0) {
            step_position = (step_position + 1) % 8;
        } else {
            step_position = (step_position - 1 + 8) % 8;
        }

        sleep_us(m_DurationMicroseconds);
    }
}

void Stepper::fullStep(const int steps) const {

    static int step_position = 0;

    for (int i = 0; i < abs(steps); i++) {
        // Set coils according to the current step
        setCoilA(k_SequenceFullStep[step_position][0]);
        setCoilB(k_SequenceFullStep[step_position][1]);

        // Move to the next step
        if (steps > 0) {
            step_position = (step_position + 1) % 4;
        } else {
            step_position = (step_position - 1 + 4) % 4;
        }

        sleep_us(m_DurationMicroseconds * 2);
    }
}

void Stepper::enableMicrostepping() {
    gpio_set_function(m_GpioPositiveA, GPIO_FUNC_PWM);
    gpio_set_function(m_GpioNegativeA, GPIO_FUNC_PWM);
    gpio_set_function(m_GpioPositiveB, GPIO_FUNC_PWM);
    gpio_set_function(m_GpioNegativeB, GPIO_FUNC_PWM);
    m_Microstep = true;
}
void Stepper::disableMicrostepping() {
    gpio_set_function(m_GpioPositiveA, GPIO_FUNC_SIO);
    gpio_set_function(m_GpioNegativeA, GPIO_FUNC_SIO);
    gpio_set_function(m_GpioPositiveB, GPIO_FUNC_SIO);
    gpio_set_function(m_GpioNegativeB, GPIO_FUNC_SIO);
    m_Microstep = false;
}
void Stepper::initGpio() const {
    // Initialize all pins as outputs
    gpio_init(m_GpioPositiveA);
    gpio_init(m_GpioNegativeA);
    gpio_init(m_GpioPositiveB);
    gpio_init(m_GpioNegativeB);

    // Set pin output to 12 mA for direct connection
    // If the motor could spin from 3.3V and consume <= 12 mA per coil - connect directly.
    // Otherwise, use dedicated driver like TI DRV8836.
    gpio_set_drive_strength(m_GpioPositiveA, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(m_GpioNegativeA, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(m_GpioPositiveB, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(m_GpioNegativeB, GPIO_DRIVE_STRENGTH_12MA);

    gpio_set_dir(m_GpioPositiveA, GPIO_OUT);
    gpio_set_dir(m_GpioNegativeA, GPIO_OUT);
    gpio_set_dir(m_GpioPositiveB, GPIO_OUT);
    gpio_set_dir(m_GpioNegativeB, GPIO_OUT);
}

void Stepper::setCoilA(const int8_t direction) const {
    if (direction > 0) {
        // Forward: A+ high, A- low
        gpio_put(m_GpioPositiveA, true);
        gpio_put(m_GpioNegativeA, false);
    } else if (direction < 0) {
        // Reverse: A+ low, A- high
        gpio_put(m_GpioPositiveA, false);
        gpio_put(m_GpioNegativeA, true);
    } else {
        // Off: both low
        gpio_put(m_GpioPositiveA, false);
        gpio_put(m_GpioNegativeA, false);
    }
}

void Stepper::setCoilB(const int8_t direction) const {
    if (direction > 0) {
        // Forward: B+ high, B- low
        gpio_put(m_GpioPositiveB, true);
        gpio_put(m_GpioNegativeB, false);
    } else if (direction < 0) {
        // Reverse: B+ low, B- high
        gpio_put(m_GpioPositiveB, false);
        gpio_put(m_GpioNegativeB, true);
    } else {
        // Off: both low
        gpio_put(m_GpioPositiveB, false);
        gpio_put(m_GpioNegativeB, false);
    }
}

void Stepper::off() const {
    setCoilA(0);
    setCoilB(0);
}



constexpr auto PWM_WRAP = 255;

struct PwmOut {
    uint slice;
    uint channel;
};

void set_pwm(const PwmOut out, const float val) {
    const int level = static_cast<int>((std::fabs(val)) * PWM_WRAP);
    pwm_set_chan_level(out.slice, out.channel, level);
}

void step_motor(const int step_index) {
    const float angle = (2.0f * M_PI * step_index) / 64;
    float sin_a = sinf(angle);
    float cos_a = cosf(angle);

    // Coil A
    // set_pwm(a_positive, sin_a > 0 ? sin_a : 0.0f);
    // set_pwm(a_negative, sin_a < 0 ? -sin_a : 0.0f);

    // Coil B
    // set_pwm(b_positive, cos_a > 0 ? cos_a : 0.0f);
    // set_pwm(b_negative, cos_a < 0 ? -cos_a : 0.0f);
}