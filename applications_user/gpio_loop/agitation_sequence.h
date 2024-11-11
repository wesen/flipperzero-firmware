#pragma once

#ifndef __cplusplus
// Add bool, true, false definitions for C
#ifndef __bool_true_false_are_defined
typedef enum {
    false = 0,
    true = 1
} bool;
#define __bool_true_false_are_defined 1
#endif
#endif
// #include <furi.h>
// #include <furi_hal_gpio.h>
typedef char FuriString;

#include <stddef.h>
#include <inttypes.h>

/**
 * @brief Movement types for agitation sequence
 */
typedef enum {
    AgitationMovementTypeCW, // Clockwise movement
    AgitationMovementTypeCCW, // Counter-clockwise movement
    AgitationMovementTypePause, // Pause/wait
    AgitationMovementTypeLoop, // Repeating sequence
} AgitationMovementType;

//------------------------------------------------------------------------------
// Static (const) versions of structures
//------------------------------------------------------------------------------

typedef struct AgitationMovementStatic AgitationMovementStatic;
typedef struct AgitationStepStatic AgitationStepStatic;

/**
 * @brief Static version of movement
 * For loops, duration is ignored. For other types, count and sequence are ignored.
 */
struct AgitationMovementStatic {
    AgitationMovementType type;
    union {
        // For regular movements
        uint32_t duration;
        // For loops
        struct {
            uint32_t count; // 0 = infinite
            const AgitationMovementStatic* sequence;
            size_t sequence_length;
        } loop;
    };
};

/**
 * @brief Static version of step
 */
struct AgitationStepStatic {
    const char* name;
    const char* description;
    float temperature;
    const AgitationMovementStatic* sequence;
    size_t sequence_length;
};

/**
 * @brief Static version of process
 */
typedef struct {
    const char* process_name;
    const char* film_type;
    const char* tank_type;
    const char* chemistry;
    float temperature;
    const AgitationStepStatic* steps;
    size_t steps_length;
} AgitationProcessStatic;

//------------------------------------------------------------------------------
// Dynamic versions of structures (with FuriString)
//------------------------------------------------------------------------------

/**
 * @brief Dynamic version of movement
 */
typedef struct {
    AgitationMovementType type;
    union {
        // For regular movements
        uint32_t duration;
        // For loops
        struct {
            uint32_t count;
            struct AgitationMovement* sequence;
            size_t sequence_length;
        } loop;
    };
} AgitationMovement;

/**
 * @brief Dynamic version of step
 */
typedef struct {
    FuriString* name;
    FuriString* description;
    float temperature;
    AgitationMovement* sequence;
    size_t sequence_length;
} AgitationStep;

/**
 * @brief Dynamic version of process
 */
typedef struct {
    FuriString* process_name;
    FuriString* film_type;
    FuriString* tank_type;
    FuriString* chemistry;
    float temperature;
    AgitationStep* steps;
    size_t steps_length;
} AgitationProcess;

//------------------------------------------------------------------------------
// Example of static initialization
//------------------------------------------------------------------------------

/**
 * @brief Standard inversion sequence
 */
#define AGITATION_STANDARD_INVERSION                         \
    {                                                        \
        {.type = AgitationMovementTypeCW, .duration = 1},    \
        {.type = AgitationMovementTypePause, .duration = 1}, \
        {.type = AgitationMovementTypeCCW, .duration = 1},   \
        {.type = AgitationMovementTypePause, .duration = 1}, \
    }

/**
 * @brief Initial agitation sequence with loop
 */
#define AGITATION_INITIAL_SEQUENCE                                         \
    {                                                                      \
        {.type = AgitationMovementTypeLoop,                                \
         .loop =                                                           \
             {.count = 4,                                                  \
              .sequence =                                                  \
                  (AgitationMovementStatic[]){                             \
                      {.type = AgitationMovementTypeCW, .duration = 1},    \
                      {.type = AgitationMovementTypePause, .duration = 1}, \
                      {.type = AgitationMovementTypeCCW, .duration = 1},   \
                      {.type = AgitationMovementTypePause, .duration = 1}, \
                  },                                                       \
              .sequence_length = 4}},                                      \
        {.type = AgitationMovementTypePause, .duration = 24},              \
    }

//------------------------------------------------------------------------------
// Conversion functions
//------------------------------------------------------------------------------

/**
 * @brief Convert static process to dynamic
 * @param static_process Static process definition
 * @return AgitationProcess* Newly allocated dynamic process
 */
AgitationProcess* agitation_process_from_static(const AgitationProcessStatic* static_process);

/**
 * @brief Convert static step to dynamic
 * @param static_step Static step definition
 * @return AgitationStep* Newly allocated dynamic step
 */
AgitationStep* agitation_step_from_static(const AgitationStepStatic* static_step);

//------------------------------------------------------------------------------
// Original dynamic API functions
//------------------------------------------------------------------------------

AgitationProcess* agitation_process_alloc();
void agitation_process_free(AgitationProcess* process);
AgitationStep* agitation_step_alloc();
void agitation_step_free(AgitationStep* step);

void agitation_sequence_add_movement(
    AgitationMovement** sequence,
    size_t* length,
    AgitationMovementType type,
    uint32_t duration);

uint32_t agitation_sequence_get_duration(AgitationMovement* sequence, size_t length);
bool agitation_sequence_validate(AgitationMovement* sequence, size_t length);
AgitationProcess* agitation_process_from_yaml(const char* yaml_content);
FuriString* agitation_process_to_yaml(AgitationProcess* process);
