I'll create a comprehensive architecture document.

```markdown:applications_user/film_developer/docs/ARCHITECTURE.md
# Film Developer Architecture

## Overview
The Film Developer application implements a flexible agitation system for film development using a motor control system. The architecture is designed for embedded systems with strict memory constraints and no dynamic allocation.

## Core Components

### 1. Movement System

#### AgitationMovement
The base class for all movement types, implementing a command pattern:
- Motor movements (CW/CCW)
- Pauses
- Loops
- User wait states

Each movement is self-contained and responsible for:
- Tracking its own state
- Managing its duration
- Controlling motor behavior
- Determining completion status

### 2. Memory Management

#### Static Allocation Strategy
- Fixed-size pools for movement objects
- Predetermined maximum sequence lengths
- Compile-time constants for all limits
- Zero heap allocation

#### MovementFactory
Manages a static pool of movement objects:
- Pre-allocated memory pool
- Object recycling
- Thread-safe allocation (if needed)
- Compile-time size verification

### 3. Loop Management

#### LoopStack
Manages nested loop execution:
- Fixed-depth stack (max 3 levels)
- Each level tracks:
  - Current sequence
  - Iteration count
  - Duration limits
  - Execution state
- Built-in safety checks for stack overflow/underflow

### 4. Execution Engine

#### AgitationInterpreter
Core execution engine that:
- Processes movement sequences
- Manages loop stack
- Controls motor states
- Handles timing
- Provides execution status

#### MotorController
Abstracts hardware control:
- Direct motor control
- Safety interlocks
- State validation
- Hardware abstraction layer

## Memory Constraints

### Fixed Limits
```cpp
static constexpr size_t MAX_SEQUENCE_LENGTH = 32;   // Maximum movements in a sequence
static constexpr size_t MAX_LOOP_DEPTH = 3;         // Maximum nested loops
static constexpr size_t MAX_MOVEMENTS = 64;         // Maximum concurrent movement objects
```

### Memory Usage
- Movement objects: ~16 bytes each
- Loop context: ~32 bytes each
- Total static allocation: < 2KB

## Execution Flow

1. **Sequence Definition**
   ```cpp
   // Example sequence creation
   const AgitationMovement* sequence[] = {
       MovementFactory::createCW(2),      // 2s clockwise
       MovementFactory::createPause(1),   // 1s pause
       MovementFactory::createCCW(2),     // 2s counter-clockwise
       MovementFactory::createPause(1)    // 1s pause
   };
   ```

2. **Interpreter Setup**
   ```cpp
   AgitationInterpreter interpreter(sequence, 4, motor_controller);
   ```

3. **Execution**
   ```cpp
   while (!interpreter.isComplete()) {
       interpreter.tick();  // Called every second
   }
   ```

## Safety Features

1. **Bounds Checking**
   - Stack depth validation
   - Sequence length verification
   - Movement pool overflow protection

2. **State Validation**
   - Motor state consistency
   - Loop nesting validation
   - Duration limits

3. **Resource Management**
   - Static allocation only
   - Deterministic memory usage
   - No fragmentation risks

## Extension Points

1. **New Movement Types**
   - Inherit from AgitationMovement
   - Implement required virtual methods
   - Register with MovementFactory

2. **Custom Sequences**
   - Pre-defined sequence templates
   - Runtime sequence composition
   - Nested loop patterns

3. **Hardware Abstraction**
   - Customizable motor control
   - Platform-specific optimizations
   - Sensor integration

## Performance Considerations

1. **Time Complexity**
   - O(1) movement creation
   - O(1) stack operations
   - O(1) tick processing

2. **Space Complexity**
   - Fixed memory footprint
   - No runtime allocation
   - Predictable stack usage

## Usage Example

```cpp
// Define a basic agitation pattern
const AgitationMovement* gentle_sequence[] = {
    MovementFactory::createCW(2),
    MovementFactory::createPause(1),
    MovementFactory::createCCW(2),
    MovementFactory::createPause(1)
};

// Create a looped sequence
const AgitationMovement* development_sequence[] = {
    MovementFactory::createLoop(gentle_sequence, 4, 5, 60),  // 5 iterations, max 60s
    MovementFactory::createWaitUser(),                       // Wait for user input
    MovementFactory::createLoop(gentle_sequence, 4, 3, 30)   // 3 iterations, max 30s
};

// Execute sequence
AgitationInterpreter interpreter(development_sequence, 3, motor_controller);
while (!interpreter.isComplete()) {
    interpreter.tick();
    // Handle other system tasks
}
```

---



```markdown:applications_user/film_developer/docs/SPECIFICATION.md
# Film Developer - Technical Specification

## Overview
This document specifies the technical implementation of the Film Developer's agitation system. The system is designed for embedded environments with strict memory constraints and uses C++ for better abstraction without dynamic allocation.

## Core Components

### 1. Movement System

#### Base Movement Class
```cpp
class AgitationMovement {
public:
    enum class Type {
        CW,
        CCW,
        Pause,
        Loop,
        WaitUser
    };

    explicit AgitationMovement(Type type, uint32_t duration = 0);
    virtual ~AgitationMovement() = default;

    virtual bool execute(class MotorController& motor) = 0;
    virtual bool isComplete() const = 0;
    virtual void reset() = 0;

protected:
    Type type;
    uint32_t duration;
    uint32_t elapsed_time{0};
};
```

#### Concrete Movement Classes
```cpp
class MotorMovement : public AgitationMovement {
public:
    explicit MotorMovement(Type type, uint32_t duration);
    bool execute(MotorController& motor) override;
    bool isComplete() const override;
    void reset() override;
};

class PauseMovement : public AgitationMovement {
public:
    explicit PauseMovement(uint32_t duration);
    bool execute(MotorController& motor) override;
    bool isComplete() const override;
    void reset() override;
};

class LoopMovement : public AgitationMovement {
public:
    LoopMovement(const AgitationMovement** sequence, 
                 size_t sequence_length,
                 uint32_t iterations,
                 uint32_t max_duration);
    bool execute(MotorController& motor) override;
    bool isComplete() const override;
    void reset() override;

private:
    const AgitationMovement** sequence;
    size_t sequence_length;
    uint32_t iterations;
    uint32_t max_duration;
};

class WaitUserMovement : public AgitationMovement {
public:
    WaitUserMovement();
    bool execute(MotorController& motor) override;
    bool isComplete() const override;
    void reset() override;
    
    void acknowledgeUser();
private:
    bool user_acknowledged{false};
};
```

### 2. Movement Factory
```cpp
class MovementFactory {
public:
    static constexpr size_t MAX_MOVEMENTS = 64;

    static AgitationMovement* createCW(uint32_t duration);
    static AgitationMovement* createCCW(uint32_t duration);
    static AgitationMovement* createPause(uint32_t duration);
    static AgitationMovement* createLoop(const AgitationMovement** sequence,
                                       size_t sequence_length,
                                       uint32_t iterations,
                                       uint32_t max_duration);
    static AgitationMovement* createWaitUser();
    
    static void reset();  // Resets the factory state

private:
    static uint8_t movement_pool[MAX_MOVEMENTS * sizeof(AgitationMovement)];
    static size_t current_pool_index;
    
    static AgitationMovement* allocateMovement(size_t size);
};
```

### 3. Loop Management
```cpp
class LoopStack {
public:
    static constexpr size_t MAX_DEPTH = 3;
    
    struct LoopContext {
        const AgitationMovement** sequence;
        size_t sequence_length;
        size_t current_index{0};
        uint32_t remaining_iterations{0};
        uint32_t original_count{0};
        uint32_t elapsed_duration{0};
        uint32_t max_duration{0};
        bool should_stop{false};
    };

    void pushLoop(const AgitationMovement** sequence,
                 size_t sequence_length,
                 uint32_t iterations,
                 uint32_t max_duration);
    bool popLoop();
    bool shouldContinueLoop() const;
    void updateElapsedTime();
    bool hasReachedMaxDuration() const;
    
    LoopContext& current();
    bool empty() const;
    size_t depth() const;

private:
    LoopContext stack[MAX_DEPTH];
    size_t current_depth{0};
};
```

### 4. Motor Control
```cpp
class MotorController {
public:
    MotorController(void (*motor_cw)(bool), void (*motor_ccw)(bool));
    
    void clockwise(bool enable);
    void counterClockwise(bool enable);
    void stop();
    
    bool isRunning() const;

private:
    void (*motor_cw_callback)(bool);
    void (*motor_ccw_callback)(bool);
    bool is_running{false};
};
```

### 5. Main Interpreter


````markdown:applications_user/film_developer/docs/SPECIFICATION.md (continued)

class AgitationInterpreter {
public:
    static constexpr size_t MAX_SEQUENCE_LENGTH = 32;

    AgitationInterpreter(const AgitationMovement** sequence,
                        size_t sequence_length,
                        MotorController& motor);

    bool tick();  // Returns false when sequence is complete
    void reset();
    bool isComplete() const;
    
    // Optional: Status reporting
    size_t getCurrentIndex() const;
    uint32_t getElapsedTime() const;
    size_t getLoopDepth() const;

private:
    const AgitationMovement** sequence;
    size_t sequence_length;
    size_t current_index{0};
    LoopStack loop_stack;
    MotorController& motor;
    uint32_t total_elapsed_time{0};
    
    bool handleLoopLogic();
    void executeCurrentMovement();
};
````

## Memory Layout and Constraints

### Static Memory Allocation
All components use static allocation with fixed sizes:
- Movement Pool: `64 * sizeof(AgitationMovement)` bytes
- Loop Stack: `3 * sizeof(LoopContext)` bytes
- Maximum sequence length: 32 movements
- Maximum loop nesting: 3 levels

### Memory Usage Breakdown
```cpp
// Approximate sizes per component
struct LoopContext          ~32 bytes
class AgitationMovement     ~16 bytes
class MotorMovement         ~20 bytes
class LoopMovement          ~32 bytes
class PauseMovement         ~16 bytes
class WaitUserMovement      ~16 bytes
```

## Execution Model

### Timing and Control Flow
1. System calls `tick()` every second
2. Interpreter processes current movement
3. Movement updates its state
4. Loop stack manages nested iterations
5. Motor control signals are updated

### Example Usage
```cpp
// Initialize motor controller
void motor_cw(bool enable) { /* Hardware control */ }
void motor_ccw(bool enable) { /* Hardware control */ }
MotorController motor(motor_cw, motor_ccw);

// Create movement sequence
const AgitationMovement* sequence[] = {
    MovementFactory::createCW(2),
    MovementFactory::createPause(1),
    MovementFactory::createCCW(2),
    MovementFactory::createPause(1)
};

// Initialize interpreter
AgitationInterpreter interpreter(sequence, 4, motor);

// Main execution loop
while (!interpreter.isComplete()) {
    interpreter.tick();
    // Wait 1 second
}
```

## Error Handling

### Movement Factory
- Returns nullptr if movement pool is exhausted
- Asserts on invalid parameters
- Validates sequence lengths

### Loop Stack
- Checks for stack overflow/underflow
- Validates loop parameters
- Enforces maximum duration limits

### Interpreter
- Validates sequence bounds
- Handles null movements
- Manages motor safety states

## Extension Points

### New Movement Types
To add a new movement type:
1. Create new class inheriting from AgitationMovement
2. Implement required virtual methods
3. Add factory method to MovementFactory
4. Update movement pool size if needed

### Example New Movement
```cpp
class RampMovement : public AgitationMovement {
public:
    RampMovement(uint32_t duration, uint8_t start_power, uint8_t end_power);
    bool execute(MotorController& motor) override;
    bool isComplete() const override;
    void reset() override;

private:
    uint8_t start_power;
    uint8_t end_power;
    uint8_t current_power;
};
```


## Implementation Notes

1. All virtual functions should be marked final in concrete classes
2. Use explicit constructors to prevent implicit conversions
3. Keep movement state minimal to reduce memory usage
4. Implement move semantics where beneficial
5. Use const correctness throughout
