#!/bin/bash

cat << 'EOC'
# GUI Modules Context
UI component modules including:
- Dialog and popup systems
- Input handlers (text, number, byte)
- Menu systems
- File browser
- Widget system
Provides ready-to-use UI components.
EOC

echo -e "\n---\n"

# Get basic UI modules
pinocchio catter print -f "dialog_ex\.(c|h)$" \
    -f "popup\.(c|h)$" \
    -f "menu\.(c|h)$" \
    -f "submenu\.(c|h)$" \
    -d xml applications/services/gui/modules

# Get input-related modules
pinocchio catter print -f "text_input\.(c|h)$" \
    -f "number_input\.(c|h)$" \
    -f "byte_input\.(c|h)$" \
    -d xml applications/services/gui/modules

# Get widget system
pinocchio catter print -f "widget\.(c|h)$" \
    -f "widget_elements/.*\.(c|h)$" \
    -d xml applications/services/gui/modules 