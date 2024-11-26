## Basic Logging Structure

The Flipper Zero uses a macro-based logging system with different severity levels. Here's how to implement logging in your code:

```c
#define TAG "YourAppName"

// Error logging
FURI_LOG_E(TAG, "Error message: %s", error_string);

// Warning logging
FURI_LOG_W(TAG, "Warning message: %d", warning_value);

// Info logging
FURI_LOG_I(TAG, "Info message");

// Debug logging
FURI_LOG_D(TAG, "Debug message: %f", debug_value);

// Trace logging
FURI_LOG_T(TAG, "Trace message");
```[1][4]

## Log Format

Each log entry follows this pattern:
- First parameter: TAG - identifies the source of the log
- Second parameter: Format string (printf-style)
- Additional parameters: Variables to be formatted[1]

## Raw Logging

For cases where you need logging without tags, you can use the RAW variants:

```c
// Raw error logging
FURI_LOG_RAW_E("Raw error message");

// Raw warning logging
FURI_LOG_RAW_W("Raw warning message");
```[3]

## Color Support

The logging system includes color coding for different log levels using ANSI escape codes:
- Errors: Red
- Warnings: Yellow
- Debug: Default terminal color
- Trace: Purple[3]

Citations:
[1] https://gist.github.com/jaflo/50c35c46f3ecada7a18c9e5cc203a3f8
[2] https://developer.flipper.net/flipperzero/doxygen/dev_board_reading_logs.html
[3] https://developer.flipper.net/flipperzero/doxygen/log_8h.html
[4] https://www.reddit.com/r/FlipperZeroDev/comments/10j52z5/howto_view_and_debug_your_application_through/