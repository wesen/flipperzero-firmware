# Creating Multi-Scene Applications with Scene Manager

This tutorial explains how to create Flipper Zero applications with multiple scenes using the Scene Manager. We'll use the Timer app as an example.

## Core Concepts

### Scene Manager Overview
The Scene Manager handles navigation between different screens (scenes) in your application. It maintains a scene stack and manages transitions between scenes.

Each scene has three key handlers:
- on_enter: Called when entering the scene
- on_event: Called to handle events while in the scene  
- on_exit: Called when leaving the scene

### Basic Scene Setup

1. First define your scenes in an enum:

|||c
typedef enum {
    MySceneMain,  // Main screen
    MySceneMenu,  // Menu screen
    MySceneSettings, // Settings screen
    MySceneNum,   // Must be last - total number of scenes
} MyScene;
|||

2. Create a scene configuration file (my_scene_config.h):

|||c
ADD_SCENE(my_app, main, Main)
ADD_SCENE(my_app, menu, Menu) 
ADD_SCENE(my_app, settings, Settings)
|||

3. Create scene handlers for each scene:

|||c
void my_scene_main_on_enter(void* context) {
    MyApp* app = context;
    // Setup scene
}

bool my_scene_main_on_event(void* context, SceneManagerEvent event) {
    MyApp* app = context;
    bool consumed = false;
    // Handle events
    return consumed;
}

void my_scene_main_on_exit(void* context) {
    MyApp* app = context;
    // Cleanup
}
|||

### Scene Manager Integration

1. Initialize Scene Manager in your app allocation:

|||c
typedef struct {
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    // Other app data...
} MyApp;

MyApp* my_app_alloc() {
    MyApp* app = malloc(sizeof(MyApp));
    app->scene_manager = scene_manager_alloc(&my_scene_handlers, app);
    // Setup other components...
    return app;
}
|||

2. Connect Scene Manager to View Dispatcher:

|||c
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(app->view_dispatcher, app_back_event_callback);
|||

### Scene Navigation

To move between scenes:

|||c
// Go to next scene
scene_manager_next_scene(app->scene_manager, MySceneSettings);

// Handle back navigation
bool app_back_event_callback(void* context) {
    MyApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

// Handle custom events
bool app_custom_event_callback(void* context, uint32_t event) {
    MyApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}
|||

### Views and Scenes

Each scene typically manages one or more views. Common views include:
- Submenu (menu lists)
- Variable Item List (settings)
- Custom views (specialized UI)

Example of setting up a menu scene:

|||c
void my_scene_menu_on_enter(void* context) {
    MyApp* app = context;
    Submenu* submenu = app->submenu;
    
    submenu_add_item(submenu, "Start", MyMenuIndexStart, menu_callback, app);
    submenu_add_item(submenu, "Settings", MyMenuIndexSettings, menu_callback, app);
    
    view_dispatcher_switch_to_view(app->view_dispatcher, MyViewSubmenu);
}
|||

### Best Practices

1. Scene State Management
- Use scene_manager_get_scene_state() and scene_manager_set_scene_state() to preserve scene state
- Clean up resources in on_exit handlers
- Initialize/reset views in on_enter handlers

2. Event Handling
- Return true from on_event when event is consumed
- Handle both custom events and back navigation
- Use enum for custom event types

3. Memory Management
- Free all resources in app_free()
- Reset views when exiting scenes
- Check for NULL pointers

### Common Patterns

1. Menu-based Navigation:
|||c
bool menu_scene_on_event(void* context, SceneManagerEvent event) {
    MyApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case MyMenuIndexStart:
            scene_manager_next_scene(app->scene_manager, MySceneStart);
            consumed = true;
            break;
        }
    }
    return consumed;
}
|||

2. Settings Scene:
|||c
void settings_scene_on_enter(void* context) {
    MyApp* app = context;
    VariableItemList* list = app->variable_item_list;
    
    variable_item_list_add(list, "Setting1", 3, setting_changed, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, MyViewSettings);
}
|||

## Conclusion

The Scene Manager provides a robust way to handle multiple screens in your application. Key points to remember:
- Define clear scene transitions
- Handle all events appropriately
- Clean up resources properly
- Use appropriate view types for each scene

For a complete example, study the Timer app source code which demonstrates these concepts in practice. 