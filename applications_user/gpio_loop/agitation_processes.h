#pragma once

typedef char FuriString;
#include "agitation_sequence.h"

//------------------------------------------------------------------------------
// Common Base Sequences
//------------------------------------------------------------------------------

/**
 * @brief Basic inversion sequence (CW -> Pause -> CCW -> Pause)
 */
static const AgitationMovementStatic STANDARD_INVERSION[] = {
    {.type = AgitationMovementTypeCW, .duration = 1},
    {.type = AgitationMovementTypePause, .duration = 1},
    {.type = AgitationMovementTypeCCW, .duration = 1},
    {.type = AgitationMovementTypePause, .duration = 1},
};
static const size_t STANDARD_INVERSION_LENGTH = 4;

/**
 * @brief Gentle continuous base sequence
 */
static const AgitationMovementStatic CONTINUOUS_GENTLE_SEQ[] = {
    {.type = AgitationMovementTypeCW, .duration = 2},
    {.type = AgitationMovementTypePause, .duration = 1},
    {.type = AgitationMovementTypeCCW, .duration = 2},
    {.type = AgitationMovementTypePause, .duration = 1},
};
static const size_t CONTINUOUS_GENTLE_SEQ_LENGTH = 4;

//------------------------------------------------------------------------------
// Common Development Patterns
//------------------------------------------------------------------------------

/**
 * @brief Continuous gentle agitation (for C41/E6)
 */
static const AgitationMovementStatic CONTINUOUS_GENTLE[] = {
    {.type = AgitationMovementTypeLoop,
     .loop = {
         .count = 0, // Continuous
         .sequence = (const struct AgitationMovementStatic*)CONTINUOUS_GENTLE_SEQ,
         .sequence_length = CONTINUOUS_GENTLE_SEQ_LENGTH}}};
static const size_t CONTINUOUS_GENTLE_LENGTH = 1;

/**
 * @brief Ilford standard agitation (4 inversions + 30s pause)
 */
static const AgitationMovementStatic ILFORD_STANDARD[] = {
    {.type = AgitationMovementTypeLoop,
     .loop =
         {.count = 4,
          .sequence = (const struct     AgitationMovementStatic*)STANDARD_INVERSION,
          .sequence_length = STANDARD_INVERSION_LENGTH}},
    {.type = AgitationMovementTypePause, .duration = 30},
};
static const size_t ILFORD_STANDARD_LENGTH = 2;

/**
 * @brief Initial agitation sequence (4 inversions + 24s pause)
 */
static const AgitationMovementStatic INITIAL_AGITATION[] = {
    {.type = AgitationMovementTypeLoop,
     .loop =
         {.count = 4,
          .sequence = (const struct AgitationMovementStatic*)STANDARD_INVERSION,
          .sequence_length = STANDARD_INVERSION_LENGTH}},
    {.type = AgitationMovementTypePause, .duration = 24},
};
static const size_t INITIAL_AGITATION_LENGTH = 2;

//------------------------------------------------------------------------------
// B&W Development Sequences as Steps
//------------------------------------------------------------------------------

/**
 * @brief Standard B&W Initial Agitation Step
 */
static const AgitationStepStatic BW_INITIAL_AGITATION_STEP = {
    .name = "Initial Agitation",
    .description = "First round of agitation to ensure even development",
    .temperature = 20.0f, // Typical B&W development temperature
    .sequence = INITIAL_AGITATION,
    .sequence_length = INITIAL_AGITATION_LENGTH
};

/**
 * @brief Standard B&W Periodic Agitation Step
 */
static const AgitationStepStatic BW_PERIODIC_AGITATION_STEP = {
    .name = "Periodic Agitation",
    .description = "Continued agitation during development",
    .temperature = 20.0f, // Maintain consistent temperature
    .sequence = (const AgitationMovementStatic[]){
        {.type = AgitationMovementTypeLoop,
         .loop = {
             .count = 2,
             .sequence = (const struct AgitationMovementStatic*)STANDARD_INVERSION,
             .sequence_length = STANDARD_INVERSION_LENGTH
         }}
    },
    .sequence_length = 1
};

/**
 * @brief Standard B&W Development Process
 */
static const AgitationProcessStatic BW_STANDARD_DEV_STATIC = {
    .process_name = "Black and White Standard Development",
    .film_type = "Black and White Negative",
    .tank_type = "Developing Tank",
    .chemistry = "B&W Developer",
    .temperature = 20.0f,
    .steps = (const AgitationStepStatic[]){
        BW_INITIAL_AGITATION_STEP,
        BW_PERIODIC_AGITATION_STEP
    },
    .steps_length = 2
};

/**
 * @brief Stand Development Initial Agitation Step
 */
static const AgitationStepStatic STAND_DEV_INITIAL_STEP = {
    .name = "Initial Agitation",
    .description = "Initial agitation before long stand period",
    .temperature = 20.0f, // Typical B&W development temperature
    .sequence = (const AgitationMovementStatic[]){
        {.type = AgitationMovementTypeLoop,
         .loop = {
             .count = 3,
             .sequence = (const struct AgitationMovementStatic*)STANDARD_INVERSION,
             .sequence_length = STANDARD_INVERSION_LENGTH
         }}
    },
    .sequence_length = 1
};

/**
 * @brief Stand Development Long Stand Step
 */
static const AgitationStepStatic STAND_DEV_LONG_STAND_STEP = {
    .name = "Long Stand",
    .description = "Extended period with minimal agitation",
    .temperature = 20.0f,
    .sequence = (const AgitationMovementStatic[]){
        {.type = AgitationMovementTypePause, .duration = 3600} // 1 hour stand
    },
    .sequence_length = 1
};

/**
 * @brief Stand Development Process
 */
static const AgitationProcessStatic STAND_DEV_STATIC = {
    .process_name = "Black and White Stand Development",
    .film_type = "Black and White Negative",
    .tank_type = "Developing Tank",
    .chemistry = "B&W Developer",
    .temperature = 20.0f,
    .steps = (const AgitationStepStatic[]){
        STAND_DEV_INITIAL_STEP,
        STAND_DEV_LONG_STAND_STEP
    },
    .steps_length = 2
};

//------------------------------------------------------------------------------
// Continuous Gentle Agitation Process
//------------------------------------------------------------------------------

/**
 * @brief Continuous Gentle Agitation Step
 */
static const AgitationStepStatic CONTINUOUS_GENTLE_STEP = {
    .name = "Continuous Gentle Agitation",
    .description = "Gentle, continuous movement for consistent development",
    .temperature = 38.0f, // Typical color development temperature
    .sequence = CONTINUOUS_GENTLE,
    .sequence_length = CONTINUOUS_GENTLE_LENGTH
};

/**
 * @brief Continuous Gentle Agitation Process
 */
static const AgitationProcessStatic CONTINUOUS_GENTLE_STATIC = {
    .process_name = "Continuous Gentle Agitation",
    .film_type = "Various",
    .tank_type = "Developing Tank",
    .chemistry = "Various",
    .temperature = 38.0f,
    .steps = (const AgitationStepStatic[]){
        CONTINUOUS_GENTLE_STEP
    },
    .steps_length = 1
};

//------------------------------------------------------------------------------
// Color Development Sequences
//------------------------------------------------------------------------------

/**
 * @brief C41 step sequence (continuous gentle agitation)
 */
static const AgitationMovementStatic C41_STEP[] = {
    {.type = AgitationMovementTypeLoop,
     .loop = {
         .count = 0, // Continuous
         .sequence = (const struct AgitationMovementStatic*)CONTINUOUS_GENTLE_SEQ,
         .sequence_length = CONTINUOUS_GENTLE_SEQ_LENGTH}}};
static const size_t C41_STEP_LENGTH = 1;

//------------------------------------------------------------------------------
// Color Development Sequences - C41 Full Process
//------------------------------------------------------------------------------

/**
 * @brief C41 Pre-Wash (Optional warm rinse)
 */
static const AgitationMovementStatic C41_PRE_WASH[] = {
    {.type = AgitationMovementTypeCW, .duration = 5},
    {.type = AgitationMovementTypePause, .duration = 1},
    {.type = AgitationMovementTypeCCW, .duration = 5},
    {.type = AgitationMovementTypePause, .duration = 1},
};
static const size_t C41_PRE_WASH_LENGTH = 4;

/**
 * @brief C41 Color Developer Stage (Continuous Gentle Agitation)
 */
static const AgitationMovementStatic C41_COLOR_DEVELOPER[] = {
    {.type = AgitationMovementTypeLoop,
     .loop = {
         .count = 0, // Continuous
         .sequence = (const struct AgitationMovementStatic*)CONTINUOUS_GENTLE_SEQ,
         .sequence_length = CONTINUOUS_GENTLE_SEQ_LENGTH}}};
static const size_t C41_COLOR_DEVELOPER_LENGTH = 1;

/**
 * @brief C41 Bleach Stage (Periodic Gentle Agitation)
 */
static const AgitationMovementStatic C41_BLEACH[] = {
    {.type = AgitationMovementTypeLoop,
     .loop =
         {.count = 3,
          .sequence = (const struct AgitationMovementStatic*)STANDARD_INVERSION,
          .sequence_length = STANDARD_INVERSION_LENGTH}},
    {.type = AgitationMovementTypePause, .duration = 15},
};
static const size_t C41_BLEACH_LENGTH = 2;

/**
 * @brief C41 Stabilizer/Final Rinse Stage (Gentle Agitation)
 */
static const AgitationMovementStatic C41_STABILIZER[] = {
    {.type = AgitationMovementTypeCW, .duration = 3},
    {.type = AgitationMovementTypePause, .duration = 1},
    {.type = AgitationMovementTypeCCW, .duration = 3},
    {.type = AgitationMovementTypePause, .duration = 1},
};
static const size_t C41_STABILIZER_LENGTH = 4;

/**
 * @brief Complete C41 Development Sequence
 * Includes Pre-Wash, Color Developer, Bleach, and Stabilizer stages
 */
static const AgitationMovementStatic C41_FULL_PROCESS[] = {
    // Pre-Wash (Optional)
    {.type = AgitationMovementTypeLoop,
     .loop =
         {.count = 1,
          .sequence = (const struct AgitationMovementStatic*)C41_PRE_WASH,
          .sequence_length = C41_PRE_WASH_LENGTH}},

    // Color Developer (Continuous Gentle)
    {.type = AgitationMovementTypeLoop,
     .loop =
         {.count = 0, // Continuous
          .sequence = (const struct AgitationMovementStatic*)C41_COLOR_DEVELOPER,
          .sequence_length = C41_COLOR_DEVELOPER_LENGTH}},

    // Bleach Stage
    {.type = AgitationMovementTypeLoop,
     .loop =
         {.count = 1,
          .sequence = (const struct AgitationMovementStatic*)C41_BLEACH,
          .sequence_length = C41_BLEACH_LENGTH}},

    // Stabilizer/Final Rinse
    {.type = AgitationMovementTypeLoop,
     .loop = {
         .count = 1,
         .sequence = (const struct AgitationMovementStatic*)C41_STABILIZER,
         .sequence_length = C41_STABILIZER_LENGTH}}};
static const size_t C41_FULL_PROCESS_LENGTH = 4;

//------------------------------------------------------------------------------
// C41 Full Process as Steps
//------------------------------------------------------------------------------

/**
 * @brief C41 Pre-Wash Step
 */
static const AgitationStepStatic C41_PRE_WASH_STEP = {
    .name = "Pre-Wash",
    .description = "Optional warm rinse before color development",
    .temperature = 38.0f, // Typical pre-wash temperature
    .sequence = C41_PRE_WASH,
    .sequence_length = C41_PRE_WASH_LENGTH
};

/**
 * @brief C41 Color Developer Step
 */
static const AgitationStepStatic C41_COLOR_DEVELOPER_STEP = {
    .name = "Color Developer",
    .description = "Main color development stage with continuous gentle agitation",
    .temperature = 38.0f, // Standard C41 color developer temperature
    .sequence = C41_COLOR_DEVELOPER,
    .sequence_length = C41_COLOR_DEVELOPER_LENGTH
};

/**
 * @brief C41 Bleach Step
 */
static const AgitationStepStatic C41_BLEACH_STEP = {
    .name = "Bleach",
    .description = "Bleach stage with periodic gentle agitation",
    .temperature = 38.0f, // Typical bleach temperature
    .sequence = C41_BLEACH,
    .sequence_length = C41_BLEACH_LENGTH
};

/**
 * @brief C41 Stabilizer Step
 */
static const AgitationStepStatic C41_STABILIZER_STEP = {
    .name = "Stabilizer",
    .description = "Final rinse and stabilization stage",
    .temperature = 38.0f, // Room temperature or slightly warmer
    .sequence = C41_STABILIZER,
    .sequence_length = C41_STABILIZER_LENGTH
};

/**
 * @brief Complete C41 Development Process
 * Includes Pre-Wash, Color Developer, Bleach, and Stabilizer steps
 */
static const AgitationProcessStatic C41_FULL_PROCESS_STATIC = {
    .process_name = "C41 Color Film Development",
    .film_type = "Color Negative",
    .tank_type = "Developing Tank",
    .chemistry = "C41 Color Chemistry",
    .temperature = 38.0f,
    .steps = (const AgitationStepStatic[]){
        C41_PRE_WASH_STEP,
        C41_COLOR_DEVELOPER_STEP,
        C41_BLEACH_STEP,
        C41_STABILIZER_STEP
    },
    .steps_length = 4
};