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
    stepper.m_DurationMicroseconds = 25'000;
    // stepper.m_StepAngle = 18;

    // ReSharper disable once CppDFAEndlessLoop
    while (true) {
        // Assume one step = 18° (for larger motors it is usually 1.8°)
        // 20 half steps = half circle
        // 40 half steps or 20 full steps = full circle
        // 80 = 720°

        stepper.fullStep(20);
        stepper.off();
        sleep_ms(1000);

        stepper.halfStep(40);
        stepper.off();
        sleep_ms(1000);

        tight_loop_contents();
    }

    return 0;
}