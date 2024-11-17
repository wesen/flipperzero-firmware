# Context Documentation Enhancement

Streamlined context descriptions in Flipper OS component scripts for clearer documentation.

- Removed indirect language from context descriptions
- Made all subsystem descriptions more direct and concise
- Maintained clear linkage to Furi system documentation
- Preserved functional subsystem organization

# GUI Demo Application
Added a simple GUI demo application that demonstrates basic Flipper Zero GUI concepts including:
- ViewPort management
- Canvas drawing primitives
- Input handling
- State management with mutex protection
- Proper resource cleanup

# HAL System Script Organization
Created scripts to organize and query HAL system components for better documentation and analysis.

- Created hal_core.sh for core hardware abstraction components
- Created hal_usb.sh for USB and serial communication interfaces
- Created hal_wireless.sh for wireless communication protocols
- Created hal_power.sh for power management functionality
- Created hal_storage.sh for storage and memory interfaces
- Created hal_peripherals.sh for peripheral interfaces
- Created hal_security.sh for security and crypto operations
- Created hal_system.sh for system-level functionality
- Created hal_io.sh for input/output functionality

# HAL Debug Examples Organization
Created scripts to demonstrate HAL functionality through debug applications.

- Created core_examples.sh for core HAL usage demonstrations
- Created usb_examples.sh for USB and serial interface examples
- Created wireless_examples.sh for wireless protocol demos
- Created power_examples.sh for power management examples
- Created storage_examples.sh for storage operation demos
- Created peripheral_examples.sh for peripheral interface tests
- Created security_examples.sh for security feature examples
- Created system_examples.sh for system functionality demos
- Created io_examples.sh for input/output testing examples

# GPIO Loop Application Updates

## Enhanced Process Control
Added new process control features:
- Added pause/resume functionality with motor safety
- Added step skip capability
- Added current step restart option
- Updated UI with button hints
- Improved status display with pause indication

## C41 Process Integration
Added support for executing C41 film development process with visual feedback and motor control.
- Implemented process interpreter integration
- Added GUI with process status display
- Added motor control via GPIO pins
- Added temperature tracking display
- Improved error handling and cleanup

## Motor Control Safety Improvements
Enhanced motor control safety and reliability:
- Updated for active low pin operation
- Added motor stop safety function
- Prevented simultaneous motor activation
- Added safety delays between motor switching
- Improved motor state initialization and cleanup

# Process Organization

Split agitation processes into separate header files for better organization and maintainability:
- Created common_sequences.h for shared sequence definitions
- Separated B&W standard, stand development, continuous gentle, and C41 processes into individual files
- Created main agitation_processes.h to include all process definitions

# Duration-based Loop Support

Added support for duration-based loops in the agitation DSL and interpreter to allow more flexible process timing control.

- Added max_duration field to loop instructions
- Updated DSL spec to document duration-based loops
- Modified interpreter to track elapsed time in loops
- Updated C41 process to use duration-based timing
- Added duration tracking to loop stack entries

# C++ Support Test

Added C++ test files to verify C++ compilation support in the project.

- Added motor_controller.hpp with a simple MotorController class
- Added motor_controller.cpp as companion implementation file
- Implemented singleton pattern for global motor controller access

# UI Enhancement: Display Loop Duration and Remaining Time

Updated the GPIO Loop UI to show timing information instead of temperature:
- Shows elapsed/total time when in a loop with max duration
- Shows elapsed time for continuous loops  
- Shows remaining time for non-loop movements

# C++ Refactor of Agitation Interpreter

Refactored the agitation interpreter to C++ for better type safety and maintainability.

- Converted agitation interpreter to C++ class
- Added proper encapsulation of internal state
- Improved type safety with enum class
- Maintained embedded-friendly implementation (no STL, no dynamic allocation)

# C++ Agitation Movement System Implementation

Implemented the core movement system classes for the film developer agitation control:
- Created base AgitationMovement class with concrete implementations
- Implemented static MovementFactory for memory-constrained allocation
- Added motor, pause, loop and wait user movement types
- Designed for zero dynamic allocation and embedded constraints

# Motor Controller C++ Implementation

Implemented C++ MotorController class to manage film developer motor hardware:
- Encapsulated GPIO pin control in RAII class
- Added safety delays between motor direction changes
- Maintained active-low pin logic from C implementation
- Added running state tracking
- Prevented object copying for hardware safety

# Movement Loader Implementation

Added MovementLoader class to convert static movement declarations to C++ movement objects:
- Created static sequence loader with safety limits
- Added support for nested loop sequences
- Maintained zero dynamic allocation design
- Added basic test coverage for C41 process
- Modified MovementLoader to take MovementFactory as constructor parameter
- Removed static methods and made them instance methods
- Updated factory method calls to use injected factory instance

# Refactor Film Developer Motor Control

Replaced direct GPIO control with MotorController class for better encapsulation and safety

- Removed dependency on io.c/io.h
- Added motor controller instance to FilmDeveloperApp
- Updated motor control callbacks to use MotorController
- Added proper cleanup of motor controller resources

# Add Debug Printing to Movement Classes

Added print() method to AgitationMovement base class and all derived movement classes to help with debugging and monitoring movement execution.

- Added virtual print() method to AgitationMovement base class
- Implemented print() in MotorMovement to show direction and timing
- Implemented print() in PauseMovement to show duration
- Implemented print() in WaitUserMovement to show acknowledgment status
- Implemented print() in LoopMovement to show iteration status and current movement

# Enhanced Movement Debug Output

Improved movement debug printing with detailed timing information.

- Added remaining time display to all timed movements
- Added elapsed time display to all movements
- Added total duration display where applicable
- Improved formatting consistency across movement types

# Add Debug Print Infrastructure

Added debug printing infrastructure with conditional compilation:
- Created debug.hpp with DEBUG_PRINT macro
- Added NDEBUG-aware compilation to disable debug prints in release builds
- Integrated with movement system debug printing

# Add Host System Test Infrastructure

Created test infrastructure to run and debug film developer code on host system:
- Added test-film_developer directory with standalone test program
- Created Makefile for host system compilation
- Added mock MotorController implementation for testing
- Implemented C41 process printing test

# Enhanced Test Build System

Improved Makefile for test environment:
- Added automatic dependency tracking for header files
- Included all film_developer source files in build
- Maintained source directory structure in object files
- Added film_developer to include paths

# Add Host/Embedded Environment Support

Added support for building and testing on host system:
- Added HOST compilation flag to differentiate environments
- Created host-specific MotorController interface
- Added TestMotorController mock implementation
- Updated Makefile to exclude embedded-only files
- Added proper virtual destructor to MotorController
- Fixed unused parameter warnings in test code

# Enhanced Host System Support for Agitation Sequence

Added comprehensive host system support to agitation sequence structures:
- Created minimal FuriString implementation for host system
- Added debug print methods to dynamic structures when building for host
- Added string allocation/deallocation functions for host
- Maintained full compatibility with embedded system
- Added detailed structure printing for debugging

# Fix Placement New Compilation Error

Fixed compilation error in MovementFactory:
- Added <new> header for placement new operator
- Maintained zero dynamic allocation design
- Fixed build for host system compilation

# Add Movement Testing Infrastructure

Enhanced test infrastructure with mock controller and improved test coverage:
- Created MockController with state tracking and safety checks
- Added detailed test output for sequence execution
- Added motor state consistency checks
- Added safety limits to prevent infinite loops
- Improved test feedback with DEBUG_PRINT

# Refactor Motor Controller for Better Abstraction

Split motor controller into abstract base class and embedded implementation:
- Created pure virtual MotorController base class
- Moved embedded implementation to separate files
- Added proper virtual destructor
- Protected constructor to prevent direct instantiation
- Maintained copy prevention for all derived classes
- Separated GPIO handling into embedded-specific code

# Improve Motor Controller Dependency Management

Enhanced motor controller handling in interpreter and main app:
- Made motor controller a dependency passed to interpreter
- Removed motor controller ownership from interpreter
- Added proper controller selection for host/embedded builds
- Updated initialization to pass motor controller through
- Fixed potential memory leaks in cleanup

# Update Test Application for MovementLoader Changes
Updated test application to use new MovementLoader dependency injection pattern.

- Removed static MovementFactory::reset() call
- Created MovementFactory instance
- Created MovementLoader instance with factory dependency
- Updated sequence loading to use loader instance
- Added movement_loader.hpp include

# Update Agitation Interpreter for MovementLoader Changes
Modified agitation interpreter to use new MovementLoader dependency injection pattern.

- Removed static MovementFactory::reset() call
- Created local MovementFactory instance
- Created MovementLoader instance with factory dependency
- Updated sequence loading to use loader instance
- Added movement_loader.hpp include

# Memory Management Improvements in Movement System
Improved memory safety by eliminating shared static data in movement sequences. Factory now properly allocates sequence storage from its pool.

- Modified MovementFactory to allocate sequence storage from pool
- Updated MovementLoader to take destination array parameter
- Removed static sequence array from MovementLoader
- Updated test application to use local sequence arrays

# Update Agitation Interpreter for New Movement Loader API
Updated agitation interpreter to use the new MovementLoader interface with explicit sequence storage.

- Modified agitation_interpreter_init to allocate sequence storage
- Updated sequence loading to use new three-parameter loadSequence API
- Maintained static allocation for embedded constraints

Simplified Agitation Control Architecture
Merged agitation interpreter into process interpreter to reduce complexity and remove unnecessary abstraction layer. This simplifies the code structure while maintaining all existing functionality.

- Removed agitation_interpreter.cpp and .hpp
- Added movement sequence handling directly in process interpreter
- Simplified initialization and execution flow
- Maintained existing debug output and error handling

Merge Interpreter State into Process Interpreter
Consolidated interpreter state fields into process interpreter header to complete the merge of the two components. This simplifies the state management and removes redundant structures while maintaining all functionality.

- Merged loop state tracking structures into process interpreter
- Added movement sequence state fields
- Added current movement tracking fields
- Updated function signatures for motor controller
- Removed dependency on agitation_interpreter.hpp

# Transform AgitationProcessInterpreterState to C++ Class

Modernized the codebase by converting the C-style AgitationProcessInterpreterState struct into a proper C++ class for better encapsulation and maintainability.

- Created new AgitationProcessInterpreter class with proper member functions
- Encapsulated internal state with private members and public interface
- Updated film developer application to use the new class interface
- Improved type safety with enum class for process states

# Make MovementLoader a Member of AgitationProcessInterpreter

Improved encapsulation and resource management by making MovementLoader and sequence storage permanent members of the AgitationProcessInterpreter class.

- Added MovementFactory as class member
- Added MovementLoader as class member initialized with factory
- Added loaded_sequence array as class member
- Removed static sequence array from initializeMovementSequence
- Updated initialization to properly construct loader with factory

# Clean Up Movement Sequence Management

Improved movement sequence handling in AgitationProcessInterpreter for better reliability and safety.

- Added proper initialization of loaded_sequence array in constructor
- Added sequence cleanup in initializeMovementSequence
- Added debug output for sequence loading
- Fixed memory safety by ensuring array is zeroed before loading new sequence
- Improved code organization and readability

## Fix Step Skip Implementation
Added proper step skip method to process interpreter to fix linter error and improve encapsulation.

- Added skipToNextStep() method to AgitationProcessInterpreter
- Updated film_developer.cpp to use new method instead of direct index manipulation
- Fixed linter error related to step index increment

## Add Memory Allocation Error Handling
Added proper error handling for movement allocation failures to prevent undefined behavior.

- Added null checks in MovementFactory allocation methods
- Added error propagation through MovementLoader
- Added Error state to AgitationProcessInterpreter
- Added debug messages for allocation failures
- Added memory safety checks before sequence copying

## Improve Movement Factory Memory Safety
Enhanced movement factory with better memory safety checks and debugging capabilities.

- Added pre-allocation safety checks to prevent placement new failures
- Added methods to query pool space and allocation possibility
- Added detailed debug messages for allocation failures
- Added pool usage statistics
- Separated memory allocation from object construction
- Added total size calculation for complex movements

# Time Tracking and Step Control Improvements

Added better time tracking and step control to improve user experience and code maintainability.

- Added timeElapsed() and timeRemaining() methods to movement classes
- Improved LoopMovement to track time across sequences
- Added advanceToNextStep() helper to process interpreter
- Updated status display to show elapsed/total time

# Improved Duration Calculation for Loop Movements

Enhanced time tracking for loop movements to better handle both duration-limited and iteration-based sequences.

- Added getDuration() method to calculate total movement duration
- Updated timeRemaining() to use getDuration() for more accurate calculations
- Fixed duration calculation for nested sequences

Enhanced Process Interpreter Information Display
Added getCurrentStep and getCurrentMovement helper methods to provide more detailed information about the current state of film development process execution.
- Added getCurrentStep() to get information about the current development step
- Added getCurrentMovement() to get information about the current agitation movement
- Enhanced test program to display more detailed process state information

Enhanced Test Program Output
Improved test program output with more detailed process execution information.
- Added detailed step information display
- Added process state tracking and display
- Added movement timing progress information
- Added percentage completion tracking
- Improved formatting and organization of debug output

Interactive Test Program
Enhanced test program with interactive features for better debugging experience.
- Added step-by-step execution with Enter key
- Added screen clearing between ticks for cleaner output
- Added quit option with 'q' key
- Improved visual separation between execution states

Enhanced Movement Execution Logging
Added detailed execution logging to all movement types for better debugging and monitoring.
- Added execution logging to MotorMovement with direction and progress
- Added execution logging to PauseMovement with progress
- Added execution logging to LoopMovement with iteration and sequence position
- Added execution logging to WaitUserMovement with state and elapsed time

# Motor Direction Display

Added direction display (CW/CCW) to the UI to better show motor movement state.

- Added getDirectionString() method to MotorController interface
- Implemented direction tracking in both mock and embedded controllers
- Updated UI to show current movement direction

# Scene Manager Tutorial Refactor

Refactored the Scene Manager tutorial to follow the recommended pattern with:
- Separated views into their own files
- Proper scene configuration using macros
- Better organized scene management
- Cleaner code structure following Flipper Zero conventions

# Film Photography Menu Demo App

Added a new demo application that shows how to implement nested submenus for selecting photography film and developer types. The app demonstrates proper view management, callback handling, and state management using the submenu widget.

- Created test_menu_film application with three submenu views
- Implemented film type selection submenu
- Implemented developer selection submenu
- Added state tracking for selections