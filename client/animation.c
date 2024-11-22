#include "animation.h"
#include <glib.h>

gboolean move_money_label_step(gpointer user_data) {
    LabelAnimationData *data = (LabelAnimationData *)user_data;

    // Update label's x position
    data->start_x += data->step;
    gtk_fixed_move(GTK_FIXED(data->container), data->label, data->start_x, data->y);

    // Check if animation is complete
    if (data->start_x >= data->final_x) {
        g_free(data);
        return FALSE; // Stop the timeout
    }

    return TRUE; // Continue the timeout
}

gboolean start_label_animation(gpointer user_data) {
    LabelAnimationData *data = (LabelAnimationData *)user_data;
    g_timeout_add(16, move_money_label_step, data); // Run the animation at ~60 FPS
    return FALSE; // Only start the animation once
}

void animate_money_labels(GtkFixed *container, GtkWidget *label, int start_x, int final_x, int y, int duration_ms, int delay_ms) {
    int frames = duration_ms / 16; // ~60 FPS
    int step = (final_x - start_x) / frames;

    LabelAnimationData *data = g_malloc(sizeof(LabelAnimationData));
    data->label = label;
    data->container = container;
    data->start_x = start_x;
    data->final_x = final_x;
    data->y = y;
    data->step = step;
    data->delay = delay_ms;

    // Delay the start of the animation
    g_timeout_add(data->delay, start_label_animation, data);
}
