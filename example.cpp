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
    stepper.m_DurationMicroseconds = 4'000;
    // stepper.m_StepAngle = 18;

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