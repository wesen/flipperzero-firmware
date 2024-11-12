/*********************************************
 * Flip.x0 Tutorial
 *                                by M1ch3al
 * 0x01 - my first app
 ********************************************
 */

#include <stdio.h>
#include <furi.h>
#include <gui/gui.h>

// This function is for drawing the screen GUI, everytime
// the Flip.x0 refresh the display
static void draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    // This istruction no need details :)
    canvas_clear(canvas);

    // Set the font, the position and the content of the label
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 32, 13, "My First App");

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 15, 40, " press back to exit FAP");

    canvas_draw_line(canvas, 2, 23, 126, 23);
}

// This function is an handler for the user input (the buttons on the right
// of the Flip.x0 used for navigate, confirm and back)
static void input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

// Main entry of the application as defined inside the application.fam
int32_t my_first_app_main(void* p) {
    UNUSED(p);

    // Current event of type InputEvent
    InputEvent event;

    // Event queue for 8 elements of size InputEvent
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    // ViewPort is need to draw the GUI
    ViewPort* view_port = view_port_alloc();

    // we give to this [view_port_draw_callback_set] the function defined
    // before (draw_callback) for drawing the GUI on the Flip.x0 display
    view_port_draw_callback_set(view_port, draw_callback, NULL);

    // The same concept is with [view_port_input_callback_set] associated to the (input_callback)
    // function defined before.
    view_port_input_callback_set(view_port, input_callback, event_queue);

    // You need to create a GUI structure and associate it to the viewport previously defined
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Infinite loop...(like arduino or similar)
    while(true) {
        // We continue (indefinitely) to get out of the queue all the events stacked inside
        furi_check(furi_message_queue_get(event_queue, &event, FuriWaitForever) == FuriStatusOk);

        // If the event from the queue is the press of the back button, we exit from the loop
        if(event.key == InputKeyBack) {
            break;
        }
    }

    // once exit from the loop, we need to free resources:
    // clear all the element inside the queue
    furi_message_queue_free(event_queue);

    // We remove the gui from the associated view port
    gui_remove_view_port(gui, view_port);

    // Freeing up memory removing the view_port and close
    // the GUI record
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);
    return 0;
}
