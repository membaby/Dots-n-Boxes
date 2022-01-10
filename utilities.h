#include <stdio.h>
#include <stdlib.h>
#include "database.h"
#include "logging.h"
#include <gtk/gtk.h>
#include <stdbool.h>
#include <conio.h>
#include <time.h>
#include <pthread.h>

// GTKWidget VARIABLES
GtkWidget *window, *table, *logo, *displayMessage, *btn_new, *btn_load, *btn_save, *btn_top10, *lbl_remaining_bricks, *lbl_time;
GtkWidget *frame_new, *frame_load, *frame_save, *frame_top, *frame_game, *frame_new_table, *logo2;
GtkWidget *label_level, *label_mode, *label_p1, *label_p2, *btn_beginner, *btn_expert, *btn_single, *btn_multiplayer, *btn_start;
GtkWidget *textbox_p1, *textbox_p2, *game_main_table, *game_ltable, *game_rtable, *game_lframe, *game_rframe;
GtkWidget *lbl_NAME1, *lbl_NAME2, *lbl_SCORE1, *lbl_SCORE2, *lbl_turn, *lbl_turn_title, *btn_undo, *btn_redo, *lbl_VS, *top10_scores[12], *top10_names[12];

pthread_t musicThread, timerThread;

// FUNCTIONS
static void load_css(void){
    GtkCssProvider *provider;
    GdkDisplay *display;
    GdkScreen *screen;
    const gchar *css_style_file = "styles.css";
    GFile *css_fp = g_file_new_for_path(css_style_file);
    GError *error = 0;
    provider = gtk_css_provider_new();
    display = gdk_display_get_default();
    screen = gdk_display_get_default_screen(display);
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_css_provider_load_from_file(provider, css_fp, &error);
    g_object_unref(provider);
}

void play_music(int i) {
    if (i == 1){
        while (backgroundMusic) {
            PlaySound("Music/background.wav", NULL, NULL);
        }
        PlaySound(NULL, NULL, NULL);
        gtk_label_set_text(displayMessage, "");
        GtkWidget *image = gtk_image_new_from_file("Images/logo-muted.png");
        gtk_button_set_image(logo, image);
    } else if (i == 2 && !backgroundMusic){
        PlaySound("Music/click.wav", NULL, NULL);
    }
}

void storeMove(){
    storedMoves_INDEX += 1;
    undoRedoCursor += 1;
    for (int i=0; i<boxesY*2+1; i++){
        for (int j=0; j<boxesY+1; j++){
            storedMoves[storedMoves_INDEX][i][j] = game_bits[i][j];
        }
    }
    if (nowPlayingBIT) playingHistory[storedMoves_INDEX] = 1;
    else playingHistory[storedMoves_INDEX] = 0;
}

void undoRedo(GtkWidget *widget, int undo){
    if (!game_in_progress && undo){
        Menu_Button_Click(btn_new, 0);
        return;
    } else if (!game_in_progress && !undo){
        gtk_main_quit();
        return;
    }

    redoundo = 1;
    int ready = 1;
    if (undo){
        if (undoRedoCursor > 0){
            if (game_mode == 1){
                undoRedoCursor -= 2;
                storedMoves_INDEX -= 2;
            } else {
                undoRedoCursor -= 1;
                storedMoves_INDEX -= 1;
            }
            max_redo += 1;
        }
        else ready = 0;
    } else {
        if (max_redo){
            if (game_mode == 1){
                undoRedoCursor += 2;
                storedMoves_INDEX += 2;
            } else {
                undoRedoCursor += 1;
                storedMoves_INDEX += 1;
            }

            max_redo -= 1;
        }
        else ready = 0;
    }

    if (ready){
        for (int i=0; i<boxesY*2+1; i++){
            for (int j=0; j<boxesY+1; j++){
                if (storedMoves[undoRedoCursor][i][j] != game_bits[i][j]){
                    if (i == 0 || i % 2 == 0) {
                        g_object_set_data(G_OBJECT(game[i][j*2+1]), "y", GINT_TO_POINTER(i));
                        toggleBrick(game[i][j*2+1], j);
                    } else {
                        g_object_set_data(G_OBJECT(game[i][j*2]), "y", GINT_TO_POINTER(i));
                        toggleBrick(game[i][j*2], j);
                    }
                }
            }
        }
        nowPlayingBIT = playingHistory[undoRedoCursor];
        updateGameInfo(0);
    }
    redoundo = 0;
}

void ExitButton(int visible){
    if (visible == 1){
        gtk_button_set_label(GTK_BUTTON(btn_undo), "Main Menu");
        gtk_button_set_label(GTK_BUTTON(btn_redo), "Exit Game");
        gtk_widget_set_name(GTK_BUTTON(btn_undo), "exitbutton");
        gtk_widget_set_name(GTK_BUTTON(btn_redo), "exitbutton");
    } else {
        gtk_button_set_label(GTK_BUTTON(btn_undo), "Undo");
        gtk_button_set_label(GTK_BUTTON(btn_redo), "Redo");
        gtk_widget_set_name(GTK_BUTTON(btn_undo), "GAME_TAB");
        gtk_widget_set_name(GTK_BUTTON(btn_redo), "GAME_TAB");
    }
}

void updateGameInfo(int stop){
    char player1SCORE[200], player2SCORE[200], play1[200], play2[200], nowP[200], remaining_bricks[50];
    if (player1.currentScore > player1.bestScore) player1.bestScore = player1.currentScore;
    else if (player2.currentScore > player2.bestScore) player2.bestScore = player2.currentScore;
    sprintf(player1SCORE, "Score: %d (BEST: %d)\nMoves: %d", player1.currentScore, player1.bestScore, player1.moves);
    sprintf(player2SCORE, "Score: %d (BEST: %d)\nMoves: %d", player2.currentScore, player2.bestScore, player2.moves);
    sprintf(play1, "%s", player1.name);
    sprintf(play2, "%s", player2.name);

    sprintf(remaining_bricks, "%d Lines Remaining", game_bits_size - player1.moves - player2.moves);
    gtk_label_set_text(lbl_remaining_bricks, remaining_bricks);

    if (nowPlayingBIT) sprintf(nowP, "%s", player1.name);
    else sprintf(nowP, "%s", player2.name);
    gtk_label_set_text(lbl_NAME1, play1);
    gtk_label_set_text(lbl_NAME2, play2);

    gtk_label_set_text(lbl_SCORE1, player1SCORE);
    gtk_label_set_text(lbl_SCORE2, player2SCORE);
    gtk_label_set_text(lbl_turn, nowP);

    if (stop){
        ExitButton(1);
        game_in_progress = 0;
        char text[500];
        if (player1.currentScore != player2.currentScore){
            if (player1.currentScore > player2.currentScore) winner = player1;
            else if (player1.currentScore < player2.currentScore) winner = player2;
            sprintf(text, "█ ██ ███ %s won the game! (Score: %d) ███ ██ █", winner.name, winner.currentScore);
            gtk_label_set_text(displayMessage, text);
            sprintf(text, "%s won the game! (Score: %d)", winner.name, winner.currentScore);
                if (getScore(winner.name) < winner.currentScore)
                updateScore(winner.name, winner.bestScore);
        } else {
            sprintf(text, "█ ██ ███ No winners! (Score: %d) ███ ██ █", player1.currentScore);
            gtk_label_set_text(displayMessage, text);
            sprintf(text, "No winners! (Score: %d)", player1.currentScore);
        }
        logging("INFO", text);
    }
}

bool IntelliBot = FALSE;

void nextTurn(){
    if (!game_in_progress || loading) return;
    if (nowPlayingBIT){
        if (!change_score) nowPlayingBIT = 0;
    } else {
        if (!change_score) nowPlayingBIT = 1;
    }
    change_score = 0;
    max_redo = 0;

    int N = 0;
    if (nowPlayingBIT == 0 && game_mode == 1){
        srand(time(0));
        int random_choice_x=1, random_choice_y=1;
        while (game_in_progress){
            random_choice_y = rand() % (boxesY*2+1);
            random_choice_x = rand() % (boxesY+1);
            if (game_bits[random_choice_y][random_choice_x]) {N += 1; if (N==500){break;} continue;}
            else if (random_choice_x == boxesY && (random_choice_y+1) % 2){
                continue;
            }
            else {
                if (random_choice_y == 0 || random_choice_y % 2 == 0) {
                    g_object_set_data(G_OBJECT(game[random_choice_y][random_choice_x*2+1]), "y", GINT_TO_POINTER(random_choice_y));
                    toggleBrick(game[random_choice_y][random_choice_x*2+1], random_choice_x);
                    break;
                } else {
                    g_object_set_data(G_OBJECT(game[random_choice_y][random_choice_x*2]), "y", GINT_TO_POINTER(random_choice_y));
                    toggleBrick(game[random_choice_y][random_choice_x*2], random_choice_x);
                    break;
                }
            }
        }
    }

    bool end_game = TRUE;
    for (int i=0; i<boxesY*2+1; i++){
        for (int j=0; j<boxesY+1; j++){
            if ((j < boxesY || (i+1) % 2 == 0) && !game_bits[i][j]){
                end_game = FALSE;
                break;
            }
        }
    }
    if (end_game) updateGameInfo(1);
    updateGameInfo(0);
    storeMove();
}

void stopGame(){
    GList *children, *iter;
    children = gtk_container_get_children(GTK_TABLE(game_rtable));
    for (iter = children; iter != NULL; iter = g_list_next(iter))
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    g_list_free(children);

    for (int i=0; i<20; i++){
        for (int j=0; j<20; j++){
            Boxes[i][j] = 0;
        }
    }
    game_in_progress = 0;
    storedMoves_INDEX = -1;
    undoRedoCursor = -1;
    max_redo = 0;
    change_score=0;
}

void toggleBrick(GtkWidget *widget, int x){
    int y = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "y"));
    if ((game_bits[y][x] == 1 || game_bits[y][x] == 2) && redoundo){
        char log[200];
        sprintf(log, "(ToggleBrick) Set Brick [X: %d, Y: %d] to %d", x, y, 0);
        logging("DEBUG", log);

        if (y == 0 || y % 2 == 0) {
            GtkWidget *image = gtk_image_new_from_file("Images/Row.jpg");
            gtk_button_set_image(GTK_BUTTON(game[y][x*2+1]), image);
        } else {
            GtkWidget *image = gtk_image_new_from_file("Images/Column.jpg");
            gtk_button_set_image(GTK_BUTTON(game[y][x*2]), image);
        }
        game_bits[y][x] = 0;
        if (nowPlayingBIT) player1.moves -= 1;
        else player2.moves -= 1;

    } else if (game_bits[y][x] == 0){
        char log[200];
        sprintf(log, "(ToggleBrick) Set Brick [X: %d, Y: %d] to %d", x, y, nowPlayingBIT+1);
        logging("DEBUG", log);
        GtkWidget *image;
        if (y == 0 || y % 2 == 0) {
            if (nowPlayingBIT) image = gtk_image_new_from_file("Images/Row_1.jpg");
            else  image = gtk_image_new_from_file("Images/Row_2.jpg");
            gtk_button_set_image(GTK_BUTTON(game[y][x*2+1]), image);
        } else {
            if (nowPlayingBIT) image = gtk_image_new_from_file("Images/Column_1.jpg");
            else  image = gtk_image_new_from_file("Images/Column_2.jpg");
            gtk_button_set_image(GTK_BUTTON(game[y][x*2]), image);
        }
        game_bits[y][x] = nowPlayingBIT+1;
        if (nowPlayingBIT) player1.moves += 1;
        else player2.moves += 1;
    } else {
        return;
    }
    for (int Y=0; Y<=boxesY; Y++){
        for(int X=0; X<boxesY; X++){
            if (game_bits[Y*2][X] && game_bits[Y*2+1][X] && game_bits[Y*2+1][X+1] && game_bits[Y*2+2][X]){
                if (Boxes[Y][X] && !gtk_widget_is_visible(Boxes[Y][X])){
                    GtkWidget *image;
                    if (nowPlayingBIT) {
                        image = gtk_image_new_from_file("Images/Box_1.jpg");
                        g_object_set_data(G_OBJECT(Boxes[Y][X]), "owner", GINT_TO_POINTER(nowPlayingBIT)+1);
                    } else {
                        image = gtk_image_new_from_file("Images/Box_2.jpg");
                        g_object_set_data(G_OBJECT(Boxes[Y][X]), "owner", GINT_TO_POINTER(nowPlayingBIT)+1);
                    }
                    gtk_button_set_image(GTK_BUTTON(Boxes[Y][X]), image);
                    gtk_widget_show(Boxes[Y][X]);
                    change_score += 1;
                    if (Boxes[Y+1][X] && GPOINTER_TO_INT(g_object_get_data(G_OBJECT(Boxes[Y][X]), "owner")) == GPOINTER_TO_INT(g_object_get_data(G_OBJECT(Boxes[Y+1][X]), "owner")))
                        gtk_widget_hide(game[Y*2+2][X*2+1]);
                    if (Boxes[Y-1][X] && GPOINTER_TO_INT(g_object_get_data(G_OBJECT(Boxes[Y][X]), "owner")) == GPOINTER_TO_INT(g_object_get_data(G_OBJECT(Boxes[Y-1][X]), "owner")))
                        gtk_widget_hide(game[Y*2][X*2+1]);
                    if (Boxes[Y][X+1] && GPOINTER_TO_INT(g_object_get_data(G_OBJECT(Boxes[Y][X]), "owner")) == GPOINTER_TO_INT(g_object_get_data(G_OBJECT(Boxes[Y][X+1]), "owner")))
                        gtk_widget_hide(game[Y*2+1][X*2+2]);
                    if (Boxes[Y][X-1] && GPOINTER_TO_INT(g_object_get_data(G_OBJECT(Boxes[Y][X]), "owner")) == GPOINTER_TO_INT(g_object_get_data(G_OBJECT(Boxes[Y][X-1]), "owner")))
                        gtk_widget_hide(game[Y*2+1][X*2]);
                }
            } else {
                if (Boxes[Y][X] && gtk_widget_is_visible(Boxes[Y][X])){
                    gtk_widget_hide(Boxes[Y][X]);
                    g_object_set_data(G_OBJECT(Boxes[Y][X]), "owner", GINT_TO_POINTER(0));
                    change_score -= 1;
                    if (!gtk_widget_is_visible(game[Y*2+1][X*2+2])) gtk_widget_show(game[Y*2+1][X*2+2]);
                    if (!gtk_widget_is_visible(game[Y*2+1][X*2])) gtk_widget_show(game[Y*2+1][X*2]);
                    if (!gtk_widget_is_visible(game[Y*2+2][X*2+1])) gtk_widget_show(game[Y*2+2][X*2+1]);
                    if (!gtk_widget_is_visible(game[Y*2][X*2+1])) gtk_widget_show(game[Y*2][X*2+1]);
                }

            }
        }
    }

    if (nowPlayingBIT){
        if (change_score == 1 || change_score == -1) player1.currentScore += change_score;
        else if (change_score == 2 || change_score == -2) player1.currentScore += change_score;
    } else {
        if (change_score == 1 || change_score == -1) player2.currentScore += change_score;
        else if (change_score == 2 || change_score == -2) player2.currentScore += change_score;
    }

    if (game_in_progress && !redoundo) nextTurn();
    else change_score = 0;
}

void createGame(){
    if (!loading){
        int p1_ = gtk_entry_get_text_length(GTK_ENTRY(textbox_p1));
        int p2_ = gtk_entry_get_text_length(GTK_ENTRY(textbox_p2));
        if (!game_level || !game_mode || !p1_ || !p2_){
            if (!game_level){
                gtk_label_set_text(displayMessage, "Please select a game level!");
            } else if (!game_mode){
                gtk_label_set_text(displayMessage, "Please select a game mode!");
            } else if (!p1_){
                gtk_label_set_text(displayMessage, "Please enter Player 1 name!");
            } else if (!p2_){
                gtk_label_set_text(displayMessage, "Please enter Player 2 name!");
            }
            return;
        } else {
            gtk_label_set_text(displayMessage, "");
        }
        game_time = 0;
    }

    int size_x, size_y;
    if (game_level == 2){
        size_x = 2;
        size_y = 2;
    } else {
        size_x = 5;
        size_y = 5;
    }

    int mX=0, mY=0;
    boxesX=0, boxesY=0;
    for (int i=1; i<=size_y*2+1; i++){
        if (i % 2){
            GtkWidget *image = gtk_image_new_from_file ("Images/Edge.jpg");
            game[i-1][0] = gtk_button_new();
            gtk_button_set_image(GTK_BUTTON(game[i-1][0]), image);
            gtk_table_attach(GTK_TABLE(game_rtable), game[i-1][0], 0, 1, i-1, i, GTK_FILL, GTK_FILL, 0, 0);
            for (int j=1; j<=size_x*2; j++){
                if (j%2){
                    GtkWidget *image = gtk_image_new_from_file("Images/Row.jpg");
                    game[i-1][j] = gtk_button_new();
                    gtk_button_set_image(GTK_BUTTON(game[i-1][j]), image);
                    game_bits[mY][mX] = 0;
                    char data[30];
                    sprintf(data, "%d %d", mY, mX);
                    g_object_set_data(G_OBJECT(game[i-1][j]), "y", GINT_TO_POINTER(mY));
                    g_signal_connect(game[i-1][j], "clicked", G_CALLBACK(toggleBrick), mX);
                    gtk_table_attach(GTK_TABLE(game_rtable), game[i-1][j], j, j+1, i-1, i, GTK_FILL, GTK_FILL, 0, 0);
                    mX += 1;
                } else {
                    GtkWidget *image = gtk_image_new_from_file ("Images/Edge.jpg");
                    game[i-1][j] = gtk_button_new();
                    gtk_button_set_image(GTK_BUTTON(game[i-1][j]), image);
                    gtk_table_attach(GTK_TABLE(game_rtable), game[i-1][j], j, j+1, i-1, i, GTK_FILL, GTK_FILL, 0, 0);
                }
                gtk_widget_show(game[i-1][j]);
            }
        } else {
            GtkWidget *image = gtk_image_new_from_file ("Images/Column.jpg");
            game[i-1][0] = gtk_button_new();
            gtk_button_set_image(GTK_BUTTON(game[i-1][0]), image);
            game_bits[mY][mX] = 0;
            char data[30];
            sprintf(data, "%d %d", mY, mX);
            g_object_set_data(G_OBJECT(game[i-1][0]), "y", GINT_TO_POINTER(mY));
            g_signal_connect(game[i-1][0], "clicked", G_CALLBACK(toggleBrick), mX);
            gtk_table_attach(GTK_TABLE(game_rtable), game[i-1][0], 0, 1, i-1, i, GTK_FILL, GTK_FILL, 0, 0);
            mX += 1;
            for (int j=1; j<=size_x*2; j++){
                if (j%2){
                    GtkWidget *image = gtk_image_new_from_file ("Images/Box_1.jpg");
                    Boxes[boxesY][boxesX] = gtk_button_new();
                    gtk_button_set_image(GTK_BUTTON(Boxes[boxesY][boxesX]), image);
                    g_object_set_data(G_OBJECT(Boxes[boxesY][boxesX]), "owner", GINT_TO_POINTER(0));
                    gtk_table_attach(GTK_TABLE(game_rtable), Boxes[boxesY][boxesX], j, j+1, i-1, i, GTK_FILL, GTK_FILL, 0, 0);
                    boxesX += 1;
                } else {
                    GtkWidget *image = gtk_image_new_from_file ("Images/Column.jpg");
                    game[i-1][j] = gtk_button_new();
                    gtk_button_set_image(GTK_BUTTON(game[i-1][j]), image);
                    game_bits[mY][mX] = 0;
                    char data[30];
                    sprintf(data, "%d %d", mY, mX);
                    g_object_set_data(G_OBJECT(game[i-1][j]), "y", GINT_TO_POINTER(mY));
                    g_signal_connect(game[i-1][j], "clicked", G_CALLBACK(toggleBrick), mX);
                    gtk_table_attach(GTK_TABLE(game_rtable), game[i-1][j], j, j+1, i-1, i, GTK_FILL, GTK_FILL, 0, 0);
                    gtk_widget_show(game[i-1][j]);
                    mX += 1;
                }
            }
        }

        gtk_widget_show(game[i-1][0]);
        mY += 1;
        mX = 0;
        if (boxesX) boxesY += 1;
        boxesX = 0;
    }

    for (int x=0; x<size_y*2+1; x++){
        for (int y=0; y<size_x*2+1; y++){
            if (game[x][y]) gtk_widget_set_name(game[x][y], "GAMEBUTTON");
        }
    }

    game_bits_size = (size_x+size_x+1)*(size_y)+size_x;
    ExitButton(0);
    Menu_Button_Click(btn_start, 4);
    game_in_progress = 1;
    if (game_in_progress) nextTurn();
    logging("INFO", "GAME IN PROGRESS");
}
