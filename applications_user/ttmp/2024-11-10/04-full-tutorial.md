# Understanding Static Data Structures in Embedded Systems

## Foundation and Core Concepts

## Memory Architecture in Embedded Systems

Modern embedded systems typically contain two fundamentally different types of memory: ROM (usually Flash memory) and RAM. Understanding the characteristics and constraints of these memory types is crucial for making informed decisions about data structure design. ROM, while persistent and space-efficient, can only be written to a limited number of times and is relatively slow to access. RAM is fast and freely modifiable but volatile and typically much more limited in embedded systems. This division creates a fundamental tension in our design decisions.

When your program starts executing, it's organized into several distinct sections, each with different properties and purposes:

```
Memory Map:
+------------------+
|      Stack      |  <- Dynamic, grows down
|        ↓        |     Automatic variables
|                 |     Function calls
+------------------+
|       ↑         |  <- Dynamic, grows up
|      Heap       |     Allocated at runtime
|                 |     (malloc/free)
+------------------+
|      .bss       |  <- Zero-initialized data
|                 |     Global/static variables
+------------------+
|      .data      |  <- Initialized data
|                 |     Global/static variables
+------------------+
|    .rodata      |  <- Constants and literals
|                 |     String constants
+------------------+
|      .text      |  <- Program code
|                 |     Constants
Low Memory
```

Understanding this layout is crucial because it affects both compile-time and runtime behavior of your program. The `.text` and `.rodata` sections reside in ROM (Flash memory), while `.data`, `.bss`, heap, and stack live in RAM. The `.data` section is special because it requires space in both ROM (to store the initial values) and RAM (for the actual variables during execution).

## Basic Principles of Static Data

When we talk about "static data" in embedded systems, we're usually referring to data that:

1. Has a fixed size known at compile time
2. Has a fixed location in memory
3. Exists for the entire program lifetime
4. Preferably lives entirely in ROM

Here's a simple example that illustrates these principles:

```c
// In header (.h)
extern const uint8_t PROTOCOL_VERSION;
extern const char* DEVICE_NAME;
extern const uint32_t SUPPORTED_FEATURES[];
extern const size_t SUPPORTED_FEATURES_COUNT;

// In source (.c)
const uint8_t PROTOCOL_VERSION = 1;
const char* DEVICE_NAME = "Device-1";
const uint32_t SUPPORTED_FEATURES[] = {
    0x00000001,  // Feature 1
    0x00000002,  // Feature 2
    0x00000004   // Feature 3
};
const size_t SUPPORTED_FEATURES_COUNT =
    sizeof(SUPPORTED_FEATURES) / sizeof(SUPPORTED_FEATURES[0]);
```

This seemingly simple code actually embodies several important principles. Let's examine why each decision matters:

### Declaration vs Definition Separation

The separation between header and source files isn't just about C language conventions - it serves crucial purposes in embedded systems:

1. **Memory Efficiency**: By declaring variables as `extern` in the header, we ensure that only one copy of the data exists in ROM, regardless of how many source files include the header.

2. **Link-Time Flexibility**: The linker can place the actual data wherever it's most efficient in ROM, while maintaining correct references from all parts of the program.

3. **Encapsulation**: Other modules can reference the data without needing to know its exact values or internal organization.

### Size Management

Notice how we handle array sizes:

```c
extern const size_t SUPPORTED_FEATURES_COUNT;
const size_t SUPPORTED_FEATURES_COUNT =
    sizeof(SUPPORTED_FEATURES) / sizeof(SUPPORTED_FEATURES[0]);
```

This pattern is vastly superior to using `#define` for several reasons:

1. **Type Safety**: The size is a proper variable with a defined type, not a preprocessor macro.
2. **Automatic Updates**: The size automatically stays in sync with the array because it's computed from the actual data.
3. **ROM Residency**: Both the array and its size live in ROM, preventing any possibility of corruption.
4. **Debug Friendly**: Being a real variable makes it visible to debuggers and can be examined at runtime.

### String Constants

The string handling deserves special attention:

```c
const char* DEVICE_NAME = "Device-1";
```

This creates two distinct pieces in ROM:

1. The string literal "Device-1\0" (including null terminator)
2. A pointer to that string

The memory layout looks like this:

```
ROM:
0x0800_0000: 44 65 76 69 63 65 2D 31 00  // "Device-1\0"
0x0800_0009: 00 00 08 00                 // Pointer (0x08000000)
```

This organization provides several benefits:

1. The string can't be accidentally modified (it's in ROM)
2. The pointer itself is also immutable (it's a const pointer in ROM)
3. Multiple references to the same string can share the same storage
4. String access is efficient (just one pointer dereference)

# Complex Data Structures and Memory Management

## Structured Data in ROM

When working with structures in embedded systems, we need to carefully consider how each field affects memory layout and runtime behavior. Let's examine a real-world example:

```c
typedef struct {
    uint8_t id;
    const char* name;
    uint32_t capabilities;
    const uint8_t* calibration_data;
    size_t calibration_size;
} DeviceConfig;
```

This structure seems straightforward, but it embodies several important design decisions. Let's create a static instance:

```c
// In source file
static const uint8_t DEVICE_1_CALIBRATION[] = {
    0x01, 0x02, 0x03, 0x04  // Calibration values
};

const DeviceConfig DEFAULT_DEVICE = {
    .id = 1,
    .name = "Primary Device",
    .capabilities = 0x00000001,
    .calibration_data = DEVICE_1_CALIBRATION,
    .calibration_size = sizeof(DEVICE_1_CALIBRATION)
};
```

### Memory Layout Analysis

This single structure instance creates a complex but efficient memory layout:

```
ROM Layout:
0x0800_0000: "Primary Device\0"    // String literal
0x0800_000E: 01 02 03 04          // DEVICE_1_CALIBRATION array

0x0800_0012: // DEFAULT_DEVICE structure
             01                    // id (1 byte)
             [pad 3]              // Alignment padding
             0x0800_0000          // name pointer
             01 00 00 00          // capabilities
             0x0800_000E          // calibration_data pointer
             04 00 00 00          // calibration_size
```

Several important points about this layout:

1. **Alignment**: The compiler adds padding after `id` to ensure proper alignment of the pointers that follow. This is crucial for both performance and correctness on many architectures.

2. **Pointer Resolution**: All pointers in the structure are resolved by the linker to absolute ROM addresses. This means no runtime relocation is needed.

3. **Data Sharing**: The string and calibration data can be shared by multiple structure instances if needed, saving ROM space.

4. **Access Efficiency**: Despite being in ROM, access to this data is efficient because:
   - The structure layout is fixed and known at compile time
   - All pointers are pre-resolved
   - No runtime initialization is required

## Self-Referential Structures

One of the most challenging aspects of static data in embedded systems is handling self-referential structures. Consider this common scenario:

```c
typedef struct Node {
    uint32_t value;
    const struct Node* next;
} Node;
```

### The Wrong Way

Many programmers initially try something like this:

```c
// This will NOT compile
const Node LINKED_LIST[] = {
    {.value = 1, .next = &LINKED_LIST[1]},  // Error!
    {.value = 2, .next = &LINKED_LIST[2]},  // Error!
    {.value = 3, .next = NULL}
};
```

This fails because we're asking the compiler to do something impossible: use the address of an array element before the array itself has an address. The compilation process can't resolve these self-references because:

1. At compile time, the final address of `LINKED_LIST` isn't known yet
2. The relative offsets between elements aren't enough - we need absolute addresses
3. The compiler needs all values to be known at compile time when initializing const data

### The Correct Approach

Here's how to properly create a self-referential static structure:

```c
// In header
extern const Node LINKED_LIST[];

// In source
const Node LINKED_LIST[] = {
    {.value = 1, .next = &LINKED_LIST[1]},  // Works!
    {.value = 2, .next = &LINKED_LIST[2]},  // Works!
    {.value = 3, .next = NULL}
};
```

This works because:

1. The declaration and definition are separated
2. The linker (not the compiler) resolves the addresses
3. When the linker runs, it:
   - Allocates space for the entire array
   - Knows the final address of each element
   - Can fill in the correct pointer values

The resulting memory layout might look like:

```
ROM:
0x0800_0000: // First node
             01 00 00 00          // value = 1
             0x0800_0008          // next -> points to second node

0x0800_0008: // Second node
             02 00 00 00          // value = 2
             0x0800_0010          // next -> points to third node

0x0800_0010: // Third node
             03 00 00 00          // value = 3
             00 00 00 00          // next = NULL
```

The key insight is that the linker can do what the compiler cannot: resolve addresses across the entire program after all locations are finalized.

This pattern becomes especially important when dealing with complex data structures like our agitation sequences, where we need to reference other sequences within a sequence definition.

# Advanced Patterns and Application to Real Systems

## Nested and Hierarchical Data Structures

When working with complex embedded systems, we often need to represent hierarchical or nested data structures that live entirely in ROM. This presents unique challenges, especially when these structures need to reference each other. Let's examine how to build these correctly.

### Composite Pattern Example

Consider a system that needs to represent a menu structure:

```c
typedef struct {
    const char* label;
    uint8_t command_id;
} MenuItem;

typedef struct {
    const char* title;
    const MenuItem* items;
    size_t items_count;
    const struct MenuPage* subpages;  // Self-referential!
    size_t subpages_count;
} MenuPage;
```

To implement this correctly, we need to carefully consider the order of declarations and definitions:

```c
// In header file
extern const MenuItem SYSTEM_ITEMS[];
extern const size_t SYSTEM_ITEMS_COUNT;
extern const MenuPage SYSTEM_PAGES[];
extern const size_t SYSTEM_PAGES_COUNT;

// In source file
// First, define leaf items that don't depend on anything else
static const MenuItem SETTINGS_ITEMS[] = {
    {.label = "Brightness", .command_id = 1},
    {.label = "Volume", .command_id = 2}
};
static const size_t SETTINGS_ITEMS_COUNT = 2;

static const MenuItem MAIN_ITEMS[] = {
    {.label = "Start", .command_id = 3},
    {.label = "Stop", .command_id = 4}
};
static const size_t MAIN_ITEMS_COUNT = 2;

// Now we can define pages that reference the items
const MenuPage SYSTEM_PAGES[] = {
    {  // Main page
        .title = "Main Menu",
        .items = MAIN_ITEMS,
        .items_count = MAIN_ITEMS_COUNT,
        .subpages = &SYSTEM_PAGES[1],  // Points to Settings page
        .subpages_count = 1
    },
    {  // Settings page
        .title = "Settings",
        .items = SETTINGS_ITEMS,
        .items_count = SETTINGS_ITEMS_COUNT,
        .subpages = NULL,
        .subpages_count = 0
    }
};
const size_t SYSTEM_PAGES_COUNT = 2;
```

This structure creates a complex but efficient ROM layout:

```
ROM Layout:
// String literals
0x0800_0000: "Brightness\0"
0x0800_000A: "Volume\0"
0x0800_0011: "Start\0"
0x0800_0017: "Stop\0"
0x0800_001C: "Main Menu\0"
0x0800_0026: "Settings\0"

// SETTINGS_ITEMS array
0x0800_0030: ptr to "Brightness"   // label
0x0800_0034: 01                    // command_id
0x0800_0035: [3 bytes padding]
0x0800_0038: ptr to "Volume"       // label
0x0800_003C: 02                    // command_id
0x0800_003D: [3 bytes padding]

// MAIN_ITEMS array
0x0800_0040: ptr to "Start"        // label
0x0800_0044: 03                    // command_id
0x0800_0045: [3 bytes padding]
0x0800_0048: ptr to "Stop"         // label
0x0800_004C: 04                    // command_id
0x0800_004D: [3 bytes padding]

// SYSTEM_PAGES array
0x0800_0050: ptr to "Main Menu"    // title
0x0800_0054: ptr to MAIN_ITEMS     // items
0x0800_0058: 02 00 00 00          // items_count
0x0800_005C: ptr to &SYSTEM_PAGES[1] // subpages
0x0800_0060: 01 00 00 00          // subpages_count

0x0800_0064: ptr to "Settings"     // title
0x0800_0068: ptr to SETTINGS_ITEMS // items
0x0800_006C: 02 00 00 00          // items_count
0x0800_0070: 00 00 00 00          // subpages (NULL)
0x0800_0074: 00 00 00 00          // subpages_count
```

## Application to Agitation Sequences

Now let's apply these principles to our agitation sequence problem. The key challenge is representing sequences that can contain both simple movements and loops of other sequences.

### Basic Movement Structure

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

This structure is complex because:

1. It uses a union to efficiently represent different types of movements
2. It needs to reference other sequences when used as a loop
3. All data must reside in ROM

### Building Basic Sequences

First, we define our fundamental sequences:

```c
// In header
extern const AgitationMovementStatic STANDARD_INVERSION[];
extern const size_t STANDARD_INVERSION_LENGTH;

// In source
const AgitationMovementStatic STANDARD_INVERSION[] = {
    {.type = AgitationMovementTypeCW, .duration = 1},
    {.type = AgitationMovementTypePause, .duration = 1},
    {.type = AgitationMovementTypeCCW, .duration = 1},
    {.type = AgitationMovementTypePause, .duration = 1},
};
const size_t STANDARD_INVERSION_LENGTH = 4;
```

This base sequence is straightforward because it doesn't reference any other sequences. However, when we want to create more complex sequences that use loops, we need to be careful:

```c
// In header
extern const AgitationMovementStatic ILFORD_STANDARD[];
extern const size_t ILFORD_STANDARD_LENGTH;

// In source
const AgitationMovementStatic ILFORD_STANDARD[] = {
    {
        .type = AgitationMovementTypeLoop,
        .loop = {
            .count = 4,
            .sequence = STANDARD_INVERSION,     // References previous sequence
            .sequence_length = STANDARD_INVERSION_LENGTH
        }
    },
    {
        .type = AgitationMovementTypePause,
        .duration = 30
    }
};
const size_t ILFORD_STANDARD_LENGTH = 2;
```

The resulting memory layout is quite sophisticated:

```
ROM Layout:
// STANDARD_INVERSION sequence
0x0800_0000: TYPE_CW, 1           // First movement
0x0800_0008: TYPE_PAUSE, 1        // Second movement
0x0800_0010: TYPE_CCW, 1          // Third movement
0x0800_0018: TYPE_PAUSE, 1        // Fourth movement
0x0800_0020: 04 00 00 00         // STANDARD_INVERSION_LENGTH

// ILFORD_STANDARD sequence
0x0800_0024: TYPE_LOOP            // First movement type
0x0800_0025: 04 00 00 00         // loop.count
0x0800_0029: 0x0800_0000         // loop.sequence (points to STANDARD_INVERSION)
0x0800_002D: 04 00 00 00         // loop.sequence_length
0x0800_0031: TYPE_PAUSE          // Second movement type
0x0800_0032: 1E 00 00 00         // duration (30)
0x0800_0036: 02 00 00 00         // ILFORD_STANDARD_LENGTH
```

### Key Implementation Points

1. **Sequence Dependencies**: Always define base sequences before sequences that reference them. This ensures all references can be resolved by the linker.

2. **Memory Efficiency**: By sharing common sequences (like STANDARD_INVERSION), we save ROM space and make the code more maintainable.

3. **Runtime Access**: Despite the complex structure, accessing the sequences at runtime is efficient because:

   - All addresses are resolved at link time
   - No dynamic memory allocation is needed
   - The structure is read-only, preventing corruption

4. **Type Safety**: The union structure ensures type-safe access to the different kinds of movements, while still keeping everything in ROM.

This pattern allows us to represent complex agitation sequences efficiently while ensuring they are:

- Immutable (stored in ROM)
- Memory efficient (sharing common subsequences)
- Fast to access
- Safe from corruption
- Easy to maintain and modify
