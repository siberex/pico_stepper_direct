/**
 * pico_stepper_direct
 *
 * Author: Stephen Jingle <sib.li>
 * Created: 21 Jun 2025
 */

#include "stepper.h"

#include <cmath>


PwmGpio getGpioPwmSlice(const uint gpio) {
    const uint slice = pwm_gpio_to_slice_num(gpio);
    const uint channel = pwm_gpio_to_channel(gpio);
    return {slice, channel, gpio};
}

void initGpioPwm(const PwmGpio &gpioPwm) {
    gpio_set_function(gpioPwm.gpio, GPIO_FUNC_PWM);
    pwm_set_wrap(gpioPwm.slice, PWM_WRAP);
    pwm_set_enabled(gpioPwm.slice, true);
}

void initGpioPwm(const uint gpio) {
    const PwmGpio gpioPwm = getGpioPwmSlice(gpio);
    initGpioPwm(gpioPwm);
}

void deinitGpioPwm(const PwmGpio &gpioPwm) {
    pwm_set_enabled(gpioPwm.slice, false);
    gpio_deinit(gpioPwm.gpio);
}

void deinitGpioPwm(const uint gpio) {
    const PwmGpio gpioPwm = getGpioPwmSlice(gpio);
    deinitGpioPwm(gpioPwm);
}

void setPwm(const PwmGpio &out, const float val) {
    // Convert interval [0; 1] → [0; PWM_WRAP]
    // Additionally, accepts negative values and values > 1
    const int level = static_cast<int>(std::fabs(val) * PWM_WRAP) % PWM_WRAP;
    pwm_set_chan_level(out.slice, out.channel, level);
}

void initGpioSio(const uint gpio) {
    // Initialize pin as SIO output
    gpio_init(gpio);

    // Set pin output to 12 mA for direct connection
    // If the motor could spin from 3.3V and consume <= 12 mA per coil - connect directly.
    // Otherwise, use dedicated driver like TI DRV8836.
    gpio_set_drive_strength(gpio, GPIO_DRIVE_STRENGTH_12MA);

    gpio_set_dir(gpio, GPIO_OUT);
}


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
    if (m_Microstep) return;

    auto halfStepDuration = m_DurationMicroseconds / 2;
    if (halfStepDuration == 0) halfStepDuration = 1;

    static int sequenceIndex = 0;

    for (int i = 0; i < abs(steps); i++) {
        setCoilA(k_SequenceHalfStep[sequenceIndex][0]);
        setCoilB(k_SequenceHalfStep[sequenceIndex][1]);

        if (steps > 0) {
            sequenceIndex = (sequenceIndex + 1) % 8;
        } else {
            sequenceIndex = (sequenceIndex - 1 + 8) % 8;
        }

        sleep_us(halfStepDuration);
    }
}

void Stepper::fullStep(const int steps) const {
    if (m_Microstep) return;

    static int sequenceIndex = 0;

    for (int i = 0; i < abs(steps); i++) {
        // Set coils according to the current step
        setCoilA(k_SequenceFullStep[sequenceIndex][0]);
        setCoilB(k_SequenceFullStep[sequenceIndex][1]);

        // Move to the next step
        if (steps > 0) {
            sequenceIndex = (sequenceIndex + 1) % 4;
        } else {
            sequenceIndex = (sequenceIndex - 1 + 4) % 4;
        }

        sleep_us(m_DurationMicroseconds);
    }
}

void Stepper::fullStepSinglePhase(const int steps) const {
    if (m_Microstep) return;

    static int sequenceIndex = 0;

    for (int i = 0; i < abs(steps); i++) {
        // Set coils according to the current step
        setCoilA(k_SequenceFullStepSinglePhase[sequenceIndex][0]);
        setCoilB(k_SequenceFullStepSinglePhase[sequenceIndex][1]);

        // Move to the next step
        if (steps > 0) {
            sequenceIndex = (sequenceIndex + 1) % 4;
        } else {
            sequenceIndex = (sequenceIndex - 1 + 4) % 4;
        }

        sleep_us(m_DurationMicroseconds);
    }
}

void Stepper::microStep(const int steps) const {
    if (!m_Microstep) return;

    // m_DurationMicroseconds is relative to full step mode, which consists of 4 repeating steps, so multiply by 4
    auto microstepDuration = static_cast<int>(m_DurationMicroseconds / m_Microsteps * 4);
    if (microstepDuration == 0) microstepDuration = 1;

    for (int i = 0; i < abs(steps); i++) {

        if (steps > 0) {
            // Rotate forward
            for (unsigned int sequenceIndex = 0; sequenceIndex < m_Microsteps; ++sequenceIndex) {
                setMicroStep(sequenceIndex);
                sleep_us(microstepDuration);
            }
        } else {
            // Rotate backwards
            for (unsigned int sequenceIndex = m_Microsteps - 1; sequenceIndex > 0; --sequenceIndex) {
                setMicroStep(sequenceIndex);
                sleep_us(microstepDuration);
            }
        }
    }
}

void Stepper::setMicroStep(const unsigned int sequenceIndex) const {
    // ReSharper disable once CppDFAConstantConditions
    if (!m_Microstep) return;

    // This is merely a phase angle for the current microstep, not the motor shaft angle
    const auto angle = static_cast<float>(2.0f * M_PI * sequenceIndex / m_Microsteps);
    const float phaseSin = sinf(angle);
    const float phaseCos = cosf(angle);

    const auto aPositive = getGpioPwmSlice(m_GpioPositiveA);
    const auto aNegative = getGpioPwmSlice(m_GpioNegativeA);
    const auto bPositive = getGpioPwmSlice(m_GpioPositiveB);
    const auto bNegative = getGpioPwmSlice(m_GpioNegativeB);

    // Coil A
    setPwm(aPositive, phaseCos > 0 ? phaseCos : 0.0f);
    setPwm(aNegative, phaseCos < 0 ? -phaseCos : 0.0f);

    // Coil B
    setPwm(bPositive, phaseSin > 0 ? phaseSin : 0.0f);
    setPwm(bNegative, phaseSin < 0 ? -phaseSin : 0.0f);
}

void Stepper::enableMicrostepping() const {
    initGpioPwm(m_GpioPositiveA);
    initGpioPwm(m_GpioNegativeA);
    initGpioPwm(m_GpioPositiveB);
    initGpioPwm(m_GpioNegativeB);
    m_Microstep = true;
}

void Stepper::disableMicrostepping() const {
    deinitGpioPwm(m_GpioPositiveA);
    deinitGpioPwm(m_GpioNegativeA);
    deinitGpioPwm(m_GpioPositiveB);
    deinitGpioPwm(m_GpioNegativeB);
    gpio_set_function(m_GpioPositiveA, GPIO_FUNC_SIO);
    gpio_set_function(m_GpioNegativeA, GPIO_FUNC_SIO);
    gpio_set_function(m_GpioPositiveB, GPIO_FUNC_SIO);
    gpio_set_function(m_GpioNegativeB, GPIO_FUNC_SIO);
    m_Microstep = false;
}

void Stepper::initGpio() const {
    // Initialize all pins as outputs
    initGpioSio(m_GpioPositiveA);
    initGpioSio(m_GpioNegativeA);
    initGpioSio(m_GpioPositiveB);
    initGpioSio(m_GpioNegativeB);
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
    if (m_Microstep) {
        const auto aPositive = getGpioPwmSlice(m_GpioPositiveA);
        const auto aNegative = getGpioPwmSlice(m_GpioNegativeA);
        const auto bPositive = getGpioPwmSlice(m_GpioPositiveB);
        const auto bNegative = getGpioPwmSlice(m_GpioNegativeB);

        // Coil A
        setPwm(aPositive, 0.0f);
        setPwm(aNegative, 0.0f);

        // Coil B
        setPwm(bPositive, 0.0f);
        setPwm(bNegative, 0.0f);
    } else {
        setCoilA(0);
        setCoilB(0);
    }
}
