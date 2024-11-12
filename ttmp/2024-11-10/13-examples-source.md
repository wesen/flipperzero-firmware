=== BEGIN: applications/examples/application.fam ===
# Placeholder
App(
    appid="example_apps",
    name="Example apps bundle",
    apptype=FlipperAppType.METAPACKAGE,
)

=== END: applications/examples/application.fam ===

=== BEGIN: applications/examples/example_adc/application.fam ===
App(
    appid="example_adc",
    name="Example: ADC",
    apptype=FlipperAppType.EXTERNAL,
    entry_point="example_adc_main",
    requires=["gui"],
    stack_size=1 * 1024,
    fap_category="Examples",
)

=== END: applications/examples/example_adc/application.fam ===

=== BEGIN: applications/examples/example_adc/example_adc.c ===
/**
 * @file example_adc.c
 * @brief ADC example.
 */
#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <gui/elements.h>
#include <input/input.h>

const uint8_t font[] =
    "`\2\3\2\3\4\1\2\4\5\11\0\376\6\376\7\377\1M\2\263\3\370 \6\315\364\371\6!\12\315"
    "\364\201\260\35\312Q\0\42\11\315tJI\316\13\0#\14\315\264\223dP*\203R'\1$\15\315\264"
    "\262A\311\266D\251l\71\0%\15\315\264\7%\61)J\42\345 \0&\14\315\264\263$\13\223\266$"
    "\7\1'\10\315\364\201\60\347\10(\10\315\364\32[\313\0)\11\315\64\322b[\35\2*\12\315\264\263"
    "(\222j\71\15+\11\315\364I\331\226\23\1,\10\315\364\271\205Y\10-\10\315\364\31t\26\0.\10"
    "\315\364\71\346(\0/\14\315\364\221\60\13\263\60\13C\0\60\13\315\264\245Jb)E:\12\61\12\315"
    "\364\201Ll\333A\0\62\12\315\264\245bV\33r\20\63\13\315\264\245Z\232D\221\216\2\64\14\315\364"
    "\201LJ\242!\313v\20\65\14\315t\207$\134\223(\322Q\0\66\13\315\264\245p\252D\221\216\2\67"
    "\12\315t\207\60+\326a\0\70\13\315\264\245\222T\211\42\35\5\71\13\315\264\245J\24\215\221\216\2:"
    "\11\315\364i\71!G\1;\12\315\364I\71!\314B\0<\11\315\364\341\254Z\7\1=\12\315\364)"
    "C<\344$\0>\11\315\364\301\264V\207\1\77\12\315\264\245Z\35\312a\0@\14\315\264\245J\242$"
    "J\272\203\0A\15\315\264\245J\224\14I\224D\71\10B\13\315t\247\312T\211\222\35\5C\12\315\264"
    "\245JX\212t\24D\15\315t\247J\224DI\224\354(\0E\14\315t\207$\234\302p\310A\0F"
    "\12\315t\207$\234\302:\1G\14\315\264\245J\230(Q\244\243\0H\17\315t\243$J\206$J\242"
    "$\312A\0I\11\315\264\267\260m\7\1J\12\315\364\221\260%\212t\24K\14\315t\243\244\244iI"
    "T\7\1L\11\315t\303\216C\16\2M\17\315t\243dH\206$J\242$\312A\0N\16\315t\243"
    "D\251(Q\22%Q\16\2O\15\315\264\245J\224DI\24\351(\0P\12\315t\247J\224LaN"
    "Q\15\315\264\245J\224DI\42\251\61\0R\14\315t\247J\224L\225(\7\1S\13\315\264\245\222\232"
    "D\221\216\2T\10\315\264\267\260;\12U\16\315t\243$J\242$J\242HG\1V\15\315t\243$"
    "J\242$Jj\71\14W\17\315t\243$J\242dH\206$\312A\0X\15\315t\243$\212\64\251\22"
    "\345 \0Y\13\315t\243$Jja\35\6Z\12\315t\207\60k\34r\20[\10\315\264\264\260G\31"
    "\134\12\315\264\303\64L\303\64\14]\10\315t\304\276\351\0^\11\315\364\201,\311\271\1_\7\315\364y"
    "\35\4`\10\315t\322\234'\0a\14\315\364IK\224$R\222\203\0b\13\315t\303p\252D\311\216"
    "\2c\12\315\364IR%\335A\0d\14\315\364\221\60Z\242$\212v\20e\12\315\364I\322\220\244;"
    "\10f\12\315\364\221,\333\302:\12g\14\315\364IK\224D\321\30I\0h\14\315t\303p\252DI"
    "\224\203\0i\12\315\364\201\34\21k;\10j\12\315\364\201\34\21\273e\0k\13\315t\303J\244%Q"
    "\35\4l\10\315\264\305n;\10m\14\315\364)CRQ\22\245\216\1n\13\315\364)%\245\224D\71"
    "\10o\12\315\364IR%\212t\24p\13\315\364)S%J\246\60\4q\13\315\364IK\224D\321X"
    "\1r\11\315\364)%\245\230\23s\12\315\364I\313\232\354(\0t\13\315\364\201\60\333\302\64\7\1u"
    "\15\315\364)Q\22%\211\224\344 \0v\13\315\364)Q\22%\265\34\6w\13\315\364)\25%Q\272"
    "\203\0x\12\315\364)Q\244Iu\20y\15\315\364)Q\22%Q\64F\22\0z\12\315\364)CV"
    "\33r\20{\12\315\364\212\265\64\254&\0|\7\315\264\302~\7}\12\315t\322\260\232\205\265\14~\11"
    "\315\364II;\13\0\177\6\315\364\371\6\0\0\0\4\377\377\0";

#define FONT_HEIGHT (8u)

typedef float (*ValueConverter)(FuriHalAdcHandle* handle, uint16_t value);

typedef struct {
    const GpioPinRecord* pin;
    float value;
    ValueConverter converter;
    const char* suffix;
} DataItem;

typedef struct {
    size_t count;
    DataItem* items;
} Data;

const GpioPinRecord item_vref = {.name = "VREF", .channel = FuriHalAdcChannelVREFINT};
const GpioPinRecord item_temp = {.name = "TEMP", .channel = FuriHalAdcChannelTEMPSENSOR};
const GpioPinRecord item_vbat = {.name = "VBAT", .channel = FuriHalAdcChannelVBAT};

static void app_draw_callback(Canvas* canvas, void* ctx) {
    furi_assert(ctx);
    Data* data = ctx;

    canvas_set_custom_u8g2_font(canvas, font);
    char buffer[64];
    int32_t x = 0, y = FONT_HEIGHT;
    for(size_t i = 0; i < data->count; i++) {
        if(i == canvas_height(canvas) / FONT_HEIGHT) {
            x = 64;
            y = FONT_HEIGHT;
        }

        snprintf(
            buffer,
            sizeof(buffer),
            "%4s: %4.0f%s\n",
            data->items[i].pin->name,
            (double)data->items[i].value,
            data->items[i].suffix);
        canvas_draw_str(canvas, x, y, buffer);
        y += FONT_HEIGHT;
    }
}

static void app_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

int32_t example_adc_main(void* p) {
    UNUSED(p);

    // Data
    Data data = {};
    for(size_t i = 0; i < gpio_pins_count; i++) {
        if(gpio_pins[i].channel != FuriHalAdcChannelNone) {
            data.count++;
        }
    }
    data.count += 3; // Special channels
    data.items = malloc(data.count * sizeof(DataItem));
    size_t item_pos = 0;
    for(size_t i = 0; i < gpio_pins_count; i++) {
        if(gpio_pins[i].channel != FuriHalAdcChannelNone) {
            furi_hal_gpio_init(gpio_pins[i].pin, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
            data.items[item_pos].pin = &gpio_pins[i];
            data.items[item_pos].converter = furi_hal_adc_convert_to_voltage;
            data.items[item_pos].suffix = "mV";
            item_pos++;
        }
    }
    data.items[item_pos].pin = &item_vref;
    data.items[item_pos].converter = furi_hal_adc_convert_vref;
    data.items[item_pos].suffix = "mV";
    item_pos++;
    data.items[item_pos].pin = &item_temp;
    data.items[item_pos].converter = furi_hal_adc_convert_temp;
    data.items[item_pos].suffix = "C";
    item_pos++;
    data.items[item_pos].pin = &item_vbat;
    data.items[item_pos].converter = furi_hal_adc_convert_vbat;
    data.items[item_pos].suffix = "mV";
    item_pos++;
    furi_assert(item_pos == data.count);

    // Alloc message queue
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    // Configure view port
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, app_draw_callback, &data);
    view_port_input_callback_set(view_port, app_input_callback, event_queue);

    // Register view port in GUI
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Initialize ADC
    FuriHalAdcHandle* adc_handle = furi_hal_adc_acquire();
    furi_hal_adc_configure(adc_handle);

    // Process events
    InputEvent event;
    bool running = true;
    while(running) {
        if(furi_message_queue_get(event_queue, &event, 100) == FuriStatusOk) {
            if(event.type == InputTypePress && event.key == InputKeyBack) {
                running = false;
            }
        } else {
            for(size_t i = 0; i < data.count; i++) {
                data.items[i].value = data.items[i].converter(
                    adc_handle, furi_hal_adc_read(adc_handle, data.items[i].pin->channel));
            }
            view_port_update(view_port);
        }
    }

    furi_hal_adc_release(adc_handle);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    furi_record_close(RECORD_GUI);
    free(data.items);

    return 0;
}

=== END: applications/examples/example_adc/example_adc.c ===

=== BEGIN: applications/examples/example_apps_assets/README.md ===
# Apps Assets folder Example {#example_app_assets}

This example shows how to use the Apps Assets folder to store data that is not part of the application itself, but is required for its operation, and that data is provided with the application.

## Source code

Source code for this example can be found [here](https://github.com/flipperdevices/flipperzero-firmware/tree/dev/applications/examples/example_apps_assets).

## What is the Apps Assets Folder?

The **Apps Assets** folder is a folder where external applications unpack their assets.

The path to the current application folder is related to the `appid` of the app. The `appid` is used to identify the app in the app store and is stored in the `application.fam` file. 
The Apps Assets folder is located only on the external storage, the SD card.

For example, if the `appid` of the app is `snake_game`, the path to the Apps Assets folder will be `/ext/apps_assets/snake_game`. But using raw paths is not recommended, because the path to the Apps Assets folder can change in the future. Use the `/assets` alias instead.

## How to get the path to the Apps Assets folder?

You can use `/assets` alias to get the path to the current application data folder. For example, if you want to open a file `database.txt` in the Apps Assets folder, you can use the next path: `/data/database.txt`. But this way is not recommended, because even the `/assets` alias can change in the future.

We recommend to use the `APP_ASSETS_PATH` macro to get the path to the Apps Assets folder. For example, if you want to open a file `database.txt` in the Apps Assets folder, you can use the next path: `APP_ASSETS_PATH("database.txt")`.

## What is the difference between the Apps Assets folder and the Apps Data folder?

The Apps Assets folder is used to store the data <u>provided</u> with the application. For example, if you want to create a game, you can store game levels (content data) in the Apps Assets folder.

The Apps Data folder is used to store data <u>generated</u> by the application. For example, if you want to create a game, you can save the progress of the game (user-generated data) in the Apps Data folder.

## How to provide the data with the app?

To provide data with an application, you need to create a folder inside your application folder (eg "files") and place the data in it. After that, you need to add `fap_file_assets="files"` to your application.fam file.

For example, if you want to provide game levels with the application, you need to create a "levels" folder inside the "files" folder and put the game levels in it. After that, you need to add `fap_file_assets="files"` to your application.fam file. The final application folder structure will look like this:

```
snake_game
├── application.fam
├── snake_game.c
└── files
    └── levels
        ├── level1.txt
        ├── level2.txt
        └── level3.txt
```

When app is launched, the `files` folder will be unpacked to the Apps Assets folder. The final structure of the Apps Assets folder will look like this:

```
/assets
├── .assets.signature
└── levels
    ├── level1.txt
    ├── level2.txt
    └── level3.txt
```

## When will the data be unpacked?

The data is unpacked when the application starts, if the application is launched for the first time, or if the data within the application is updated.

When an application is compiled, the contents of the "files" folder are hashed and stored within the application itself. When the application starts, this hash is compared to the hash stored in the `.assets.signature` file. If the hashes differ or the `.assets.signature` file does not exist, the application folder is deleted and the new data is unpacked.

=== END: applications/examples/example_apps_assets/README.md ===

=== BEGIN: applications/examples/example_apps_assets/application.fam ===
App(
    appid="example_apps_assets",
    name="Example: Apps Assets",
    apptype=FlipperAppType.EXTERNAL,
    entry_point="example_apps_assets_main",
    requires=["gui"],
    stack_size=4 * 1024,
    fap_category="Examples",
    fap_file_assets="files",
)

=== END: applications/examples/example_apps_assets/application.fam ===

=== BEGIN: applications/examples/example_apps_assets/example_apps_assets.c ===
/**
 * @file example_apps_assets.c
 * @brief Application assets example.
 */
#include <furi.h>
#include <storage/storage.h>
#include <toolbox/stream/stream.h>
#include <toolbox/stream/file_stream.h>

// Define log tag
#define TAG "ExampleAppsAssets"

static void example_apps_data_print_file_content(Storage* storage, const char* path) {
    Stream* stream = file_stream_alloc(storage);
    FuriString* line = furi_string_alloc();

    FURI_LOG_I(TAG, "----------------------------------------");
    FURI_LOG_I(TAG, "File \"%s\" content:", path);
    if(file_stream_open(stream, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        while(stream_read_line(stream, line)) {
            furi_string_replace_all(line, "\r", "");
            furi_string_replace_all(line, "\n", "");
            FURI_LOG_I(TAG, "%s", furi_string_get_cstr(line));
        }
    } else {
        FURI_LOG_E(TAG, "Failed to open file");
    }
    FURI_LOG_I(TAG, "----------------------------------------");

    furi_string_free(line);
    file_stream_close(stream);
    stream_free(stream);
}

// Application entry point
int32_t example_apps_assets_main(void* p) {
    // Mark argument as unused
    UNUSED(p);

    // Open storage
    Storage* storage = furi_record_open(RECORD_STORAGE);

    example_apps_data_print_file_content(storage, APP_ASSETS_PATH("test_asset.txt"));
    example_apps_data_print_file_content(storage, APP_ASSETS_PATH("poems/a jelly-fish.txt"));
    example_apps_data_print_file_content(storage, APP_ASSETS_PATH("poems/theme in yellow.txt"));
    example_apps_data_print_file_content(storage, APP_ASSETS_PATH("poems/my shadow.txt"));

    // Close storage
    furi_record_close(RECORD_STORAGE);

    return 0;
}

=== END: applications/examples/example_apps_assets/example_apps_assets.c ===

=== BEGIN: applications/examples/example_apps_assets/files/poems/a jelly-fish.txt ===
A Jelly-Fish by Marianne Moore

Visible, invisible,
A fluctuating charm,
An amber-colored amethyst
Inhabits it; your arm
Approaches, and
It opens and
It closes;
You have meant
To catch it,
And it shrivels;
You abandon
Your intent—
It opens, and it
Closes and you
Reach for it—
The blue
Surrounding it
Grows cloudy, and
It floats away
From you.

source: "https://poets.org/anthology/poems-your-poetry-project-public-domain"
=== END: applications/examples/example_apps_assets/files/poems/a jelly-fish.txt ===

=== BEGIN: applications/examples/example_apps_assets/files/poems/my shadow.txt ===
My Shadow by Robert Louis Stevenson

I have a little shadow that goes in and out with me,
And what can be the use of him is more than I can see.
He is very, very like me from the heels up to the head;
And I see him jump before me, when I jump into my bed.

The funniest thing about him is the way he likes to grow—
Not at all like proper children, which is always very slow;
For he sometimes shoots up taller like an India-rubber ball,
And he sometimes gets so little that there’s none of him at all.

He hasn’t got a notion of how children ought to play,
And can only make a fool of me in every sort of way.
He stays so close beside me, he’s a coward you can see;
I’d think shame to stick to nursie as that shadow sticks to me!

One morning, very early, before the sun was up,
I rose and found the shining dew on every buttercup;
But my lazy little shadow, like an arrant sleepy-head,
Had stayed at home behind me and was fast asleep in bed.

source: "https://poets.org/anthology/poems-your-poetry-project-public-domain"
=== END: applications/examples/example_apps_assets/files/poems/my shadow.txt ===

=== BEGIN: applications/examples/example_apps_assets/files/poems/theme in yellow.txt ===
Theme in Yellow by Carl Sandburg

I spot the hills 
With yellow balls in autumn. 
I light the prairie cornfields 
Orange and tawny gold clusters 
And I am called pumpkins. 
On the last of October 
When dusk is fallen 
Children join hands 
And circle round me 
Singing ghost songs 
And love to the harvest moon; 
I am a jack-o'-lantern 
With terrible teeth 
And the children know 
I am fooling.

source: "https://poets.org/anthology/poems-your-poetry-project-public-domain"
=== END: applications/examples/example_apps_assets/files/poems/theme in yellow.txt ===

=== BEGIN: applications/examples/example_apps_assets/files/test_asset.txt ===
## This is test file content
=== END: applications/examples/example_apps_assets/files/test_asset.txt ===

=== BEGIN: applications/examples/example_apps_data/README.md ===
# Apps Data folder Example {#example_app_data}

This example demonstrates how to utilize the Apps Data folder to store data that is not part of the app itself, such as user data, configuration files, and so forth.

## Source code

Source code for this example can be found [here](https://github.com/flipperdevices/flipperzero-firmware/tree/dev/applications/examples/example_apps_data).

## What is the Apps Data Folder?

The **Apps Data** folder is a folder used to store data for external apps that are not part of the main firmware. 

The path to the current application folder is related to the `appid` of the app. The `appid` is used to identify the app in the app store and is stored in the `application.fam` file. 
The Apps Data folder is located only on the external storage, the SD card.

For example, if the `appid` of the app is `snake_game`, the path to the Apps Data folder will be `/ext/apps_data/snake_game`. But using raw paths is not recommended, because the path to the Apps Data folder can change in the future. Use the `/data` alias instead.

## How to get the path to the Apps Data folder?

You can use `/data` alias to get the path to the current application data folder. For example, if you want to open a file `config.txt` in the Apps Data folder, you can use the next path: `/data/config.txt`. But this way is not recommended, because even the `/data` alias can change in the future.

We recommend to use the `APP_DATA_PATH` macro to get the path to the Apps Data folder. For example, if you want to open a file `config.txt` in the Apps Data folder, you can use the next path: `APP_DATA_PATH("config.txt")`.

## What is the difference between the Apps Assets folder and the Apps Data folder?

The Apps Assets folder is used to store the data <u>provided</u> with the application. For example, if you want to create a game, you can store game levels (content data) in the Apps Assets folder.

The Apps Data folder is used to store data <u>generated</u> by the application. For example, if you want to create a game, you can save the progress of the game (user-generated data) in the Apps Data folder.

=== END: applications/examples/example_apps_data/README.md ===

=== BEGIN: applications/examples/example_apps_data/application.fam ===
App(
    appid="example_apps_data",
    name="Example: Apps Data",
    apptype=FlipperAppType.EXTERNAL,
    entry_point="example_apps_data_main",
    requires=["gui"],
    stack_size=1 * 1024,
    fap_category="Examples",
)

=== END: applications/examples/example_apps_data/application.fam ===

=== BEGIN: applications/examples/example_apps_data/example_apps_data.c ===
/**
 * @file example_apps_data.c
 * @brief Application data example.
 */
#include <furi.h>
#include <storage/storage.h>

// Define log tag
#define TAG "ExampleAppsData"

// Application entry point
int32_t example_apps_data_main(void* p) {
    // Mark argument as unused
    UNUSED(p);

    // Open storage
    Storage* storage = furi_record_open(RECORD_STORAGE);

    // Allocate file
    File* file = storage_file_alloc(storage);

    // Get the path to the current application data folder
    // That is: /ext/apps_data/<app_name>
    // And it will create folders in the path if they don't exist
    // In this example it will create /ext/apps_data/example_apps_data
    // And file will be /ext/apps_data/example_apps_data/test.txt

    // Open file, write data and close it
    if(!storage_file_open(file, APP_DATA_PATH("test.txt"), FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        FURI_LOG_E(TAG, "Failed to open file");
    }
    if(!storage_file_write(file, "Hello World!", strlen("Hello World!"))) {
        FURI_LOG_E(TAG, "Failed to write to file");
    }
    storage_file_close(file);

    // Deallocate file
    storage_file_free(file);

    // Close storage
    furi_record_close(RECORD_STORAGE);

    return 0;
}

=== END: applications/examples/example_apps_data/example_apps_data.c ===

=== BEGIN: applications/examples/example_ble_beacon/application.fam ===
App(
    appid="example_ble_beacon",
    name="Example: BLE Beacon",
    apptype=FlipperAppType.EXTERNAL,
    entry_point="ble_beacon_app",
    requires=["gui"],
    stack_size=1 * 1024,
    fap_icon="example_ble_beacon_10px.png",
    fap_category="Examples",
    fap_icon_assets="images",
)

=== END: applications/examples/example_ble_beacon/application.fam ===

=== BEGIN: applications/examples/example_ble_beacon/ble_beacon_app.c ===
#include "ble_beacon_app.h"

#include <extra_beacon.h>
#include <furi_hal_version.h>

#include <string.h>

#define TAG "BleBeaconApp"

static bool ble_beacon_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    BleBeaconApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool ble_beacon_app_back_event_callback(void* context) {
    furi_assert(context);
    BleBeaconApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void ble_beacon_app_tick_event_callback(void* context) {
    furi_assert(context);
    BleBeaconApp* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

static void ble_beacon_app_restore_beacon_state(BleBeaconApp* app) {
    // Restore beacon data from service
    GapExtraBeaconConfig* local_config = &app->beacon_config;
    const GapExtraBeaconConfig* config = furi_hal_bt_extra_beacon_get_config();
    if(config) {
        // We have a config, copy it
        memcpy(local_config, config, sizeof(app->beacon_config));
    } else {
        // No config, set up default values - they will stay until overriden or device is reset
        local_config->min_adv_interval_ms = 50;
        local_config->max_adv_interval_ms = 150;

        local_config->adv_channel_map = GapAdvChannelMapAll;
        local_config->adv_power_level = GapAdvPowerLevel_0dBm;

        local_config->address_type = GapAddressTypePublic;
        memcpy(
            local_config->address, furi_hal_version_get_ble_mac(), sizeof(local_config->address));
        // Modify MAC address to make it different from the one used by the main app
        local_config->address[0] ^= 0xFF;
        local_config->address[3] ^= 0xFF;

        furi_check(furi_hal_bt_extra_beacon_set_config(local_config));
    }

    // Get beacon state
    app->is_beacon_active = furi_hal_bt_extra_beacon_is_active();

    // Restore last beacon data
    app->beacon_data_len = furi_hal_bt_extra_beacon_get_data(app->beacon_data);
}

static BleBeaconApp* ble_beacon_app_alloc(void) {
    BleBeaconApp* app = malloc(sizeof(BleBeaconApp));

    app->gui = furi_record_open(RECORD_GUI);

    app->scene_manager = scene_manager_alloc(&ble_beacon_app_scene_handlers, app);
    app->view_dispatcher = view_dispatcher_alloc();

    app->status_string = furi_string_alloc();

    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, ble_beacon_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, ble_beacon_app_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, ble_beacon_app_tick_event_callback, 100);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, BleBeaconAppViewSubmenu, submenu_get_view(app->submenu));

    app->dialog_ex = dialog_ex_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, BleBeaconAppViewDialog, dialog_ex_get_view(app->dialog_ex));

    app->byte_input = byte_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, BleBeaconAppViewByteInput, byte_input_get_view(app->byte_input));

    ble_beacon_app_restore_beacon_state(app);

    return app;
}

static void ble_beacon_app_free(BleBeaconApp* app) {
    view_dispatcher_remove_view(app->view_dispatcher, BleBeaconAppViewByteInput);
    view_dispatcher_remove_view(app->view_dispatcher, BleBeaconAppViewSubmenu);
    view_dispatcher_remove_view(app->view_dispatcher, BleBeaconAppViewDialog);

    free(app->byte_input);
    free(app->submenu);
    free(app->dialog_ex);

    free(app->scene_manager);
    free(app->view_dispatcher);

    free(app->status_string);

    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_GUI);
    app->gui = NULL;

    free(app);
}

int32_t ble_beacon_app(void* args) {
    UNUSED(args);

    BleBeaconApp* app = ble_beacon_app_alloc();

    scene_manager_next_scene(app->scene_manager, BleBeaconAppSceneRunBeacon);

    view_dispatcher_run(app->view_dispatcher);

    ble_beacon_app_free(app);
    return 0;
}

void ble_beacon_app_update_state(BleBeaconApp* app) {
    furi_hal_bt_extra_beacon_stop();

    furi_check(furi_hal_bt_extra_beacon_set_config(&app->beacon_config));

    app->beacon_data_len = 0;
    while((app->beacon_data[app->beacon_data_len] != 0) &&
          (app->beacon_data_len < sizeof(app->beacon_data))) {
        app->beacon_data_len++;
    }

    FURI_LOG_I(TAG, "beacon_data_len: %d", app->beacon_data_len);

    furi_check(furi_hal_bt_extra_beacon_set_data(app->beacon_data, app->beacon_data_len));

    if(app->is_beacon_active) {
        furi_check(furi_hal_bt_extra_beacon_start());
    }
}

=== END: applications/examples/example_ble_beacon/ble_beacon_app.c ===

=== BEGIN: applications/examples/example_ble_beacon/ble_beacon_app.h ===
/**
 * @file ble_beacon_app.h
 * @brief BLE beacon example.
 */
#pragma once

#include "extra_beacon.h"
#include <furi.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/scene_manager.h>
#include <gui/view_dispatcher.h>

#include <gui/modules/widget.h>
#include <gui/modules/submenu.h>
#include <gui/modules/byte_input.h>
#include <gui/modules/dialog_ex.h>

#include <rpc/rpc_app.h>
#include <notification/notification_messages.h>

#include <furi_hal_bt.h>

#include "scenes/scenes.h"
#include <stdint.h>

typedef struct {
    Gui* gui;
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;

    Submenu* submenu;
    ByteInput* byte_input;
    DialogEx* dialog_ex;

    FuriString* status_string;

    GapExtraBeaconConfig beacon_config;
    uint8_t beacon_data[EXTRA_BEACON_MAX_DATA_SIZE];
    uint8_t beacon_data_len;
    bool is_beacon_active;
} BleBeaconApp;

typedef enum {
    BleBeaconAppViewSubmenu,
    BleBeaconAppViewByteInput,
    BleBeaconAppViewDialog,
} BleBeaconAppView;

typedef enum {
    BleBeaconAppCustomEventDataEditResult = 100,
} BleBeaconAppCustomEvent;

void ble_beacon_app_update_state(BleBeaconApp* app);

=== END: applications/examples/example_ble_beacon/ble_beacon_app.h ===

=== BEGIN: applications/examples/example_ble_beacon/scenes/scene_config.h ===
ADD_SCENE(ble_beacon_app, menu, Menu)
ADD_SCENE(ble_beacon_app, input_mac_addr, InputMacAddress)
ADD_SCENE(ble_beacon_app, input_beacon_data, InputBeaconData)
ADD_SCENE(ble_beacon_app, run_beacon, RunBeacon)

=== END: applications/examples/example_ble_beacon/scenes/scene_config.h ===

=== BEGIN: applications/examples/example_ble_beacon/scenes/scene_input_beacon_data.c ===
#include "../ble_beacon_app.h"

static void ble_beacon_app_scene_add_type_byte_input_callback(void* context) {
    BleBeaconApp* ble_beacon = context;
    view_dispatcher_send_custom_event(
        ble_beacon->view_dispatcher, BleBeaconAppCustomEventDataEditResult);
}

void ble_beacon_app_scene_input_beacon_data_on_enter(void* context) {
    BleBeaconApp* ble_beacon = context;
    byte_input_set_header_text(ble_beacon->byte_input, "Enter beacon data");

    byte_input_set_result_callback(
        ble_beacon->byte_input,
        ble_beacon_app_scene_add_type_byte_input_callback,
        NULL,
        context,
        ble_beacon->beacon_data,
        sizeof(ble_beacon->beacon_data));

    view_dispatcher_switch_to_view(ble_beacon->view_dispatcher, BleBeaconAppViewByteInput);
}

bool ble_beacon_app_scene_input_beacon_data_on_event(void* context, SceneManagerEvent event) {
    BleBeaconApp* ble_beacon = context;
    SceneManager* scene_manager = ble_beacon->scene_manager;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == BleBeaconAppCustomEventDataEditResult) {
            ble_beacon_app_update_state(ble_beacon);
            scene_manager_previous_scene(scene_manager);
            return true;
        }
    }

    return false;
}

void ble_beacon_app_scene_input_beacon_data_on_exit(void* context) {
    BleBeaconApp* ble_beacon = context;

    byte_input_set_result_callback(ble_beacon->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(ble_beacon->byte_input, NULL);
}

=== END: applications/examples/example_ble_beacon/scenes/scene_input_beacon_data.c ===

=== BEGIN: applications/examples/example_ble_beacon/scenes/scene_input_mac_addr.c ===
#include "../ble_beacon_app.h"

static void ble_beacon_app_scene_add_type_byte_input_callback(void* context) {
    BleBeaconApp* ble_beacon = context;
    view_dispatcher_send_custom_event(
        ble_beacon->view_dispatcher, BleBeaconAppCustomEventDataEditResult);
}

void ble_beacon_app_scene_input_mac_addr_on_enter(void* context) {
    BleBeaconApp* ble_beacon = context;
    byte_input_set_header_text(ble_beacon->byte_input, "Enter MAC (reversed)");

    byte_input_set_result_callback(
        ble_beacon->byte_input,
        ble_beacon_app_scene_add_type_byte_input_callback,
        NULL,
        context,
        ble_beacon->beacon_config.address,
        sizeof(ble_beacon->beacon_config.address));

    view_dispatcher_switch_to_view(ble_beacon->view_dispatcher, BleBeaconAppViewByteInput);
}

bool ble_beacon_app_scene_input_mac_addr_on_event(void* context, SceneManagerEvent event) {
    BleBeaconApp* ble_beacon = context;
    SceneManager* scene_manager = ble_beacon->scene_manager;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == BleBeaconAppCustomEventDataEditResult) {
            ble_beacon_app_update_state(ble_beacon);
            scene_manager_previous_scene(scene_manager);
            return true;
        }
    }

    return false;
}

void ble_beacon_app_scene_input_mac_addr_on_exit(void* context) {
    BleBeaconApp* ble_beacon = context;

    byte_input_set_result_callback(ble_beacon->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(ble_beacon->byte_input, NULL);
}

=== END: applications/examples/example_ble_beacon/scenes/scene_input_mac_addr.c ===

=== BEGIN: applications/examples/example_ble_beacon/scenes/scene_menu.c ===
#include "../ble_beacon_app.h"

enum SubmenuIndex {
    SubmenuIndexSetMac,
    SubmenuIndexSetData,
};

static void ble_beacon_app_scene_menu_submenu_callback(void* context, uint32_t index) {
    BleBeaconApp* ble_beacon = context;
    view_dispatcher_send_custom_event(ble_beacon->view_dispatcher, index);
}

void ble_beacon_app_scene_menu_on_enter(void* context) {
    BleBeaconApp* ble_beacon = context;
    Submenu* submenu = ble_beacon->submenu;

    submenu_add_item(
        submenu,
        "Set MAC",
        SubmenuIndexSetMac,
        ble_beacon_app_scene_menu_submenu_callback,
        ble_beacon);
    submenu_add_item(
        submenu,
        "Set Data",
        SubmenuIndexSetData,
        ble_beacon_app_scene_menu_submenu_callback,
        ble_beacon);

    view_dispatcher_switch_to_view(ble_beacon->view_dispatcher, BleBeaconAppViewSubmenu);
}

bool ble_beacon_app_scene_menu_on_event(void* context, SceneManagerEvent event) {
    BleBeaconApp* ble_beacon = context;
    SceneManager* scene_manager = ble_beacon->scene_manager;

    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        const uint32_t submenu_index = event.event;
        if(submenu_index == SubmenuIndexSetMac) {
            scene_manager_next_scene(scene_manager, BleBeaconAppSceneInputMacAddress);
            consumed = true;
        } else if(submenu_index == SubmenuIndexSetData) {
            scene_manager_next_scene(scene_manager, BleBeaconAppSceneInputBeaconData);
            consumed = true;
        }
    }

    return consumed;
}

void ble_beacon_app_scene_menu_on_exit(void* context) {
    BleBeaconApp* ble_beacon = context;
    submenu_reset(ble_beacon->submenu);
}

=== END: applications/examples/example_ble_beacon/scenes/scene_menu.c ===

=== BEGIN: applications/examples/example_ble_beacon/scenes/scene_run_beacon.c ===
#include "../ble_beacon_app.h"
#include <example_ble_beacon_icons.h>

static void
    ble_beacon_app_scene_run_beacon_confirm_dialog_callback(DialogExResult result, void* context) {
    BleBeaconApp* ble_beacon = context;

    view_dispatcher_send_custom_event(ble_beacon->view_dispatcher, result);
}

static void update_status_text(BleBeaconApp* ble_beacon) {
    DialogEx* dialog_ex = ble_beacon->dialog_ex;

    dialog_ex_set_header(dialog_ex, "BLE Beacon Demo", 64, 0, AlignCenter, AlignTop);

    FuriString* status = ble_beacon->status_string;

    furi_string_reset(status);

    furi_string_cat_str(status, "Status: ");
    if(ble_beacon->is_beacon_active) {
        furi_string_cat_str(status, "Running\n");
    } else {
        furi_string_cat_str(status, "Stopped\n");
    }

    // Output MAC in reverse order
    for(int i = sizeof(ble_beacon->beacon_config.address) - 1; i >= 0; i--) {
        furi_string_cat_printf(status, "%02X", ble_beacon->beacon_config.address[i]);
        if(i > 0) {
            furi_string_cat_str(status, ":");
        }
    }

    furi_string_cat_printf(status, "\nData length: %d", ble_beacon->beacon_data_len);

    dialog_ex_set_text(dialog_ex, furi_string_get_cstr(status), 0, 29, AlignLeft, AlignCenter);

    dialog_ex_set_icon(dialog_ex, 93, 20, &I_lighthouse_35x44);

    dialog_ex_set_left_button_text(dialog_ex, "Config");

    dialog_ex_set_center_button_text(dialog_ex, ble_beacon->is_beacon_active ? "Stop" : "Start");

    dialog_ex_set_result_callback(
        dialog_ex, ble_beacon_app_scene_run_beacon_confirm_dialog_callback);
    dialog_ex_set_context(dialog_ex, ble_beacon);
}

void ble_beacon_app_scene_run_beacon_on_enter(void* context) {
    BleBeaconApp* ble_beacon = context;

    update_status_text(ble_beacon);

    view_dispatcher_switch_to_view(ble_beacon->view_dispatcher, BleBeaconAppViewDialog);
}

bool ble_beacon_app_scene_run_beacon_on_event(void* context, SceneManagerEvent event) {
    BleBeaconApp* ble_beacon = context;
    SceneManager* scene_manager = ble_beacon->scene_manager;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == DialogExResultLeft) {
            scene_manager_next_scene(scene_manager, BleBeaconAppSceneMenu);
            return true;
        } else if(event.event == DialogExResultCenter) {
            ble_beacon->is_beacon_active = !ble_beacon->is_beacon_active;
            ble_beacon_app_update_state(ble_beacon);
            update_status_text(ble_beacon);
            return true;
        }
    }
    return false;
}

void ble_beacon_app_scene_run_beacon_on_exit(void* context) {
    BleBeaconApp* ble_beacon = context;
    UNUSED(ble_beacon);
}

=== END: applications/examples/example_ble_beacon/scenes/scene_run_beacon.c ===

=== BEGIN: applications/examples/example_ble_beacon/scenes/scenes.c ===
#include "scenes.h"

// Generate scene on_enter handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_enter,
void (*const ble_beacon_app_on_enter_handlers[])(void*) = {
#include "scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_event handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_event,
bool (*const ble_beacon_app_on_event_handlers[])(void* context, SceneManagerEvent event) = {
#include "scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_exit handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_exit,
void (*const ble_beacon_app_on_exit_handlers[])(void* context) = {
#include "scene_config.h"
};
#undef ADD_SCENE

// Initialize scene handlers configuration structure
const SceneManagerHandlers ble_beacon_app_scene_handlers = {
    .on_enter_handlers = ble_beacon_app_on_enter_handlers,
    .on_event_handlers = ble_beacon_app_on_event_handlers,
    .on_exit_handlers = ble_beacon_app_on_exit_handlers,
    .scene_num = BleBeaconAppSceneNum,
};

=== END: applications/examples/example_ble_beacon/scenes/scenes.c ===

=== BEGIN: applications/examples/example_ble_beacon/scenes/scenes.h ===
#pragma once

#include <gui/scene_manager.h>

// Generate scene id and total number
#define ADD_SCENE(prefix, name, id) BleBeaconAppScene##id,
typedef enum {
#include "scene_config.h"
    BleBeaconAppSceneNum,
} BleBeaconAppScene;
#undef ADD_SCENE

extern const SceneManagerHandlers ble_beacon_app_scene_handlers;

// Generate scene on_enter handlers declaration
#define ADD_SCENE(prefix, name, id) void prefix##_scene_##name##_on_enter(void*);
#include "scene_config.h"
#undef ADD_SCENE

// Generate scene on_event handlers declaration
#define ADD_SCENE(prefix, name, id) \
    bool prefix##_scene_##name##_on_event(void* context, SceneManagerEvent event);
#include "scene_config.h"
#undef ADD_SCENE

// Generate scene on_exit handlers declaration
#define ADD_SCENE(prefix, name, id) void prefix##_scene_##name##_on_exit(void* context);
#include "scene_config.h"
#undef ADD_SCENE

=== END: applications/examples/example_ble_beacon/scenes/scenes.h ===

=== BEGIN: applications/examples/example_custom_font/application.fam ===
App(
    appid="example_custom_font",
    name="Example: custom font",
    apptype=FlipperAppType.EXTERNAL,
    entry_point="example_custom_font_main",
    requires=["gui"],
    stack_size=1 * 1024,
    fap_category="Examples",
)

=== END: applications/examples/example_custom_font/application.fam ===

=== BEGIN: applications/examples/example_custom_font/example_custom_font.c ===
/**
 * @file example_custom_font.c
 * @brief Custom font example.
 */
#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <input/input.h>

//This arrays contains the font itself. You can use any u8g2 font you want

/*
  Fontname: -Misc-Fixed-Medium-R-Normal--6-60-75-75-C-40-ISO10646-1
  Copyright: Public domain font.  Share and enjoy.
  Glyphs: 191/919
  BBX Build Mode: 0
*/
const uint8_t u8g2_font_4x6_t_cyrillic[] =
    "\277\0\2\2\3\3\2\4\4\4\6\0\377\5\377\5\377\0\356\1\334\2\301 \5\200\315\0!\6\351\310"
    "\254\0\42\6\223\313$\25#\12\254\310\244\64T\32*\1$\11\263\307\245\241\301H\11%\10\253\310d"
    "\324F\1&\11\254\310\305\24\253\230\2'\5\321\313\10(\7\362\307\251f\0)\10\262\307\304T)\0"
    "*\7\253\310\244j\65+\10\253\310\305\264b\2,\6\222\307)\0-\5\213\312\14.\5\311\310\4/"
    "\7\253\310Ve\4\60\10\253\310UCU\0\61\7\253\310%Y\15\62\7\253\310\65S\32\63\10\253\310"
    "\314\224\301\2\64\10\253\310$\65b\1\65\10\253\310\214\250\301\2\66\7\253\310M\325\2\67\10\253\310\314"
    "TF\0\70\7\253\310\255\326\2\71\7\253\310\265\344\2:\6\341\310\304\0;\7\252\307e\250\0<\10"
    "\253\310\246\32d\20=\6\233\311l\60>\11\253\310d\220A*\1\77\11\253\310h\220\62L\0@\7"
    "\253\310-\33\10A\10\253\310UC\251\0B\10\253\310\250\264\322\2C\10\253\310U\62U\0D\10\253"
    "\310\250d-\0E\10\253\310\214\250\342\0F\10\253\310\214\250b\4G\10\253\310\315\244\222\0H\10\253"
    "\310$\65\224\12I\7\253\310\254X\15J\7\253\310\226\252\2K\10\253\310$\265\222\12L\7\253\310\304"
    "\346\0M\10\253\310\244\61\224\12N\10\253\310\252\241$\0O\7\253\310UV\5P\10\253\310\250\264b"
    "\4Q\10\263\307UV\15\2R\10\253\310\250\264\222\12S\10\253\310m\220\301\2T\7\253\310\254\330\2"
    "U\7\253\310$\327\10V\10\253\310$k\244\4W\10\253\310$\65\206\12X\10\253\310$\325R\1Y"
    "\10\253\310$UV\0Z\7\253\310\314T\16[\6\352\310\254J\134\11\253\310\304\14\62\210\1]\6\252"
    "\310\250j^\5\223\313\65_\5\213\307\14`\6\322\313\304\0a\7\243\310-\225\4b\10\253\310D\225"
    "\324\2c\7\243\310\315\14\4d\10\253\310\246\245\222\0e\6\243\310USf\10\253\310\246\264b\2g"
    "\10\253\307\255$\27\0h\10\253\310D\225\254\0i\10\253\310e$\323\0j\10\263\307fX.\0k"
    "\10\253\310\304\264\222\12l\7\253\310\310\326\0m\10\243\310\244\241T\0n\7\243\310\250d\5o\7\243"
    "\310U\252\2p\10\253\307\250\264b\4q\10\253\307-\225d\0r\10\243\310\244\25#\0s\10\243\310"
    "\215\14\26\0t\10\253\310\245\25\63\10u\7\243\310$+\11v\7\243\310$\253\2w\10\243\310$\65"
    "T\0x\7\243\310\244\62\25y\10\253\307$\225\344\2z\7\243\310\314\224\6{\10\263\307\246$k\20"
    "|\6\351\310\14\1}\11\263\307d\20UL\21~\7\224\313%\225\0\0\0\0\4\377\377\4\1\11\253"
    "\310\244\261\342\0\4\2\11\253\310\214\250\222\12\4\3\10\253\310\16Y\2\4\4\11\253\310M\225\201\0\4"
    "\5\11\253\310m\220\301\2\4\6\10\253\310\254X\15\4\7\11\253\310\244\221b\32\4\10\10\253\310\226\252"
    "\2\4\11\11\254\310L\325Z\2\4\12\11\254\310\244\326JK\4\13\11\253\310\250\250\222\12\4\14\10\253"
    "\310\312\264\12\4\16\11\263\307\244\32u\2\4\17\11\263\307$\327H\11\4\20\11\253\310UC\251\0\4"
    "\21\11\253\310\214\250\322\2\4\22\11\253\310\250\264\322\2\4\23\10\253\310\214\330\4\4\24\11\263\307\254\245"
    "\206\12\4\25\11\253\310\214\250\342\0\4\26\12\253\310\244\221\322H\1\4\27\12\253\310h\220\62X\0\4"
    "\30\11\253\310\304\64T\14\4\31\11\263\307\315\64T\14\4\32\11\253\310$\265\222\12\4\33\10\253\310-"
    "W\0\4\34\11\253\310\244\241\254\0\4\35\11\253\310$\65\224\12\4\36\10\253\310UV\5\4\37\10\253"
    "\310\214\344\12\4 \11\253\310\250\264b\4\4!\11\253\310U\62U\0\4\42\10\253\310\254\330\2\4#"
    "\11\263\307$\253L\21\4$\12\253\310\245\221FJ\0\4%\11\253\310$\325R\1\4&\10\253\310$"
    "\327\10\4'\11\253\310$\225d\1\4(\11\253\310$\65\216\0\4)\12\264\307\244\326#\203\0\4*"
    "\13\254\310h\220\201LI\1\4+\12\254\310D\271\324H\1\4,\11\253\310\304\250\322\2\4-\11\253"
    "\310h\220\344\2\4.\12\254\310\244\244.\225\0\4/\11\253\310\255\264T\0\4\60\10\243\310-\225\4"
    "\4\61\11\253\310\315\221*\0\4\62\11\243\310\14\225\26\0\4\63\10\243\310\214X\2\4\64\11\253\307-"
    "\65T\0\4\65\7\243\310US\4\66\11\244\310$S%\1\4\67\11\243\310\254\14\26\0\4\70\11\243"
    "\310\244\61T\0\4\71\11\253\310\244\326P\1\4:\10\243\310$\265\12\4;\7\243\310-+\4<\11"
    "\243\310\244\241T\0\4=\11\243\310\244\241T\0\4>\10\243\310U\252\2\4\77\10\243\310\214d\5\4"
    "@\11\253\307\250\264b\4\4A\10\243\310\315\14\4\4B\10\243\310\254X\1\4C\11\253\307$\225\344"
    "\2\4D\12\263\307\305\224T\231\0\4E\10\243\310\244\62\25\4F\11\253\307$k\304\0\4G\11\243"
    "\310$\225d\0\4H\10\243\310\244q\4\4I\11\254\307\244\364\310 \4J\12\244\310h SR\0"
    "\4K\11\244\310\304\245F\12\4L\11\243\310D\225\26\0\4M\10\243\310H\271\0\4N\12\244\310\244"
    "\244\226J\0\4O\10\243\310\255\264\2\4Q\10\253\310\244\326\24\4R\11\263\307D\25U\31\4S\11"
    "\253\310\246\64b\4\4T\11\243\310\215\224\201\0\4U\11\243\310\215\14\26\0\4V\11\253\310e$\323"
    "\0\4W\11\253\310\244\14d\32\4X\11\263\307fX.\0\4Y\10\244\310\251\326\22\4Z\11\244\310"
    "\244\264\322\22\4[\11\253\310D\25U\1\4\134\10\253\310\312\264\12\4^\11\263\307\244\32u\2\4_"
    "\11\253\307$k\244\4\4\220\10\253\310\16Y\2\4\221\10\243\310\16\31\1\4\222\11\253\310\251\264b\2"
    "\4\223\11\243\310\251\264\22\0\0";

// Screen is 128x64 px
static void app_draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);

    canvas_clear(canvas);

    canvas_set_custom_u8g2_font(canvas, u8g2_font_4x6_t_cyrillic);

    canvas_draw_str(canvas, 0, 6, "This is a tiny custom font");
    canvas_draw_str(canvas, 0, 12, "012345.?! ,:;\"\'@#$%");
    canvas_draw_str(canvas, 0, 18, "И немного юникода");
}

static void app_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);

    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

int32_t example_custom_font_main(void* p) {
    UNUSED(p);
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    // Configure view port
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, app_draw_callback, NULL);
    view_port_input_callback_set(view_port, app_input_callback, event_queue);

    // Register view port in GUI
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    InputEvent event;

    bool running = true;

    while(running) {
        if(furi_message_queue_get(event_queue, &event, 100) == FuriStatusOk) {
            if((event.type == InputTypePress) || (event.type == InputTypeRepeat)) {
                switch(event.key) {
                case InputKeyBack:
                    running = false;
                    break;
                default:
                    break;
                }
            }
        }
    }

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);

    furi_record_close(RECORD_GUI);

    return 0;
}

=== END: applications/examples/example_custom_font/example_custom_font.c ===

=== BEGIN: applications/examples/example_event_loop/application.fam ===
App(
    appid="example_event_loop_event_flags",
    name="Example: Event Loop Event Flags",
    apptype=FlipperAppType.EXTERNAL,
    sources=["example_event_loop_event_flags.c"],
    entry_point="example_event_loop_event_flags_app",
    fap_category="Examples",
)

App(
    appid="example_event_loop_timer",
    name="Example: Event Loop Timer",
    apptype=FlipperAppType.EXTERNAL,
    sources=["example_event_loop_timer.c"],
    entry_point="example_event_loop_timer_app",
    fap_category="Examples",
)

App(
    appid="example_event_loop_mutex",
    name="Example: Event Loop Mutex",
    apptype=FlipperAppType.EXTERNAL,
    sources=["example_event_loop_mutex.c"],
    entry_point="example_event_loop_mutex_app",
    fap_category="Examples",
)

App(
    appid="example_event_loop_stream_buffer",
    name="Example: Event Loop Stream Buffer",
    apptype=FlipperAppType.EXTERNAL,
    sources=["example_event_loop_stream_buffer.c"],
    entry_point="example_event_loop_stream_buffer_app",
    fap_category="Examples",
)

App(
    appid="example_event_loop_multi",
    name="Example: Event Loop Multi",
    apptype=FlipperAppType.EXTERNAL,
    sources=["example_event_loop_multi.c"],
    entry_point="example_event_loop_multi_app",
    requires=["gui"],
    fap_category="Examples",
)

=== END: applications/examples/example_event_loop/application.fam ===

=== BEGIN: applications/examples/example_event_loop/example_event_loop_event_flags.c ===
/**
 * @file example_event_loop_event_flags.c
 * @brief Example application demonstrating the use of the FuriEventFlag primitive in FuriEventLoop instances.
 *
 * This application receives keystrokes from the input service and sets the appropriate flags,
 * which are subsequently processed in the event loop
 */

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_port.h>

#include <furi_hal_random.h>

#define TAG "ExampleEventLoopEventFlags"

typedef struct {
    Gui* gui;
    ViewPort* view_port;
    FuriEventLoop* event_loop;
    FuriEventFlag* event_flag;
} EventLoopEventFlagsApp;

typedef enum {
    EventLoopEventFlagsOk = (1 << 0),
    EventLoopEventFlagsUp = (1 << 1),
    EventLoopEventFlagsDown = (1 << 2),
    EventLoopEventFlagsLeft = (1 << 3),
    EventLoopEventFlagsRight = (1 << 4),
    EventLoopEventFlagsBack = (1 << 5),
    EventLoopEventFlagsExit = (1 << 6),
} EventLoopEventFlags;

#define EVENT_LOOP_EVENT_FLAGS_MASK                                                 \
    (EventLoopEventFlagsOk | EventLoopEventFlagsUp | EventLoopEventFlagsDown |      \
     EventLoopEventFlagsLeft | EventLoopEventFlagsRight | EventLoopEventFlagsBack | \
     EventLoopEventFlagsExit)

// This function is executed in the GUI context each time an input event occurs (e.g. the user pressed a key)
static void event_loop_event_flags_app_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    EventLoopEventFlagsApp* app = context;
    UNUSED(app);

    if(event->type == InputTypePress) {
        if(event->key == InputKeyOk) {
            furi_event_flag_set(app->event_flag, EventLoopEventFlagsOk);
        } else if(event->key == InputKeyUp) {
            furi_event_flag_set(app->event_flag, EventLoopEventFlagsUp);
        } else if(event->key == InputKeyDown) {
            furi_event_flag_set(app->event_flag, EventLoopEventFlagsDown);
        } else if(event->key == InputKeyLeft) {
            furi_event_flag_set(app->event_flag, EventLoopEventFlagsLeft);
        } else if(event->key == InputKeyRight) {
            furi_event_flag_set(app->event_flag, EventLoopEventFlagsRight);
        } else if(event->key == InputKeyBack) {
            furi_event_flag_set(app->event_flag, EventLoopEventFlagsBack);
        }
    } else if(event->type == InputTypeLong) {
        if(event->key == InputKeyBack) {
            furi_event_flag_set(app->event_flag, EventLoopEventFlagsExit);
        }
    }
}

// This function is executed each time a new event flag is inserted in the input event flag.
static void
    event_loop_event_flags_app_event_flags_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    EventLoopEventFlagsApp* app = context;

    furi_assert(object == app->event_flag);

    EventLoopEventFlags events =
        furi_event_flag_wait(app->event_flag, EVENT_LOOP_EVENT_FLAGS_MASK, FuriFlagWaitAny, 0);
    furi_check((events) != 0);

    if(events & EventLoopEventFlagsOk) {
        FURI_LOG_I(TAG, "Press \"Ok\"");
    }
    if(events & EventLoopEventFlagsUp) {
        FURI_LOG_I(TAG, "Press \"Up\"");
    }
    if(events & EventLoopEventFlagsDown) {
        FURI_LOG_I(TAG, "Press \"Down\"");
    }
    if(events & EventLoopEventFlagsLeft) {
        FURI_LOG_I(TAG, "Press \"Left\"");
    }
    if(events & EventLoopEventFlagsRight) {
        FURI_LOG_I(TAG, "Press \"Right\"");
    }
    if(events & EventLoopEventFlagsBack) {
        FURI_LOG_I(TAG, "Press \"Back\"");
    }
    if(events & EventLoopEventFlagsExit) {
        FURI_LOG_I(TAG, "Exit App");
        furi_event_loop_stop(app->event_loop);
    }
}

static EventLoopEventFlagsApp* event_loop_event_flags_app_alloc(void) {
    EventLoopEventFlagsApp* app = malloc(sizeof(EventLoopEventFlagsApp));

    // Create event loop instances.
    app->event_loop = furi_event_loop_alloc();
    // Create event flag instances.
    app->event_flag = furi_event_flag_alloc();

    // Create GUI instance.
    app->gui = furi_record_open(RECORD_GUI);
    app->view_port = view_port_alloc();
    // Gain exclusive access to the input events
    view_port_input_callback_set(app->view_port, event_loop_event_flags_app_input_callback, app);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    // Notify the event loop about incoming messages in the event flag
    furi_event_loop_subscribe_event_flag(
        app->event_loop,
        app->event_flag,
        FuriEventLoopEventIn | FuriEventLoopEventFlagEdge,
        event_loop_event_flags_app_event_flags_callback,
        app);

    return app;
}

static void event_loop_event_flags_app_free(EventLoopEventFlagsApp* app) {
    gui_remove_view_port(app->gui, app->view_port);

    furi_record_close(RECORD_GUI);
    app->gui = NULL;

    // Delete all instances
    view_port_free(app->view_port);
    app->view_port = NULL;

    // IMPORTANT: The user code MUST unsubscribe from all events before deleting the event loop.
    // Failure to do so will result in a crash.
    furi_event_loop_unsubscribe(app->event_loop, app->event_flag);

    furi_event_flag_free(app->event_flag);
    app->event_flag = NULL;

    furi_event_loop_free(app->event_loop);
    app->event_loop = NULL;

    free(app);
}

static void event_loop_event_flags_app_run(EventLoopEventFlagsApp* app) {
    FURI_LOG_I(TAG, "Press keys to see them printed here.");
    FURI_LOG_I(TAG, "Quickly press different keys to generate events.");
    FURI_LOG_I(TAG, "Long press \"Back\" to exit app.");

    // Run the application event loop. This call will block until the application is about to exit.
    furi_event_loop_run(app->event_loop);
}

/*******************************************************************
 *                     vvv START HERE vvv
 *
 * The application's entry point - referenced in application.fam
 *******************************************************************/
int32_t example_event_loop_event_flags_app(void* arg) {
    UNUSED(arg);

    EventLoopEventFlagsApp* app = event_loop_event_flags_app_alloc();
    event_loop_event_flags_app_run(app);
    event_loop_event_flags_app_free(app);

    return 0;
}

=== END: applications/examples/example_event_loop/example_event_loop_event_flags.c ===

=== BEGIN: applications/examples/example_event_loop/example_event_loop_multi.c ===
/**
 * @file example_event_loop_multi.c
 * @brief Example application that demonstrates multiple primitives used with two FuriEventLoop instances.
 *
 * This application simulates a complex use case of having two concurrent event loops (each one executing in
 * its own thread) using a stream buffer for communication and additional timers and message passing to handle
 * the keypad input. Additionally, it shows how to use thread signals to stop an event loop in another thread.
 * The GUI functionality is there only for the purpose of exclusive access to the input events.
 *
 * The application's functionality consists of the following:
 * - Print keypad key names and types when pressed,
 * - If the Back key is long-pressed, a countdown starts upon completion of which the app exits,
 * - The countdown can be cancelled by long-pressing the Ok button, it also resets the counter,
 * - Blocks of random data are periodically generated in a separate thread,
 * - When ready, the main application thread gets notified and prints the data.
 */

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_port.h>

#include <furi_hal_random.h>

#define TAG "ExampleEventLoopMulti"

#define COUNTDOWN_START_VALUE   (5UL)
#define COUNTDOWN_INTERVAL_MS   (1000UL)
#define WORKER_DATA_INTERVAL_MS (1500UL)

#define INPUT_QUEUE_SIZE   (8)
#define STREAM_BUFFER_SIZE (16)

typedef struct {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
    FuriStreamBuffer* stream_buffer;
} EventLoopMultiAppWorker;

typedef struct {
    Gui* gui;
    ViewPort* view_port;
    FuriThread* worker_thread;
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriEventLoopTimer* exit_timer;
    FuriStreamBuffer* stream_buffer;
    uint32_t exit_countdown_value;
} EventLoopMultiApp;

/*
 * Worker functions
 */

// This function is executed each time the data is taken out of the stream buffer. It is used to restart the worker timer.
static void
    event_loop_multi_app_stream_buffer_worker_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    EventLoopMultiAppWorker* worker = context;

    furi_assert(object == worker->stream_buffer);

    FURI_LOG_I(TAG, "Data was removed from buffer");
    // Restart the timer to generate another block of random data.
    furi_event_loop_timer_start(worker->timer, WORKER_DATA_INTERVAL_MS);
}

// This function is executed when the worker timer expires. The timer will NOT restart automatically
// since it is of one-shot type.
static void event_loop_multi_app_worker_timer_callback(void* context) {
    furi_assert(context);
    EventLoopMultiAppWorker* worker = context;

    // Generate a block of random data.
    uint8_t data[STREAM_BUFFER_SIZE];
    furi_hal_random_fill_buf(data, sizeof(data));
    // Put the generated data in the stream buffer.
    // IMPORTANT: No waiting in the event handlers!
    furi_check(
        furi_stream_buffer_send(worker->stream_buffer, &data, sizeof(data), 0) == sizeof(data));
}

static EventLoopMultiAppWorker*
    event_loop_multi_app_worker_alloc(FuriStreamBuffer* stream_buffer) {
    EventLoopMultiAppWorker* worker = malloc(sizeof(EventLoopMultiAppWorker));
    // Create the worker event loop.
    worker->event_loop = furi_event_loop_alloc();
    // Create the timer governing the data generation.
    // It is of one-shot type, i.e. it will not restart automatically upon expiration.
    worker->timer = furi_event_loop_timer_alloc(
        worker->event_loop,
        event_loop_multi_app_worker_timer_callback,
        FuriEventLoopTimerTypeOnce,
        worker);

    // Using the same stream buffer as the main thread (it was already created beforehand).
    worker->stream_buffer = stream_buffer;
    // Notify the worker event loop about data being taken out of the stream buffer.
    furi_event_loop_subscribe_stream_buffer(
        worker->event_loop,
        worker->stream_buffer,
        FuriEventLoopEventOut | FuriEventLoopEventFlagEdge,
        event_loop_multi_app_stream_buffer_worker_callback,
        worker);

    return worker;
}

static void event_loop_multi_app_worker_free(EventLoopMultiAppWorker* worker) {
    // IMPORTANT: The user code MUST unsubscribe from all events before deleting the event loop.
    // Failure to do so will result in a crash.
    furi_event_loop_unsubscribe(worker->event_loop, worker->stream_buffer);
    // IMPORTANT: All timers MUST be deleted before deleting the associated event loop.
    // Failure to do so will result in a crash.
    furi_event_loop_timer_free(worker->timer);
    // Now it is okay to delete the event loop.
    furi_event_loop_free(worker->event_loop);

    free(worker);
}

static void event_loop_multi_app_worker_run(EventLoopMultiAppWorker* worker) {
    furi_event_loop_timer_start(worker->timer, WORKER_DATA_INTERVAL_MS);
    furi_event_loop_run(worker->event_loop);
}

// This function is the worker thread body and (obviously) is executed in the worker thread.
static int32_t event_loop_multi_app_worker_thread(void* context) {
    furi_assert(context);
    EventLoopMultiApp* app = context;

    // Because an event loop is used, it MUST be created in the thread it will be run in.
    // Therefore, the worker creation and deletion is handled in the worker thread.
    EventLoopMultiAppWorker* worker = event_loop_multi_app_worker_alloc(app->stream_buffer);
    event_loop_multi_app_worker_run(worker);
    event_loop_multi_app_worker_free(worker);

    return 0;
}

/*
 * Main application functions
 */

// This function is executed in the GUI context each time an input event occurs (e.g. the user pressed a key)
static void event_loop_multi_app_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    EventLoopMultiApp* app = context;
    // Pass the event to the the application's input queue
    furi_check(furi_message_queue_put(app->input_queue, event, FuriWaitForever) == FuriStatusOk);
}

// This function is executed each time new data is available in the stream buffer.
static void
    event_loop_multi_app_stream_buffer_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    EventLoopMultiApp* app = context;

    furi_assert(object == app->stream_buffer);
    // Get the data from the stream buffer
    uint8_t data[STREAM_BUFFER_SIZE];
    // IMPORTANT: No waiting in the event handlers!
    furi_check(
        furi_stream_buffer_receive(app->stream_buffer, &data, sizeof(data), 0) == sizeof(data));

    // Format the data for printing and print it to the debug output.
    FuriString* tmp_str = furi_string_alloc();
    for(uint32_t i = 0; i < sizeof(data); ++i) {
        furi_string_cat_printf(tmp_str, "%02X ", data[i]);
    }

    FURI_LOG_I(TAG, "Received data: %s", furi_string_get_cstr(tmp_str));
    furi_string_free(tmp_str);
}

// This function is executed each time a new message is inserted in the input queue.
static void event_loop_multi_app_input_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    EventLoopMultiApp* app = context;

    furi_assert(object == app->input_queue);

    InputEvent event;
    // IMPORTANT: No waiting in the event handlers!
    furi_check(furi_message_queue_get(app->input_queue, &event, 0) == FuriStatusOk);

    if(event.type == InputTypeLong) {
        // The user has long-pressed the Back key, try starting the countdown.
        if(event.key == InputKeyBack) {
            if(!furi_event_loop_timer_is_running(app->exit_timer)) {
                // Actually start the countdown
                FURI_LOG_I(TAG, "Starting exit countdown!");
                furi_event_loop_timer_start(app->exit_timer, COUNTDOWN_INTERVAL_MS);

            } else {
                // The countdown is already in progress, print a warning message
                FURI_LOG_W(TAG, "Countdown has already been started");
            }

            // The user has long-pressed the Ok key, try stopping the countdown.
        } else if(event.key == InputKeyOk) {
            if(furi_event_loop_timer_is_running(app->exit_timer)) {
                // Actually cancel the countdown
                FURI_LOG_I(TAG, "Exit countdown cancelled!");
                app->exit_countdown_value = COUNTDOWN_START_VALUE;
                furi_event_loop_timer_stop(app->exit_timer);

            } else {
                // The countdown is not running, print a warning message
                FURI_LOG_W(TAG, "Countdown has not been started yet");
            }

        } else {
            // Not a Back or Ok key, just print its name.
            FURI_LOG_I(TAG, "Long press: %s", input_get_key_name(event.key));
        }

    } else if(event.type == InputTypeShort) {
        // Not a long press, just print the key's name.
        FURI_LOG_I(TAG, "Short press: %s", input_get_key_name(event.key));
    }
}

// This function is executed each time the countdown timer expires.
static void event_loop_multi_app_exit_timer_callback(void* context) {
    furi_assert(context);
    EventLoopMultiApp* app = context;

    FURI_LOG_I(TAG, "Exiting in %lu ...", app->exit_countdown_value);

    // If the coundown value has reached 0, exit the application
    if(app->exit_countdown_value == 0) {
        FURI_LOG_I(TAG, "Exiting NOW!");

        // Send a signal to the worker thread to exit.
        // A signal handler that handles FuriSignalExit is already set by default.
        furi_thread_signal(app->worker_thread, FuriSignalExit, NULL);
        // Request the application event loop to stop.
        furi_event_loop_stop(app->event_loop);

        // Otherwise just decrement it and wait for the next time the timer expires.
    } else {
        app->exit_countdown_value -= 1;
    }
}

static EventLoopMultiApp* event_loop_multi_app_alloc(void) {
    EventLoopMultiApp* app = malloc(sizeof(EventLoopMultiApp));
    // Create event loop instances.
    app->event_loop = furi_event_loop_alloc();

    // Create a worker thread instance. The worker event loop will execute inside it.
    app->worker_thread = furi_thread_alloc_ex(
        "EventLoopMultiWorker", 1024, event_loop_multi_app_worker_thread, app);
    // Create a message queue to receive the input events.
    app->input_queue = furi_message_queue_alloc(INPUT_QUEUE_SIZE, sizeof(InputEvent));
    // Create a stream buffer to receive the generated data.
    app->stream_buffer = furi_stream_buffer_alloc(STREAM_BUFFER_SIZE, STREAM_BUFFER_SIZE);
    // Create a timer to run the countdown.
    app->exit_timer = furi_event_loop_timer_alloc(
        app->event_loop,
        event_loop_multi_app_exit_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        app);

    app->gui = furi_record_open(RECORD_GUI);
    app->view_port = view_port_alloc();
    // Start the countdown from this value
    app->exit_countdown_value = COUNTDOWN_START_VALUE;
    // Gain exclusive access to the input events
    view_port_input_callback_set(app->view_port, event_loop_multi_app_input_callback, app);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);
    // Notify the event loop about incoming messages in the queue
    furi_event_loop_subscribe_message_queue(
        app->event_loop,
        app->input_queue,
        FuriEventLoopEventIn,
        event_loop_multi_app_input_queue_callback,
        app);
    // Notify the event loop about new data in the stream buffer
    furi_event_loop_subscribe_stream_buffer(
        app->event_loop,
        app->stream_buffer,
        FuriEventLoopEventIn | FuriEventLoopEventFlagEdge,
        event_loop_multi_app_stream_buffer_callback,
        app);

    return app;
}

static void event_loop_multi_app_free(EventLoopMultiApp* app) {
    gui_remove_view_port(app->gui, app->view_port);
    furi_record_close(RECORD_GUI);
    // IMPORTANT: The user code MUST unsubscribe from all events before deleting the event loop.
    // Failure to do so will result in a crash.
    furi_event_loop_unsubscribe(app->event_loop, app->input_queue);
    furi_event_loop_unsubscribe(app->event_loop, app->stream_buffer);
    // Delete all instances
    view_port_free(app->view_port);
    furi_message_queue_free(app->input_queue);
    furi_stream_buffer_free(app->stream_buffer);
    // IMPORTANT: All timers MUST be deleted before deleting the associated event loop.
    // Failure to do so will result in a crash.
    furi_event_loop_timer_free(app->exit_timer);
    furi_thread_free(app->worker_thread);
    furi_event_loop_free(app->event_loop);

    free(app);
}

static void event_loop_multi_app_run(EventLoopMultiApp* app) {
    FURI_LOG_I(TAG, "Press keys to see them printed here.");
    FURI_LOG_I(TAG, "Long press \"Back\" to exit after %lu seconds.", COUNTDOWN_START_VALUE);
    FURI_LOG_I(TAG, "Long press \"Ok\" to cancel the countdown.");

    // Start the worker thread
    furi_thread_start(app->worker_thread);
    // Run the application event loop. This call will block until the application is about to exit.
    furi_event_loop_run(app->event_loop);
    // Wait for the worker thread to finish.
    furi_thread_join(app->worker_thread);
}

/*******************************************************************
 *                     vvv START HERE vvv
 *
 * The application's entry point - referenced in application.fam
 *******************************************************************/
int32_t example_event_loop_multi_app(void* arg) {
    UNUSED(arg);

    EventLoopMultiApp* app = event_loop_multi_app_alloc();
    event_loop_multi_app_run(app);
    event_loop_multi_app_free(app);

    return 0;
}

=== END: applications/examples/example_event_loop/example_event_loop_multi.c ===

=== BEGIN: applications/examples/example_event_loop/example_event_loop_mutex.c ===
/**
 * @file example_event_loop_mutex.c
 * @brief Example application that demonstrates the FuriEventLoop and FuriMutex integration.
 *
 * This application simulates a use case where a time-consuming blocking operation is executed
 * in a separate thread and a mutex is being used for synchronization. The application runs 10 iterations
 * of the above mentioned simulated work and prints the results to the debug output each time, then exits.
 */

#include <furi.h>
#include <furi_hal_random.h>

#define TAG "ExampleEventLoopMutex"

#define WORKER_ITERATION_COUNT (10)
// We are interested in IN events (for the mutex, that means that the mutex has been released),
// using edge trigger mode (reacting only to changes in mutex state) and
// employing one-shot mode to automatically unsubscribe before the event is processed.
#define MUTEX_EVENT_AND_FLAGS \
    (FuriEventLoopEventIn | FuriEventLoopEventFlagEdge | FuriEventLoopEventFlagOnce)

typedef struct {
    FuriEventLoop* event_loop;
    FuriThread* worker_thread;
    FuriMutex* worker_mutex;
    uint8_t worker_result;
} EventLoopMutexApp;

// This funciton is being run in a separate thread to simulate lenghty blocking operations
static int32_t event_loop_mutex_app_worker_thread(void* context) {
    furi_assert(context);
    EventLoopMutexApp* app = context;

    FURI_LOG_I(TAG, "Worker thread started");

    // Run 10 iterations of simulated work
    for(uint32_t i = 0; i < WORKER_ITERATION_COUNT; ++i) {
        FURI_LOG_I(TAG, "Doing work ...");
        // Take the mutex so that no-one can access the worker_result variable
        furi_check(furi_mutex_acquire(app->worker_mutex, FuriWaitForever) == FuriStatusOk);
        // Simulate a blocking operation with a random delay between 900 and 1100 ms
        const uint32_t work_time_ms = 900 + furi_hal_random_get() % 200;
        furi_delay_ms(work_time_ms);
        // Simulate a result with a random number between 0 and 255
        app->worker_result = furi_hal_random_get() % 0xFF;

        FURI_LOG_I(TAG, "Work done in %lu ms", work_time_ms);
        // Release the mutex, which will notify the event loop that the result is ready
        furi_check(furi_mutex_release(app->worker_mutex) == FuriStatusOk);
        // Return control to the scheduler so that the event loop can take the mutex in its turn
        furi_thread_yield();
    }

    FURI_LOG_I(TAG, "All work done, worker thread out!");
    // Request the event loop to stop
    furi_event_loop_stop(app->event_loop);

    return 0;
}

// This function is being run each time when the mutex gets released
static void event_loop_mutex_app_event_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    EventLoopMutexApp* app = context;
    furi_assert(object == app->worker_mutex);

    // Take the mutex so that no-one can access the worker_result variable
    // IMPORTANT: the wait time MUST be 0, i.e. the event loop event callbacks
    // must NOT ever block. If it is possible that the mutex will be taken by
    // others, then the event callback code must take it into account.
    furi_check(furi_mutex_acquire(app->worker_mutex, 0) == FuriStatusOk);
    // Access the worker_result variable and print it.
    FURI_LOG_I(TAG, "Result available! Value: %u", app->worker_result);
    // Release the mutex, enabling the worker thread to continue when it's ready
    furi_check(furi_mutex_release(app->worker_mutex) == FuriStatusOk);
    // Subscribe for the mutex release events again, since we were unsubscribed automatically
    // before processing the event.
    furi_event_loop_subscribe_mutex(
        app->event_loop,
        app->worker_mutex,
        MUTEX_EVENT_AND_FLAGS,
        event_loop_mutex_app_event_callback,
        app);
}

static EventLoopMutexApp* event_loop_mutex_app_alloc(void) {
    EventLoopMutexApp* app = malloc(sizeof(EventLoopMutexApp));

    // Create an event loop instance.
    app->event_loop = furi_event_loop_alloc();
    // Create a worker thread instance.
    app->worker_thread = furi_thread_alloc_ex(
        "EventLoopMutexWorker", 1024, event_loop_mutex_app_worker_thread, app);
    // Create a mutex instance.
    app->worker_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    // Subscribe for the mutex release events.
    // Note that since FuriEventLoopEventFlagOneShot is used, we will be automatically unsubscribed
    // from events before entering the event processing callback. This is necessary in order to not
    // trigger on events caused by releasing the mutex in the callback.
    furi_event_loop_subscribe_mutex(
        app->event_loop,
        app->worker_mutex,
        MUTEX_EVENT_AND_FLAGS,
        event_loop_mutex_app_event_callback,
        app);

    return app;
}

static void event_loop_mutex_app_free(EventLoopMutexApp* app) {
    // IMPORTANT: The user code MUST unsubscribe from all events before deleting the event loop.
    // Failure to do so will result in a crash.
    furi_event_loop_unsubscribe(app->event_loop, app->worker_mutex);
    // Delete all instances
    furi_thread_free(app->worker_thread);
    furi_mutex_free(app->worker_mutex);
    furi_event_loop_free(app->event_loop);

    free(app);
}

static void event_loop_mutex_app_run(EventLoopMutexApp* app) {
    furi_thread_start(app->worker_thread);
    furi_event_loop_run(app->event_loop);
    furi_thread_join(app->worker_thread);
}

// The application's entry point - referenced in application.fam
int32_t example_event_loop_mutex_app(void* arg) {
    UNUSED(arg);

    EventLoopMutexApp* app = event_loop_mutex_app_alloc();
    event_loop_mutex_app_run(app);
    event_loop_mutex_app_free(app);

    return 0;
}

=== END: applications/examples/example_event_loop/example_event_loop_mutex.c ===

=== BEGIN: applications/examples/example_event_loop/example_event_loop_stream_buffer.c ===
/**
 * @file example_event_loop_stream_buffer.c
 * @brief Example application that demonstrates the FuriEventLoop and FuriStreamBuffer integration.
 *
 * This application simulates a use case where some data data stream comes from a separate thread (or hardware)
 * and a stream buffer is used to act as an intermediate buffer. The worker thread produces 10 iterations of 32
 * bytes of simulated data, and each time when the buffer is half-filled, the data is taken out of it and printed
 * to the debug output. After completing all iterations, the application exits.
 */

#include <furi.h>
#include <furi_hal_random.h>

#define TAG "ExampleEventLoopStreamBuffer"

#define WORKER_ITERATION_COUNT (10)

#define STREAM_BUFFER_SIZE            (32)
#define STREAM_BUFFER_TRIG_LEVEL      (STREAM_BUFFER_SIZE / 2)
#define STREAM_BUFFER_EVENT_AND_FLAGS (FuriEventLoopEventIn | FuriEventLoopEventFlagEdge)

typedef struct {
    FuriEventLoop* event_loop;
    FuriThread* worker_thread;
    FuriStreamBuffer* stream_buffer;
} EventLoopStreamBufferApp;

// This funciton is being run in a separate thread to simulate data coming from a producer thread or some device.
static int32_t event_loop_stream_buffer_app_worker_thread(void* context) {
    furi_assert(context);
    EventLoopStreamBufferApp* app = context;

    FURI_LOG_I(TAG, "Worker thread started");

    for(uint32_t i = 0; i < WORKER_ITERATION_COUNT; ++i) {
        // Produce 32 bytes of simulated data.
        for(uint32_t j = 0; j < STREAM_BUFFER_SIZE; ++j) {
            // Simulate incoming data by generating a random byte.
            uint8_t data = furi_hal_random_get() % 0xFF;
            // Put the byte in the buffer. Depending on the use case, it may or may be not acceptable
            // to wait for free space to become available.
            furi_check(
                furi_stream_buffer_send(app->stream_buffer, &data, 1, FuriWaitForever) == 1);
            // Delay between 30 and 50 ms to slow down the output for clarity.
            furi_delay_ms(30 + furi_hal_random_get() % 20);
        }
    }

    FURI_LOG_I(TAG, "All work done, worker thread out!");
    // Request the event loop to stop
    furi_event_loop_stop(app->event_loop);

    return 0;
}

// This function is being run each time when the number of bytes in the buffer is above its trigger level.
static void
    event_loop_stream_buffer_app_event_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    EventLoopStreamBufferApp* app = context;

    furi_assert(object == app->stream_buffer);

    // Temporary buffer that can hold at most half of the stream buffer's capacity.
    uint8_t data[STREAM_BUFFER_TRIG_LEVEL];
    // Receive the data. It is guaranteed that the amount of data in the buffer will be equal to
    // or greater than the trigger level, therefore, no waiting delay is necessary.
    furi_check(
        furi_stream_buffer_receive(app->stream_buffer, data, sizeof(data), 0) == sizeof(data));

    // Format the data for printing and print it to the debug output.
    FuriString* tmp_str = furi_string_alloc();
    for(uint32_t i = 0; i < sizeof(data); ++i) {
        furi_string_cat_printf(tmp_str, "%02X ", data[i]);
    }

    FURI_LOG_I(TAG, "Received data: %s", furi_string_get_cstr(tmp_str));
    furi_string_free(tmp_str);
}

static EventLoopStreamBufferApp* event_loop_stream_buffer_app_alloc(void) {
    EventLoopStreamBufferApp* app = malloc(sizeof(EventLoopStreamBufferApp));

    // Create an event loop instance.
    app->event_loop = furi_event_loop_alloc();
    // Create a worker thread instance.
    app->worker_thread = furi_thread_alloc_ex(
        "EventLoopStreamBufferWorker", 1024, event_loop_stream_buffer_app_worker_thread, app);
    // Create a stream_buffer instance.
    app->stream_buffer = furi_stream_buffer_alloc(STREAM_BUFFER_SIZE, STREAM_BUFFER_TRIG_LEVEL);
    // Subscribe for the stream buffer IN events in edge triggered mode.
    furi_event_loop_subscribe_stream_buffer(
        app->event_loop,
        app->stream_buffer,
        STREAM_BUFFER_EVENT_AND_FLAGS,
        event_loop_stream_buffer_app_event_callback,
        app);

    return app;
}

static void event_loop_stream_buffer_app_free(EventLoopStreamBufferApp* app) {
    // IMPORTANT: The user code MUST unsubscribe from all events before deleting the event loop.
    // Failure to do so will result in a crash.
    furi_event_loop_unsubscribe(app->event_loop, app->stream_buffer);
    // Delete all instances
    furi_thread_free(app->worker_thread);
    furi_stream_buffer_free(app->stream_buffer);
    furi_event_loop_free(app->event_loop);

    free(app);
}

static void event_loop_stream_buffer_app_run(EventLoopStreamBufferApp* app) {
    furi_thread_start(app->worker_thread);
    furi_event_loop_run(app->event_loop);
    furi_thread_join(app->worker_thread);
}

// The application's entry point - referenced in application.fam
int32_t example_event_loop_stream_buffer_app(void* arg) {
    UNUSED(arg);

    EventLoopStreamBufferApp* app = event_loop_stream_buffer_app_alloc();
    event_loop_stream_buffer_app_run(app);
    event_loop_stream_buffer_app_free(app);

    return 0;
}

=== END: applications/examples/example_event_loop/example_event_loop_stream_buffer.c ===

=== BEGIN: applications/examples/example_event_loop/example_event_loop_timer.c ===
/**
 * @file example_event_loop_timer.c
 * @brief Example application that demonstrates FuriEventLoop's software timer capability.
 *
 * This application prints a countdown from 10 to 0 to the debug output and then exits.
 * Despite only one timer being used in this example for clarity, an event loop instance can have
 * an arbitrary number of independent timers of any type (periodic or one-shot).
 *
 */
#include <furi.h>

#define TAG "ExampleEventLoopTimer"

#define COUNTDOWN_START_VALUE (10)
#define COUNTDOWN_INTERVAL_MS (1000)

typedef struct {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
    uint32_t countdown_value;
} EventLoopTimerApp;

// This function is called each time the timer expires (i.e. once per 1000 ms (1s) in this example)
static void event_loop_timer_callback(void* context) {
    furi_assert(context);
    EventLoopTimerApp* app = context;

    // Print the countdown value
    FURI_LOG_I(TAG, "T-00:00:%02lu", app->countdown_value);

    if(app->countdown_value == 0) {
        // If the countdown reached 0, print the final line and stop the event loop
        FURI_LOG_I(TAG, "Blast off to adventure!");
        // After this call, the control will be returned back to event_loop_timers_app_run()
        furi_event_loop_stop(app->event_loop);

    } else {
        // Decrement the countdown value
        app->countdown_value -= 1;
    }
}

static EventLoopTimerApp* event_loop_timer_app_alloc(void) {
    EventLoopTimerApp* app = malloc(sizeof(EventLoopTimerApp));

    // Create an event loop instance.
    app->event_loop = furi_event_loop_alloc();
    // Create a software timer instance.
    // The timer is bound to the event loop instance and will execute in its context.
    // Here, the timer type is periodic, i.e. it will restart automatically after expiring.
    app->timer = furi_event_loop_timer_alloc(
        app->event_loop, event_loop_timer_callback, FuriEventLoopTimerTypePeriodic, app);
    // The countdown value will be tracked in this variable.
    app->countdown_value = COUNTDOWN_START_VALUE;

    return app;
}

static void event_loop_timer_app_free(EventLoopTimerApp* app) {
    // IMPORTANT: All event loop timers MUST be deleted BEFORE deleting the event loop itself.
    // Failure to do so will result in a crash.
    furi_event_loop_timer_free(app->timer);
    // With all timers deleted, it's safe to delete the event loop.
    furi_event_loop_free(app->event_loop);
    free(app);
}

static void event_loop_timer_app_run(EventLoopTimerApp* app) {
    FURI_LOG_I(TAG, "All systems go! Prepare for countdown!");

    // Timers can be started either before the event loop is run, or in any
    // callback function called by a running event loop.
    furi_event_loop_timer_start(app->timer, COUNTDOWN_INTERVAL_MS);
    // This call will block until furi_event_loop_stop() is called.
    furi_event_loop_run(app->event_loop);
}

// The application's entry point - referenced in application.fam
int32_t example_event_loop_timer_app(void* arg) {
    UNUSED(arg);

    EventLoopTimerApp* app = event_loop_timer_app_alloc();
    event_loop_timer_app_run(app);
    event_loop_timer_app_free(app);

    return 0;
}

=== END: applications/examples/example_event_loop/example_event_loop_timer.c ===

=== BEGIN: applications/examples/example_images/ReadMe.md ===
# Application icons {#example_app_images}

## Source code

Source code for this example can be found [here](https://github.com/flipperdevices/flipperzero-firmware/tree/dev/applications/examples/example_images).

## General principle

To use icons, do the following:

* Add a line to the application manifest: `fap_icon_assets="folder"`, where `folder` points to the folder where your icons are located
* Add `#include "application_id_icons.h"` to the application code, where `application_id` is the appid from the manifest
* Every icon in the folder will be available as a `I_icon_name` variable, where `icon_name` is the name of the icon file without the extension

## Example

We have an application with the following manifest:

```
App(
    appid="example_images",
    ...
    fap_icon_assets="images",
)
```

So the icons are in the `images` folder and will be available in the generated `example_images_icons.h` file.

The example code is located in `example_images_main.c` and contains the following line:

```
#include "example_images_icons.h"
```

Image `dolphin_71x25.png` is available as `I_dolphin_71x25`.

=== END: applications/examples/example_images/ReadMe.md ===

=== BEGIN: applications/examples/example_images/application.fam ===
App(
    appid="example_images",
    name="Example: Images",
    apptype=FlipperAppType.EXTERNAL,
    entry_point="example_images_main",
    requires=["gui"],
    stack_size=1 * 1024,
    fap_category="Examples",
    fap_icon_assets="images",
)

=== END: applications/examples/example_images/application.fam ===

=== BEGIN: applications/examples/example_images/example_images.c ===
/**
 * @file example_images.c
 * @brief Custom images example.
 */
#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <input/input.h>

/* Magic happens here -- this file is generated by fbt.
 * Just set fap_icon_assets in application.fam and #include {APPID}_icons.h */
#include "example_images_icons.h"

typedef struct {
    int32_t x, y;
} ImagePosition;

static ImagePosition image_position = {.x = 0, .y = 0};

// Screen is 128x64 px
static void app_draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);

    canvas_clear(canvas);
    canvas_draw_icon(canvas, image_position.x, image_position.y, &I_dolphin_71x25);
}

static void app_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);

    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

int32_t example_images_main(void* p) {
    UNUSED(p);
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    // Configure view port
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, app_draw_callback, NULL);
    view_port_input_callback_set(view_port, app_input_callback, event_queue);

    // Register view port in GUI
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    InputEvent event;

    bool running = true;
    while(running) {
        if(furi_message_queue_get(event_queue, &event, 100) == FuriStatusOk) {
            if((event.type == InputTypePress) || (event.type == InputTypeRepeat)) {
                switch(event.key) {
                case InputKeyLeft:
                    image_position.x -= 2;
                    break;
                case InputKeyRight:
                    image_position.x += 2;
                    break;
                case InputKeyUp:
                    image_position.y -= 2;
                    break;
                case InputKeyDown:
                    image_position.y += 2;
                    break;
                default:
                    running = false;
                    break;
                }
            }
        }
        view_port_update(view_port);
    }

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);

    furi_record_close(RECORD_GUI);

    return 0;
}

=== END: applications/examples/example_images/example_images.c ===

=== BEGIN: applications/examples/example_number_input/ReadMe.md ===
# Number Input {#example_number_input}

Simple keyboard that limits user inputs to a full number (integer). Useful to enforce correct entries without the need for intense validations after a user input. 

## Source code

Source code for this example can be found [here](https://github.com/flipperdevices/flipperzero-firmware/tree/dev/applications/examples/example_number_input).

## General principle

Definition of min/max values is required. Numbers are of type int32_t. If negative numbers are allowed within min - max, an additional button is displayed to switch the sign between + and -. 

It is also possible to define a header text, as shown in this example app with the 3 different input options. 
=== END: applications/examples/example_number_input/ReadMe.md ===

=== BEGIN: applications/examples/example_number_input/application.fam ===
App(
    appid="example_number_input",
    name="Example: Number Input",
    apptype=FlipperAppType.EXTERNAL,
    entry_point="example_number_input",
    requires=["gui"],
    stack_size=1 * 1024,
    fap_icon="example_number_input_10px.png",
    fap_category="Examples",
)

=== END: applications/examples/example_number_input/application.fam ===

=== BEGIN: applications/examples/example_number_input/example_number_input.c ===
#include "example_number_input.h"

bool example_number_input_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    ExampleNumberInput* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool example_number_input_back_event_callback(void* context) {
    furi_assert(context);
    ExampleNumberInput* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static ExampleNumberInput* example_number_input_alloc() {
    ExampleNumberInput* app = malloc(sizeof(ExampleNumberInput));
    app->gui = furi_record_open(RECORD_GUI);

    app->view_dispatcher = view_dispatcher_alloc();

    app->scene_manager = scene_manager_alloc(&example_number_input_scene_handlers, app);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, example_number_input_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, example_number_input_back_event_callback);

    app->number_input = number_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        ExampleNumberInputViewIdNumberInput,
        number_input_get_view(app->number_input));

    app->dialog_ex = dialog_ex_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        ExampleNumberInputViewIdShowNumber,
        dialog_ex_get_view(app->dialog_ex));

    app->current_number = 5;
    app->min_value = INT32_MIN;
    app->max_value = INT32_MAX;

    return app;
}

static void example_number_input_free(ExampleNumberInput* app) {
    furi_assert(app);

    view_dispatcher_remove_view(app->view_dispatcher, ExampleNumberInputViewIdShowNumber);
    dialog_ex_free(app->dialog_ex);

    view_dispatcher_remove_view(app->view_dispatcher, ExampleNumberInputViewIdNumberInput);
    number_input_free(app->number_input);

    scene_manager_free(app->scene_manager);
    view_dispatcher_free(app->view_dispatcher);

    furi_record_close(RECORD_GUI);
    app->gui = NULL;

    //Remove whatever is left
    free(app);
}

int32_t example_number_input(void* p) {
    UNUSED(p);
    ExampleNumberInput* app = example_number_input_alloc();

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    scene_manager_next_scene(app->scene_manager, ExampleNumberInputSceneShowNumber);

    view_dispatcher_run(app->view_dispatcher);

    example_number_input_free(app);

    return 0;
}

=== END: applications/examples/example_number_input/example_number_input.c ===

=== BEGIN: applications/examples/example_number_input/example_number_input.h ===
#pragma once

#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <gui/elements.h>
#include <gui/scene_manager.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/number_input.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <input/input.h>

#include "scenes/example_number_input_scene.h"

typedef struct ExampleNumberInputShowNumber ExampleNumberInputShowNumber;

typedef enum {
    ExampleNumberInputViewIdShowNumber,
    ExampleNumberInputViewIdNumberInput,
} ExampleNumberInputViewId;

typedef struct {
    Gui* gui;
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;

    NumberInput* number_input;
    DialogEx* dialog_ex;

    int32_t current_number;
    int32_t min_value;
    int32_t max_value;
} ExampleNumberInput;

=== END: applications/examples/example_number_input/example_number_input.h ===

=== BEGIN: applications/examples/example_number_input/scenes/example_number_input_scene.c ===
#include "example_number_input_scene.h"

// Generate scene on_enter handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_enter,
void (*const example_number_input_on_enter_handlers[])(void*) = {
#include "example_number_input_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_event handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_event,
bool (*const example_number_input_on_event_handlers[])(void* context, SceneManagerEvent event) = {
#include "example_number_input_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_exit handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_exit,
void (*const example_number_input_on_exit_handlers[])(void* context) = {
#include "example_number_input_scene_config.h"
};
#undef ADD_SCENE

// Initialize scene handlers configuration structure
const SceneManagerHandlers example_number_input_scene_handlers = {
    .on_enter_handlers = example_number_input_on_enter_handlers,
    .on_event_handlers = example_number_input_on_event_handlers,
    .on_exit_handlers = example_number_input_on_exit_handlers,
    .scene_num = ExampleNumberInputSceneNum,
};

=== END: applications/examples/example_number_input/scenes/example_number_input_scene.c ===

=== BEGIN: applications/examples/example_number_input/scenes/example_number_input_scene.h ===
#pragma once

#include <gui/scene_manager.h>

// Generate scene id and total number
#define ADD_SCENE(prefix, name, id) ExampleNumberInputScene##id,
typedef enum {
#include "example_number_input_scene_config.h"
    ExampleNumberInputSceneNum,
} ExampleNumberInputScene;
#undef ADD_SCENE

extern const SceneManagerHandlers example_number_input_scene_handlers;

// Generate scene on_enter handlers declaration
#define ADD_SCENE(prefix, name, id) void prefix##_scene_##name##_on_enter(void*);
#include "example_number_input_scene_config.h"
#undef ADD_SCENE

// Generate scene on_event handlers declaration
#define ADD_SCENE(prefix, name, id) \
    bool prefix##_scene_##name##_on_event(void* context, SceneManagerEvent event);
#include "example_number_input_scene_config.h"
#undef ADD_SCENE

// Generate scene on_exit handlers declaration
#define ADD_SCENE(prefix, name, id) void prefix##_scene_##name##_on_exit(void* context);
#include "example_number_input_scene_config.h"
#undef ADD_SCENE

=== END: applications/examples/example_number_input/scenes/example_number_input_scene.h ===

=== BEGIN: applications/examples/example_number_input/scenes/example_number_input_scene_config.h ===
ADD_SCENE(example_number_input, input_number, InputNumber)
ADD_SCENE(example_number_input, show_number, ShowNumber)
ADD_SCENE(example_number_input, input_max, InputMax)
ADD_SCENE(example_number_input, input_min, InputMin)

=== END: applications/examples/example_number_input/scenes/example_number_input_scene_config.h ===

=== BEGIN: applications/examples/example_number_input/scenes/example_number_input_scene_input_max.c ===
#include "../example_number_input.h"

void example_number_input_scene_input_max_callback(void* context, int32_t number) {
    ExampleNumberInput* app = context;
    app->max_value = number;
    view_dispatcher_send_custom_event(app->view_dispatcher, 0);
}

void example_number_input_scene_input_max_on_enter(void* context) {
    furi_assert(context);
    ExampleNumberInput* app = context;
    NumberInput* number_input = app->number_input;

    number_input_set_header_text(number_input, "Enter the maximum value");
    number_input_set_result_callback(
        number_input,
        example_number_input_scene_input_max_callback,
        context,
        app->max_value,
        app->min_value,
        INT32_MAX);

    view_dispatcher_switch_to_view(app->view_dispatcher, ExampleNumberInputViewIdNumberInput);
}

bool example_number_input_scene_input_max_on_event(void* context, SceneManagerEvent event) {
    ExampleNumberInput* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_previous_scene(app->scene_manager);
        return true;
    }
    return consumed;
}

void example_number_input_scene_input_max_on_exit(void* context) {
    UNUSED(context);
}

=== END: applications/examples/example_number_input/scenes/example_number_input_scene_input_max.c ===

=== BEGIN: applications/examples/example_number_input/scenes/example_number_input_scene_input_min.c ===
#include "../example_number_input.h"

void example_number_input_scene_input_min_callback(void* context, int32_t number) {
    ExampleNumberInput* app = context;
    app->min_value = number;
    view_dispatcher_send_custom_event(app->view_dispatcher, 0);
}

void example_number_input_scene_input_min_on_enter(void* context) {
    furi_assert(context);
    ExampleNumberInput* app = context;
    NumberInput* number_input = app->number_input;

    number_input_set_header_text(number_input, "Enter the minimum value");
    number_input_set_result_callback(
        number_input,
        example_number_input_scene_input_min_callback,
        context,
        app->min_value,
        INT32_MIN,
        app->max_value);

    view_dispatcher_switch_to_view(app->view_dispatcher, ExampleNumberInputViewIdNumberInput);
}

bool example_number_input_scene_input_min_on_event(void* context, SceneManagerEvent event) {
    ExampleNumberInput* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_previous_scene(app->scene_manager);
        return true;
    }
    return consumed;
}

void example_number_input_scene_input_min_on_exit(void* context) {
    UNUSED(context);
}

=== END: applications/examples/example_number_input/scenes/example_number_input_scene_input_min.c ===

=== BEGIN: applications/examples/example_number_input/scenes/example_number_input_scene_input_number.c ===
#include "../example_number_input.h"

void example_number_input_scene_input_number_callback(void* context, int32_t number) {
    ExampleNumberInput* app = context;
    app->current_number = number;
    view_dispatcher_send_custom_event(app->view_dispatcher, 0);
}

void example_number_input_scene_input_number_on_enter(void* context) {
    furi_assert(context);
    ExampleNumberInput* app = context;
    NumberInput* number_input = app->number_input;

    char str[50];
    snprintf(str, sizeof(str), "Set Number (%ld - %ld)", app->min_value, app->max_value);

    number_input_set_header_text(number_input, str);
    number_input_set_result_callback(
        number_input,
        example_number_input_scene_input_number_callback,
        context,
        app->current_number,
        app->min_value,
        app->max_value);

    view_dispatcher_switch_to_view(app->view_dispatcher, ExampleNumberInputViewIdNumberInput);
}

bool example_number_input_scene_input_number_on_event(void* context, SceneManagerEvent event) {
    ExampleNumberInput* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) { //Back button pressed
        scene_manager_previous_scene(app->scene_manager);
        return true;
    }
    return consumed;
}

void example_number_input_scene_input_number_on_exit(void* context) {
    UNUSED(context);
}

=== END: applications/examples/example_number_input/scenes/example_number_input_scene_input_number.c ===

=== BEGIN: applications/examples/example_number_input/scenes/example_number_input_scene_show_number.c ===
#include "../example_number_input.h"

static void
    example_number_input_scene_confirm_dialog_callback(DialogExResult result, void* context) {
    ExampleNumberInput* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, result);
}

static void example_number_input_scene_update_view(void* context) {
    ExampleNumberInput* app = context;
    DialogEx* dialog_ex = app->dialog_ex;

    dialog_ex_set_header(dialog_ex, "The number is", 64, 0, AlignCenter, AlignTop);

    char buffer[12] = {};
    snprintf(buffer, sizeof(buffer), "%ld", app->current_number);
    dialog_ex_set_text(dialog_ex, buffer, 64, 29, AlignCenter, AlignCenter);

    dialog_ex_set_left_button_text(dialog_ex, "Min");
    dialog_ex_set_right_button_text(dialog_ex, "Max");
    dialog_ex_set_center_button_text(dialog_ex, "Change");

    dialog_ex_set_result_callback(dialog_ex, example_number_input_scene_confirm_dialog_callback);
    dialog_ex_set_context(dialog_ex, app);
}

void example_number_input_scene_show_number_on_enter(void* context) {
    furi_assert(context);
    ExampleNumberInput* app = context;

    example_number_input_scene_update_view(app);

    view_dispatcher_switch_to_view(app->view_dispatcher, ExampleNumberInputViewIdShowNumber);
}

bool example_number_input_scene_show_number_on_event(void* context, SceneManagerEvent event) {
    ExampleNumberInput* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DialogExResultCenter:
            scene_manager_next_scene(app->scene_manager, ExampleNumberInputSceneInputNumber);
            consumed = true;
            break;
        case DialogExResultLeft:
            scene_manager_next_scene(app->scene_manager, ExampleNumberInputSceneInputMin);
            consumed = true;
            break;
        case DialogExResultRight:
            scene_manager_next_scene(app->scene_manager, ExampleNumberInputSceneInputMax);
            consumed = true;
            break;
        default:
            break;
        }
    }

    return consumed;
}

void example_number_input_scene_show_number_on_exit(void* context) {
    UNUSED(context);
}

=== END: applications/examples/example_number_input/scenes/example_number_input_scene_show_number.c ===

=== BEGIN: applications/examples/example_plugins/application.fam ===
App(
    appid="example_plugins",
    name="Example: App w/plugin",
    apptype=FlipperAppType.EXTERNAL,
    entry_point="example_plugins_app",
    stack_size=2 * 1024,
    fap_category="Examples",
    sources=["*.c", "!plugin*.c"],
)

App(
    appid="example_plugins_multi",
    name="Example: App w/plugins",
    apptype=FlipperAppType.EXTERNAL,
    entry_point="example_plugins_multi_app",
    stack_size=2 * 1024,
    fap_category="Examples",
)

App(
    appid="example_plugin1",
    apptype=FlipperAppType.PLUGIN,
    entry_point="example_plugin1_ep",
    requires=["example_plugins", "example_plugins_multi"],
    sources=["plugin1.c"],
)

App(
    appid="example_plugin2",
    apptype=FlipperAppType.PLUGIN,
    entry_point="example_plugin2_ep",
    requires=["example_plugins_multi"],
    sources=["plugin2.c"],
)

=== END: applications/examples/example_plugins/application.fam ===

=== BEGIN: applications/examples/example_plugins/example_plugins.c ===
/**
 * @file example_plugins.c
 * @brief Plugin host application example.
 *
 * Loads a single plugin and calls its methods.
 */

#include "plugin_interface.h"

#include <furi.h>

#include <flipper_application/flipper_application.h>
#include <loader/firmware_api/firmware_api.h>
#include <storage/storage.h>

#define TAG "ExamplePlugins"

int32_t example_plugins_app(void* p) {
    UNUSED(p);

    FURI_LOG_I(TAG, "Starting");

    Storage* storage = furi_record_open(RECORD_STORAGE);

    FlipperApplication* app = flipper_application_alloc(storage, firmware_api_interface);

    do {
        FlipperApplicationPreloadStatus preload_res =
            flipper_application_preload(app, APP_DATA_PATH("plugins/example_plugin1.fal"));

        if(preload_res != FlipperApplicationPreloadStatusSuccess) {
            FURI_LOG_E(TAG, "Failed to preload plugin");
            break;
        }

        if(!flipper_application_is_plugin(app)) {
            FURI_LOG_E(TAG, "Plugin file is not a library");
            break;
        }

        FlipperApplicationLoadStatus load_status = flipper_application_map_to_memory(app);
        if(load_status != FlipperApplicationLoadStatusSuccess) {
            FURI_LOG_E(TAG, "Failed to load plugin file");
            break;
        }

        const FlipperAppPluginDescriptor* app_descriptor =
            flipper_application_plugin_get_descriptor(app);

        FURI_LOG_I(
            TAG,
            "Loaded plugin for appid '%s', API %lu",
            app_descriptor->appid,
            app_descriptor->ep_api_version);

        furi_check(app_descriptor->ep_api_version == PLUGIN_API_VERSION);
        furi_check(strcmp(app_descriptor->appid, PLUGIN_APP_ID) == 0);

        const ExamplePlugin* plugin = app_descriptor->entry_point;

        FURI_LOG_I(TAG, "Plugin name: %s", plugin->name);
        FURI_LOG_I(TAG, "Plugin method1: %d", plugin->method1());
        FURI_LOG_I(TAG, "Plugin method2(7,8): %d", plugin->method2(7, 8));
        FURI_LOG_I(TAG, "Plugin method2(1337,228): %d", plugin->method2(1337, 228));
    } while(false);
    flipper_application_free(app);

    furi_record_close(RECORD_STORAGE);
    FURI_LOG_I(TAG, "Goodbye!");

    return 0;
}

=== END: applications/examples/example_plugins/example_plugins.c ===

=== BEGIN: applications/examples/example_plugins/example_plugins_multi.c ===
/**
 * @file example_plugins_multi.c
 * @brief Advanced plugin host application example.
 *
 * It uses PluginManager to load all plugins from a directory
 */

#include "plugin_interface.h"

#include <flipper_application/flipper_application.h>
#include <flipper_application/plugins/plugin_manager.h>
#include <loader/firmware_api/firmware_api.h>

#include <furi.h>

#define TAG "ExamplePlugins"

int32_t example_plugins_multi_app(void* p) {
    UNUSED(p);

    FURI_LOG_I(TAG, "Starting");

    PluginManager* manager =
        plugin_manager_alloc(PLUGIN_APP_ID, PLUGIN_API_VERSION, firmware_api_interface);

    if(plugin_manager_load_all(manager, APP_DATA_PATH("plugins")) != PluginManagerErrorNone) {
        FURI_LOG_E(TAG, "Failed to load all libs");
        return 0;
    }

    uint32_t plugin_count = plugin_manager_get_count(manager);
    FURI_LOG_I(TAG, "Loaded %lu plugin(s)", plugin_count);

    for(uint32_t i = 0; i < plugin_count; i++) {
        const ExamplePlugin* plugin = plugin_manager_get_ep(manager, i);
        FURI_LOG_I(TAG, "plugin name: %s", plugin->name);
        FURI_LOG_I(TAG, "plugin method1: %d", plugin->method1());
        FURI_LOG_I(TAG, "plugin method2(7,8): %d", plugin->method2(7, 8));
    }

    plugin_manager_free(manager);
    FURI_LOG_I(TAG, "Goodbye!");

    return 0;
}

=== END: applications/examples/example_plugins/example_plugins_multi.c ===

=== BEGIN: applications/examples/example_plugins/plugin1.c ===
/**
 * @file plugin1.c
 * @brief Plugin example 1.
 *
 * A simple plugin implementing example_plugins application's plugin interface
 */

#include "plugin_interface.h"

#include <flipper_application/flipper_application.h>

static int example_plugin1_method1(void) {
    return 42;
}

static int example_plugin1_method2(int arg1, int arg2) {
    return arg1 + arg2;
}

/* Actual implementation of app<>plugin interface */
static const ExamplePlugin example_plugin1 = {
    .name = "Demo App Plugin 1",
    .method1 = &example_plugin1_method1,
    .method2 = &example_plugin1_method2,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor example_plugin1_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &example_plugin1,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* example_plugin1_ep(void) {
    return &example_plugin1_descriptor;
}

=== END: applications/examples/example_plugins/plugin1.c ===

=== BEGIN: applications/examples/example_plugins/plugin2.c ===
/**
 * @file plugin2.c
 * @brief Plugin example 2.
 *
 * Second plugin implementing example_plugins application's plugin interface
 */

#include "plugin_interface.h"

#include <flipper_application/flipper_application.h>

static int example_plugin2_method1(void) {
    return 1337;
}

static int example_plugin2_method2(int arg1, int arg2) {
    return arg1 - arg2;
}

/* Actual implementation of app<>plugin interface */
static const ExamplePlugin example_plugin2 = {
    .name = "Demo App Plugin 2",
    .method1 = &example_plugin2_method1,
    .method2 = &example_plugin2_method2,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor example_plugin2_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &example_plugin2,
};

/* Plugin entry point - must return a pointer to const descriptor */
const FlipperAppPluginDescriptor* example_plugin2_ep(void) {
    return &example_plugin2_descriptor;
}

=== END: applications/examples/example_plugins/plugin2.c ===

=== BEGIN: applications/examples/example_plugins/plugin_interface.h ===
/**
 * @file plugin_interface.h
 * @brief Example plugin interface.
 *
 * Common interface between a plugin and host application
 */
#pragma once

#define PLUGIN_APP_ID      "example_plugins"
#define PLUGIN_API_VERSION 1

typedef struct {
    const char* name;
    int (*method1)(void);
    int (*method2)(int, int);
} ExamplePlugin;

=== END: applications/examples/example_plugins/plugin_interface.h ===

=== BEGIN: applications/examples/example_plugins_advanced/app_api.c ===
#include "app_api.h"

/* Actual implementation of app's API and its private state */

static uint32_t accumulator = 0;

void app_api_accumulator_set(uint32_t value) {
    accumulator = value;
}

uint32_t app_api_accumulator_get(void) {
    return accumulator;
}

void app_api_accumulator_add(uint32_t value) {
    accumulator += value;
}

void app_api_accumulator_sub(uint32_t value) {
    accumulator -= value;
}

void app_api_accumulator_mul(uint32_t value) {
    accumulator *= value;
}

=== END: applications/examples/example_plugins_advanced/app_api.c ===

=== BEGIN: applications/examples/example_plugins_advanced/app_api.h ===
/**
 * @file app_api.h
 * @brief Application API example.
 *
 * This file contains an API that is internally implemented by the application
 * It is also exposed to plugins to allow them to use the application's API.
 */
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void app_api_accumulator_set(uint32_t value);

uint32_t app_api_accumulator_get(void);

void app_api_accumulator_add(uint32_t value);

void app_api_accumulator_sub(uint32_t value);

void app_api_accumulator_mul(uint32_t value);

#ifdef __cplusplus
}
#endif

=== END: applications/examples/example_plugins_advanced/app_api.h ===

=== BEGIN: applications/examples/example_plugins_advanced/app_api_interface.h ===
#pragma once

#include <flipper_application/api_hashtable/api_hashtable.h>

/* 
 * Resolver interface with private application's symbols. 
 * Implementation is contained in app_api_table.c
 */
extern const ElfApiInterface* const application_api_interface;

=== END: applications/examples/example_plugins_advanced/app_api_interface.h ===

=== BEGIN: applications/examples/example_plugins_advanced/app_api_table.cpp ===
#include <flipper_application/api_hashtable/api_hashtable.h>
#include <flipper_application/api_hashtable/compilesort.hpp>

/* 
 * This file contains an implementation of a symbol table 
 * with private app's symbols. It is used by composite API resolver
 * to load plugins that use internal application's APIs.
 */
#include "app_api_table_i.h"

static_assert(!has_hash_collisions(app_api_table), "Detected API method hash collision!");

constexpr HashtableApiInterface applicaton_hashtable_api_interface{
    {
        .api_version_major = 0,
        .api_version_minor = 0,
        /* generic resolver using pre-sorted array */
        .resolver_callback = &elf_resolve_from_hashtable,
    },
    /* pointers to application's API table boundaries */
    app_api_table.cbegin(),
    app_api_table.cend(),
};

/* Casting to generic resolver to use in Composite API resolver */
extern "C" const ElfApiInterface* const application_api_interface =
    &applicaton_hashtable_api_interface;

=== END: applications/examples/example_plugins_advanced/app_api_table.cpp ===

=== BEGIN: applications/examples/example_plugins_advanced/app_api_table_i.h ===
#include "app_api.h"

/* 
 * A list of app's private functions and objects to expose for plugins.
 * It is used to generate a table of symbols for import resolver to use.
 * TBD: automatically generate this table from app's header files
 */
static constexpr auto app_api_table = sort(create_array_t<sym_entry>(
    API_METHOD(app_api_accumulator_set, void, (uint32_t)),
    API_METHOD(app_api_accumulator_get, uint32_t, ()),
    API_METHOD(app_api_accumulator_add, void, (uint32_t)),
    API_METHOD(app_api_accumulator_sub, void, (uint32_t)),
    API_METHOD(app_api_accumulator_mul, void, (uint32_t))));

=== END: applications/examples/example_plugins_advanced/app_api_table_i.h ===

=== BEGIN: applications/examples/example_plugins_advanced/application.fam ===
App(
    appid="example_advanced_plugins",
    name="Example: advanced plugins",
    apptype=FlipperAppType.EXTERNAL,
    entry_point="example_advanced_plugins_app",
    stack_size=2 * 1024,
    fap_category="Examples",
    sources=["*.c*", "!plugin*.c"],
)

App(
    appid="advanced_plugin1",
    apptype=FlipperAppType.PLUGIN,
    entry_point="advanced_plugin1_ep",
    requires=["example_advanced_plugins"],
    sources=["plugin1.c"],
    fal_embedded=True,
)

App(
    appid="advanced_plugin2",
    apptype=FlipperAppType.PLUGIN,
    entry_point="advanced_plugin2_ep",
    requires=["example_advanced_plugins"],
    sources=["plugin2.c"],
    fal_embedded=True,
)

=== END: applications/examples/example_plugins_advanced/application.fam ===

=== BEGIN: applications/examples/example_plugins_advanced/example_advanced_plugins.c ===
#include "app_api.h"
#include "plugin_interface.h"
#include "app_api_interface.h"

#include <flipper_application/flipper_application.h>
#include <flipper_application/plugins/plugin_manager.h>
#include <flipper_application/plugins/composite_resolver.h>

#include <loader/firmware_api/firmware_api.h>

#define TAG "ExampleAdvancedPlugins"

int32_t example_advanced_plugins_app(void* p) {
    UNUSED(p);

    FURI_LOG_I(TAG, "Starting");

    CompositeApiResolver* resolver = composite_api_resolver_alloc();
    composite_api_resolver_add(resolver, firmware_api_interface);
    composite_api_resolver_add(resolver, application_api_interface);

    PluginManager* manager = plugin_manager_alloc(
        PLUGIN_APP_ID, PLUGIN_API_VERSION, composite_api_resolver_get(resolver));

    do {
        // For built-in .fals (fal_embedded==True), use APP_ASSETS_PATH
        // Otherwise, use APP_DATA_PATH
        if(plugin_manager_load_all(manager, APP_ASSETS_PATH("plugins")) !=
           PluginManagerErrorNone) {
            FURI_LOG_E(TAG, "Failed to load all libs");
            break;
        }

        uint32_t plugin_count = plugin_manager_get_count(manager);
        FURI_LOG_I(TAG, "Loaded libs: %lu", plugin_count);

        for(uint32_t i = 0; i < plugin_count; i++) {
            const AdvancedPlugin* plugin = plugin_manager_get_ep(manager, i);
            FURI_LOG_I(TAG, "plugin name: %s. Calling methods", plugin->name);
            plugin->method1(228);
            plugin->method2();
            FURI_LOG_I(TAG, "Accumulator: %lu", app_api_accumulator_get());
        }
    } while(0);

    plugin_manager_free(manager);
    composite_api_resolver_free(resolver);
    FURI_LOG_I(TAG, "Goodbye!");

    return 0;
}

=== END: applications/examples/example_plugins_advanced/example_advanced_plugins.c ===

=== BEGIN: applications/examples/example_plugins_advanced/plugin1.c ===
/**
 * @file plugin1.c
 * @brief Plugin example 1.
 *
 * This plugin uses both firmware's API interface and private application headers.
 * It can be loaded by a plugin manager that uses CompoundApiInterface,
 * which combines both interfaces.
 */

#include "app_api.h"
#include "plugin_interface.h"

#include <flipper_application/flipper_application.h>
#include <furi.h>

static void advanced_plugin1_method1(int arg1) {
    /* This function is implemented inside host application */
    app_api_accumulator_add(arg1);
}

static void advanced_plugin1_method2(void) {
    /* Accumulator value is stored inside host application */
    FURI_LOG_I("TEST", "Plugin 1, accumulator: %lu", app_api_accumulator_get());
}

/* Actual implementation of app<>plugin interface */
static const AdvancedPlugin advanced_plugin1 = {
    .name = "Advanced Plugin 1",
    .method1 = &advanced_plugin1_method1,
    .method2 = &advanced_plugin1_method2,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor advanced_plugin1_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &advanced_plugin1,
};

/* Plugin entry point - must return a pointer to const descriptor */
const FlipperAppPluginDescriptor* advanced_plugin1_ep(void) {
    return &advanced_plugin1_descriptor;
}

=== END: applications/examples/example_plugins_advanced/plugin1.c ===

=== BEGIN: applications/examples/example_plugins_advanced/plugin2.c ===
/**
 * @file plugin2.c
 * @brief Plugin example 2.
 *
 * This plugin uses both firmware's API interface and private application headers.
 * It can be loaded by a plugin manager that uses CompoundApiInterface,
 * which combines both interfaces.
 */

#include "app_api.h"
#include "plugin_interface.h"

#include <flipper_application/flipper_application.h>
#include <furi.h>

static void advanced_plugin2_method1(int arg1) {
    /* This function is implemented inside host application */
    app_api_accumulator_mul(arg1);
}

static void advanced_plugin2_method2(void) {
    /* Accumulator value is stored inside host application */
    FURI_LOG_I("TEST", "Plugin 2, accumulator: %lu", app_api_accumulator_get());
}

/* Actual implementation of app<>plugin interface */
static const AdvancedPlugin advanced_plugin2 = {
    .name = "Advanced Plugin 2",
    .method1 = &advanced_plugin2_method1,
    .method2 = &advanced_plugin2_method2,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor advanced_plugin2_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &advanced_plugin2,
};

/* Plugin entry point - must return a pointer to const descriptor */
const FlipperAppPluginDescriptor* advanced_plugin2_ep(void) {
    return &advanced_plugin2_descriptor;
}

=== END: applications/examples/example_plugins_advanced/plugin2.c ===

=== BEGIN: applications/examples/example_plugins_advanced/plugin_interface.h ===
/**
 * @file plugin_interface.h
 * @brief Example plugin interface.
 *
 * Common interface between a plugin and host application
 */
#pragma once

#define PLUGIN_APP_ID      "example_plugins_advanced"
#define PLUGIN_API_VERSION 1

typedef struct {
    const char* name;
    void (*method1)(int);
    void (*method2)(void);
} AdvancedPlugin;

=== END: applications/examples/example_plugins_advanced/plugin_interface.h ===

=== BEGIN: applications/examples/example_thermo/README.md ===
# 1-Wire Thermometer {#example_thermo}

This example application demonstrates the use of the 1-Wire library with a DS18B20 thermometer. 
It also covers basic GUI, input handling, threads and localisation.

## Source code

Source code for this example can be found [here](https://github.com/flipperdevices/flipperzero-firmware/tree/dev/applications/examples/example_thermo).

## Electrical connections

Before launching the application, connect the sensor to Flipper's external GPIO according to the table below:
| DS18B20 | Flipper |
| :-----: | :-----: |
| VDD | 9 |
| GND | 18 |
| DQ  | 17 |

*NOTE 1*: GND is also available on pins 8 and 11.

*NOTE 2*: For any other pin than 17, connect an external 4.7k pull-up resistor to pin 9.

## Launching the application

In order to launch this demo, follow the steps below:
1. Make sure your Flipper has an SD card installed.
2. Connect your Flipper to the computer via a USB cable.
3. Run `./fbt launch APPSRC=example_thermo` in your terminal emulator of choice.

## Changing the data pin

It is possible to use other GPIO pin as a 1-Wire data pin. In order to change it, set the `THERMO_GPIO_PIN` macro to any of the options listed below:

```c
/* Possible GPIO pin choices:
 - gpio_ext_pc0
 - gpio_ext_pc1
 - gpio_ext_pc3
 - gpio_ext_pb2
 - gpio_ext_pb3
 - gpio_ext_pa4
 - gpio_ext_pa6
 - gpio_ext_pa7
 - gpio_ibutton
*/

#define THERMO_GPIO_PIN (gpio_ibutton)
```
Do not forget about the external pull-up resistor as these pins do not have one built-in.

With the changes been made, recompile and launch the application again. 
The on-screen text should reflect it by asking to connect the thermometer to another pin.

=== END: applications/examples/example_thermo/README.md ===

=== BEGIN: applications/examples/example_thermo/application.fam ===
App(
    appid="example_thermo",
    name="Example: Thermometer",
    apptype=FlipperAppType.EXTERNAL,
    entry_point="example_thermo_main",
    requires=["gui"],
    stack_size=1 * 1024,
    fap_icon="example_thermo_10px.png",
    fap_category="Examples",
)

=== END: applications/examples/example_thermo/application.fam ===

=== BEGIN: applications/examples/example_thermo/example_thermo.c ===
/**
 * @file example_thermo.c
 * @brief 1-Wire thermometer example.
 *
 * This file contains an example application that reads and displays
 * the temperature from a DS18B20 1-wire thermometer.
 *
 * It also covers basic GUI, input handling, threads and localisation.
 *
 * References:
 * [1] DS18B20 Datasheet: https://www.analog.com/media/en/technical-documentation/data-sheets/DS18B20.pdf
 */

#include <gui/gui.h>
#include <gui/view_port.h>

#include <core/thread.h>
#include <core/kernel.h>

#include <locale/locale.h>

#include <one_wire/maxim_crc.h>
#include <one_wire/one_wire_host.h>

#include <furi_hal_power.h>

#define UPDATE_PERIOD_MS 1000UL
#define TEXT_STORE_SIZE  64U

#define DS18B20_CMD_SKIP_ROM        0xccU
#define DS18B20_CMD_CONVERT         0x44U
#define DS18B20_CMD_READ_SCRATCHPAD 0xbeU

#define DS18B20_CFG_RESOLUTION_POS  5U
#define DS18B20_CFG_RESOLUTION_MASK 0x03U
#define DS18B20_DECIMAL_PART_MASK   0x0fU

#define DS18B20_SIGN_MASK 0xf0U

/* Possible GPIO pin choices:
 - gpio_ext_pc0
 - gpio_ext_pc1
 - gpio_ext_pc3
 - gpio_ext_pb2
 - gpio_ext_pb3
 - gpio_ext_pa4
 - gpio_ext_pa6
 - gpio_ext_pa7
 - gpio_ibutton
*/

#define THERMO_GPIO_PIN (gpio_ibutton)

/* Flags which the reader thread responds to */
typedef enum {
    ReaderThreadFlagExit = 1,
} ReaderThreadFlag;

typedef union {
    struct {
        uint8_t temp_lsb; /* Least significant byte of the temperature */
        uint8_t temp_msb; /* Most significant byte of the temperature */
        uint8_t user_alarm_high; /* User register 1 (Temp high alarm) */
        uint8_t user_alarm_low; /* User register 2 (Temp low alarm) */
        uint8_t config; /* Configuration register */
        uint8_t reserved[3]; /* Not used */
        uint8_t crc; /* CRC checksum for error detection */
    } fields;
    uint8_t bytes[9];
} DS18B20Scratchpad;

/* Application context structure */
typedef struct {
    Gui* gui;
    ViewPort* view_port;
    FuriThread* reader_thread;
    FuriMessageQueue* event_queue;
    OneWireHost* onewire;
    float temp_celsius;
    bool has_device;
} ExampleThermoContext;

/*************** 1-Wire Communication and Processing *****************/

/* Commands the thermometer to begin measuring the temperature. */
static void example_thermo_request_temperature(ExampleThermoContext* context) {
    OneWireHost* onewire = context->onewire;

    /* All 1-wire transactions must happen in a critical section, i.e
       not interrupted by other threads. */
    FURI_CRITICAL_ENTER();

    bool success = false;
    do {
        /* Each communication with a 1-wire device starts by a reset.
           The function will return true if a device responded with a presence pulse. */
        if(!onewire_host_reset(onewire)) break;
        /* After the reset, a ROM operation must follow.
           If there is only one device connected, the "Skip ROM" command is most appropriate
           (it can also be used to address all of the connected devices in some cases).*/
        onewire_host_write(onewire, DS18B20_CMD_SKIP_ROM);
        /* After the ROM operation, a device-specific command is issued.
           In this case, it's a request to start measuring the temperature. */
        onewire_host_write(onewire, DS18B20_CMD_CONVERT);

        success = true;
    } while(false);

    context->has_device = success;

    FURI_CRITICAL_EXIT();
}

/* Reads the measured temperature from the thermometer. */
static void example_thermo_read_temperature(ExampleThermoContext* context) {
    /* If there was no device detected, don't try to read the temperature */
    if(!context->has_device) {
        return;
    }

    OneWireHost* onewire = context->onewire;

    /* All 1-wire transactions must happen in a critical section, i.e
       not interrupted by other threads. */
    FURI_CRITICAL_ENTER();

    bool success = false;

    do {
        DS18B20Scratchpad buf;

        /* Attempt reading the temperature 10 times before giving up */
        size_t attempts_left = 10;
        do {
            /* Each communication with a 1-wire device starts by a reset.
            The function will return true if a device responded with a presence pulse. */
            if(!onewire_host_reset(onewire)) continue;

            /* After the reset, a ROM operation must follow.
            If there is only one device connected, the "Skip ROM" command is most appropriate
            (it can also be used to address all of the connected devices in some cases).*/
            onewire_host_write(onewire, DS18B20_CMD_SKIP_ROM);

            /* After the ROM operation, a device-specific command is issued.
            This time, it will be the "Read Scratchpad" command which will
            prepare the device's internal buffer memory for reading. */
            onewire_host_write(onewire, DS18B20_CMD_READ_SCRATCHPAD);

            /* The actual reading happens here. A total of 9 bytes is read. */
            onewire_host_read_bytes(onewire, buf.bytes, sizeof(buf.bytes));

            /* Calculate the checksum and compare it with one provided by the device. */
            const uint8_t crc = maxim_crc8(buf.bytes, sizeof(buf.bytes) - 1, MAXIM_CRC8_INIT);

            /* Checksums match, exit the loop */
            if(crc == buf.fields.crc) break;

        } while(--attempts_left);

        if(attempts_left == 0) break;

        /* Get the measurement resolution from the configuration register. (See [1] page 9) */
        const uint8_t resolution_mode = (buf.fields.config >> DS18B20_CFG_RESOLUTION_POS) &
                                        DS18B20_CFG_RESOLUTION_MASK;

        /* Generate a mask for undefined bits in the decimal part. (See [1] page 6) */
        const uint8_t decimal_mask =
            (DS18B20_DECIMAL_PART_MASK << (DS18B20_CFG_RESOLUTION_MASK - resolution_mode)) &
            DS18B20_DECIMAL_PART_MASK;

        /* Get the integer and decimal part of the temperature (See [1] page 6) */
        const uint8_t integer_part = (buf.fields.temp_msb << 4U) | (buf.fields.temp_lsb >> 4U);
        const uint8_t decimal_part = buf.fields.temp_lsb & decimal_mask;

        /* Calculate the sign of the temperature (See [1] page 6) */
        const bool is_negative = (buf.fields.temp_msb & DS18B20_SIGN_MASK) != 0;

        /* Combine the integer and decimal part together */
        const float temp_celsius_abs = integer_part + decimal_part / 16.f;

        /* Set the appropriate sign */
        context->temp_celsius = is_negative ? -temp_celsius_abs : temp_celsius_abs;

        success = true;
    } while(false);

    context->has_device = success;

    FURI_CRITICAL_EXIT();
}

/* Periodically requests measurements and reads temperature. This function runs in a separare thread. */
static int32_t example_thermo_reader_thread_callback(void* ctx) {
    ExampleThermoContext* context = ctx;

    for(;;) {
        /* Tell the termometer to start measuring the temperature. The process may take up to 750ms. */
        example_thermo_request_temperature(context);

        /* Wait for the measurement to finish. At the same time wait for an exit signal. */
        const uint32_t flags =
            furi_thread_flags_wait(ReaderThreadFlagExit, FuriFlagWaitAny, UPDATE_PERIOD_MS);

        /* If an exit signal was received, return from this thread. */
        if(flags != (unsigned)FuriFlagErrorTimeout) break;

        /* The measurement is now ready, read it from the termometer. */
        example_thermo_read_temperature(context);
    }

    return 0;
}

/*************** GUI, Input and Main Loop *****************/

/* Draw the GUI of the application. The screen is completely redrawn during each call. */
static void example_thermo_draw_callback(Canvas* canvas, void* ctx) {
    ExampleThermoContext* context = ctx;
    char text_store[TEXT_STORE_SIZE];
    const size_t middle_x = canvas_width(canvas) / 2U;

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, middle_x, 12, AlignCenter, AlignBottom, "Thermometer Demo");
    canvas_draw_line(canvas, 0, 16, 128, 16);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, middle_x, 30, AlignCenter, AlignBottom, "Connect thermometer");

    snprintf(
        text_store,
        TEXT_STORE_SIZE,
        "to GPIO pin %ld",
        furi_hal_resources_get_ext_pin_number(&THERMO_GPIO_PIN));
    canvas_draw_str_aligned(canvas, middle_x, 42, AlignCenter, AlignBottom, text_store);

    canvas_set_font(canvas, FontKeyboard);

    if(context->has_device) {
        float temp;
        char temp_units;

        /* The application is locale-aware.
           Change Settings->System->Units to check it out. */
        switch(locale_get_measurement_unit()) {
        case LocaleMeasurementUnitsMetric:
            temp = context->temp_celsius;
            temp_units = 'C';
            break;
        case LocaleMeasurementUnitsImperial:
            temp = locale_celsius_to_fahrenheit(context->temp_celsius);
            temp_units = 'F';
            break;
        default:
            furi_crash("Illegal measurement units");
        }
        /* If a reading is available, display it */
        snprintf(text_store, TEXT_STORE_SIZE, "Temperature: %+.1f%c", (double)temp, temp_units);
    } else {
        /* Or show a message that no data is available */
        strlcpy(text_store, "-- No data --", TEXT_STORE_SIZE);
    }

    canvas_draw_str_aligned(canvas, middle_x, 58, AlignCenter, AlignBottom, text_store);
}

/* This function is called from the GUI thread. All it does is put the event
   into the application's queue so it can be processed later. */
static void example_thermo_input_callback(InputEvent* event, void* ctx) {
    ExampleThermoContext* context = ctx;
    furi_message_queue_put(context->event_queue, event, FuriWaitForever);
}

/* Starts the reader thread and handles the input */
static void example_thermo_run(ExampleThermoContext* context) {
    /* Enable power on external pins */
    furi_hal_power_enable_otg();

    /* Configure the hardware in host mode */
    onewire_host_start(context->onewire);

    /* Start the reader thread. It will talk to the thermometer in the background. */
    furi_thread_start(context->reader_thread);

    /* An endless loop which handles the input*/
    for(bool is_running = true; is_running;) {
        InputEvent event;
        /* Wait for an input event. Input events come from the GUI thread via a callback. */
        const FuriStatus status =
            furi_message_queue_get(context->event_queue, &event, FuriWaitForever);

        /* This application is only interested in short button presses. */
        if((status != FuriStatusOk) || (event.type != InputTypeShort)) {
            continue;
        }

        /* When the user presses the "Back" button, break the loop and exit the application. */
        if(event.key == InputKeyBack) {
            is_running = false;
        }
    }

    /* Signal the reader thread to cease operation and exit */
    furi_thread_flags_set(furi_thread_get_id(context->reader_thread), ReaderThreadFlagExit);

    /* Wait for the reader thread to finish */
    furi_thread_join(context->reader_thread);

    /* Reset the hardware */
    onewire_host_stop(context->onewire);

    /* Disable power on external pins */
    furi_hal_power_disable_otg();
}

/******************** Initialisation & startup *****************************/

/* Allocate the memory and initialise the variables */
static ExampleThermoContext* example_thermo_context_alloc(void) {
    ExampleThermoContext* context = malloc(sizeof(ExampleThermoContext));

    context->view_port = view_port_alloc();
    view_port_draw_callback_set(context->view_port, example_thermo_draw_callback, context);
    view_port_input_callback_set(context->view_port, example_thermo_input_callback, context);

    context->event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    context->reader_thread = furi_thread_alloc();
    furi_thread_set_stack_size(context->reader_thread, 1024U);
    furi_thread_set_context(context->reader_thread, context);
    furi_thread_set_callback(context->reader_thread, example_thermo_reader_thread_callback);

    context->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(context->gui, context->view_port, GuiLayerFullscreen);

    context->onewire = onewire_host_alloc(&THERMO_GPIO_PIN);

    return context;
}

/* Release the unused resources and deallocate memory */
static void example_thermo_context_free(ExampleThermoContext* context) {
    view_port_enabled_set(context->view_port, false);
    gui_remove_view_port(context->gui, context->view_port);

    onewire_host_free(context->onewire);
    furi_thread_free(context->reader_thread);
    furi_message_queue_free(context->event_queue);
    view_port_free(context->view_port);

    furi_record_close(RECORD_GUI);
}

/* The application's entry point. Execution starts from here. */
int32_t example_thermo_main(void* p) {
    UNUSED(p);

    /* Allocate all of the necessary structures */
    ExampleThermoContext* context = example_thermo_context_alloc();

    /* Start the application's main loop. It won't return until the application was requested to exit. */
    example_thermo_run(context);

    /* Release all unneeded resources */
    example_thermo_context_free(context);

    return 0;
}

=== END: applications/examples/example_thermo/example_thermo.c ===

=== BEGIN: applications/examples/example_view_dispatcher/application.fam ===
App(
    appid="example_view_dispatcher",
    name="Example: ViewDispatcher",
    apptype=FlipperAppType.EXTERNAL,
    entry_point="example_view_dispatcher_app",
    requires=["gui"],
    fap_category="Examples",
)

=== END: applications/examples/example_view_dispatcher/application.fam ===

=== BEGIN: applications/examples/example_view_dispatcher/example_view_dispatcher.c ===
/**
 * @file example_view_dispatcher.c
 * @brief Example application demonstrating the usage of the ViewDispatcher library.
 *
 * This application can display one of two views: either a Widget or a Submenu.
 * Each view has its own way of switching to another one:
 *
 * - A center button in the Widget view.
 * - A submenu item in the Submenu view
 *
 * Press either to switch to a different view. Press Back to exit the application.
 *
 */

#include <gui/gui.h>
#include <gui/view_dispatcher.h>

#include <gui/modules/widget.h>
#include <gui/modules/submenu.h>

// Enumeration of the view indexes.
typedef enum {
    ViewIndexWidget,
    ViewIndexSubmenu,
    ViewIndexCount,
} ViewIndex;

// Enumeration of submenu items.
typedef enum {
    SubmenuIndexNothing,
    SubmenuIndexSwitchView,
} SubmenuIndex;

// Main application structure.
typedef struct {
    ViewDispatcher* view_dispatcher;
    Widget* widget;
    Submenu* submenu;
} ExampleViewDispatcherApp;

// This function is called when the user has pressed the Back key.
static bool example_view_dispatcher_app_navigation_callback(void* context) {
    furi_assert(context);
    ExampleViewDispatcherApp* app = context;
    // Back means exit the application, which can be done by stopping the ViewDispatcher.
    view_dispatcher_stop(app->view_dispatcher);
    return true;
}

// This function is called when there are custom events to process.
static bool example_view_dispatcher_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    ExampleViewDispatcherApp* app = context;
    // The event numerical value can mean different things (the application is responsible to uphold its chosen convention)
    // In this example, the only possible meaning is the view index to switch to.
    furi_assert(event < ViewIndexCount);
    // Switch to the requested view.
    view_dispatcher_switch_to_view(app->view_dispatcher, event);

    return true;
}

// This function is called when the user presses the "Switch View" button on the Widget view.
static void example_view_dispatcher_app_button_callback(
    GuiButtonType button_type,
    InputType input_type,
    void* context) {
    furi_assert(context);
    ExampleViewDispatcherApp* app = context;
    // Only request the view switch if the user short-presses the Center button.
    if(button_type == GuiButtonTypeCenter && input_type == InputTypeShort) {
        // Request switch to the Submenu view via the custom event queue.
        view_dispatcher_send_custom_event(app->view_dispatcher, ViewIndexSubmenu);
    }
}

// This function is called when the user activates the "Switch View" submenu item.
static void example_view_dispatcher_app_submenu_callback(void* context, uint32_t index) {
    furi_assert(context);
    ExampleViewDispatcherApp* app = context;
    // Only request the view switch if the user activates the "Switch View" item.
    if(index == SubmenuIndexSwitchView) {
        // Request switch to the Widget view via the custom event queue.
        view_dispatcher_send_custom_event(app->view_dispatcher, ViewIndexWidget);
    }
}

// Application constructor function.
static ExampleViewDispatcherApp* example_view_dispatcher_app_alloc() {
    ExampleViewDispatcherApp* app = malloc(sizeof(ExampleViewDispatcherApp));
    // Access the GUI API instance.
    Gui* gui = furi_record_open(RECORD_GUI);
    // Create and initialize the Widget view.
    app->widget = widget_alloc();
    widget_add_string_multiline_element(
        app->widget, 64, 32, AlignCenter, AlignCenter, FontSecondary, "Press the Button below");
    widget_add_button_element(
        app->widget,
        GuiButtonTypeCenter,
        "Switch View",
        example_view_dispatcher_app_button_callback,
        app);
    // Create and initialize the Submenu view.
    app->submenu = submenu_alloc();
    submenu_add_item(app->submenu, "Do Nothing", SubmenuIndexNothing, NULL, NULL);
    submenu_add_item(
        app->submenu,
        "Switch View",
        SubmenuIndexSwitchView,
        example_view_dispatcher_app_submenu_callback,
        app);
    // Create the ViewDispatcher instance.
    app->view_dispatcher = view_dispatcher_alloc();
    // Let the GUI know about this ViewDispatcher instance.
    view_dispatcher_attach_to_gui(app->view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    // Register the views within the ViewDispatcher instance. This alone will not show any of them on the screen.
    // Each view must have its own index to refer to it later (it is best done via an enumeration as shown here).
    view_dispatcher_add_view(app->view_dispatcher, ViewIndexWidget, widget_get_view(app->widget));
    view_dispatcher_add_view(
        app->view_dispatcher, ViewIndexSubmenu, submenu_get_view(app->submenu));
    // Set the custom event callback. It will be called each time a custom event is scheduled
    // using the view_dispatcher_send_custom_callback() function.
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, example_view_dispatcher_app_custom_event_callback);
    // Set the navigation, or back button callback. It will be called if the user pressed the Back button
    // and the event was not handled in the currently displayed view.
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, example_view_dispatcher_app_navigation_callback);
    // The context will be passed to the callbacks as a parameter, so we have access to our application object.
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    return app;
}

// Application destructor function.
static void example_view_dispatcher_app_free(ExampleViewDispatcherApp* app) {
    // All views must be un-registered (removed) from a ViewDispatcher instance
    // before deleting it. Failure to do so will result in a crash.
    view_dispatcher_remove_view(app->view_dispatcher, ViewIndexWidget);
    view_dispatcher_remove_view(app->view_dispatcher, ViewIndexSubmenu);
    // Now it is safe to delete the ViewDispatcher instance.
    view_dispatcher_free(app->view_dispatcher);
    // Delete the views
    widget_free(app->widget);
    submenu_free(app->submenu);
    // End access to hte the GUI API.
    furi_record_close(RECORD_GUI);
    // Free the remaining memory.
    free(app);
}

static void example_view_dispatcher_app_run(ExampleViewDispatcherApp* app) {
    // Display the Widget view on the screen.
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewIndexWidget);
    // This function will block until view_dispatcher_stop() is called.
    // Internally, it uses a FuriEventLoop (see FuriEventLoop examples for more info on this).
    view_dispatcher_run(app->view_dispatcher);
}

/*******************************************************************
 *                     vvv START HERE vvv
 *
 * The application's entry point - referenced in application.fam
 *******************************************************************/
int32_t example_view_dispatcher_app(void* arg) {
    UNUSED(arg);

    ExampleViewDispatcherApp* app = example_view_dispatcher_app_alloc();
    example_view_dispatcher_app_run(app);
    example_view_dispatcher_app_free(app);

    return 0;
}

=== END: applications/examples/example_view_dispatcher/example_view_dispatcher.c ===

=== BEGIN: applications/examples/example_view_holder/application.fam ===
App(
    appid="example_view_holder",
    name="Example: ViewHolder",
    apptype=FlipperAppType.EXTERNAL,
    entry_point="example_view_holder_app",
    requires=["gui"],
    fap_category="Examples",
)

=== END: applications/examples/example_view_holder/application.fam ===

=== BEGIN: applications/examples/example_view_holder/example_view_holder.c ===
/**
 * @file example_view_holder.c
 * @brief Example application demonstrating the usage of the ViewHolder library.
 *
 * This application will display a text box with some scrollable text in it.
 * Press the Back key to exit the application.
 */

#include <gui/gui.h>
#include <gui/view_holder.h>
#include <gui/modules/text_box.h>

#include <api_lock.h>

// This function will be called when the user presses the Back button.
static void example_view_holder_back_callback(void* context) {
    FuriApiLock exit_lock = context;
    // Unlock the exit lock, thus enabling the app to exit.
    api_lock_unlock(exit_lock);
}

int32_t example_view_holder_app(void* arg) {
    UNUSED(arg);

    // Access the GUI API instance.
    Gui* gui = furi_record_open(RECORD_GUI);
    // Create a TextBox view. The Gui object only accepts
    // ViewPort instances, so we will need to address that later.
    TextBox* text_box = text_box_alloc();
    // Set some text so that the text box is not empty.
    text_box_set_text(
        text_box,
        "ViewHolder is being used\n"
        "to show this TextBox view.\n\n"
        "Scroll down to see more.\n\n\n"
        "Press \"Back\" to exit.");

    // Create a ViewHolder instance. It will serve as an adapter to convert
    // between the View type provided by the TextBox view and the ViewPort type
    // that the GUI can actually display.
    ViewHolder* view_holder = view_holder_alloc();
    // Let the GUI know about this ViewHolder instance.
    view_holder_attach_to_gui(view_holder, gui);
    // Set the view that we want to display.
    view_holder_set_view(view_holder, text_box_get_view(text_box));

    // The part below is not really related to this example, but is necessary for it to function.
    // We need to somehow stall the application thread so that the view stays on the screen (otherwise
    // the app will just exit and won't display anything) and at the same time we need a way to quit out
    // of the application.

    // In this example, a simple FuriApiLock instance is used. A real-world application is likely to have some
    // kind of event handling loop here instead. (see the ViewDispatcher example or one of FuriEventLoop
    // examples for that).

    // Create a pre-locked FuriApiLock instance.
    FuriApiLock exit_lock = api_lock_alloc_locked();
    // Set a Back event callback for the ViewHolder instance. It will be called when the user
    // presses the Back button. We pass the exit lock instance as the context to be able to access
    // it inside the callback function.
    view_holder_set_back_callback(view_holder, example_view_holder_back_callback, exit_lock);

    // This call will block the application thread from running until the exit lock gets unlocked somehow
    // (the only way it can happen in this example is via the back callback).
    api_lock_wait_unlock_and_free(exit_lock);

    // The back key has been pressed, which unlocked the exit lock. The application is about to exit.

    // The view must be removed from a ViewHolder instance before deleting it.
    view_holder_set_view(view_holder, NULL);
    // Delete everything to prevent memory leaks.
    view_holder_free(view_holder);
    text_box_free(text_box);
    // End access to the GUI API.
    furi_record_close(RECORD_GUI);

    return 0;
}

=== END: applications/examples/example_view_holder/example_view_holder.c ===


