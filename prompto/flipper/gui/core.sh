#!/bin/bash

cat << 'EOC'
# GUI Core Context
Core GUI service components containing:
- Main GUI service (gui.c/h)
- Canvas handling (canvas.c/h)
- View management (view.c/h)
- Icon handling (icon.c/h)
Essential components for Flipper's graphical interface.
EOC

echo -e "\n---\n"

# Get core GUI files
pinocchio catter print -f "gui\.(c|h)$" \
    -f "gui_i\.h$" \
    -f "canvas\.(c|h)$" \
    -f "canvas_i\.h$" \
    -d xml applications/services/gui

# Get view-related core files
pinocchio catter print -f "view\.(c|h)$" \
    -f "view_i\.h$" \
    -d xml applications/services/gui

# Get icon-related files
pinocchio catter print -f "icon\.(c|h)$" \
    -f "icon_i\.h$" \
    -d xml applications/services/gui 