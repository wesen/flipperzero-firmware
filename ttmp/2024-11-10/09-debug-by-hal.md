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
