
  # Using catter to gather source code for LLMs:                              
                                                                              
  Process and analyze source code for LLM context preparation and token       
  analysis                                                                    
                                                                              
  The `pinocchio catter` command is a tool for preparing and analyzing source 
  code for Large Language Model (LLM) contexts. It offers two main            
  subcommands: `print` for outputting and processing file contents, and       
  `stats` for analyzing codebase statistics.                                  
                                                                              
  ## File Filtering System                                                    
                                                                              
  The catter command provides a powerful and flexible file filtering system   
  that helps you precisely control which files are processed.                 
                                                                              
  ### Default Filters                                                         
                                                                              
  By default, catter excludes common binary and non-text files:               
                                                                              
  1. Binary File Extensions:                                                  
      • Images: `.png`, `.jpg`, `.jpeg`, `.gif`, `.bmp`, `.tiff`, `.webp`     
      • Audio: `.mp3`, `.wav`, `.ogg`, `.flac`                                
      • Video: `.mp4`, `.avi`, `.mov`, `.wmv`                                 
      • Archives: `.zip`, `.tar`, `.gz`, `.rar`                               
      • Executables: `.exe`, `.dll`, `.so`, `.dylib`                          
      • Documents: `.pdf`, `.doc`, `.docx`, `.xls`, `.xlsx`                   
      • Data: `.bin`, `.dat`, `.db`, `.sqlite`                                
      • Fonts: `.woff`, `.ttf`, `.eot`, `.svg`, `.woff2`                      
      • Lock files: `.lock`                                                   
  2. Excluded Directories:                                                    
      • Version Control: `.git`, `.svn`                                       
      • Dependencies: `node_modules`, `vendor`                                
      • IDE/Editor: `.history`, `.idea`, `.vscode`                            
      • Build: `build`, `dist`, `sorbet`                                      
      • Documentation: `.yardoc`                                              
  3. Excluded Filenames (regex patterns):                                     
      • `.*-lock\.json$`                                                      
      • `go\.sum$`                                                            
      • `yarn\.lock$`                                                         
      • `package-lock\.json$`                                                 
                                                                              
                                                                              
  These defaults can be disabled using the `--disable-default-filters` flag.  
                                                                              
  ### Filter Configuration Options                                            
                                                                              
  #### Extension-based Filtering                                              
                                                                              
  The following examples use these flags:                                     
                                                                              
  • `-i, --include`: Specify file extensions to include                       
  • `-e, --exclude`: Specify file extensions to exclude                       
                                                                              
    # Include only specific extensions                                        
    pinocchio catter print -i .go,.js,.py                                     
                                                                              
    # Exclude specific extensions                                             
    pinocchio catter print -e .test.js,.spec.py                               
                                                                              
    # Combine include and exclude                                             
    pinocchio catter print -i .go,.js -e .test.js                             
                                                                              
  #### Pattern Matching                                                       
                                                                              
  Flags used:                                                                 
                                                                              
  • `-f, --match-filename`: Match filenames using regex patterns              
  • `-p, --match-path`: Match file paths using regex patterns                 
  • `--exclude-match-filename`: Exclude files matching regex patterns         
  • `--exclude-match-path`: Exclude paths matching regex patterns             
                                                                              
    # Match test files                                                        
    pinocchio catter print -f "^test_.*\.py$"                                 
                                                                              
    # Match multiple patterns                                                 
    pinocchio catter print -f "^main.*" -f "^app.*"                           
                                                                              
    # Match specific directories while excluding tests                        
    pinocchio catter print -p "src/models/" --exclude-match-path              
  "internal/testing/"                                                         
                                                                              
  #### Directory Exclusion                                                    
                                                                              
  Using `-x, --exclude-dirs` to specify directories to skip:                  
                                                                              
    # Exclude multiple directories                                            
    pinocchio catter print -x tests,docs,examples,vendor                      
                                                                              
  #### Size and Binary Filtering                                              
                                                                              
  Flags:                                                                      
                                                                              
  • `--max-file-size`: Maximum size for individual files (bytes)              
  • `--filter-binary`: Control binary file filtering                          
                                                                              
    # Set maximum file size and include binary files                          
    pinocchio catter print --max-file-size 500000 --filter-binary=false       
                                                                              
  #### GitIgnore Integration                                                  
                                                                              
    # Use repository's .gitignore rules (default)                             
    pinocchio catter print .                                                  
                                                                              
    # Disable .gitignore rules                                                
    pinocchio catter print --disable-gitignore .                              
                                                                              
  ### YAML Configuration                                                      
                                                                              
  Create a `.catter-filter.yaml` file to define reusable filter profiles:     
                                                                              
    profiles:                                                                 
      go-only:                                                                
        include-exts: [.go]                                                   
        exclude-dirs: [vendor, test]                                          
        exclude-match-filenames: [".*_test\\.go$"]                            
        max-file-size: 1048576  # 1MB                                         
        filter-binary-files: true                                             
                                                                              
      docs:                                                                   
        include-exts: [.md, .rst, .txt]                                       
        match-paths: ["docs/", "README"]                                      
        exclude-dirs: [node_modules, vendor]                                  
                                                                              
      tests:                                                                  
        match-filenames: ["^test_", "_test\\.go$"]                            
        exclude-dirs: [vendor]                                                
                                                                              
  Use profiles with:                                                          
                                                                              
    pinocchio catter print --filter-profile go-only .                         
                                                                              
  ### Debugging Filters                                                       
                                                                              
  Use the verbose flag to see which files are being included or excluded:     
                                                                              
    pinocchio catter print --verbose .                                        
                                                                              
  Print current filter configuration:                                         
                                                                              
    pinocchio catter print --print-filters                                    
                                                                              
  ### Filter Precedence                                                       
                                                                              
  Filters are applied in the following order:                                 
                                                                              
  1. GitIgnore rules (unless disabled)                                        
  2. File size limits                                                         
  3. Default exclusions (unless disabled)                                     
  4. Extension includes                                                       
  5. Extension excludes                                                       
  6. Filename pattern matches                                                 
  7. Path pattern matches                                                     
  8. Directory exclusions                                                     
  9. Binary file filtering                                                    
                                                                              
  A file must pass all applicable filters to be included in the output.       
                                                                              
  ### Best Practices                                                          
                                                                              
  1. **Start Broad, Then Narrow**                                             
    # Start with extension filtering                                          
    pinocchio catter print --include .py .                                    
                                                                              
    # Add specific patterns                                                   
    pinocchio catter print --include .py --match-filename "^(?!test_).*\.py$" 
                                                                              
  2. **Use Multiple Filter Types**                                            
    # Combine different filter types for precision                            
    pinocchio catter print \                                                  
      --include .go \                                                         
      --exclude-dirs vendor,test \                                            
      --match-path "src/" \                                                   
      --exclude-match-filename "_test\.go$"                                   
                                                                              
  3. **Profile-based Workflow**                                               
      • Create profiles for common tasks                                      
      • Use environment variables for profile selection                       
      • Share profiles across team members                                    
  4. **Performance Considerations**                                           
      • Start with directory exclusions for large codebases                   
      • Use file size limits for large files                                  
      • Enable binary filtering to avoid processing non-text files            
                                                                              
                                                                              
  ## Common Use Cases                                                         
                                                                              
  ### 1. Preparing Code for LLM Prompts                                       
                                                                              
  Flags used:                                                                 
                                                                              
  • `-d, --delimiter`: Output format (markdown, xml, simple, begin-end)       
  • `-s, --stats`: Statistics detail level (overview, dir, full)              
                                                                              
    # Get Python files with context headers                                   
    pinocchio catter print -i .py -x tests/ -d markdown src/                  
                                                                              
    # Process specific files with token statistics                            
    pinocchio catter stats -s full main.go utils.go config.go                 
                                                                              
  ### 2. Token-Aware Processing                                               
                                                                              
  Flags:                                                                      
                                                                              
  • `--max-tokens`: Limit total tokens processed                              
  • `--max-lines`: Limit lines per file                                       
  • `--glazed`: Enable structured output                                      
                                                                              
    # Limit tokens while getting detailed stats                               
    pinocchio catter print --max-tokens 4000 --max-lines 100 --glazed src/    
                                                                              
    # Get structured stats output                                             
    pinocchio catter stats --glazed -s full . | glazed format -f json         
                                                                              
  ## Command Reference                                                        
                                                                              
  ### Print Command                                                           
                                                                              
  `pinocchio catter print [flags] <paths...>`                                 
                                                                              
  Main flags:                                                                 
                                                                              
  • `--max-file-size`: Limit individual file sizes (default: 1MB)             
  • `--max-total-size`: Limit total processed size (default: 10MB)            
  • `-i, --include`: File extensions to include (e.g., .go,.js)               
  • `-e, --exclude`: File extensions to exclude                               
  • `-d, --delimiter`: Output format (default, xml, markdown, simple, begin-end)
  • `--max-lines`: Maximum lines per file                                     
  • `--max-tokens`: Maximum tokens per file                                   
  • `--glazed`: Enable structured output                                      
                                                                              
  Filtering options:                                                          
                                                                              
  • `-f, --match-filename`: Regex patterns for filenames                      
  • `-p, --match-path`: Regex patterns for file paths                         
  • `-x, --exclude-dirs`: Directories to exclude                              
  • `--disable-gitignore`: Ignore .gitignore rules                            
                                                                              
  ### Stats Command                                                           
                                                                              
  `pinocchio catter stats [flags] <paths...>`                                 
                                                                              
  Main flags:                                                                 
                                                                              
  • `-s, --stats`: Statistics detail level (overview, dir, full)              
  • `--glazed`: Enable structured output (default: true)                      
                                                                              
  The stats command provides:                                                 
                                                                              
  • Total token counts                                                        
  • File and directory statistics                                             
  • Extension-based analysis                                                  
  • Line counts and file sizes                                                
                                                                              
  ## Advanced Usage                                                           
                                                                              
  ### 1. Using YAML Configuration                                             
                                                                              
  Create a `.catter-filter.yaml` file for persistent settings:                
                                                                              
    profiles:                                                                 
      python-only:                                                            
        include-exts: [.py]                                                   
        exclude-dirs: [venv, __pycache__]                                     
      api-docs:                                                               
        match-paths: ["api/", "docs/"]                                        
        include-exts: [.md, .rst]                                             
                                                                              
  Use profiles:                                                               
                                                                              
    pinocchio catter print --filter-profile python-only .                     
                                                                              
  ### 2. Structured Output                                                    
                                                                              
  Generate machine-readable output:                                           
                                                                              
    # Get JSON-formatted stats                                                
    pinocchio catter stats --glazed -s full . | glazed format -f json         
                                                                              
    # Process output with other tools                                         
    pinocchio catter print --glazed src/ | glazed filter --col Content        
                                                                              
  ### 3. Context-Aware Processing                                             
                                                                              
  Maintain code context with delimiters:                                      
                                                                              
    # XML format for structured parsing / claude                              
    pinocchio catter print -d xml src/                                        
                                                                              
    # Markdown format separator                                               
    pinocchio catter print -d markdown --include .md,.rst docs/               
                                                                              
  ### 4. Gitignore Integration                                                
                                                                              
  Respect repository settings:                                                
                                                                              
    # Use repository's .gitignore                                             
    pinocchio catter print .                                                  
                                                                              
    # Override gitignore rules                                                
    pinocchio catter print --disable-gitignore .                              
                                                                              
  ## Tips and Best Practices                                                  
                                                                              
  1. **Token Optimization**                                                   
      • Use `--max-tokens` to stay within API limits                          
      • Combine with `--max-lines` for reasonable chunk sizes                 
      • Use stats command to analyze token usage patterns                     
  2. **Filtering Strategy**                                                   
      • Start with broad filters and refine                                   
      • Use `--print-filters` to verify configuration                         
      • Combine path and filename patterns for precision                      
  3. **Output Management**                                                    
      • Choose appropriate delimiters for your use case                       
      • Use structured output for automation                                  
      • Consider file size limits for large codebases                         
  4. **Configuration Management**                                             
      • Use YAML profiles for repeated tasks                                  
      • Set CATTER_PROFILE environment variable                               
      • Create project-specific filter configurations                         
                                                                              
                                                                              
  ## Error Handling                                                           
                                                                              
  Common error scenarios and solutions:                                       
                                                                              
  1. **Size Limits**                                                          
      • "maximum total size limit reached": Increase `--max-total-size`       
      • "maximum tokens limit reached": Adjust `--max-tokens`                 
  2. **Filter Issues**                                                        
      • No files processed: Check filter patterns                             
      • Unexpected files: Verify .gitignore settings                          
  3. **Performance**                                                          
      • Large directories: Use specific paths                                 
      • Memory usage: Set appropriate size limits                             
                                                                              
                                                                              
  ## Integration Examples                                                     
                                                                              
  ### 1. With LLM Tools                                                       
                                                                              
    # Prepare code for OpenAI API                                             
    pinocchio catter print --max-tokens 4000 -d markdown src/ > context.md    
                                                                              
    # Generate documentation                                                  
    pinocchio catter print --include .go --exclude-dirs vendor/ . | pinocchio code
  professional --context - "Generate documentation"                           
                                                                              
  ### 2. With Development Workflows                                           
                                                                              
    # Code review preparation                                                 
    pinocchio catter print --match-path "changed/" -d markdown > review.md    
                                                                              
    # Documentation updates                                                   
    pinocchio catter stats -s full . > codebase-metrics.json                  

---
Debug applications which use furi_hal functionality


applications/debug/event_loop_blink_test/event_loop_blink_test.c:#include <furi_hal_resources.h>
applications/debug/event_loop_blink_test/event_loop_blink_test.c:        furi_hal_gpio_init_simple(blink_gpio_pins[i], GpioModeOutputPushPull);
applications/debug/event_loop_blink_test/event_loop_blink_test.c:        furi_hal_gpio_write(blink_gpio_pins[i], false);
applications/debug/event_loop_blink_test/event_loop_blink_test.c:    furi_hal_gpio_init_simple(&gpio_ext_pc0, GpioModeOutputPushPull);
applications/debug/event_loop_blink_test/event_loop_blink_test.c:    furi_hal_gpio_write(&gpio_ext_pc0, false);
applications/debug/event_loop_blink_test/event_loop_blink_test.c:        furi_hal_gpio_write(blink_gpio_pins[i], false);
applications/debug/event_loop_blink_test/event_loop_blink_test.c:        furi_hal_gpio_init_simple(blink_gpio_pins[i], GpioModeAnalog);
applications/debug/event_loop_blink_test/event_loop_blink_test.c:    furi_hal_gpio_write(&gpio_ext_pc0, false);
applications/debug/event_loop_blink_test/event_loop_blink_test.c:    furi_hal_gpio_init_simple(&gpio_ext_pc0, GpioModeAnalog);
applications/debug/event_loop_blink_test/event_loop_blink_test.c:    furi_hal_gpio_write(gpio, !furi_hal_gpio_read(gpio));
applications/debug/event_loop_blink_test/event_loop_blink_test.c:    furi_hal_gpio_write(&gpio_ext_pc0, !furi_hal_gpio_read(&gpio_ext_pc0));
applications/debug/subghz_test/subghz_test_app.c:#include <furi_hal.h>
applications/debug/infrared_test/infrared_test.c:#include <furi_hal_infrared.h>
applications/debug/infrared_test/infrared_test.c:    furi_hal_infrared_set_tx_output(FuriHalInfraredTxPinExtPA7);
applications/debug/infrared_test/infrared_test.c:    furi_hal_infrared_async_tx_set_data_isr_callback(infrared_test_app_tx_data_callback, &app);
applications/debug/infrared_test/infrared_test.c:    furi_hal_infrared_async_tx_start(CARRIER_FREQ_HZ, CARRIER_DUTY);
applications/debug/infrared_test/infrared_test.c:    furi_hal_infrared_async_tx_wait_termination();
applications/debug/infrared_test/infrared_test.c:    furi_hal_infrared_set_tx_output(FuriHalInfraredTxPinInternal);
applications/debug/subghz_test/scenes/subghz_test_scene_show_only_rx.c:    if(!furi_hal_region_is_provisioned()) {
applications/debug/subghz_test/protocol/princeton_for_testing.c:#include <furi_hal.h>
applications/debug/crash_test/crash_test.c:#include <furi_hal.h>
applications/debug/crash_test/crash_test.c:        furi_halt("Crash test: furi_halt");
applications/debug/unit_tests/application.fam:    appid="test_furi_hal",
applications/debug/unit_tests/application.fam:    sources=["tests/common/*.c", "tests/furi_hal/*.c"],
applications/debug/unit_tests/application.fam:    appid="test_furi_hal_crypto",
applications/debug/unit_tests/application.fam:    sources=["tests/common/*.c", "tests/furi_hal_crypto/*.c"],
applications/debug/subghz_test/views/subghz_test_static.c:#include <furi_hal.h>
applications/debug/subghz_test/views/subghz_test_static.c:                    furi_hal_subghz_idle();
applications/debug/subghz_test/views/subghz_test_static.c:                    furi_hal_subghz_set_frequency_and_path(
applications/debug/subghz_test/views/subghz_test_static.c:                    if(!furi_hal_subghz_tx()) {
applications/debug/subghz_test/views/subghz_test_static.c:                        furi_hal_subghz_start_async_tx(
applications/debug/subghz_test/views/subghz_test_static.c:                        furi_hal_subghz_stop_async_tx();
applications/debug/subghz_test/views/subghz_test_static.c:    furi_hal_subghz_reset();
applications/debug/subghz_test/views/subghz_test_static.c:    furi_hal_subghz_load_custom_preset(subghz_device_cc1101_preset_ook_650khz_async_regs);
applications/debug/subghz_test/views/subghz_test_static.c:    furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
applications/debug/subghz_test/views/subghz_test_static.c:    furi_hal_gpio_write(&gpio_cc1101_g0, false);
applications/debug/subghz_test/views/subghz_test_static.c:    furi_hal_subghz_sleep();
applications/debug/subghz_test/views/subghz_test_carrier.c:#include <furi_hal.h>
applications/debug/subghz_test/views/subghz_test_carrier.c:            furi_hal_subghz_idle();
applications/debug/subghz_test/views/subghz_test_carrier.c:                furi_hal_subghz_set_frequency(subghz_frequencies_testing[model->frequency]);
applications/debug/subghz_test/views/subghz_test_carrier.c:            furi_hal_subghz_set_path(model->path);
applications/debug/subghz_test/views/subghz_test_carrier.c:                furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);
applications/debug/subghz_test/views/subghz_test_carrier.c:                furi_hal_subghz_rx();
applications/debug/subghz_test/views/subghz_test_carrier.c:                furi_hal_gpio_init(
applications/debug/subghz_test/views/subghz_test_carrier.c:                furi_hal_gpio_write(&gpio_cc1101_g0, true);
applications/debug/subghz_test/views/subghz_test_carrier.c:                if(!furi_hal_subghz_tx()) {
applications/debug/subghz_test/views/subghz_test_carrier.c:                    furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);
applications/debug/subghz_test/views/subghz_test_carrier.c:    furi_hal_subghz_reset();
applications/debug/subghz_test/views/subghz_test_carrier.c:    furi_hal_subghz_load_custom_preset(subghz_device_cc1101_preset_ook_650khz_async_regs);
applications/debug/subghz_test/views/subghz_test_carrier.c:    furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);
applications/debug/subghz_test/views/subghz_test_carrier.c:                furi_hal_subghz_set_frequency(subghz_frequencies_testing[model->frequency]);
applications/debug/subghz_test/views/subghz_test_carrier.c:    furi_hal_subghz_rx();
applications/debug/subghz_test/views/subghz_test_carrier.c:    furi_hal_subghz_sleep();
applications/debug/subghz_test/views/subghz_test_carrier.c:                model->rssi = furi_hal_subghz_get_rssi();
applications/debug/ccid_test/iso7816/iso7816_handler.c:#include <furi_hal.h>
applications/debug/ccid_test/iso7816/iso7816_handler.c:    furi_hal_usb_ccid_set_callbacks(ccid_callbacks, iso7816_handler);
applications/debug/ccid_test/iso7816/iso7816_handler.c:    furi_hal_usb_ccid_set_callbacks(NULL, NULL);
applications/debug/subghz_test/helpers/subghz_test_types.h:#include <furi_hal.h>
applications/debug/subghz_test/views/subghz_test_packet.c:#include <furi_hal.h>
applications/debug/subghz_test/views/subghz_test_packet.c:                model->rssi = furi_hal_subghz_get_rssi();
applications/debug/subghz_test/views/subghz_test_packet.c:                furi_hal_subghz_stop_async_rx();
applications/debug/subghz_test/views/subghz_test_packet.c:                furi_hal_subghz_stop_async_tx();
applications/debug/subghz_test/views/subghz_test_packet.c:                furi_hal_subghz_set_frequency(subghz_frequencies_testing[model->frequency]);
applications/debug/subghz_test/views/subghz_test_packet.c:            furi_hal_subghz_set_path(model->path);
applications/debug/subghz_test/views/subghz_test_packet.c:                furi_hal_subghz_start_async_rx(subghz_test_packet_rx_callback, instance);
applications/debug/subghz_test/views/subghz_test_packet.c:                if(!furi_hal_subghz_start_async_tx(
applications/debug/subghz_test/views/subghz_test_packet.c:    furi_hal_subghz_reset();
applications/debug/subghz_test/views/subghz_test_packet.c:    furi_hal_subghz_load_custom_preset(subghz_device_cc1101_preset_ook_650khz_async_regs);
applications/debug/subghz_test/views/subghz_test_packet.c:                furi_hal_subghz_set_frequency(subghz_frequencies_testing[model->frequency]);
applications/debug/subghz_test/views/subghz_test_packet.c:    furi_hal_subghz_start_async_rx(subghz_test_packet_rx_callback, instance);
applications/debug/subghz_test/views/subghz_test_packet.c:                furi_hal_subghz_stop_async_rx();
applications/debug/subghz_test/views/subghz_test_packet.c:                furi_hal_subghz_stop_async_tx();
applications/debug/subghz_test/views/subghz_test_packet.c:    furi_hal_subghz_sleep();
applications/debug/ccid_test/ccid_test_app.c:#include <furi_hal.h>
applications/debug/ccid_test/ccid_test_app.c:    FuriHalUsbInterface* usb_mode_prev = furi_hal_usb_get_config();
applications/debug/ccid_test/ccid_test_app.c:    furi_hal_usb_unlock();
applications/debug/ccid_test/ccid_test_app.c:    furi_check(furi_hal_usb_set_config(&usb_ccid, &app->ccid_cfg) == true);
applications/debug/ccid_test/ccid_test_app.c:    furi_hal_usb_ccid_insert_smartcard();
applications/debug/ccid_test/ccid_test_app.c:    furi_hal_usb_set_config(usb_mode_prev, NULL);
applications/debug/usb_test/usb_test.c:#include <furi_hal.h>
applications/debug/usb_test/usb_test.c:        furi_hal_usb_enable();
applications/debug/usb_test/usb_test.c:        furi_hal_usb_disable();
applications/debug/usb_test/usb_test.c:        furi_hal_usb_reinit();
applications/debug/usb_test/usb_test.c:        furi_hal_usb_set_config(&usb_cdc_single, NULL);
applications/debug/usb_test/usb_test.c:        furi_hal_usb_set_config(&usb_cdc_dual, NULL);
applications/debug/usb_test/usb_test.c:        furi_hal_usb_set_config(&usb_hid, NULL);
applications/debug/usb_test/usb_test.c:        furi_hal_usb_set_config(&usb_hid, &app->hid_cfg);
applications/debug/usb_test/usb_test.c:        furi_hal_usb_set_config(&usb_hid_u2f, NULL);
applications/debug/file_browser_test/scenes/file_browser_scene_start.c:#include <furi_hal.h>
applications/debug/blink_test/blink_test.c:#include <furi_hal.h>
applications/debug/usb_mouse/usb_mouse.c:#include <furi_hal.h>
applications/debug/usb_mouse/usb_mouse.c:    FuriHalUsbInterface* usb_mode_prev = furi_hal_usb_get_config();
applications/debug/usb_mouse/usb_mouse.c:    furi_hal_usb_unlock();
applications/debug/usb_mouse/usb_mouse.c:    furi_check(furi_hal_usb_set_config(&usb_hid, NULL) == true);
applications/debug/usb_mouse/usb_mouse.c:                    furi_hal_hid_mouse_press(HID_MOUSE_BTN_RIGHT);
applications/debug/usb_mouse/usb_mouse.c:                    furi_hal_hid_mouse_release(HID_MOUSE_BTN_RIGHT);
applications/debug/usb_mouse/usb_mouse.c:                        furi_hal_hid_mouse_press(HID_MOUSE_BTN_LEFT);
applications/debug/usb_mouse/usb_mouse.c:                        furi_hal_hid_mouse_release(HID_MOUSE_BTN_LEFT);
applications/debug/usb_mouse/usb_mouse.c:                        furi_hal_hid_mouse_move(MOUSE_MOVE_SHORT, 0);
applications/debug/usb_mouse/usb_mouse.c:                        furi_hal_hid_mouse_move(MOUSE_MOVE_LONG, 0);
applications/debug/usb_mouse/usb_mouse.c:                        furi_hal_hid_mouse_move(-MOUSE_MOVE_SHORT, 0);
applications/debug/usb_mouse/usb_mouse.c:                        furi_hal_hid_mouse_move(-MOUSE_MOVE_LONG, 0);
applications/debug/usb_mouse/usb_mouse.c:                        furi_hal_hid_mouse_move(0, MOUSE_MOVE_SHORT);
applications/debug/usb_mouse/usb_mouse.c:                        furi_hal_hid_mouse_move(0, MOUSE_MOVE_LONG);
applications/debug/usb_mouse/usb_mouse.c:                        furi_hal_hid_mouse_move(0, -MOUSE_MOVE_SHORT);
applications/debug/usb_mouse/usb_mouse.c:                        furi_hal_hid_mouse_move(0, -MOUSE_MOVE_LONG);
applications/debug/usb_mouse/usb_mouse.c:    furi_hal_usb_set_config(usb_mode_prev, NULL);
applications/debug/file_browser_test/file_browser_app.c:#include <furi_hal.h>
applications/debug/accessor/helpers/wiegand.cpp:#include <furi_hal.h>
applications/debug/accessor/helpers/wiegand.cpp:    furi_hal_gpio_init_simple(pinD0, GpioModeInterruptFall); // Set D0 pin as input
applications/debug/accessor/helpers/wiegand.cpp:    furi_hal_gpio_init_simple(pinD1, GpioModeInterruptFall); // Set D1 pin as input
applications/debug/accessor/helpers/wiegand.cpp:    furi_hal_gpio_add_int_callback(pinD0, input_isr_d0, this);
applications/debug/accessor/helpers/wiegand.cpp:    furi_hal_gpio_add_int_callback(pinD1, input_isr_d1, this);
applications/debug/accessor/helpers/wiegand.cpp:    furi_hal_gpio_remove_int_callback(pinD0);
applications/debug/accessor/helpers/wiegand.cpp:    furi_hal_gpio_remove_int_callback(pinD1);
applications/debug/accessor/helpers/wiegand.cpp:    furi_hal_gpio_init_simple(pinD0, GpioModeAnalog);
applications/debug/accessor/helpers/wiegand.cpp:    furi_hal_gpio_init_simple(pinD1, GpioModeAnalog);
applications/debug/accessor/accessor_app.cpp:#include <furi_hal.h>
applications/debug/accessor/accessor_app.cpp:    furi_hal_power_enable_otg();
applications/debug/accessor/accessor_app.cpp:    furi_hal_power_disable_otg();
applications/debug/expansion_test/expansion_test.c:#include <furi_hal_resources.h>
applications/debug/expansion_test/expansion_test.c:#include <furi_hal_serial.h>
applications/debug/expansion_test/expansion_test.c:#include <furi_hal_serial_control.h>
applications/debug/expansion_test/expansion_test.c:        const uint8_t data = furi_hal_serial_async_rx(handle);
applications/debug/expansion_test/expansion_test.c:    instance->handle = furi_hal_serial_control_acquire(MODULE_SERIAL_ID);
applications/debug/expansion_test/expansion_test.c:    furi_hal_serial_init(instance->handle, EXPANSION_PROTOCOL_DEFAULT_BAUD_RATE);
applications/debug/expansion_test/expansion_test.c:    furi_hal_serial_async_rx_start(
applications/debug/expansion_test/expansion_test.c:    furi_hal_serial_control_release(instance->handle);
applications/debug/expansion_test/expansion_test.c:    furi_hal_serial_tx(instance->handle, data, data_size);
applications/debug/expansion_test/expansion_test.c:    furi_hal_serial_tx_wait_complete(instance->handle);
applications/debug/expansion_test/expansion_test.c:    furi_hal_serial_tx(instance->handle, &init, sizeof(init));
applications/debug/expansion_test/expansion_test.c:    furi_hal_serial_tx_wait_complete(instance->handle);
applications/debug/expansion_test/expansion_test.c:        furi_hal_serial_set_br(instance->handle, 230400);
applications/debug/lfrfid_debug/scenes/lfrfid_debug_app_scene_tune.c:#include <furi_hal.h>
applications/debug/lfrfid_debug/scenes/lfrfid_debug_app_scene_tune.c:    furi_hal_gpio_write(&gpio_ext_pa7, !level);
applications/debug/lfrfid_debug/scenes/lfrfid_debug_app_scene_tune.c:    furi_hal_gpio_init_simple(&gpio_ext_pa7, GpioModeOutputPushPull);
applications/debug/lfrfid_debug/scenes/lfrfid_debug_app_scene_tune.c:    furi_hal_rfid_comp_set_callback(comparator_trigger_callback, app);
applications/debug/lfrfid_debug/scenes/lfrfid_debug_app_scene_tune.c:    furi_hal_rfid_comp_start();
applications/debug/lfrfid_debug/scenes/lfrfid_debug_app_scene_tune.c:    furi_hal_rfid_tim_read_start(125000, 0.5);
applications/debug/lfrfid_debug/scenes/lfrfid_debug_app_scene_tune.c:        furi_hal_rfid_set_read_period(lfrfid_debug_view_tune_get_arr(app->tune_view));
applications/debug/lfrfid_debug/scenes/lfrfid_debug_app_scene_tune.c:        furi_hal_rfid_set_read_pulse(lfrfid_debug_view_tune_get_ccr(app->tune_view));
applications/debug/lfrfid_debug/scenes/lfrfid_debug_app_scene_tune.c:    furi_hal_rfid_comp_stop();
applications/debug/lfrfid_debug/scenes/lfrfid_debug_app_scene_tune.c:    furi_hal_rfid_comp_set_callback(NULL, NULL);
applications/debug/lfrfid_debug/scenes/lfrfid_debug_app_scene_tune.c:    furi_hal_gpio_init_simple(&gpio_ext_pa7, GpioModeAnalog);
applications/debug/lfrfid_debug/scenes/lfrfid_debug_app_scene_tune.c:    furi_hal_rfid_tim_read_stop();
applications/debug/lfrfid_debug/scenes/lfrfid_debug_app_scene_tune.c:    furi_hal_rfid_pins_reset();
applications/debug/vibro_test/vibro_test.c:#include <furi_hal.h>
applications/debug/display_test/display_test.c:#include <furi_hal.h>
applications/debug/lfrfid_debug/lfrfid_debug_i.h:#include <furi_hal.h>
applications/debug/unit_tests/tests/varint/varint_test.c:#include <furi_hal.h>
applications/debug/uart_echo/uart_echo.c:#include <furi_hal.h>
applications/debug/uart_echo/uart_echo.c:        uint8_t data = furi_hal_serial_async_rx(handle);
applications/debug/uart_echo/uart_echo.c:                    furi_hal_serial_tx(app->serial_handle, data, length);
applications/debug/uart_echo/uart_echo.c:            furi_hal_serial_tx(app->serial_handle, (uint8_t*)"\r\nDetect IDLE\r\n", 15);
applications/debug/uart_echo/uart_echo.c:                furi_hal_serial_tx(app->serial_handle, (uint8_t*)"\r\nDetect ORE\r\n", 14);
applications/debug/uart_echo/uart_echo.c:                furi_hal_serial_tx(app->serial_handle, (uint8_t*)"\r\nDetect FE\r\n", 13);
applications/debug/uart_echo/uart_echo.c:                furi_hal_serial_tx(app->serial_handle, (uint8_t*)"\r\nDetect NE\r\n", 13);
applications/debug/uart_echo/uart_echo.c:    app->serial_handle = furi_hal_serial_control_acquire(FuriHalSerialIdUsart);
applications/debug/uart_echo/uart_echo.c:    furi_hal_serial_init(app->serial_handle, baudrate);
applications/debug/uart_echo/uart_echo.c:    furi_hal_serial_async_rx_start(app->serial_handle, uart_echo_on_irq_cb, app, true);
applications/debug/uart_echo/uart_echo.c:    furi_hal_serial_deinit(app->serial_handle);
applications/debug/uart_echo/uart_echo.c:    furi_hal_serial_control_release(app->serial_handle);
applications/debug/bt_debug_app/bt_debug_app.c:#include <furi_hal_bt.h>
applications/debug/bt_debug_app/bt_debug_app.c:    if(!furi_hal_bt_is_testing_supported()) {
applications/debug/bt_debug_app/bt_debug_app.c:    const bool was_active = furi_hal_bt_is_active();
applications/debug/bt_debug_app/bt_debug_app.c:    furi_hal_bt_stop_advertising();
applications/debug/bt_debug_app/bt_debug_app.c:        furi_hal_bt_start_advertising();
applications/debug/unit_tests/tests/subghz/subghz_test.c:#include <furi_hal.h>
applications/debug/unit_tests/tests/subghz/subghz_test.c:    furi_hal_subghz_reset();
applications/debug/unit_tests/tests/subghz/subghz_test.c:    furi_hal_subghz_load_custom_preset(subghz_device_cc1101_preset_ook_650khz_async_regs);
applications/debug/unit_tests/tests/subghz/subghz_test.c:    furi_hal_subghz_set_frequency_and_path(433920000);
applications/debug/unit_tests/tests/subghz/subghz_test.c:    if(!furi_hal_subghz_start_async_tx(subghz_hal_async_tx_test_yield, &test)) {
applications/debug/unit_tests/tests/subghz/subghz_test.c:    FuriHalCortexTimer timer = furi_hal_cortex_timer_get(30000000);
applications/debug/unit_tests/tests/subghz/subghz_test.c:    while(!furi_hal_subghz_is_async_tx_complete()) {
applications/debug/unit_tests/tests/subghz/subghz_test.c:        if(furi_hal_cortex_timer_is_expired(timer)) {
applications/debug/unit_tests/tests/subghz/subghz_test.c:            furi_hal_subghz_stop_async_tx();
applications/debug/unit_tests/tests/subghz/subghz_test.c:            furi_hal_subghz_sleep();
applications/debug/unit_tests/tests/subghz/subghz_test.c:    furi_hal_subghz_stop_async_tx();
applications/debug/unit_tests/tests/subghz/subghz_test.c:    furi_hal_subghz_sleep();
applications/debug/unit_tests/tests/subghz/subghz_test.c:        "Test furi_hal_async_tx normal");
applications/debug/unit_tests/tests/subghz/subghz_test.c:        "Test furi_hal_async_tx invalid start");
applications/debug/unit_tests/tests/subghz/subghz_test.c:        "Test furi_hal_async_tx invalid mid");
applications/debug/unit_tests/tests/subghz/subghz_test.c:        "Test furi_hal_async_tx invalid end");
applications/debug/unit_tests/tests/subghz/subghz_test.c:        "Test furi_hal_async_tx reset start");
applications/debug/unit_tests/tests/subghz/subghz_test.c:        "Test furi_hal_async_tx reset mid");
applications/debug/unit_tests/tests/subghz/subghz_test.c:        "Test furi_hal_async_tx reset end");
applications/debug/locale_test/locale_test.c:    furi_hal_rtc_get_datetime(&datetime);
applications/debug/unit_tests/tests/expansion/expansion_test.c:#include <furi_hal_random.h>
applications/debug/unit_tests/tests/expansion/expansion_test.c:        furi_hal_random_fill_buf(garbage_data, sizeof(garbage_data));
applications/debug/unit_tests/tests/power/power_test.c:#include <furi_hal.h>
applications/debug/unit_tests/tests/power/power_test.c:    furi_hal_power_set_battery_charge_voltage_limit(4.208f);
applications/debug/unit_tests/tests/power/power_test.c:        furi_hal_power_set_battery_charge_voltage_limit(charge_volt);
applications/debug/unit_tests/tests/power/power_test.c:            (double)charge_volt, (double)furi_hal_power_get_battery_charge_voltage_limit());
applications/debug/unit_tests/tests/power/power_test.c:    furi_hal_power_set_battery_charge_voltage_limit(4.016f);
applications/debug/unit_tests/tests/power/power_test.c:    mu_assert_double_eq(4.016, (double)furi_hal_power_get_battery_charge_voltage_limit());
applications/debug/unit_tests/tests/power/power_test.c:    furi_hal_power_set_battery_charge_voltage_limit(3.841f);
applications/debug/unit_tests/tests/power/power_test.c:    mu_assert_double_eq(3.840, (double)furi_hal_power_get_battery_charge_voltage_limit());
applications/debug/unit_tests/tests/power/power_test.c:    furi_hal_power_set_battery_charge_voltage_limit(3.900f);
applications/debug/unit_tests/tests/power/power_test.c:    mu_assert_double_eq(3.888, (double)furi_hal_power_get_battery_charge_voltage_limit());
applications/debug/unit_tests/tests/power/power_test.c:    furi_hal_power_set_battery_charge_voltage_limit(4.200f);
applications/debug/unit_tests/tests/power/power_test.c:    mu_assert_double_eq(4.192, (double)furi_hal_power_get_battery_charge_voltage_limit());
applications/debug/unit_tests/tests/power/power_test.c:    furi_hal_power_set_battery_charge_voltage_limit(3.808f);
applications/debug/unit_tests/tests/power/power_test.c:    mu_assert_double_eq(3.840, (double)furi_hal_power_get_battery_charge_voltage_limit());
applications/debug/unit_tests/tests/power/power_test.c:    furi_hal_power_set_battery_charge_voltage_limit(1.0f);
applications/debug/unit_tests/tests/power/power_test.c:    mu_assert_double_eq(3.840, (double)furi_hal_power_get_battery_charge_voltage_limit());
applications/debug/unit_tests/tests/power/power_test.c:    furi_hal_power_set_battery_charge_voltage_limit(4.240f);
applications/debug/unit_tests/tests/power/power_test.c:    mu_assert_double_eq(4.208, (double)furi_hal_power_get_battery_charge_voltage_limit());
applications/debug/unit_tests/tests/power/power_test.c:    furi_hal_power_set_battery_charge_voltage_limit(8.105f);
applications/debug/unit_tests/tests/power/power_test.c:    mu_assert_double_eq(4.208, (double)furi_hal_power_get_battery_charge_voltage_limit());
applications/debug/bt_debug_app/views/bt_carrier_test.c:#include <furi_hal_bt.h>
applications/debug/bt_debug_app/views/bt_carrier_test.c:        furi_hal_bt_start_packet_rx(bt_carrier_test->channel, 1);
applications/debug/bt_debug_app/views/bt_carrier_test.c:        furi_hal_bt_start_tone_tx(bt_carrier_test->channel, bt_carrier_test->power);
applications/debug/bt_debug_app/views/bt_carrier_test.c:        furi_hal_bt_start_tone_tx(bt_carrier_test->channel, bt_carrier_test->power);
applications/debug/bt_debug_app/views/bt_carrier_test.c:    furi_hal_bt_stop_tone_tx();
applications/debug/bt_debug_app/views/bt_carrier_test.c:    furi_hal_bt_start_tone_tx(bt_carrier_test->channel, bt_carrier_test->power);
applications/debug/bt_debug_app/views/bt_carrier_test.c:        furi_hal_bt_stop_tone_tx();
applications/debug/bt_debug_app/views/bt_carrier_test.c:        furi_hal_bt_stop_tone_tx();
applications/debug/bt_debug_app/views/bt_carrier_test.c:        furi_hal_bt_stop_packet_test();
applications/debug/bt_debug_app/views/bt_carrier_test.c:    furi_hal_bt_stop_tone_tx();
applications/debug/bt_debug_app/views/bt_carrier_test.c:        bt_test_set_rssi(bt_carrier_test->bt_test, furi_hal_bt_get_rssi());
applications/debug/bt_debug_app/views/bt_packet_test.c:#include <furi_hal_bt.h>
applications/debug/bt_debug_app/views/bt_packet_test.c:        furi_hal_bt_start_packet_rx(bt_packet_test->channel, bt_packet_test->data_rate);
applications/debug/bt_debug_app/views/bt_packet_test.c:        furi_hal_bt_start_packet_tx(bt_packet_test->channel, 1, bt_packet_test->data_rate);
applications/debug/bt_debug_app/views/bt_packet_test.c:        furi_hal_bt_stop_packet_test();
applications/debug/bt_debug_app/views/bt_packet_test.c:        bt_test_set_packets_tx(bt_packet_test->bt_test, furi_hal_bt_get_transmitted_packets());
applications/debug/bt_debug_app/views/bt_packet_test.c:        bt_test_set_packets_rx(bt_packet_test->bt_test, furi_hal_bt_stop_packet_test());
applications/debug/bt_debug_app/views/bt_packet_test.c:        bt_test_set_rssi(bt_packet_test->bt_test, furi_hal_bt_get_rssi());
applications/debug/unit_tests/tests/nfc/nfc_test.c:#include <furi_hal.h>
applications/debug/unit_tests/tests/nfc/nfc_test.c:    furi_hal_random_fill_buf(data->uid, uid_len);
applications/debug/unit_tests/tests/nfc/nfc_test.c:    furi_hal_random_fill_buf(data->atqa, sizeof(data->atqa));
applications/debug/unit_tests/tests/nfc/nfc_test.c:    furi_hal_random_fill_buf(&data->sak, 1);
applications/debug/unit_tests/tests/nfc/nfc_test.c:        furi_hal_random_fill_buf(page.data, sizeof(MfUltralightPage));
applications/debug/unit_tests/tests/nfc/nfc_test.c:    furi_hal_random_fill_buf(block_write.data, sizeof(MfClassicBlock));
applications/debug/unit_tests/tests/nfc/nfc_test.c:        furi_hal_random_fill_buf(key_arr_ref[i].data, sizeof(MfClassicKey));
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:#include <furi_hal.h>
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:static void furi_hal_crypto_ctr_setup(void) {
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:static void furi_hal_crypto_ctr_teardown(void) {
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:static void furi_hal_crypto_gcm_setup(void) {
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:static void furi_hal_crypto_gcm_teardown(void) {
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:MU_TEST(furi_hal_crypto_ctr_1) {
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:    ret = furi_hal_crypto_ctr(key_ctr_1, iv_ctr_1, pt_ctr_1, ct, sizeof(pt_ctr_1));
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:MU_TEST(furi_hal_crypto_ctr_2) {
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:    ret = furi_hal_crypto_ctr(key_ctr_2, iv_ctr_2, pt_ctr_2, ct, sizeof(pt_ctr_2));
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:MU_TEST(furi_hal_crypto_ctr_3) {
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:    ret = furi_hal_crypto_ctr(key_ctr_3, iv_ctr_3, pt_ctr_3, ct, sizeof(pt_ctr_3));
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:MU_TEST(furi_hal_crypto_ctr_4) {
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:    ret = furi_hal_crypto_ctr(key_ctr_4, iv_ctr_4, pt_ctr_4, ct, sizeof(pt_ctr_4));
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:MU_TEST(furi_hal_crypto_ctr_5) {
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:    ret = furi_hal_crypto_ctr(key_ctr_5, iv_ctr_5, pt_ctr_5, ct, sizeof(pt_ctr_5));
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:MU_TEST(furi_hal_crypto_gcm_1) {
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:    ret = furi_hal_crypto_gcm(
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:    ret = furi_hal_crypto_gcm(
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:MU_TEST(furi_hal_crypto_gcm_2) {
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:    ret = furi_hal_crypto_gcm(
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:    ret = furi_hal_crypto_gcm(
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:MU_TEST(furi_hal_crypto_gcm_3) {
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:    ret = furi_hal_crypto_gcm(
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:    ret = furi_hal_crypto_gcm(
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:MU_TEST(furi_hal_crypto_gcm_4) {
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:    ret = furi_hal_crypto_gcm(
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:    ret = furi_hal_crypto_gcm(
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:MU_TEST_SUITE(furi_hal_crypto_ctr_test) {
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:    MU_SUITE_CONFIGURE(&furi_hal_crypto_ctr_setup, &furi_hal_crypto_ctr_teardown);
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:    MU_RUN_TEST(furi_hal_crypto_ctr_1);
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:    MU_RUN_TEST(furi_hal_crypto_ctr_2);
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:    MU_RUN_TEST(furi_hal_crypto_ctr_3);
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:    MU_RUN_TEST(furi_hal_crypto_ctr_4);
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:    MU_RUN_TEST(furi_hal_crypto_ctr_5);
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:MU_TEST_SUITE(furi_hal_crypto_gcm_test) {
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:    MU_SUITE_CONFIGURE(&furi_hal_crypto_gcm_setup, &furi_hal_crypto_gcm_teardown);
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:    MU_RUN_TEST(furi_hal_crypto_gcm_1);
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:    MU_RUN_TEST(furi_hal_crypto_gcm_2);
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:    MU_RUN_TEST(furi_hal_crypto_gcm_3);
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:    MU_RUN_TEST(furi_hal_crypto_gcm_4);
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:int run_minunit_test_furi_hal_crypto(void) {
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:    MU_RUN_SUITE(furi_hal_crypto_ctr_test);
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:    MU_RUN_SUITE(furi_hal_crypto_gcm_test);
applications/debug/unit_tests/tests/furi_hal_crypto/furi_hal_crypto_tests.c:TEST_API_DEFINE(run_minunit_test_furi_hal_crypto)
applications/debug/unit_tests/tests/bt/bt_test.c:#include <furi_hal.h>
applications/debug/unit_tests/tests/compress/compress_test.c:#include <furi_hal.h>
applications/debug/unit_tests/tests/compress/compress_test.c:#include <furi_hal_random.h>
applications/debug/unit_tests/tests/compress/compress_test.c:    furi_hal_random_fill_buf(src_buff, src_data_size);
applications/debug/unit_tests/tests/strint/strint_test.c:#include <furi_hal.h>
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:#include <furi_hal.h>
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:static void furi_hal_i2c_int_setup(void) {
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:static void furi_hal_i2c_int_teardown(void) {
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    furi_hal_i2c_release(&furi_hal_i2c_handle_power);
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:static void furi_hal_i2c_ext_setup(void) {
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:static void furi_hal_i2c_ext_teardown(void) {
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    furi_hal_i2c_release(&furi_hal_i2c_handle_external);
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:MU_TEST(furi_hal_i2c_int_1b) {
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    ret = furi_hal_i2c_read_reg_8(
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:        &furi_hal_i2c_handle_power,
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    ret = furi_hal_i2c_write_reg_8(
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:        &furi_hal_i2c_handle_power,
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    ret = furi_hal_i2c_read_reg_8(
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:        &furi_hal_i2c_handle_power,
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:MU_TEST(furi_hal_i2c_int_3b) {
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    ret = furi_hal_i2c_tx(
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:        &furi_hal_i2c_handle_power, LP5562_ADDRESS, data_many, 1, LP5562_I2C_TIMEOUT);
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    ret = furi_hal_i2c_rx(
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:        &furi_hal_i2c_handle_power,
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    ret = furi_hal_i2c_tx(
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:        &furi_hal_i2c_handle_power, LP5562_ADDRESS, data_many, DATA_SIZE, LP5562_I2C_TIMEOUT);
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    ret = furi_hal_i2c_tx(
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:        &furi_hal_i2c_handle_power, LP5562_ADDRESS, data_many, 1, LP5562_I2C_TIMEOUT);
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    ret = furi_hal_i2c_rx(
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:        &furi_hal_i2c_handle_power,
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:MU_TEST(furi_hal_i2c_int_1b_fail) {
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    ret = furi_hal_i2c_read_reg_8(
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:        &furi_hal_i2c_handle_power,
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    ret = furi_hal_i2c_read_reg_8(
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:        &furi_hal_i2c_handle_power,
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:MU_TEST(furi_hal_i2c_int_ext_3b) {
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    ret = furi_hal_i2c_tx_ext(
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:        &furi_hal_i2c_handle_power,
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    ret = furi_hal_i2c_rx_ext(
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:        &furi_hal_i2c_handle_power,
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    ret = furi_hal_i2c_rx_ext(
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:        &furi_hal_i2c_handle_power,
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    ret = furi_hal_i2c_rx_ext(
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:        &furi_hal_i2c_handle_power,
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:MU_TEST(furi_hal_i2c_ext_eeprom) {
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    if(!furi_hal_i2c_is_device_ready(&furi_hal_i2c_handle_external, EEPROM_ADDRESS, 100)) {
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:        ret = furi_hal_i2c_write_mem(
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:            &furi_hal_i2c_handle_external,
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    ret = furi_hal_i2c_read_mem(
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:        &furi_hal_i2c_handle_external, EEPROM_ADDRESS, 0, buffer, EEPROM_SIZE, 100);
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:MU_TEST_SUITE(furi_hal_i2c_int_suite) {
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    MU_SUITE_CONFIGURE(&furi_hal_i2c_int_setup, &furi_hal_i2c_int_teardown);
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    MU_RUN_TEST(furi_hal_i2c_int_1b);
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    MU_RUN_TEST(furi_hal_i2c_int_3b);
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    MU_RUN_TEST(furi_hal_i2c_int_ext_3b);
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    MU_RUN_TEST(furi_hal_i2c_int_1b_fail);
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:MU_TEST_SUITE(furi_hal_i2c_ext_suite) {
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    MU_SUITE_CONFIGURE(&furi_hal_i2c_ext_setup, &furi_hal_i2c_ext_teardown);
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    MU_RUN_TEST(furi_hal_i2c_ext_eeprom);
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:int run_minunit_test_furi_hal(void) {
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    MU_RUN_SUITE(furi_hal_i2c_int_suite);
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:    MU_RUN_SUITE(furi_hal_i2c_ext_suite);
applications/debug/unit_tests/tests/furi_hal/furi_hal_tests.c:TEST_API_DEFINE(run_minunit_test_furi_hal)
applications/debug/unit_tests/tests/furi/furi_test.c:#include <furi_hal.h>
applications/debug/unit_tests/tests/furi/furi_event_loop.c:#include <furi_hal.h>
applications/debug/unit_tests/tests/furi/furi_event_loop.c:    furi_delay_us(furi_hal_random_get() % 1000);
applications/debug/unit_tests/tests/furi/furi_event_loop.c:    furi_delay_us(furi_hal_random_get() % 1000);
---
Example scripts to find files related to a topic


<file name="prompto/flipper/communication">
<content>
#!/bin/bash

cat << 'EOC'
# Communication System Context
Inter-process communication mechanisms including:
- Message Queues: For thread-safe data exchange
- PubSub System: For publish/subscribe pattern implementation
- Stream Buffers: For continuous data streaming between components
Essential for thread coordination and data exchange in the Flipper OS.
EOC

echo -e "\n---\n"

# Get message queue implementation
pinocchio catter print -f "message_queue\.(c|h)$" -d xml furi

# Get pub/sub system
pinocchio catter print -f "pubsub\.(c|h)$" -d xml furi

# Get stream buffer implementation
pinocchio catter print -f "stream_buffer\.(c|h)$" -d xml furi 
</content>
</file>
<file name="prompto/flipper/event_system">
<content>
#!/bin/bash

cat << 'EOC'
# Event System Context
Core event handling system containing:
- Event Loop: Core event processing mechanism
- Event Flags: For event signaling between threads
- Timer System: For both one-shot and periodic timer events (FuriTimer)
Handles asynchronous operations and timing in the Flipper OS.
EOC

echo -e "\n---\n"

# Get event loop implementation
pinocchio catter print -f "event_loop.*\.(c|h)$" \
    -f "event_flag\.(c|h)$" \
    -d xml furi

# Get timer implementation
pinocchio catter print -f "^timer\.(c|h)$" \
    -f "event_loop_timer.*\.(c|h)$" \
    -d xml furi
</content>
</file>
<file name="prompto/flipper/examples">
<content>
#!/bin/bash

# Get example files showing system usage
pinocchio catter print \
    -d xml applications/examples
</content>
</file>
<file name="prompto/flipper/general">
<content>
#!/bin/bash

cat << 'EOC'
# Core System Context
Foundational Furi system files and core definitions containing:
- Basic assertions (furi_assert, furi_check)
- System delays
- Logging functionality (FURI_LOG_*)
Forms the backbone of the Flipper system.
EOC

echo -e "\n---\n"

# Get core initialization and main system files
pinocchio catter print -f "^furi\..*" -f "^flipper\..*" \
    --exclude-dirs tests \
    -d xml furi

# Get core definitions and base types
pinocchio catter print -f "common_defines\.h$" -f "base\.h$" \
    -f "check\.h$" -f "core_defines\.h$" \
    -d xml furi
</content>
</file>
<file name="prompto/flipper/hal_headers">
<content>
#!/bin/bash

cat << 'EOC'
# Furi HAL layer headers
EOC

echo -e "\n---\n"

# Get message queue implementation
pinocchio catter print -i.h targets/f7/furi_hal targets/furi_hal_include -d xml
</content>
</file>
<file name="prompto/flipper/headers">
<content>
#!/bin/bash

cat << 'EOC'
# Furi library headers
EOC

echo -e "\n---\n"

# Get core initialization and main system files
pinocchio catter print -i.h furi
</content>
</file>
<file name="prompto/flipper/logging">
<content>
#!/bin/bash

cat << 'EOC'
# Logging System Context
System-wide logging implementation providing:
- Debug level logging (FURI_LOG_D)
- Info level logging (FURI_LOG_I)
- Warning level logging (FURI_LOG_W)
- Error level logging (FURI_LOG_E)
Essential for system debugging and monitoring.
EOC

echo -e "\n---\n"

# Get logging implementation
pinocchio catter print -f "log\.(c|h)$" -d xml furi
</content>
</file>
<file name="prompto/flipper/memory_management">
<content>
#!/bin/bash

cat << 'EOC'
# Memory Management Context
Memory management subsystem containing:
- Memory Manager: Core memory allocation and tracking
- Heap Implementation: Dynamic memory management
Ensures proper resource allocation and prevents memory leaks.
EOC

echo -e "\n---\n"

prompto get flipper/general

# Get memory manager implementation
pinocchio catter print -f "^memmgr.*\.(c|h)$" \
    -d xml furi

# Get heap implementation
pinocchio catter print -f "memmgr_heap\.(c|h)$" -d xml furi
</content>
</file>
<file name="prompto/flipper/record_system">
<content>
#!/bin/bash

cat << 'EOC'
# Record System Context
Service management system handling:
- System service registration (RECORD_*)
- Service acquisition (furi_record_open)
- Service release (furi_record_close)
Manages singleton services like GUI, Storage, and Notifications.
EOC

echo -e "\n---\n"

# Get record system implementation
pinocchio catter print -f "record\.(c|h)$" -d xml furi
</content>
</file>
<file name="prompto/flipper/string_handling">
<content>
#!/bin/bash

cat << 'EOC'
# String Handling Context
String manipulation subsystem containing:
- String operations
- Text processing utilities
Essential for text manipulation throughout the Flipper OS.
EOC

echo -e "\n---\n"

# Get string implementation
pinocchio catter print -f "string\.(c|h)$" -d xml furi
</content>
</file>
<file name="prompto/flipper/string_handling.sh">
<content>
#!/bin/bash

# Get general flipper context
prompto get flipper/general

# Get string implementation
pinocchio catter print -f "string\.(c|h)$" -d xml furi 
</content>
</file>
<file name="prompto/flipper/sync_primitives">
<content>
#!/bin/bash

cat << 'EOC'
# Synchronization Primitives Context
Thread synchronization mechanisms including:
- Mutex: For mutual exclusion (furi_mutex_*)
- Semaphores: For resource management
Ensures thread-safe operations and resource coordination.
EOC

echo -e "\n---\n"

# Get mutex implementation
pinocchio catter print -f "^mutex\.(c|h)$" -d xml furi

# Get semaphore implementation
pinocchio catter print -f "^semaphore\.(c|h)$" -d xml furi 
</content>
</file>
<file name="prompto/flipper/thread_management">
<content>
#!/bin/bash

cat << 'EOC'
# Thread Management Context
Threading subsystem containing:
- Thread creation and management (furi_thread_*)
- Thread priorities (FuriThreadPriority*)
- Thread list management
Implements the cooperative threading model of Flipper OS.
EOC

echo -e "\n---\n"

# Get thread-related files
pinocchio catter print -f "^thread.*\.(c|h)$" \
    --exclude-match-filename "_test\." \
    -d xml furi

# Get thread list implementation
pinocchio catter print -f "thread_list\.(c|h)$" -d xml furi
</content>
</file>

---
FURI hal header files



targets/f7/furi_hal/furi_hal_ibutton.c
targets/f7/furi_hal/furi_hal_version.c
targets/f7/furi_hal/furi_hal_flash.c
targets/f7/furi_hal/furi_hal_pwm.c
targets/f7/furi_hal/furi_hal_region.c
targets/f7/furi_hal/furi_hal_clock.c
targets/f7/furi_hal/furi_hal_rtc.c
targets/f7/furi_hal/furi_hal_memory.c
targets/f7/furi_hal/furi_hal_mpu.c
targets/f7/furi_hal/furi_hal_sd.c
targets/f7/furi_hal/furi_hal_rfid.h
targets/f7/furi_hal/furi_hal_adc.c
targets/f7/furi_hal/furi_hal_spi_config.c
targets/f7/furi_hal/furi_hal_bus.h
targets/f7/furi_hal/furi_hal_gpio.c
targets/f7/furi_hal/furi_hal_nfc_tech_i.h
targets/f7/furi_hal/furi_hal_serial.c
targets/f7/furi_hal/furi_hal_os.h
targets/f7/furi_hal/furi_hal_nfc_iso14443b.c
targets/f7/furi_hal/furi_hal_debug.c
targets/f7/furi_hal/furi_hal_bt.c
targets/f7/furi_hal/furi_hal_resources.h
targets/f7/furi_hal/furi_hal_interrupt.c
targets/f7/furi_hal/furi_hal_usb_u2f.c
targets/f7/furi_hal/furi_hal_idle_timer.h
targets/f7/furi_hal/furi_hal_usb_cdc.c
targets/f7/furi_hal/furi_hal_usb_hid.c
targets/f7/furi_hal/furi_hal_cortex.c
targets/f7/furi_hal/furi_hal_nfc_irq.c
targets/f7/furi_hal/furi_hal_infrared.c
targets/f7/furi_hal/furi_hal_subghz.c
targets/f7/furi_hal/furi_hal_bus.c
targets/f7/furi_hal/furi_hal_resources.c
targets/f7/furi_hal/furi_hal_dma.c
targets/f7/furi_hal/furi_hal_serial_types_i.h
targets/f7/furi_hal/furi_hal_spi.c
targets/f7/furi_hal/furi_hal_version_device.c
targets/f7/furi_hal/furi_hal_i2c_config.h
targets/f7/furi_hal/furi_hal_ibutton.h
targets/f7/furi_hal/furi_hal_nfc_timer.c
targets/f7/furi_hal/furi_hal_rtc.h
targets/f7/furi_hal/furi_hal_nfc_event.c
targets/f7/furi_hal/furi_hal_speaker.c
targets/f7/furi_hal/furi_hal_target_hw.h
targets/f7/furi_hal/furi_hal_usb_cdc.h
targets/f7/furi_hal/furi_hal_nfc_i.h
targets/f7/furi_hal/furi_hal_i2c_config.c
targets/f7/furi_hal/furi_hal_i2c_types.h
targets/f7/furi_hal/furi_hal_spi_config.h
targets/f7/furi_hal/furi_hal_os.c
targets/f7/furi_hal/furi_hal_nfc.c
targets/f7/furi_hal/furi_hal_rfid.c
targets/f7/furi_hal/furi_hal_serial.h
targets/f7/furi_hal/furi_hal_serial_types.h
targets/f7/furi_hal/furi_hal_light.c
targets/f7/furi_hal/furi_hal_power.c
targets/f7/furi_hal/furi_hal_usb.c
targets/f7/furi_hal/furi_hal_usb_i.h
targets/f7/furi_hal/furi_hal_spi_types.h
targets/f7/furi_hal/furi_hal_gpio.h
targets/f7/furi_hal/furi_hal_power_config.c
targets/f7/furi_hal/furi_hal_pwm.h
targets/f7/furi_hal/furi_hal_interrupt.h
targets/f7/furi_hal/furi_hal_nfc_iso15693.c
targets/f7/furi_hal/furi_hal_usb_ccid.c
targets/f7/furi_hal/furi_hal_dma.h
targets/f7/furi_hal/furi_hal_serial_control.c
targets/f7/furi_hal/furi_hal_info.c
targets/f7/furi_hal/furi_hal_clock.h
targets/f7/furi_hal/furi_hal_i2c.c
targets/f7/furi_hal/furi_hal_nfc_felica.c
targets/f7/furi_hal/furi_hal.c
targets/f7/furi_hal/furi_hal_flash.h
targets/f7/furi_hal/furi_hal_serial_control.h
targets/f7/furi_hal/furi_hal_nfc_iso14443a.c
targets/f7/furi_hal/furi_hal_random.c
targets/f7/furi_hal/furi_hal_vibro.c
targets/f7/furi_hal/furi_hal_crypto.c
targets/f7/furi_hal/furi_hal_subghz.h
targets/furi_hal_include/furi_hal_usb_hid_u2f.h
targets/furi_hal_include/furi_hal_adc.h
targets/furi_hal_include/furi_hal_mpu.h
targets/furi_hal_include/furi_hal_usb.h
targets/furi_hal_include/furi_hal_info.h
targets/furi_hal_include/furi_hal_version.h
targets/furi_hal_include/furi_hal.h
targets/furi_hal_include/furi_hal_vibro.h
targets/furi_hal_include/furi_hal_light.h
targets/furi_hal_include/furi_hal_memory.h
targets/furi_hal_include/furi_hal_usb_ccid.h
targets/furi_hal_include/furi_hal_usb_hid.h
targets/furi_hal_include/furi_hal_random.h
targets/furi_hal_include/furi_hal_crypto.h
targets/furi_hal_include/furi_hal_debug.h
targets/furi_hal_include/furi_hal_speaker.h
targets/furi_hal_include/furi_hal_nfc.h
targets/furi_hal_include/furi_hal_bt.h
targets/furi_hal_include/furi_hal_cortex.h
targets/furi_hal_include/furi_hal_spi.h
targets/furi_hal_include/furi_hal_power.h
targets/furi_hal_include/furi_hal_i2c.h
targets/furi_hal_include/furi_hal_infrared.h
targets/furi_hal_include/furi_hal_region.h
targets/furi_hal_include/furi_hal_sd.h
