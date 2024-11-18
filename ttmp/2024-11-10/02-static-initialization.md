# Static Initialization Patterns in Embedded C

A comprehensive guide to defining constant data structures in firmware

## Table of Contents

1. [Basic Static Initialization](#basic-static-initialization)
2. [Complex Data Structures](#complex-data-structures)
3. [Self-Referential Structures](#self-referential-structures)
4. [Array of Structures](#array-of-structures)
5. [Applying to Agitation Sequences](#applying-to-agitation-sequences)

## Basic Static Initialization

### Simple Constants

The simplest form of static initialization is with basic types:

```c
// In header (.h)
extern const int MAX_RETRY_COUNT;
extern const char* DEFAULT_NAME;

// In source (.c)
const int MAX_RETRY_COUNT = 3;
const char* DEFAULT_NAME = "Device";
```

### Basic Arrays

Static arrays follow a similar pattern:

```c
// In header (.h)
extern const uint8_t SEQUENCE_BYTES[];
extern const size_t SEQUENCE_BYTES_LENGTH;

// In source (.c)
const uint8_t SEQUENCE_BYTES[] = {0x01, 0x02, 0x03, 0x04};
const size_t SEQUENCE_BYTES_LENGTH = 4;
```

## Complex Data Structures

When working with structures, we need to be more careful:

```c
// Structure definition (in header)
typedef struct {
    uint8_t id;
    const char* name;
    uint32_t timeout;
} DeviceConfig;

// In header (.h)
extern const DeviceConfig DEFAULT_CONFIG;

// In source (.c)
const DeviceConfig DEFAULT_CONFIG = {
    .id = 1,
    .name = "Default",
    .timeout = 1000
};
```

### Multiple Instances

When you need multiple instances:

```c
// In header (.h)
extern const DeviceConfig DEVICE_CONFIGS[];
extern const size_t DEVICE_CONFIGS_COUNT;

// In source (.c)
const DeviceConfig DEVICE_CONFIGS[] = {
    {
        .id = 1,
        .name = "Device1",
        .timeout = 1000
    },
    {
        .id = 2,
        .name = "Device2",
        .timeout = 2000
    }
};
const size_t DEVICE_CONFIGS_COUNT = 2;
```

## Self-Referential Structures

This is where things get tricky. Consider a node structure that points to other nodes:

```c
typedef struct Node {
    int value;
    const struct Node* next;
} Node;
```

❌ This won't work:

```c
// DON'T DO THIS - Compiler error!
const Node nodes[] = {
    {.value = 1, .next = &nodes[1]},
    {.value = 2, .next = &nodes[2]},
    {.value = 3, .next = NULL}
};
```

✅ Instead, do this:

```c
// In header (.h)
extern const Node NODES[];

// In source (.c)
const Node NODES[] = {
    {.value = 1, .next = &NODES[1]},
    {.value = 2, .next = &NODES[2]},
    {.value = 3, .next = NULL}
};
```

## Array of Structures

When dealing with arrays of structures that reference other arrays:

```c
typedef struct {
    const char* name;
    const uint8_t* data;
    size_t data_length;
} DataBlock;

// In header (.h)
extern const uint8_t BLOCK1_DATA[];
extern const uint8_t BLOCK2_DATA[];
extern const DataBlock DATA_BLOCKS[];

// In source (.c)
const uint8_t BLOCK1_DATA[] = {0x01, 0x02, 0x03};
const uint8_t BLOCK2_DATA[] = {0x04, 0x05, 0x06};

const DataBlock DATA_BLOCKS[] = {
    {
        .name = "Block1",
        .data = BLOCK1_DATA,
        .data_length = 3
    },
    {
        .name = "Block2",
        .data = BLOCK2_DATA,
        .data_length = 3
    }
};
```

## Applying to Agitation Sequences

Now let's apply these patterns to our agitation sequence problem:

1. First, split the definitions across header and source files:

```c
// agitation_sequence.h
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

2. Declare the sequences in the header:

```c
// agitation_processes.h
extern const AgitationMovementStatic STANDARD_INVERSION[];
extern const size_t STANDARD_INVERSION_LENGTH;

extern const AgitationMovementStatic CONTINUOUS_GENTLE_SEQ[];
extern const size_t CONTINUOUS_GENTLE_SEQ_LENGTH;

extern const AgitationMovementStatic CONTINUOUS_GENTLE[];
extern const size_t CONTINUOUS_GENTLE_LENGTH;
```

3. Define the sequences in the source file:

```c
// agitation_processes.c
const AgitationMovementStatic STANDARD_INVERSION[] = {
    {.type = AgitationMovementTypeCW, .duration = 1},
    {.type = AgitationMovementTypePause, .duration = 1},
    {.type = AgitationMovementTypeCCW, .duration = 1},
    {.type = AgitationMovementTypePause, .duration = 1},
};
const size_t STANDARD_INVERSION_LENGTH = 4;

const AgitationMovementStatic CONTINUOUS_GENTLE_SEQ[] = {
    {.type = AgitationMovementTypeCW, .duration = 2},
    {.type = AgitationMovementTypePause, .duration = 1},
    {.type = AgitationMovementTypeCCW, .duration = 2},
    {.type = AgitationMovementTypePause, .duration = 1},
};
const size_t CONTINUOUS_GENTLE_SEQ_LENGTH = 4;

// Now we can reference other sequences
const AgitationMovementStatic CONTINUOUS_GENTLE[] = {
    {.type = AgitationMovementTypeLoop,
     .loop = {
         .count = 0,
         .sequence = CONTINUOUS_GENTLE_SEQ,
         .sequence_length = CONTINUOUS_GENTLE_SEQ_LENGTH
     }}
};
const size_t CONTINUOUS_GENTLE_LENGTH = 1;
```

4. Building more complex sequences:

```c
const AgitationMovementStatic ILFORD_STANDARD[] = {
    {.type = AgitationMovementTypeLoop,
     .loop = {
         .count = 4,
         .sequence = STANDARD_INVERSION,
         .sequence_length = STANDARD_INVERSION_LENGTH
     }},
    {.type = AgitationMovementTypePause, .duration = 30},
};
const size_t ILFORD_STANDARD_LENGTH = 2;
```

### Key Points:

1. Always declare arrays as extern in headers
2. Keep the length constant with its array
3. Define base sequences before sequences that reference them
4. Use const consistently to keep data in ROM
5. Follow the pattern: declare in header, define in source
6. Remember that sequence references must point to already-defined sequences

### Memory Layout

This pattern ensures:

- All sequences are stored in read-only memory
- No runtime initialization is required
- Minimal RAM usage (everything is in ROM)
- Proper linking and initialization order

### Usage Example

```c
void process_sequence(const AgitationMovementStatic* sequence, size_t length) {
    for(size_t i = 0; i < length; i++) {
        if(sequence[i].type == AgitationMovementTypeLoop) {
            process_sequence(
                sequence[i].loop.sequence,
                sequence[i].loop.sequence_length
            );
        } else {
            // Process single movement
        }
    }
}

// Use it
process_sequence(ILFORD_STANDARD, ILFORD_STANDARD_LENGTH);
```

This approach gives you a maintainable, efficient, and correct way to define complex sequences in firmware while keeping everything in read-only memory and avoiding initialization issues.
