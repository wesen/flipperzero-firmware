# Film Development DSL Specification

## Overview
The Film Development DSL is a YAML-based domain-specific language designed to encode film development procedures. It provides a structured way to describe the precise sequence of agitation movements and pauses required during each step of the film development process.

## File Structure
A film development process is defined in a single YAML file containing:
- Process metadata
- A sequence of development steps
- Each step containing a sequence of movements

## Process Metadata
```yaml
process_name: string        # Name of the development process
film_type: string          # Type of film being developed
temperature: float         # Process temperature in Celsius
tank_type: string         # Type of development tank used
chemistry: string         # Chemistry kit/solutions used
```

## Steps
Each development process consists of an array of steps under the `steps` key.

### Step Properties
```yaml
name: string              # Name of the step
description: string       # Description of what the step accomplishes
temperature: float       # Temperature for this step in Celsius
sequence: array          # Array of sequence steps
```

## Sequence Steps
A sequence step can be one of four types: `CW`, `CCW`, `pause`, or `loop`. Each step must specify its type and duration.

### Movement Step (CW/CCW)
Represents a single clockwise or counter-clockwise movement of the development tank.
```yaml
type: "CW" | "CCW"       # Direction of movement
duration: integer        # Duration in seconds
```

### Pause Step
Represents a period of no movement.
```yaml
type: "pause"            # Indicates a pause
duration: integer        # Duration in seconds
```

### Loop Step
Represents a repeated sequence of movements.
```yaml
type: "loop"             # Indicates a loop
count: integer          # Number of times to repeat
sequence: array         # Array of sequence steps to repeat
```

## Example: Single Inversion
A complete inversion consists of:
```yaml
- type: "CW"
  duration: 1
- type: "pause"
  duration: 1
- type: "CCW"
  duration: 1
- type: "pause"
  duration: 1
```

## Example: Periodic Agitation
Periodic agitation with multiple inversions:
```yaml
- type: "loop"
  count: 6              # Repeat 6 times
  sequence:
    - type: "loop"      # Each agitation consists of 2 inversions
      count: 2
      sequence:
        - type: "CW"
          duration: 1
        - type: "pause"
          duration: 1
        - type: "CCW"
          duration: 1
        - type: "pause"
          duration: 1
    - type: "pause"
      duration: 24      # Long pause between agitations
```

## Validation Rules

### Movement Sequencing
1. A `CW` movement must be followed by a `pause`
2. A `CCW` movement must be followed by a `pause`
3. No direct transitions between `CW` and `CCW` movements are allowed

### Duration Rules
1. All durations must be positive integers
2. Durations are specified in seconds

### Loop Rules
1. A loop must have a positive integer count
2. A loop sequence must contain at least one movement and one pause
3. Loop nesting is allowed (e.g., for periodic agitation with multiple inversions)
4. Loop sequences must follow the same movement sequencing rules

## Total Duration Calculation
The total duration of a process can be calculated by:
1. For each step:
   - Sum the durations of all non-loop movements
   - For each loop:
     - Calculate the duration of one iteration of the loop sequence
     - Multiply by the loop count
2. Sum the durations of all steps

## Best Practices
1. Use loops for repeating patterns
2. Break down inversions into their component movements
3. Use consistent timing (1 second movements, 1 second pauses between movements)
4. Include clear step descriptions
5. Specify temperatures for each step
6. Use meaningful names for steps

## Common Patterns

### Standard Inversion
```yaml
- type: "loop"
  count: 1
  sequence:
    - type: "CW"
      duration: 1
    - type: "pause"
      duration: 1
    - type: "CCW"
      duration: 1
    - type: "pause"
      duration: 1
```

### Initial Agitation
```yaml
- type: "loop"
  count: 4              # Four inversions
  sequence:
    - type: "CW"
      duration: 1
    - type: "pause"
      duration: 1
    - type: "CCW"
      duration: 1
    - type: "pause"
      duration: 1
- type: "pause"
  duration: 24          # Rest period
```