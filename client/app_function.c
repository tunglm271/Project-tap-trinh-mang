#include "app_function.h"
#include <gst/gst.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

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

void add_css_class_to_widget(GtkWidget *widget, const gchar *css_class) {
    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    gtk_style_context_add_class(context, css_class);
}

void play_sound_effect(const char *file_path) {
    GError *error = NULL;
    GstElement *pipeline;

    // Initialize GStreamer
    gst_init(NULL, NULL);

    // Create a pipeline to play the sound
    char pipeline_description[256];
    snprintf(pipeline_description, sizeof(pipeline_description), "playbin uri=file://%s", file_path);

    pipeline = gst_parse_launch(pipeline_description, &error);
    if (!pipeline) {
        g_printerr("Failed to create pipeline: %s\n", error->message);
        g_error_free(error);
        return;
    }

    // Start playing the audio
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // Wait until playback finishes
    GstBus *bus = gst_element_get_bus(pipeline);
    gst_bus_poll(bus, GST_MESSAGE_EOS, -1);
    gst_object_unref(bus);

    // Clean up the pipeline
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
}

gboolean update_countdown(gpointer user_data) {
    CountdownData *data = (CountdownData *)user_data;
    time_t now = time(NULL);

    int time_left = (int)difftime(data->end_time, now);
    if (time_left >= 0) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.2d", time_left);
        gtk_label_set_text(data->label, buffer);
        return TRUE; // Continue the timeout
    } else {
        gtk_label_set_text(data->label, "00");
        return FALSE; // Stop the timeout
    }
}

void render_question(GtkButton *button, gpointer GameData) {
    gpointer *data = (gpointer *)GameData;
    GtkWidget *main_box = (GtkWidget *)data[0];

    remove_all_children(GTK_CONTAINER(main_box));

    // Change direction of the main box to horizontal
    gtk_orientable_set_orientation(GTK_ORIENTABLE(main_box), GTK_ORIENTATION_HORIZONTAL);

    // Create two sub-boxes
    GtkWidget *question_section = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(question_section, "question_section");
    GtkWidget *money_section = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(money_section, 200, -1);
    gtk_widget_set_name(money_section, "money_section");

    // =============================
    // Add content to the question box
    // =============================
    // Create a grid to wrap the label
    GtkWidget *wrapper_grid = gtk_grid_new();
    gtk_widget_set_size_request(wrapper_grid, 100, -1); // Set the width of the wrapper

    // Create the countdown label
    GtkWidget *countdown_label = gtk_label_new("30");
    gtk_widget_set_name(countdown_label, "countdown-label");

    // Add the label to the grid
    gtk_grid_attach(GTK_GRID(wrapper_grid), countdown_label, 0, 0, 1, 1);

    // Add the wrapper grid to the question section
    gtk_box_pack_start(GTK_BOX(question_section), wrapper_grid, FALSE, FALSE, 10);

    // Initialize countdown data
    CountdownData *countdown_data = g_malloc(sizeof(CountdownData));
    countdown_data->label = GTK_LABEL(countdown_label);
    countdown_data->end_time = time(NULL) + 30;

    // Start the countdown
    g_timeout_add(1000, (GSourceFunc)update_countdown, countdown_data);


    // Create a new label with a question
    GtkWidget *question_label = gtk_label_new("What is the capital of France?");
    gtk_widget_set_name(question_label, "question-label");

    // Create a grid for the answer buttons
    GtkWidget *grid = gtk_grid_new();
    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(grid, FALSE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 30);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 70);

    // Create answer buttons
    GtkWidget *btn1 = gtk_button_new_with_label("Berlin");
    GtkWidget *btn2 = gtk_button_new_with_label("Madrid");
    GtkWidget *btn3 = gtk_button_new_with_label("Paris");
    GtkWidget *btn4 = gtk_button_new_with_label("Rome");
    gtk_widget_set_size_request(btn1, 150, -1);
    gtk_widget_set_size_request(btn2, 150, -1);
    // Add buttons to the grid
    gtk_grid_attach(GTK_GRID(grid), btn1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn2, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn3, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn4, 1, 1, 1, 1);

    // Pack the question label and grid into the question box
    gtk_box_pack_start(GTK_BOX(question_section), question_label, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(question_section), grid, FALSE, FALSE, 10);

    // =============================
    // Add content to the money list box
    // =============================

    GtkWidget* helpButton = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(helpButton), TRUE);
    gtk_grid_set_column_spacing(GTK_GRID(helpButton), 10);
    gtk_widget_set_margin_start(helpButton, 15);  
    gtk_widget_set_margin_end(helpButton, 15);   
    gtk_widget_set_margin_top(helpButton, 10);    
    gtk_widget_set_margin_bottom(helpButton, 10);
    
    GtkWidget* callFriend =  gtk_button_new_with_label("");
    GtkWidget* help_50 =  gtk_button_new_with_label("50:50");
    GtkWidget* askPeople =  gtk_button_new_with_label("");

    gtk_widget_set_name(callFriend,"callFriend");
    gtk_widget_set_name(askPeople,"askPeople");
    add_css_class_to_widget(help_50,"helpOption");

    gtk_grid_attach(GTK_GRID(helpButton), help_50, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(helpButton), callFriend, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(helpButton), askPeople, 2, 0, 1, 1);
    gtk_box_pack_start(GTK_BOX(money_section), helpButton, FALSE, FALSE, 5);


    const char *money_labels[] = {
        "15. 150.000.000", "14. 85.000.000", "13. 60.000.000", 
        "12. 40.000.000", "11. 30.000.000", "10. 22.000.000", 
        "9. 14.000.000", "8. 10.000.000", "7. 6.000.000", 
        "6. 3.000.000", "5. 2.000.000", "4. 1.000.000", 
        "3. 600.000", "2. 400.000", "1. 200.000"
    };

    // Create and pack the labels into the box
    for (int i = 0; i < 15; i++) {
        GtkWidget *money_label = gtk_label_new(money_labels[i]);
        
        // Add CSS class to specific labels
        if (i == 0 || i == 5 || i == 10) {
            add_css_class_to_widget(money_label, "milestone");
        }

        // Pack the label into the money section
        gtk_box_pack_start(GTK_BOX(money_section), money_label, FALSE, FALSE, 5);
    }

    // =============================
    // Add both sub-boxes to the main box
    // =============================

    gtk_box_pack_start(GTK_BOX(main_box), question_section, TRUE, TRUE, 10);
    gtk_box_pack_start(GTK_BOX(main_box), money_section, FALSE, FALSE, 0);
    gtk_widget_show_all(main_box);

    g_free(data);
}

void submit_name(GtkButton *button, gpointer user_data) {
    // Extract box and entry from user_data (which is a pointer to a 2-element array)
    gpointer *data = (gpointer *)user_data;
    GtkWidget *box = (GtkWidget *)data[0];
    GtkEntry *name_input = (GtkEntry *)data[1];
    GtkEntry *password_input = (GtkEntry *)data[2];
    const gchar *text = gtk_entry_get_text(name_input);  // Get the text from the entry
    GtkWidget *welcome_text = gtk_label_new(NULL);  // Create a new label
    GtkWidget *start_btn = gtk_button_new_with_label("Start game");
    // Set the label text to "Welcome [text from entry]"
    gtk_label_set_text(GTK_LABEL(welcome_text), g_strdup_printf("Welcome %s", text));
    gtk_widget_set_name(welcome_text, "welcome-text");
    remove_all_children(GTK_CONTAINER(box));

    gpointer *gameData = g_new(gpointer, 1);
    gameData[0] = box;
    // Add the new label to the box
    gtk_box_pack_start(GTK_BOX(box), welcome_text, TRUE, TRUE, 10); // Add the label with expanding
    gtk_box_pack_start(GTK_BOX(box), start_btn, TRUE, FALSE, 0);
    g_signal_connect(start_btn, "clicked", G_CALLBACK(render_question), gameData);
    // Show the label
    gtk_widget_show_all(box);  // Make sure the label is shown

    // Free the allocated data array
    g_free(data);
}


void render_login(GtkButton *button, gpointer input_data) {
    gpointer *data = (gpointer *)input_data;
    GtkWidget *box = (GtkWidget *)data[0];
    remove_all_children(GTK_CONTAINER(box));

    GtkWidget *labelName = gtk_label_new("Enter username:");
    GtkWidget *labelPassword = gtk_label_new("Enter password");
    GtkWidget *NameInput = gtk_entry_new();
    GtkWidget *PasswordInput = gtk_entry_new();
    GtkWidget *SubmitBtn = gtk_button_new_with_label("Login");

    gpointer *user_data = g_new(gpointer, 3);
    user_data[0] = box;
    user_data[1] = NameInput;
    user_data[2] = PasswordInput;

    // Connect the submit button click event to the on_submit_clicked callback
    g_signal_connect(SubmitBtn, "clicked", G_CALLBACK(submit_name), user_data);
    g_signal_connect(PasswordInput, "activate", G_CALLBACK(submit_name), user_data);


    gtk_box_pack_start(GTK_BOX(box), labelName, FALSE, FALSE, 0);  
    gtk_box_pack_start(GTK_BOX(box), NameInput, FALSE, FALSE, 0);  
    gtk_box_pack_start(GTK_BOX(box), labelPassword, FALSE, FALSE, 0);  
    gtk_box_pack_start(GTK_BOX(box), PasswordInput, FALSE, FALSE, 0);  
    gtk_box_pack_start(GTK_BOX(box), SubmitBtn, FALSE, FALSE, 0); 

    gtk_widget_show_all(box);  
    g_free(data);
}

void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *header_bar;
    GtkWidget *title_label;

    GtkWidget *loginButton;
    GtkWidget *signUpButton;

    // Create a new window with the application
    window = gtk_application_window_new(app);
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 500);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_window_close), NULL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    header_bar = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "Custom Window Title");
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);
    title_label = gtk_label_new("Ai Là Triệu Phú");
    gtk_widget_set_name(title_label, "title");
    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), NULL);
    gtk_container_add(GTK_CONTAINER(header_bar), title_label);
    gtk_window_set_titlebar(GTK_WINDOW(window), header_bar);
    load_css(window);


    // Create a box container (vertical or horizontal)
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_name(box, "content");

    loginButton = gtk_button_new_with_label("Login");
    signUpButton = gtk_button_new_with_label("Create new account");

    // Create a tuple (array) to store box and entry
    gpointer *data = g_new(gpointer, 1);
    data[0] = box;    
    // Connect the submit button click event to the on_submit_clicked callback
    g_signal_connect(loginButton, "clicked", G_CALLBACK(render_login), data);
    gtk_box_pack_start(GTK_BOX(box), loginButton, FALSE, FALSE, 0);  // Add the "Enter your name" label
    gtk_box_pack_start(GTK_BOX(box), signUpButton, FALSE, FALSE, 0);  // Add the text entry field

    // Add the box container to the window
    gtk_container_add(GTK_CONTAINER(window), box);
    // Show all widgets in the window
    gtk_widget_show_all(window);

    char cwd[PATH_MAX];  // Store current working directory
    char sound_path[PATH_MAX * 2];  // To store the full path of the sound file

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working directory: %s\n", cwd);
        snprintf(sound_path, sizeof(sound_path), "%s/%s", cwd, "client/assets/intro.ogg");
        play_sound_effect(sound_path);
    } else {
        perror("getcwd() error");
    }
}
