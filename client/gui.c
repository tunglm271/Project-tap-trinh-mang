#include <gtk/gtk.h>

void on_button_clicked(GtkButton *button, gpointer user_data) {
    g_print("Button clicked!\n");
}

int main(int argc, char *argv[]) {
    GtkBuilder *builder;
    GtkWidget *window;
    GtkWidget *image;
    GdkPixbuf *pixbuf;

    gtk_init(&argc, &argv);

    // Load the Glade file
    builder = gtk_builder_new_from_file("client/homepage.glade");

    // Get the window and image widgets
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    image = GTK_WIDGET(gtk_builder_get_object(builder, "background_image"));

    // Load and scale the image to fit the window size
    pixbuf = gdk_pixbuf_new_from_file_at_scale("client/background-img.png", 400, 300, TRUE, NULL);
    gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);

    gtk_builder_connect_signals(builder, NULL);

    // Show the window
    gtk_widget_show_all(window);

    // Run the main GTK loop
    gtk_main();

    // Free resources
    g_object_unref(pixbuf);
    g_object_unref(builder);

    return 0;
}
