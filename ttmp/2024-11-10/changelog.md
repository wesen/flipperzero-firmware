# Furi System Shell Scripts

Created shell scripts to analyze and extract different components of the Furi system. Each script includes both general context and specific component extraction:

- Created core_system.sh for core initialization and main system files
- Created thread_management.sh for thread-related functionality
- Created memory_management.sh for memory management system
- Created event_system.sh for event loop and timer implementation
- Created sync_primitives.sh for synchronization primitives
- Created communication.sh for message queues and communication
- Created record_system.sh for record system implementation
- Created logging.sh for logging system
- Created string_handling.sh for string operations
- Created system_overview.sh for full system analysis
- Created examples.sh for example file extraction 

Refactor HAL Scripts for Directory-Specific Catter Calls

Improved script organization by separating catter calls per directory for better control and clarity. Each directory now has its own dedicated catter command instead of using regexp matches across multiple directories.

- Split multi-directory catter calls into separate calls per directory
- Removed directory patterns from filename matching flags
- Improved clarity of file gathering process 