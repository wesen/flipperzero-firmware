#!/bin/bash

cat << 'EOC'
# GUI Elements Context
Basic GUI elements containing:
- Drawing primitives
- UI element definitions
- Animation support
Fundamental building blocks for GUI components.
EOC

echo -e "\n---\n"

# Get elements implementation
pinocchio catter print -f "elements\.(c|h)$" \
    -f "icon_animation\.(c|h)$" \
    -f "icon_animation_i\.h$" \
    -d xml applications/services/gui 
    