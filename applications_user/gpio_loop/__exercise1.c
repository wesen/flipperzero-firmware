#include <inttypes.h>
#include <stdio.h>

static const int FOO[] = {1, 2, 3, 4};

typedef struct {
    const char* name;
} Owner;

typedef struct {
    const char* name;
    const Owner* owner;
} Animal;

static const Owner owner = {"John"};
static const Animal dog = {"dog", &owner};
static const Animal cat = {"cat", &owner};

void print_memory_layout() {
    printf("dog: %p\n", &dog);
    printf("cat: %p\n", &cat);
    printf("owner: %p\n", &owner);

    printf("FOO: %p\n", FOO);
    printf("FOO[0]: %p\n", &FOO[0]);
    printf("FOO[1]: %p\n", &FOO[1]);
    printf("FOO[2]: %p\n", &FOO[2]);
    printf("FOO[3]: %p\n", &FOO[3]);
}

// Enum for device modes
typedef enum {
    MODE_STANDBY,
    MODE_ACTIVE,
    MODE_DIAGNOSTIC
} DeviceMode;

// Configuration for a sensor
typedef struct {
    const char* sensor_name;
    float calibration_offset;
    float min_range;
    float max_range;
} SensorConfig;

// Device configuration
typedef struct {
    const char* device_name;
    uint8_t device_id;
    DeviceMode default_mode;
    const SensorConfig* sensors;
    size_t sensor_count;
} DeviceConfiguration;

// Static sensor configurations
static const SensorConfig TEMPERATURE_SENSORS[] = {
    {.sensor_name = "External Ambient",
     .calibration_offset = -0.5f,
     .min_range = -40.0f,
     .max_range = 85.0f},
    {.sensor_name = "Internal Core",
     .calibration_offset = 0.2f,
     .min_range = 0.0f,
     .max_range = 120.0f}};

// Complete device configuration
static const DeviceConfiguration MAIN_DEVICE_CONFIG = {
    .device_name = "Environmental Monitor",
    .device_id = 0x42,
    .default_mode = MODE_STANDBY,
    .sensors = TEMPERATURE_SENSORS,
    .sensor_count = 2};

void print_memory_layout2() {
    printf("MAIN_DEVICE_CONFIG: %p\n", &MAIN_DEVICE_CONFIG);
    printf("TEMPERATURE_SENSORS: %p\n", TEMPERATURE_SENSORS);
    printf("TEMPERATURE_SENSORS[0]: %p\n", &TEMPERATURE_SENSORS[0]);
    printf("TEMPERATURE_SENSORS[1]: %p\n", &TEMPERATURE_SENSORS[1]);
    printf("MAIN_DEVICE_CONFIG.sensors: %p\n", MAIN_DEVICE_CONFIG.sensors);
    printf("MAIN_DEVICE_CONFIG.sensors[0]: %p\n", &MAIN_DEVICE_CONFIG.sensors[0]);
    printf("MAIN_DEVICE_CONFIG.sensors[1]: %p\n", &MAIN_DEVICE_CONFIG.sensors[1]);
}

int main() {
    print_memory_layout();
    print_memory_layout2();
    return 0;
}
