#ifndef ANIMATION_H
#define ANIMATION_H

#include <gtk/gtk.h>

// Struct to store animation data
typedef struct {
    GtkWidget *label;
    GtkFixed *container;
    int start_x;
    int final_x;
    int y;
    int step;
    int delay;
} LabelAnimationData;

// Function declarations
void animate_money_labels(GtkFixed *container, GtkWidget *label, int start_x, int final_x, int y, int duration_ms, int delay_ms);

#endif // ANIMATION_H
