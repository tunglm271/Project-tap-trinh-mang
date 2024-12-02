#include "app_function.h"
#include "utils.h"
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int sock;
struct sockaddr_in serv_addr;
char buffer[BUFFER_SIZE] = {0};
int current_point = 14;
GtkWidget *window;
GtkWidget *main_box;

void render_welcome_page(const gchar *username);

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


void on_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data) {
    g_print("OK button clicked\n");
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

void handle_answer(GtkButton *button, gpointer answerData) {
    int answer = GPOINTER_TO_INT(answerData);
    printf("Answer chosen: %d\n", answer);
    memset(buffer, 0, BUFFER_SIZE);
    buffer[0] = 0x08;
    sprintf(buffer + 1, "%d", answer);
    send(sock, buffer, BUFFER_SIZE, 0);

    memset(buffer, 0, BUFFER_SIZE);
    recv(sock, buffer, BUFFER_SIZE, 0);

    if (buffer[0] == 0x09) {
       printf("Correct answer!\n");
       render_question(NULL);
       current_point++;
    } else {
        GtkWidget *dialog;
        dialog = gtk_message_dialog_new(GTK_WINDOW(window), 
                        GTK_DIALOG_DESTROY_WITH_PARENT, 
                        GTK_MESSAGE_ERROR, 
                        GTK_BUTTONS_OK, 
                        NULL);
        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        GtkWidget *dialog_label = gtk_label_new("Incorrect answer!\nYou won 1000$");
        gtk_widget_set_name(dialog_label, "dialog-text");
        gtk_container_add(GTK_CONTAINER(content_area), dialog_label);
        gtk_widget_set_halign(dialog_label, GTK_ALIGN_CENTER);
        gtk_widget_show(dialog_label);      
        g_signal_connect(dialog, "response", G_CALLBACK(on_dialog_response), NULL);
        gtk_dialog_run(GTK_DIALOG(dialog));
    }
}

void render_question(GtkButton *button) {
    
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
    
    memset(buffer, 0, BUFFER_SIZE);
    buffer[0] = 0x07;
    send(sock, buffer, BUFFER_SIZE, 0);
    
    memset(buffer, 0, BUFFER_SIZE);
    recv(sock, buffer, BUFFER_SIZE, 0);
    
    char *question = strtok(buffer, "\n"); 
    char *options[4];
    for (int i = 0; i < 4; i++) {
        options[i] = strtok(NULL, "\n");
    }

    // Create a new label with a question
    GtkWidget *question_label = gtk_label_new(question ? question : "No question received");
    gtk_widget_set_name(question_label, "question-label");

    // Create a grid for the answer buttons
    GtkWidget *grid = gtk_grid_new();
    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(grid, FALSE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 30);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 70);

    GtkWidget *buttons[4];
    for (int i = 0; i < 4; i++) {
        buttons[i] = gtk_button_new_with_label(options[i]);
        gtk_widget_set_size_request(buttons[i], 150, -1);
        g_signal_connect(buttons[i], "clicked", G_CALLBACK(handle_answer), GINT_TO_POINTER(i));
        gtk_grid_attach(GTK_GRID(grid), buttons[i], i % 2, i / 2, 1, 1);
    }

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

        if( i == current_point) {
            gtk_widget_set_name(money_label, "current-point");
        }

        // Pack the label into the money section
        gtk_box_pack_start(GTK_BOX(money_section), money_label, FALSE, FALSE, 5);
    }

    gtk_box_pack_start(GTK_BOX(main_box), question_section, TRUE, TRUE, 10);
    gtk_box_pack_start(GTK_BOX(main_box), money_section, FALSE, FALSE, 0);
    gtk_widget_show_all(main_box);

}

void render_loading(GtkButton *button, gpointer gameData) {
    gpointer *data = (gpointer *)gameData;
    GtkWidget *box = (GtkWidget *)data[0];
    GtkWidget *spinner;
    GtkWidget *label;

    remove_all_children(GTK_CONTAINER(box));
    label = gtk_label_new("Waiting for other players...");
    gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 0);
    
    // Create a spinner
    spinner = gtk_spinner_new();
    gtk_widget_set_name(spinner, "spinner");
    gtk_box_pack_start(GTK_BOX(box), spinner, TRUE, TRUE, 0);

    // Start the spinner
    gtk_spinner_start(GTK_SPINNER(spinner));

    // Show all widgets in the window
    gtk_widget_show_all(box);
    g_free(data);
}

void submit_name(GtkButton *button, gpointer user_data) {
    g_print("submit_name\n");
    gpointer *data = (gpointer *)user_data;
    GtkEntry *name_input = (GtkEntry *)data[0];
    GtkEntry *password_input = (GtkEntry *)data[1];
    const gchar *username = gtk_entry_get_text(name_input);
    const gchar *password = gtk_entry_get_text(password_input);
    memset(buffer, 0, BUFFER_SIZE);
    buffer[0] = 0x01;
    sprintf(buffer+1, "username:%s;password:%s", username, password);
    g_print("%s\n",buffer);
    send(sock, buffer, BUFFER_SIZE, 0);
    
    memset(buffer, 0, BUFFER_SIZE);
    recv(sock, buffer, BUFFER_SIZE, 0);
    if(buffer[0] == 0x02) {
        printf("dang nhap thanh cong\n");
        render_welcome_page(username);
        g_free(data);
    } else {
        remove_child_by_name(GTK_CONTAINER(main_box), "error-label");
        GtkWidget *error_label = gtk_label_new("username or password is incorrect!");
        gtk_widget_set_name(error_label, "error-label");
        gtk_box_pack_start(GTK_BOX(main_box), error_label, FALSE, FALSE, 0);
        gtk_widget_show_all(main_box);
    }
}

void render_welcome_page(const gchar *username) {
    GtkWidget *welcome_text = gtk_label_new(NULL); 
    GtkWidget *start_btn = gtk_button_new_with_label("Start game");
    gtk_label_set_text(GTK_LABEL(welcome_text), g_strdup_printf("Welcome %s", username));
    gtk_widget_set_name(welcome_text, "welcome-text");
    remove_all_children(GTK_CONTAINER(main_box));

    gtk_box_pack_start(GTK_BOX(main_box), welcome_text, TRUE, TRUE, 10);
    gtk_box_pack_start(GTK_BOX(main_box), start_btn, TRUE, FALSE, 0);
    g_signal_connect(start_btn, "clicked", G_CALLBACK(render_question), NULL);
    g_signal_connect(start_btn, "clicked", G_CALLBACK(render_question), NULL);
    gtk_widget_show_all(GTK_WIDGET(main_box));  
}


void submit_register(GtkButton *button, gpointer register_data) {
    gpointer *data = (gpointer *)register_data;
    GtkWidget *box = (GtkWidget *)data[0];
    GtkEntry *username_entry = (GtkEntry *)data[1];
    GtkEntry *password_entry = (GtkEntry *)data[2];
    GtkEntry *confirm_password_entry = (GtkEntry *)data[3];
    
    const gchar *username = gtk_entry_get_text(username_entry);
    const gchar *password = gtk_entry_get_text(password_entry);
    const gchar *confirm_password = gtk_entry_get_text(confirm_password_entry);
    
    if (strcmp(password, confirm_password) != 0) {
        GtkWidget *error_label = gtk_label_new("Passwords do not match!");
        gtk_box_pack_start(GTK_BOX(box), error_label, FALSE, FALSE, 0);
        gtk_widget_show_all(box);
    } else { 
        memset(buffer, 0, BUFFER_SIZE);
        buffer[0] = 0x04;
        sprintf(buffer+1, "username:%s;password:%s", username, password);
        send(sock, buffer, BUFFER_SIZE, 0);
        
        memset(buffer, 0, BUFFER_SIZE);
        recv(sock, buffer, BUFFER_SIZE, 0);
        if(buffer[0] == 0x05) {
        printf("dang nhap thanh cong\n");
        } else printf("dang nhap khong thanh cong\n");
    }

    g_free(data);
}


void render_login(GtkButton *button) {
    remove_all_children(GTK_CONTAINER(main_box));

    GtkWidget *labelName = gtk_label_new("Enter username:");
    GtkWidget *labelPassword = gtk_label_new("Enter password");
    GtkWidget *NameInput = gtk_entry_new();
    GtkWidget *PasswordInput = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(PasswordInput), FALSE);
    GtkWidget *SubmitBtn = gtk_button_new_with_label("Login");

    gpointer *user_data = g_new(gpointer, 2);
    user_data[0] = NameInput;
    user_data[1] = PasswordInput;


    // Connect the submit button click event to the on_submit_clicked callback
    g_signal_connect(SubmitBtn, "clicked", G_CALLBACK(submit_name), user_data);
    g_signal_connect(PasswordInput, "activate", G_CALLBACK(submit_name), user_data);


    gtk_box_pack_start(GTK_BOX(main_box), labelName, FALSE, FALSE, 0);  
    gtk_box_pack_start(GTK_BOX(main_box), NameInput, FALSE, FALSE, 0);  
    gtk_box_pack_start(GTK_BOX(main_box), labelPassword, FALSE, FALSE, 0);  
    gtk_box_pack_start(GTK_BOX(main_box), PasswordInput, FALSE, FALSE, 0);  
    gtk_box_pack_start(GTK_BOX(main_box), SubmitBtn, FALSE, FALSE, 0); 
    
    gtk_widget_show_all(main_box);  
}

void render_register(GtkButton *button) {
    remove_all_children(GTK_CONTAINER(main_box));

    GtkWidget *labelUsername = gtk_label_new("Enter username:");
    GtkWidget *labelPassword = gtk_label_new("Enter password:");
    GtkWidget *labelConfirmPassword = gtk_label_new("Confirm password:");
    GtkWidget *username_entry = gtk_entry_new();
    GtkWidget *password_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE); // Make the text invisible
    GtkWidget *confirm_password_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(confirm_password_entry), FALSE); // Make the text invisible
    GtkWidget *submit_btn = gtk_button_new_with_label("Register");

    gpointer *register_data = g_new(gpointer, 4);
    register_data[0] = main_box;
    register_data[1] = username_entry;
    register_data[2] = password_entry;
    register_data[3] = confirm_password_entry;

    g_signal_connect(submit_btn, "clicked", G_CALLBACK(submit_register), register_data);
    g_signal_connect(confirm_password_entry, "activate", G_CALLBACK(submit_register), register_data);

    gtk_box_pack_start(GTK_BOX(main_box), labelUsername, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), username_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), labelPassword, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), password_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), labelConfirmPassword, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), confirm_password_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), submit_btn, FALSE, FALSE, 0);

    gtk_widget_show_all(main_box);  
}

void activate(GtkApplication *app, gpointer user_data) {
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
    main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_name(main_box, "content");

    loginButton = gtk_button_new_with_label("Login");
    signUpButton = gtk_button_new_with_label("Create new account");
  
    // Connect the submit button click event to the on_submit_clicked callback
    g_signal_connect(loginButton, "clicked", G_CALLBACK(render_login), NULL);
    gtk_box_pack_start(GTK_BOX(main_box), loginButton, FALSE, FALSE, 0);  // Add the "Enter your name" label
    gtk_box_pack_start(GTK_BOX(main_box), signUpButton, FALSE, FALSE, 0);  // Add the text entry field
    g_signal_connect(signUpButton, "clicked", G_CALLBACK(render_register), NULL);
    // Add the box container to the window
    gtk_container_add(GTK_CONTAINER(window), main_box);
    // Show all widgets in the window
    gtk_widget_show_all(window);
    create_app_socket(&sock, &serv_addr);

    char cwd[PATH_MAX];  // Store current working directory
    char sound_path[PATH_MAX * 2];  // To store the full path of the sound file
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        g_print("Current working directory: %s\n", cwd);
        snprintf(sound_path, sizeof(sound_path), "%s/%s", cwd, "client/assets/intro.ogg");
        play_sound_effect(sound_path);
    } else {
        perror("getcwd() error");
    }
}