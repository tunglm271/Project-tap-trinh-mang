#include <gtk/gtk.h>
#include "app_function.h"

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    // Create a new GtkApplication
    app = gtk_application_new("com.example.GtkApp", G_APPLICATION_FLAGS_NONE);

    // Connect the "activate" signal to the activate function
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    // Run the application
    status = g_application_run(G_APPLICATION(app), argc, argv);

    // Free the memory allocated for the GtkApplication
    g_object_unref(app);

    return status;
}
