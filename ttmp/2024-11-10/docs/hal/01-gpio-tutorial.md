I'll create a tutorial about the Flipper Zero HAL layer, focusing on GPIO functionality. I'll break it down into sections with clear explanations and practical examples.

# Understanding Flipper Zero's Hardware Abstraction Layer (HAL)

The Hardware Abstraction Layer (HAL) provides a unified interface to interact with the Flipper Zero's hardware components. It abstracts away the complexities of direct hardware manipulation, making it easier to write portable and maintainable code.

## Part 1: GPIO Basics

The GPIO (General Purpose Input/Output) interface allows you to control individual pins on the device for digital input and output. Let's start with the basics.

### Simple LED Blink Example

We'll create a simple application that blinks an LED connected to one of the GPIO pins.

```c
#include <furi.h>
#include <furi_hal_gpio.h>

// Define GPIO pin for LED
const GpioPin* led_pin = &gpio_ext_pa7;  // External pin PA7

int32_t gpio_blink_app(void* p) {
    UNUSED(p);
    
    // Initialize pin as output
    furi_hal_gpio_init_simple(led_pin, GpioModeOutputPushPull);

    while(1) {
        // Toggle LED
        furi_hal_gpio_write(led_pin, true);  // LED on
        furi_delay_ms(500);                  // Wait 500ms
        furi_hal_gpio_write(led_pin, false); // LED off
        furi_delay_ms(500);                  // Wait 500ms
    }

    return 0;
}
```

Key concepts:
1. GPIO pins are referenced using the `GpioPin` struct
2. Pins must be initialized before use 
3. `furi_hal_gpio_init_simple()` configures basic pin setup
4. `furi_hal_gpio_write()` sets pin output state

## Part 2: GPIO Input 

Let's expand our knowledge by reading input from a GPIO pin.

### Button Input Example

We'll create an application that reads a button press and toggles an LED.

```c
#include <furi.h>
#include <furi_hal_gpio.h>

const GpioPin* led_pin = &gpio_ext_pa7;    // LED on PA7
const GpioPin* button_pin = &gpio_ext_pa4;  // Button on PA4

int32_t gpio_button_app(void* p) {
    UNUSED(p);
    
    // Initialize pins
    furi_hal_gpio_init_simple(led_pin, GpioModeOutputPushPull);
    furi_hal_gpio_init_simple(button_pin, GpioModeInput);  // Button as input
    
    bool led_state = false;
    
    while(1) {
        // Read button state (active low with pull-up)
        if(!furi_hal_gpio_read(button_pin)) {
            // Toggle LED
            led_state = !led_state;
            furi_hal_gpio_write(led_pin, led_state);
            
            // Simple debounce
            furi_delay_ms(100);
            while(!furi_hal_gpio_read(button_pin)) {
                furi_delay_ms(10);
            }
        }
        furi_delay_ms(10);
    }

    return 0;
}
```

Key concepts:
1. Input pins can be configured with different modes (input, input with pull-up/down)
2. `furi_hal_gpio_read()` returns the current pin state
3. Debouncing is important for button inputs

## Part 3: GPIO Interrupts

Instead of polling, we can use interrupts to respond to GPIO changes more efficiently.

### Interrupt-Driven Button Example

```c
#include <furi.h>
#include <furi_hal_gpio.h>

const GpioPin* led_pin = &gpio_ext_pa7;
const GpioPin* button_pin = &gpio_ext_pa4;

static bool led_state = false;

// Interrupt callback function
static void button_callback(void* context) {
    UNUSED(context);
    led_state = !led_state;
    furi_hal_gpio_write(led_pin, led_state);
}

int32_t gpio_interrupt_app(void* p) {
    UNUSED(p);
    
    // Initialize LED pin
    furi_hal_gpio_init_simple(led_pin, GpioModeOutputPushPull);
    
    // Initialize button pin with interrupt
    furi_hal_gpio_init(
        button_pin,
        GpioModeInterruptRise,  // Trigger on rising edge
        GpioPullDown,           // Pull-down resistor
        GpioSpeedLow);          // Low speed is fine for button
        
    // Attach interrupt callback
    furi_hal_gpio_add_int_callback(button_pin, button_callback, NULL);
    
    // Main loop - could do other tasks
    while(1) {
        furi_delay_ms(100);
    }
    
    // Cleanup
    furi_hal_gpio_remove_int_callback(button_pin);
    return 0;
}
```

Key concepts:
1. Interrupts can trigger on rising edge, falling edge, or both
2. Callback functions should be quick and non-blocking
3. Need to properly remove callbacks when done

## Part 4: Advanced GPIO Configuration

For more complex applications, we can use advanced GPIO configurations.

### Advanced Pin Configuration Example

```c
#include <furi.h>
#include <furi_hal_gpio.h>

const GpioPin* pin = &gpio_ext_pa7;

int32_t gpio_advanced_app(void* p) {
    UNUSED(p);
    
    // Advanced pin configuration
    furi_hal_gpio_init(
        pin,
        GpioModeOutputOpenDrain,  // Open drain output
        GpioPullUp,               // Internal pull-up
        GpioSpeedVeryHigh);       // High speed for fast switching
        
    // Example usage
    while(1) {
        // In open drain mode, we set low to sink current
        furi_hal_gpio_write(pin, false);  // Active
        furi_delay_ms(500);
        furi_hal_gpio_write(pin, true);   // High-Z state
        furi_delay_ms(500);
    }

    return 0;
}
```

Key concepts:
1. Different output modes available (push-pull, open-drain)
2. Can configure internal pull resistors
3. Speed settings affect slew rate and EMI

## Best Practices

1. Always initialize GPIO pins before use
2. Cleanup GPIO settings when your application exits
3. Use appropriate pin modes and configurations
4. Consider debouncing for button inputs
5. Keep interrupt callbacks short and non-blocking
6. Use meaningful pin names and constants
7. Check pin capabilities before using special functions
8. Consider power implications of GPIO configuration

## Debugging Tips

1. Use an oscilloscope or logic analyzer to verify timing
2. Check pin voltage levels with a multimeter
3. Verify pin configurations with furi_hal_gpio_read()
4. Use LED indicators for visual debugging
5. Add debug logging in non-interrupt code

This tutorial covered the basics of GPIO usage in the Flipper Zero HAL. The concepts can be extended to other HAL interfaces like I2C, SPI, or ADC for more complex applications.