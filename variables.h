#include <gtk/gtk.h>
#include <pthread.h>

// Constants
const char BOTNAME[] = "EmbaBy85 Robot";
const char DBNAME[] = "storage.db";

// GTKWidget VARIABLES
GtkWidget   *window, *table, *logo, *displayMessage, *btn_new, *btn_load, *btn_save, *btn_top10, *lbl_remaining_bricks, *lbl_time, *frame_new, *frame_load,
            *frame_save, *frame_top, *frame_game, *frame_new_table, *logo2, *label_level, *label_mode, *label_p1, *label_p2, *btn_beginner, *btn_expert,
            *btn_single, *btn_multiplayer, *btn_start, *textbox_p1, *textbox_p2, *game_main_table, *game_ltable, *game_rtable, *game_lframe, *game_rframe,
            *lbl_NAME1, *lbl_NAME2, *lbl_SCORE1, *lbl_SCORE2, *lbl_turn, *lbl_turn_title, *btn_undo, *btn_redo, *lbl_VS, *top10_scores[12], *top10_names[12],
            *game[100][100], *Boxes[20][20], *table_save, *label_title_save, *btn_save1, *btn_save2, *btn_save3, *table_load, *label_title_load, *btn_load1,
            *btn_load2, *btn_load3, *top10_table;

// INTEGER VARIABLES
// `game_level`:    (2) Beginner, (5) Expert
// `game_mode`:     (1) Single, (2) Multiplayer
int nowPlayingBIT=0, boxesY=0, boxesX=0, game_bits_size = 0, game_in_progress = 0, backgroundMusic = 1, loading=0, game_time=0, continueButton = 0;
int game_level, game_mode, game_bits[250][250];

// PLAY DATA
int storedMoves_INDEX = -1, undoRedoCursor = -1, max_redo = 0, change_score=0, redoundo = 0;
int storedMoves[100][100][100];
int playingHistory[100];

// SQLITE
sqlite3 *db;

// Threads
pthread_t musicThread;
