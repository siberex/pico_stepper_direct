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
Stepper::Stepper(const unsigned int positiveA, const unsigned int negativeA, const unsigned int positiveB, const unsigned int negativeB) :
    m_GpioPositiveA(positiveA),
    m_GpioNegativeA(negativeA),
    m_GpioPositiveB(positiveB),
    m_GpioNegativeB(negativeB) {
    initGpio();
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

    // Set pin output to 12mA for direct connection
    //
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