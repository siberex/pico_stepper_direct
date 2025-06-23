/**
 * pico_stepper_direct
 *
 * Author: Stephen Jingle <sib.li>
 * Created: 21 Jun 2025
 */


#pragma once

#include <cstdint>

// Pico SDK
#include "pico/stdlib.h"
#include "hardware/pwm.h"


// Full step sequence: [Coil A, Coil B]
static constexpr int8_t k_SequenceFullStep[4][2] = {
    // { A,  B}       Coils          Phase
    { 1,  0},   // A ↑, B 0     = ⬆️
    { 0,  1},   // A 0, B →     = ➡️
    {-1,  0},   // A ↓, B 0     = ⬇️
    { 0, -1}    // A 0, B ←     = ⬅️
};

// Half step sequence: [Coil A, Coil B]
static constexpr int8_t k_SequenceHalfStep[8][2] = {
    // { A,  B}       {GPIOs}        Coils       Phase
    { 1,  0},   // {1, 0, 0, 0} = A ↑, B 0  = ⬆️
    { 1,  1},   // {1, 0, 1, 0} = A ↑, B →  = ↗️
    { 0,  1},   // {0, 0, 1, 0} = A 0, B →  = ➡️
    {-1,  1},   // {0, 1, 1, 0} = A ↓, B →  = ↘️
    {-1,  0},   // {0, 1, 0, 0} = A ↓, B 0  = ⬇️
    {-1, -1},   // {0, 1, 0, 1} = A ↓, B ←  = ↙️
    { 0, -1},   // {0, 0, 0, 1} = A 0, B ←  = ⬅️
    { 1, -1}    // {1, 0, 0, 1} = A ↑, B ←  = ↖️
};

static constexpr uint8_t microstepsLinear[9]       = {0, 32, 64, 96, 128, 160, 192, 224, 255};
static constexpr uint8_t microstepsNonlinear[9]    = {0,  8, 24, 56, 128, 200, 232, 248, 255};

class Stepper {
public:
    /**
     *
     */
    mutable float m_StepAngle = 1.8f;

    mutable unsigned int m_DurationMicroseconds = 1000;
    mutable bool m_MicroStepLinear = false;

    explicit Stepper();
    explicit Stepper(unsigned int positiveA);
    explicit Stepper(unsigned int positiveA, unsigned int negativeA, unsigned int positiveB, unsigned int negativeB);
    ~Stepper();

    /**
     * Half step sequence for smoother motion
     *
     * @param steps Number of steps to run
     */
    void halfStep(int steps) const;

    /**
     * Full step sequence (jaggy and less torque, but faster)
     *
     * @param steps Number of steps to run
     */
    void fullStep(int steps) const;

    /**
     * Disable control
     */
    void off() const;

    void enableMicrostepping();
    void disableMicrostepping();

private:
    bool m_Microstep = false;

    // Coil A pins
    // positive high, negative low = ↑
    // positive low, negative high = ↓
    // both low = stall
    // both high = stall, but don't do this, please
    const unsigned int m_GpioPositiveA = 0;
    const unsigned int m_GpioNegativeA = 1;

    // Coil B pins
    // positive high, negative low = →
    // positive low, negative high = ←
    // both low = stall
    const unsigned int m_GpioPositiveB = 2;
    const unsigned int m_GpioNegativeB = 3;

    void initGpio() const;
    void setCoilA(int8_t direction) const;
    void setCoilB(int8_t direction) const;
};
