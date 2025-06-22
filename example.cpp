/**
 * pico_stepper_direct
 *
 * Author: Stephen Jingle <sib.li>
 * Created: 05 Jun 2025
 */

#include <cstdlib>

#include "pico/stdlib.h"

#include "stepper.h"

// Pin definitions
#define COIL_A_POSITIVE 0
#define COIL_A_NEGATIVE 1
#define COIL_B_POSITIVE 2
#define COIL_B_NEGATIVE 3

// Step delay in milliseconds
#define STEPPER_ONE_STEP_DELAY 8

int main() {
    stdio_init_all();

    const Stepper stepper{COIL_A_POSITIVE, COIL_A_NEGATIVE, COIL_B_POSITIVE, COIL_B_NEGATIVE};
    stepper.m_DurationMicroseconds = STEPPER_ONE_STEP_DELAY * 1000;
    stepper.m_StepAngle = 18;

    // ReSharper disable once CppDFAEndlessLoop
    while (true) {
        // Assume one step = 18° (for larger motors it is usually 1.8°)
        // 20 steps = half circle
        // 40 = full circle
        // 80 = 720°
        stepper.fullStep(40);
        sleep_ms(500);

        stepper.halfStep(-40);
        sleep_ms(500);

        tight_loop_contents();
    }

    return 0;
}