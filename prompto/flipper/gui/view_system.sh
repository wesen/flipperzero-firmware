#!/bin/bash

cat << 'EOC'
# View System Context
View management system containing:
- View dispatcher
- View port handling
- View stack management
- View holder implementation
Controls the GUI view hierarchy and navigation.
EOC

echo -e "\n---\n"

# Get view system files
pinocchio catter print -f "view_dispatcher\.(c|h)$" \
    -f "view_dispatcher_i\.h$" \
    -f "view_port\.(c|h)$" \
    -f "view_port_i\.h$" \
    -f "view_stack\.(c|h)$" \
    -f "view_holder\.(c|h)$" \
    -d xml applications/services/gui 