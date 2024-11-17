#!/bin/bash

cat << 'EOC'
# Scene Management Context
Scene handling system containing:
- Scene manager implementation
- Scene transitions
- Scene state management
Controls application screen flow and state transitions.
EOC

echo -e "\n---\n"

# Get scene management files
pinocchio catter print -f "scene_manager\.(c|h)$" \
    -f "scene_manager_i\.h$" \
    -d xml applications/services/gui 