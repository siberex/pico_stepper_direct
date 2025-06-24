/**
 * pico_stepper_direct
 *
 * Author: Stephen Jingle <sib.li>
 * Created: 05 Jun 2025
 */

#include <cstdlib>

#include "pico/stdlib.h"

#include "stepper.h"

int main() {
    stdio_init_all();

    const Stepper stepper;
    stepper.m_StepAngle = 18;
    // 50 ms per step = 1 shaft rotation per second (for step angle = 18°)
    stepper.m_DurationMicroseconds = 50'000;

    // Any value between 8 and duration/4 can be used
    //    8 is the same as full stepping
    //   16 = half-stepping
    // 1024 = super-smooth
    stepper.m_Microsteps = 1024;

    // Assume one step = 18° (for larger motors it is usually 1.8°)
    // 20 full steps = 40 half-steps = full circle
    // 20 half-steps = half-circle
    // 80 half-steps = 720° (two shaft rotations)
    // Use negative value for CCW
    const int stepsPerRevolute = static_cast<int>(360 / stepper.m_StepAngle);

    // ReSharper disable once CppDFAEndlessLoop
    while (true) {
        stepper.fullStep(stepsPerRevolute);
        sleep_ms(1000);

        stepper.halfStep(stepsPerRevolute * 2);
        sleep_ms(1000);

        // Note: It could crash silently here if the motor is connected via dual H-bridge improperly!
        // Guaranteed to crash when both coil contacts are mixed up (after-bridge A1 switched with A2 and B1 switched with B2).
        // Clang invocations inside, so you will not be able to catch any exception.
        stepper.enableMicrostepping();
        // microstep ratio = full-step number of steps / 4
        // 40 half steps = 20 full steps = 5 microsteps
        stepper.microStep(stepsPerRevolute / 4);
        stepper.disableMicrostepping();
        sleep_ms(1000);

        tight_loop_contents();
    }

    return 0;
}