#include<gtk/gtk.h>

// Activate function called when the app is launched
static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *label;

    // Create a new window with the application
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Hello GTK3");
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 100);

    // Create a new label with "Hello, World!" text
    label = gtk_label_new("Hello, World!");

    // Add the label to the window
    gtk_container_add(GTK_CONTAINER(window), label);

    // Show all widgets in the window
    gtk_widget_show_all(window);
}

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
