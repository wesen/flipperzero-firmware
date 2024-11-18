# Static Data Structure Exercises
A progressive learning path for embedded developers

## Basic Exercises

### Exercise 1: Simple Constants
Create a system configuration module that includes:
- Protocol version (uint8_t)
- Device name (string)
- Maximum retry count (uint32_t)
- List of supported commands (uint8_t array)
- Hardware revision (uint16_t)

Requirements:
- All data must be in ROM
- Accessible from multiple C files
- Include proper header guards
- Values must be available at link time

🤔 Hints:
1. Think about where string literals are stored
2. Consider alignment requirements
3. Remember the extern keyword's purpose

### Exercise 2: Array Sizes
Build a lookup table system for sensor calibration:
- Temperature points (int16_t array)
- Corresponding voltage values (uint32_t array)
- Number of points in the table
- Valid range indicators (min/max)

Requirements:
- Size must automatically update if points are added/removed
- No preprocessor macros for sizes
- Must work with different calibration tables
- Prevent size/data mismatches

🤔 Hints:
1. Think about compile-time size calculation
2. Consider the sizeof operator
3. Remember array decay rules in C

## Intermediate Exercises

### Exercise 3: Structure Arrays
Create a menu system with:
- Menu items (name, command, enabled flag)
- Menu groups (title, items array, item count)
- Command handlers (function pointers)
- Access levels per item

Requirements:
- All strings in ROM
- No dynamic allocation
- Nested menu support
- Proper alignment

🤔 Hints:
1. Consider structure padding
2. Think about const placement
3. Remember function pointer syntax

### Exercise 4: Self-Referential Data
Build a state machine definition:
- States (name, handlers)
- Transitions (current state, event, next state)
- Default state
- Error states

Requirements:
- States must reference other states
- All data in ROM
- No forward declarations
- Clean header interface

🤔 Hints:
1. Order of definition matters
2. Think about link-time resolution
3. Consider separation of declaration/definition

## Advanced Exercises

### Exercise 5: Composite Patterns
Create a device protocol system:
- Command definitions (opcode, name, payload size)
- Response definitions (status codes, payload format)
- Command groups (related commands)
- Protocol flow definitions

Requirements:
- Support command chaining
- Nested group definitions
- Cross-references between commands
- Version compatibility data

🤔 Hints:
1. Think about hierarchical structures
2. Consider circular references
3. Remember ROM space optimization

### Exercise 6: The Grand Challenge - Film Development Sequences
Create a complete film development sequence system:
- Basic movements (CW, CCW, Pause)
- Compound sequences (repeated patterns)
- Development processes (sequence of sequences)
- Temperature and time variations

#### Part A: Basic Sequences
Define the fundamental movements and simple sequences

Requirements:
- Duration control
- Direction control
- Basic pause handling
- Sequence validation

#### Part B: Compound Sequences
Build repeatable patterns using basic sequences

Requirements:
- Loop counting
- Sequence nesting
- Timing accuracy
- Memory efficiency

#### Part C: Complete Processes
Create full development processes using compound sequences

Requirements:
- Temperature specifications
- Multiple steps
- Process variations
- Documentation integration

🤔 Hints:
1. Start with the smallest building blocks
2. Think about sequence reuse
3. Consider maintenance implications
4. Remember memory layout

## Validation Challenges

For each exercise, also consider:
1. How would you validate memory usage?
2. How would you verify ROM placement?
3. How would you check alignment?
4. How would you ensure type safety?

## Tools and Techniques

Consider using these tools for validation:
- objdump for memory layout inspection
- size for section size analysis
- nm for symbol table examination
- gdb for runtime verification

## Learning Path Recommendations

1. Complete exercises in order
2. For each exercise:
   - First make it work
   - Then optimize ROM usage
   - Then improve maintainability
   - Finally add validation

3. Common patterns to master:
   - Extern declarations
   - Const correctness
   - Pointer lifetime
   - Structure alignment
   - Link-time resolution

4. Key concepts to understand:
   - Memory sections
   - Linker behavior
   - Initialization order
   - Reference resolution
   - Compilation stages

Remember: The goal is not just to make it work, but to understand why specific patterns are used in embedded systems.
