#include <inttypes.h>
#include <stdio.h>
typedef enum {
    MOVEMENT_CW,      // Clockwise
    MOVEMENT_CCW,     // Counter-clockwise
    MOVEMENT_PAUSE,   // Wait
    MOVEMENT_LOOP     // Repeat sequence
} MovementType;

typedef struct {
    MovementType type;
    union {
        uint32_t duration;  // For simple movements
        struct {
            uint32_t repeat_count;
            const struct MovementSequence* sequence;
            size_t sequence_length;
        } loop;  // For complex movements
    } data;
} MovementStep;

// Base movement sequences
static const MovementStep STANDARD_INVERSION[] = {
    { .type = MOVEMENT_CW, .data.duration = 1 },
    { .type = MOVEMENT_PAUSE, .data.duration = 1 },
    { .type = MOVEMENT_CCW, .data.duration = 1 },
    { .type = MOVEMENT_PAUSE, .data.duration = 1 }
};

// Composite sequence using base sequence
static const MovementStep ILFORD_STANDARD[] = {
    { 
        .type = MOVEMENT_LOOP, 
        .data.loop = {
            .repeat_count = 4,
            .sequence = (const struct MovementSequence *)STANDARD_INVERSION,
            .sequence_length = 4
        }
    },
    { .type = MOVEMENT_PAUSE, .data.duration = 30 }
};

int main() {
    print_memory_layout();
}

void print_memory_layout() {
    printf("Memory Layout Information:\n");
    
    // Print addresses of enum and type definitions
    printf("Enum MovementType Address: %p\n", (void*)&(MovementType){0});
    
    // Print addresses of base sequences
    printf("STANDARD_INVERSION Sequence Address: %p\n", (void*)STANDARD_INVERSION);
    printf("ILFORD_STANDARD Sequence Address: %p\n", (void*)ILFORD_STANDARD);
    
    // Print sizes of key structures
    printf("\nStructure Sizes:\n");
    printf("MovementStep Size: %zu bytes\n", sizeof(MovementStep));
    printf("MovementType Size: %zu bytes\n", sizeof(MovementType));
    
    // Print detailed addresses of individual steps in sequences
    printf("\nSTANDARD_INVERSION Step Addresses:\n");
    for (size_t i = 0; i < sizeof(STANDARD_INVERSION)/sizeof(STANDARD_INVERSION[0]); i++) {
        printf("Step %zu: %p (Type: %d, Duration: %u)\n", 
               i, 
               (void*)&STANDARD_INVERSION[i], 
               STANDARD_INVERSION[i].type, 
               STANDARD_INVERSION[i].data.duration);
    }
    
    printf("\nILFORD_STANDARD Step Addresses:\n");
    for (size_t i = 0; i < sizeof(ILFORD_STANDARD)/sizeof(ILFORD_STANDARD[0]); i++) {
        printf("Step %zu: %p (Type: %d)\n", 
               i, 
               (void*)&ILFORD_STANDARD[i], 
               ILFORD_STANDARD[i].type);
    }
}