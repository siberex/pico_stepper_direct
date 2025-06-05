/**
 * picostepper
 *
 * Author: Stephen Jingle <sib.li>
 * Created: 05 Jun 2025
 */

#include <stdlib.h>

#include "pico/stdlib.h"

// Pin definitions - connect directly to coil terminals
#define COIL_A_POSITIVE 10
#define COIL_A_NEGATIVE 11
#define COIL_B_POSITIVE 12
#define COIL_B_NEGATIVE 13

void setup_pins() {
    // Initialize all pins as outputs
    gpio_init(COIL_A_POSITIVE);
    gpio_init(COIL_A_NEGATIVE);
    gpio_init(COIL_B_POSITIVE);
    gpio_init(COIL_B_NEGATIVE);

    gpio_set_drive_strength(COIL_A_POSITIVE, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(COIL_A_NEGATIVE, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(COIL_B_POSITIVE, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(COIL_B_NEGATIVE, GPIO_DRIVE_STRENGTH_12MA);

    gpio_set_dir(COIL_A_POSITIVE, GPIO_OUT);
    gpio_set_dir(COIL_A_NEGATIVE, GPIO_OUT);
    gpio_set_dir(COIL_B_POSITIVE, GPIO_OUT);
    gpio_set_dir(COIL_B_NEGATIVE, GPIO_OUT);

}

void set_coil_a(int direction) {
    if (direction > 0) {
        // Forward: A+ high, A- low
        gpio_put(COIL_A_POSITIVE, 1);
        gpio_put(COIL_A_NEGATIVE, 0);
    } else if (direction < 0) {
        // Reverse: A+ low, A- high
        gpio_put(COIL_A_POSITIVE, 0);
        gpio_put(COIL_A_NEGATIVE, 1);
    } else {
        // Off: both low
        gpio_put(COIL_A_POSITIVE, 0);
        gpio_put(COIL_A_NEGATIVE, 0);
    }
}

void set_coil_b(int direction) {
    if (direction > 0) {
        // Forward: B+ high, B- low
        gpio_put(COIL_B_POSITIVE, 1);
        gpio_put(COIL_B_NEGATIVE, 0);
    } else if (direction < 0) {
        // Reverse: B+ low, B- high
        gpio_put(COIL_B_POSITIVE, 0);
        gpio_put(COIL_B_NEGATIVE, 1);
    } else {
        // Off: both low
        gpio_put(COIL_B_POSITIVE, 0);
        gpio_put(COIL_B_NEGATIVE, 0);
    }
}

void motor_off() {
    set_coil_a(0);
    set_coil_b(0);
}

// Full step sequence
void step_motor(int steps) {
    // Step sequence: [Coil A, Coil B]
    // Full step: A+B0, A0B+, A-B0, A0B-
    int sequence[4][2] = {
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

        sleep_ms(5); // Adjust for desired speed
    }
}

// Half step sequence for smoother motion
void half_step_motor(int steps) {
    // Half step sequence: [Coil A, Coil B]
    int sequence[8][2] = {
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

        sleep_ms(3); // Faster for half steps
    }
}

int main() {
    stdio_init_all();
    setup_pins();

    while (true) {
        // Example: rotate 200 steps forward
        step_motor(200);
        sleep_ms(1000);

        // Then 200 steps backward
        step_motor(-200);
        sleep_ms(1000);

        // Try half stepping for smoother motion
        half_step_motor(400);
        sleep_ms(1000);

        half_step_motor(-400);
        sleep_ms(1000);

        tight_loop_contents();
    }

    return 0;
}