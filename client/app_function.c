#include "app_function.h"

void load_css(GtkWidget *widget) {
    // Create a new CSS provider
    GtkCssProvider *provider = gtk_css_provider_new();

    // Load the CSS from the file
    GFile *css_file = g_file_new_for_path("client/style.css");
    gtk_css_provider_load_from_file(provider, css_file, NULL);
    g_object_unref(css_file);

    // Get the default display and screen
    GdkDisplay *display = gdk_display_get_default();
    if (display) {
        GdkScreen *screen = gdk_display_get_default_screen(display);
        if (screen) {
            // Apply the CSS provider to the screen's style context
            gtk_style_context_add_provider_for_screen(
                screen,
                GTK_STYLE_PROVIDER(provider),
                GTK_STYLE_PROVIDER_PRIORITY_USER
            );
        }
    }

    // Clean up the CSS provider
    g_object_unref(provider);
}


void remove_all_children(GtkContainer *container) {
    GList *children = gtk_container_get_children(container);
    for (GList *iter = children; iter != NULL; iter = iter->next) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
}

void on_start_game_clicked(GtkButton *button, gpointer GameData) {
    // Extract box from user_data (user_data is the same as before)
    gpointer *data = (gpointer *)GameData;
    GtkWidget *box = (GtkWidget *)data[0];

    remove_all_children(GTK_CONTAINER(box));


    // Create a new label with a question
    GtkWidget *question_label = gtk_label_new("What is the capital of France?");

    // Create a grid for the answer buttons
    GtkWidget *grid = gtk_grid_new();
     gtk_widget_set_halign(grid, GTK_ALIGN_CENTER); 
    gtk_widget_set_hexpand(grid, FALSE); 
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10); 
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);


    // Create answer buttons
    GtkWidget *btn1 = gtk_button_new_with_label("Berlin");
    GtkWidget *btn2 = gtk_button_new_with_label("Madrid");
    GtkWidget *btn3 = gtk_button_new_with_label("Paris");
    GtkWidget *btn4 = gtk_button_new_with_label("Rome");

    // Add buttons to the grid
    gtk_grid_attach(GTK_GRID(grid), btn1, 0, 0, 1, 1); // Attach button 1 to (0, 0)
    gtk_grid_attach(GTK_GRID(grid), btn2, 1, 0, 1, 1); // Attach button 2 to (1, 0)
    gtk_grid_attach(GTK_GRID(grid), btn3, 0, 1, 1, 1); // Attach button 3 to (0, 1)
    gtk_grid_attach(GTK_GRID(grid), btn4, 1, 1, 1, 1); // Attach button 4 to (1, 1)

    // Remove all existing widgets from the box (clear screen)

    // Add the question label and grid to the box
    gtk_box_pack_start(GTK_BOX(box), question_label, TRUE, TRUE, 10);  // Add question label
    gtk_box_pack_start(GTK_BOX(box), grid, TRUE, TRUE, 10);  // Add the grid containing answer buttons

    // Show all widgets in the box
    gtk_widget_show_all(box);
    
    // Free the allocated data array (optional)
    g_free(data);
}

void on_submit_clicked(GtkButton *button, gpointer user_data) {
    // Extract box and entry from user_data (which is a pointer to a 2-element array)
    gpointer *data = (gpointer *)user_data;
    GtkWidget *box = (GtkWidget *)data[0];
    GtkEntry *entry = (GtkEntry *)data[1];

    const gchar *text = gtk_entry_get_text(entry);  // Get the text from the entry
    GtkWidget *welcome_text = gtk_label_new(NULL);  // Create a new label
    GtkWidget *start_btn = gtk_button_new_with_label("Start game");
    // Set the label text to "Welcome [text from entry]"
    gtk_label_set_text(GTK_LABEL(welcome_text), g_strdup_printf("Welcome %s", text));
    gtk_widget_set_name(welcome_text, "welcome-text");
    GList *children = gtk_container_get_children(GTK_CONTAINER(box)); 
    for (GList *iter = children; iter != NULL; iter = iter->next) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));  // Destroy each child widget
    }
    g_list_free(children);  // Free the GList after use

    gpointer *gameData = g_new(gpointer, 1);
    gameData[0] = box;
    // Add the new label to the box
    gtk_box_pack_start(GTK_BOX(box), welcome_text, TRUE, TRUE, 10); // Add the label with expanding
    gtk_box_pack_start(GTK_BOX(box), start_btn, TRUE, FALSE, 0);
    g_signal_connect(start_btn, "clicked", G_CALLBACK(on_start_game_clicked), gameData);
    // Show the label
    gtk_widget_show_all(box);  // Make sure the label is shown

    // Free the allocated data array
    g_free(data);
}

void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *entry;     // Text input field
    GtkWidget *submit_btn; // Submit button
    GtkWidget *label;     // Label for "Enter your name"
    GtkWidget *header_bar;
    GtkWidget *title_label;

    // Create a new window with the application
    window = gtk_application_window_new(app);
    gtk_window_set_default_size(GTK_WINDOW(window), 700, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_window_close), NULL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    header_bar = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "Custom Window Title");
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);

    // Create a label for the title and add custom CSS class
    title_label = gtk_label_new("Ai Là Triệu Phú");
    gtk_widget_set_name(title_label, "title");

    // Remove the default title and add our custom label to the header bar
    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), NULL);
    gtk_container_add(GTK_CONTAINER(header_bar), title_label);

    // Set the header bar as the window's title bar
    gtk_window_set_titlebar(GTK_WINDOW(window), header_bar);
    load_css(window);

    // Create a box container (vertical or horizontal)
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10); // 10 is the spacing between elements
    gtk_widget_set_name(box, "content");
    // Create a label for "Enter your name"
    label = gtk_label_new("Enter your name");

    // Create a text entry (input field)
    entry = gtk_entry_new();

    // Create a submit button
    submit_btn = gtk_button_new_with_label("Submit");

    // Set the size of the submit button (optional)
    gtk_widget_set_size_request(submit_btn, 100, 50);  // Width: 100px, Height: 50px

    // Create a tuple (array) to store box and entry
    gpointer *data = g_new(gpointer, 2);
    data[0] = box;    // Store box pointer
    data[1] = entry;  // Store entry pointer

    // Connect the submit button click event to the on_submit_clicked callback
    g_signal_connect(submit_btn, "clicked", G_CALLBACK(on_submit_clicked), data);

    // Add the label, entry, and button to the box container
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);  // Add the "Enter your name" label
    gtk_box_pack_start(GTK_BOX(box), entry, FALSE, FALSE, 0);  // Add the text entry field
    gtk_box_pack_start(GTK_BOX(box), submit_btn, FALSE, FALSE, 0); // Add the submit button

    // Add the box container to the window
    gtk_container_add(GTK_CONTAINER(window), box);

    // Show all widgets in the window
    gtk_widget_show_all(window);
}