/**
 * picostepper
 *
 * Author: Stephen Jingle <sib.li>
 * Created: 05 Jun 2025
 */

#include <stdlib.h>

#include "pico/stdlib.h"

// Pin definitions
// If your motor could spin from 3.3V and consumes 12mA per coil or less - connect directly.
// Otherwise, use dedicated driver like TI DRV8836.
#define COIL_A_POSITIVE 10
#define COIL_A_NEGATIVE 11
#define COIL_B_POSITIVE 12
#define COIL_B_NEGATIVE 13

// Step delay in milliseconds
#define STEPPER_ONE_STEP_DELAY 8

void setup_pins() {
    // Initialize all pins as outputs
    gpio_init(COIL_A_POSITIVE);
    gpio_init(COIL_A_NEGATIVE);
    gpio_init(COIL_B_POSITIVE);
    gpio_init(COIL_B_NEGATIVE);

    // Set pin output to 12mA for direct connection
    gpio_set_drive_strength(COIL_A_POSITIVE, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(COIL_A_NEGATIVE, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(COIL_B_POSITIVE, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(COIL_B_NEGATIVE, GPIO_DRIVE_STRENGTH_12MA);

    gpio_set_dir(COIL_A_POSITIVE, GPIO_OUT);
    gpio_set_dir(COIL_A_NEGATIVE, GPIO_OUT);
    gpio_set_dir(COIL_B_POSITIVE, GPIO_OUT);
    gpio_set_dir(COIL_B_NEGATIVE, GPIO_OUT);

}

void set_coil_a(const int direction) {
    if (direction > 0) {
        // Forward: A+ high, A- low
        gpio_put(COIL_A_POSITIVE, true);
        gpio_put(COIL_A_NEGATIVE, false);
    } else if (direction < 0) {
        // Reverse: A+ low, A- high
        gpio_put(COIL_A_POSITIVE, false);
        gpio_put(COIL_A_NEGATIVE, true);
    } else {
        // Off: both low
        gpio_put(COIL_A_POSITIVE, false);
        gpio_put(COIL_A_NEGATIVE, false);
    }
}

void set_coil_b(const int direction) {
    if (direction > 0) {
        // Forward: B+ high, B- low
        gpio_put(COIL_B_POSITIVE, true);
        gpio_put(COIL_B_NEGATIVE, false);
    } else if (direction < 0) {
        // Reverse: B+ low, B- high
        gpio_put(COIL_B_POSITIVE, false);
        gpio_put(COIL_B_NEGATIVE, true);
    } else {
        // Off: both low
        gpio_put(COIL_B_POSITIVE, false);
        gpio_put(COIL_B_NEGATIVE, false);
    }
}

void motor_off() {
    set_coil_a(0);
    set_coil_b(0);
}

// Full step sequence
void step_motor(const int steps) {
    // Step sequence: [Coil A, Coil B]
    // Full step: A+B0, A0B+, A-B0, A0B-
    const int sequence[4][2] = {
        {1, 0},   // Step 0: A forward, B off
        {0, 1},   // Step 1: A off, B forward
        {-1, 0},  // Step 2: A reverse, B off
        {0, -1}   // Step 3: A off, B reverse
    };

    static int step_position = 0;

    for (int i = 0; i < abs(steps); i++) {
        // Set coils according to current step
        set_coil_a(sequence[step_position][0]);
        set_coil_b(sequence[step_position][1]);

        // Move to next step
        if (steps > 0) {
            step_position = (step_position + 1) % 4;
        } else {
            step_position = (step_position - 1 + 4) % 4;
        }

        sleep_ms(STEPPER_ONE_STEP_DELAY); // Adjust for desired speed
    }
}

// Half step sequence for smoother motion
void half_step_motor(const int steps) {
    // Half step sequence: [Coil A, Coil B]
    const int sequence[8][2] = {
        {1, 0},   // A+, B0
        {1, 1},   // A+, B+
        {0, 1},   // A0, B+
        {-1, 1},  // A-, B+
        {-1, 0},  // A-, B0
        {-1, -1}, // A-, B-
        {0, -1},  // A0, B-
        {1, -1}   // A+, B-
    };

    static int step_position = 0;

    for (int i = 0; i < abs(steps); i++) {
        set_coil_a(sequence[step_position][0]);
        set_coil_b(sequence[step_position][1]);

        if (steps > 0) {
            step_position = (step_position + 1) % 8;
        } else {
            step_position = (step_position - 1 + 8) % 8;
        }

        sleep_ms(STEPPER_ONE_STEP_DELAY / 2); // Faster for half steps
    }
}

int main() {
    stdio_init_all();
    setup_pins();

    while (true) {
        // Assume one step = 18° (for larger motors it is usually 1.8°)
        // 20 steps = half circle
        // 40 = full circle
        // 80 = 720°
        step_motor(40);
        sleep_ms(500);

        half_step_motor(-40);
        sleep_ms(500);

        tight_loop_contents();
    }

    return 0;
}