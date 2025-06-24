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
//  { A,  B}       Coils          Phase
    { 1,  1},   // A ↑, B →     = ↗️
    {-1,  1},   // A ↓, B →     = ↘️
    {-1, -1},   // A ↓, B ←     = ↙️
    { 1, -1},   // A ↑, B ←     = ↖️
};

static constexpr int8_t k_SequenceFullStepSinglePhase[4][2] = {
//  { A,  B}       Coils          Phase
    { 1,  0},   // A ↑, B 0     = ⬆️
    { 0,  1},   // A 0, B →     = ➡️
    {-1,  0},   // A ↓, B 0     = ⬇️
    { 0, -1},   // A 0, B ←     = ⬅️
};

// Half step sequence: [Coil A, Coil B]
// See also: https://www.monolithicpower.com/learning/resources/bipolar-stepper-motors-part-i-control-modes#:~:text=of%203D%20printers-,Stepping%20Modes,-The%20bipolar%20stepper
static constexpr int8_t k_SequenceHalfStep[8][2] = {
//  { A,  B}       {GPIOs}        Coils       Phase
    { 1,  1},   // {1, 0, 1, 0} = A ↑, B →  = ↗️
    { 0,  1},   // {0, 0, 1, 0} = A 0, B →  = ➡️
    {-1,  1},   // {0, 1, 1, 0} = A ↓, B →  = ↘️
    {-1,  0},   // {0, 1, 0, 0} = A ↓, B 0  = ⬇️
    {-1, -1},   // {0, 1, 0, 1} = A ↓, B ←  = ↙️
    { 0, -1},   // {0, 0, 0, 1} = A 0, B ←  = ⬅️
    { 1, -1},   // {1, 0, 0, 1} = A ↑, B ←  = ↖️
    { 1,  0},   // {1, 0, 0, 0} = A ↑, B 0  = ⬆️
};

// not used at the moment
static constexpr uint8_t microstepsLinear[9]       = {0, 32, 64, 96, 128, 160, 192, 224, 255};
static constexpr uint8_t microstepsNonlinear[9]    = {0,  8, 24, 56, 128, 200, 232, 248, 255};


constexpr auto PWM_WRAP = 255;

// Each slice 0 to 7 (to 11 for RP2350B) has two channels, A and B.
// Each GPIO pin is mapped according to this:
// https://github.com/raspberrypi/pico-sdk/blob/ee68c78d0afae2b69c03ae1a72bf5cc267a2d94c/src/rp2350/rp2350a_interface_pins.json#L57
struct PwmGpio {
    uint slice;
    uint channel;
    uint gpio;
};




class Stepper {
public:
    // not used internally at the moment
    mutable float m_StepAngle = 1.8f;

    /**
     * Full step duration
     *
     *
     */
    mutable unsigned int m_DurationMicroseconds = 1000;

    /**
     * Number of microsteps divisions
     */
    mutable unsigned int m_Microsteps = 64;

    // not used at the moment
    mutable bool m_MicroStepLinear = false;


    explicit Stepper();
    explicit Stepper(unsigned int positiveA);
    explicit Stepper(unsigned int positiveA, unsigned int negativeA, unsigned int positiveB, unsigned int negativeB);
    ~Stepper();

    /**
     * Half-step sequence for smoother motion
     *
     * @param steps Number of steps to run, positive for CW, negative for CCW
     */
    void halfStep(int steps) const;

    /**
     * Full step sequence using two phases (jaggy but fast)
     *
     * @param steps Number of steps to run, positive for CW, negative for CCW
     */
    void fullStep(int steps) const;

    /**
     * Full step sequence using single phase (jaggy and less torque, but fast)
     *
     * @param steps Number of steps to run, positive for CW, negative for CCW
     */
    void fullStepSinglePhase(int steps) const;

    /**
     * Microstepping emulation with PWM
     *
     * @param steps
     */
    void microStep(int steps) const;

    /**
     * Disable control
     */
    void off() const;

    void enableMicrostepping() const;
    void disableMicrostepping() const;

private:
    mutable bool m_Microstep = false;

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
    void setMicroStep(unsigned int sequenceIndex) const;
};
