#pragma once

#include "common_sequences.h"

//------------------------------------------------------------------------------
// Color Development Sequences - C41 Process
//------------------------------------------------------------------------------

/**
 * @brief C41 Pre-Wash (Optional warm rinse)
 */
static const AgitationMovementStatic C41_PRE_WASH[] = {
    {.type = AgitationMovementTypeCW, .duration = 5},
    {.type = AgitationMovementTypePause, .duration = 1},
    {.type = AgitationMovementTypeCCW, .duration = 5},
    {.type = AgitationMovementTypePause, .duration = 1},
    {.type = AgitationMovementTypeWaitUser, .message = "Pre-wash complete. Ready for developer?"},
};
static const size_t C41_PRE_WASH_LENGTH = 5;

static const AgitationMovementStatic C41_MINUTE_CYCLE[] = {
    // wait 50 seconds,
    // continuous agitation for 10 seconds
    {.type = AgitationMovementTypePause, .duration = 50},
    {.type = AgitationMovementTypeLoop,
     .loop =
         {.count = 0,
          .max_duration = 10,
          .sequence = (const struct AgitationMovementStatic*)CONTINUOUS_GENTLE_SEQ,
          .sequence_length = CONTINUOUS_GENTLE_SEQ_LENGTH}},
};
static const size_t C41_MINUTE_CYCLE_LENGTH = 2;

/**
 * @brief C41 Color Developer Stage (Continuous Gentle Agitation)
 */
static const AgitationMovementStatic C41_COLOR_DEVELOPER[] = {
    {.type = AgitationMovementTypeLoop,
     .loop =
         {.count = 0,
          .max_duration = 210,
          .sequence = (const struct AgitationMovementStatic*)C41_MINUTE_CYCLE,
          .sequence_length = C41_MINUTE_CYCLE_LENGTH}},
    {.type = AgitationMovementTypeWaitUser, .message = "Development complete. Ready for bleach?"},
};
static const size_t C41_COLOR_DEVELOPER_LENGTH = 2;

/**
 * @brief C41 Bleach Stage (Periodic Gentle Agitation)
 */
static const AgitationMovementStatic C41_BLEACH_SEQUENCE[] = {
    {.type = AgitationMovementTypeLoop,
     .loop =
         {.count = 3,
          .max_duration = 60 * 5,
          .sequence = (const struct AgitationMovementStatic*)C41_MINUTE_CYCLE,
          .sequence_length = C41_MINUTE_CYCLE_LENGTH}},
    {.type = AgitationMovementTypePause, .duration = 15},
    {.type = AgitationMovementTypeWaitUser, .message = "Bleach complete. Ready for stabilizer?"},
};
static const size_t C41_BLEACH_LENGTH = 3;

/**
 * @brief C41 Stabilizer/Final Rinse Stage (Gentle Agitation)
 */
static const AgitationMovementStatic C41_STABILIZER[] = {
    {.type = AgitationMovementTypeCW, .duration = 3},
    {.type = AgitationMovementTypePause, .duration = 1},
    {.type = AgitationMovementTypeCCW, .duration = 3},
    {.type = AgitationMovementTypePause, .duration = 1},
    {.type = AgitationMovementTypeWaitUser, .message = "Process complete! Remove film."},
};
static const size_t C41_STABILIZER_LENGTH = 5;

//------------------------------------------------------------------------------
// C41 Process Steps
//------------------------------------------------------------------------------

/**
 * @brief C41 Pre-Wash Step
 */
static const AgitationStepStatic C41_PRE_WASH_STEP = {
    .name = "Pre-Wash",
    .description = "Optional warm rinse before color development",
    .temperature = 38.0f,
    .sequence = C41_PRE_WASH,
    .sequence_length = C41_PRE_WASH_LENGTH};

/**
 * @brief C41 Color Developer Step
 */
static const AgitationStepStatic C41_COLOR_DEVELOPER_STEP = {
    .name = "Color Developer",
    .description = "Main color development stage with continuous gentle agitation",
    .temperature = 38.0f,
    .sequence = C41_COLOR_DEVELOPER,
    .sequence_length = C41_COLOR_DEVELOPER_LENGTH};

/**
 * @brief C41 Bleach Step
 */
static const AgitationStepStatic C41_BLEACH_STEP = {
    .name = "Bleach",
    .description = "Bleach stage with periodic gentle agitation",
    .temperature = 38.0f,
    .sequence = C41_BLEACH_SEQUENCE,
    .sequence_length = C41_BLEACH_LENGTH};

/**
 * @brief C41 Stabilizer Step
 */
static const AgitationStepStatic C41_STABILIZER_STEP = {
    .name = "Stabilizer",
    .description = "Final rinse and stabilization stage",
    .temperature = 38.0f,
    .sequence = C41_STABILIZER,
    .sequence_length = C41_STABILIZER_LENGTH};

// C41 Full Process Static Steps
static const AgitationStepStatic C41_FULL_PROCESS_STEPS[] =
    {C41_PRE_WASH_STEP, C41_COLOR_DEVELOPER_STEP, C41_BLEACH_STEP, C41_STABILIZER_STEP};

/**
 * @brief Complete C41 Development Process
 */
static const AgitationProcessStatic C41_FULL_PROCESS_STATIC = {
    .process_name = "C41 Color Film Development",
    .film_type = "Color Negative",
    .tank_type = "Developing Tank",
    .chemistry = "C41 Color Chemistry",
    .temperature = 38.0f,
    .steps = C41_FULL_PROCESS_STEPS,
    .steps_length = 4};
