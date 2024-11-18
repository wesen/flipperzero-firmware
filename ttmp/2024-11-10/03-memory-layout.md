# Memory Layout and Static Initialization Deep Dive
Understanding how static data structures work in embedded systems

## Table of Contents
1. [Memory Regions Overview](#memory-regions)
2. [Compilation Process](#compilation-process)
3. [Why Self-Referential Initialization Fails](#self-referential-failure)
4. [ROM vs RAM Behavior](#rom-vs-ram)
5. [Case Study: Agitation Sequences](#case-study)

## Memory Regions

### Basic Memory Map
```
High Memory
+------------------+
|      Stack      | <- Growing down
|        ↓        |    (dynamic)
|                 |
+------------------+
|       ↑         |
|      Heap       | <- Growing up
|                 |    (dynamic)
+------------------+
|   .bss (RAM)    | <- Uninitialized globals
|                 |    (zeroed at startup)
+------------------+
|   .data (RAM)   | <- Initialized globals
|                 |    (copied from ROM)
+------------------+
|   .rodata (ROM) | <- Const data
|                 |    (read-only)
+------------------+
|   .text (ROM)   | <- Program code
|                 |    (read-only)
Low Memory
```

### Memory Types

1. **ROM (Flash Memory)**
   - Read-only during execution
   - Persists after power off
   - Sections: .text, .rodata
   - Typically slower access
   - Limited write cycles

2. **RAM**
   - Read-write during execution
   - Lost after power off
   - Sections: .data, .bss, heap, stack
   - Fast access
   - Unlimited write cycles

## Compilation Process

### 1. Source Files to Object Files

When compiling this code:
```c
// In source.c
const uint8_t SEQUENCE[] = {1, 2, 3};
uint8_t runtime_data[] = {4, 5, 6};
```

The compiler creates:
```
.section .rodata
SEQUENCE:
    .byte 1
    .byte 2
    .byte 3

.section .data
runtime_data:
    .byte 4
    .byte 5
    .byte 6
```

### 2. Linking Process

The linker:
1. Collects all sections from object files
2. Assigns final memory addresses
3. Resolves references between sections
4. Creates final memory layout

### 3. Startup Code
```c
// Simplified startup pseudocode
void _start(void) {
    // Copy .data from ROM to RAM
    memcpy(&_data_start, &_data_load, data_size);
    
    // Zero .bss section
    memset(&_bss_start, 0, bss_size);
    
    // Call main
    main();
}
```

## Why Self-Referential Initialization Fails

❌ This fails:
```c
const struct Node nodes[] = {
    {.value = 1, .next = &nodes[1]},  // Error!
    {.value = 2, .next = NULL}
};
```

Because during compilation:
1. The compiler doesn't know the final address of `nodes`
2. It can't compute `&nodes[1]` at compile time
3. The initialization needs to be "relocatable"

✅ This works:
```c
// In header:
extern const struct Node NODES[];

// In source:
const struct Node NODES[] = {
    {.value = 1, .next = &NODES[1]}, // OK!
    {.value = 2, .next = NULL}
};
```

Because:
1. The symbol `NODES` is defined before use
2. The linker can resolve the reference
3. The final address is known at link time

## ROM vs RAM Behavior

### ROM (Const) Data
```c
const uint8_t rom_data[] = {1, 2, 3};
```

Memory layout:
```
ROM:
  0x08000000: 01 02 03  // Actual data

Symbol table:
  rom_data = 0x08000000
```

Behavior:
- Data lives in ROM only
- Direct read access
- No RAM used
- Cannot be modified

### RAM Data
```c
uint8_t ram_data[] = {1, 2, 3};
```

Memory layout:
```
ROM (Load region):
  0x08000100: 01 02 03  // Initial values

RAM (Execution region):
  0x20000000: ?? ?? ??  // Before startup
  0x20000000: 01 02 03  // After startup copy
```

Behavior:
- Initial data stored in ROM
- Copied to RAM at startup
- Uses both ROM and RAM
- Can be modified

## Case Study: Agitation Sequences

Let's analyze our agitation sequence structures:

```c
typedef struct {
    AgitationMovementType type;
    union {
        uint32_t duration;
        struct {
            uint32_t count;
            const struct AgitationMovementStatic* sequence;
            size_t sequence_length;
        } loop;
    };
} AgitationMovementStatic;
```

### Memory Layout Example

```c
const AgitationMovementStatic STANDARD_INVERSION[] = {
    {.type = AgitationMovementTypeCW, .duration = 1},
    {.type = AgitationMovementTypePause, .duration = 1},
};

const AgitationMovementStatic COMPLEX_SEQUENCE[] = {
    {.type = AgitationMovementTypeLoop,
     .loop = {
         .count = 4,
         .sequence = STANDARD_INVERSION,
         .sequence_length = 2
     }}
};
```

ROM Layout:
```
0x08000000: [STANDARD_INVERSION]
            type=CW, duration=1
            type=PAUSE, duration=1

0x08000010: [COMPLEX_SEQUENCE]
            type=LOOP
            count=4
            sequence=0x08000000  // Points to STANDARD_INVERSION
            length=2
```

Key points:
1. All data is in ROM
2. Pointers are absolute addresses
3. No RAM usage during execution
4. References resolved at link time

### How Links Are Resolved

1. **Compilation Stage**
```
// Object file contains:
COMPLEX_SEQUENCE:
    .word LOOP_TYPE
    .word 4
    .word STANDARD_INVERSION  // Relocation needed
    .word 2
```

2. **Linking Stage**
```
// Linker:
1. Assigns STANDARD_INVERSION to 0x08000000
2. Updates reference in COMPLEX_SEQUENCE
3. Final binary contains actual address
```

3. **Runtime**
```
// No initialization needed
// Direct ROM access works because:
- All addresses are final
- All data is read-only
- All references are resolved
```

### Optimization Benefits

1. **Space Efficiency**
   - No RAM overhead
   - Shared sequences save ROM space
   - Dense packing in ROM

2. **Performance**
   - No initialization time
   - Direct access
   - No RAM fragmentation

3. **Reliability**
   - Data cannot be corrupted
   - No dynamic allocation
   - Power-safe

## Best Practices Summary

1. Keep all sequence data const
2. Define base sequences before dependent ones
3. Use extern declarations in headers
4. Keep length constants with arrays
5. Understand ROM vs RAM tradeoffs
6. Let the linker resolve references
7. Use appropriate section attributes

This organization ensures optimal memory usage, reliable execution, and maintainable code for embedded systems like the Flipper Zero.