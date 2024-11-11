#include <gtk/gtk.h>

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *image;
    GdkPixbuf *pixbuf, *scaled_pixbuf;

    gtk_init(&argc, &argv);

    // Load the Glade file
    GtkBuilder *builder = gtk_builder_new_from_file("client/homepage.glade");

    // Create window from Glade
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));

    // Load and scale the image
    image = GTK_WIDGET(gtk_builder_get_object(builder, "image"));
    pixbuf = gdk_pixbuf_new_from_file("client/background-img.png", NULL);
    
    // Scale the image to fit the window size (e.g., 800x500)
    scaled_pixbuf = gdk_pixbuf_scale_simple(pixbuf, 800, 500, GDK_INTERP_BILINEAR);
    
    // Set the scaled image to the GtkImage widget
    gtk_image_set_from_pixbuf(GTK_IMAGE(image), scaled_pixbuf);

    // Show the window
    gtk_widget_show_all(window);

    // Main GTK loop
    gtk_main();

    // Cleanup
    g_object_unref(pixbuf);
    g_object_unref(scaled_pixbuf);

    return 0;
}
