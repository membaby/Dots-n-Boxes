#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

sqlite3 *db;

struct player {
    char name[200];
    int currentScore;
    int bestScore;
} player1, player2;

int getScore(char *name){

    sqlite3_open("scores.db", &db);
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS scores(name text, score int);", 0, 0, 0);
    char query[200];
    sqlite3_stmt *stmt;
    sprintf(query, "SELECT score FROM scores WHERE name='%s'", name);
    sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    sqlite3_step(stmt);
    int score = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return score;
}

void updateScore(char *name, int score){

    sqlite3_open("scores.db", &db);
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS scores(name text, score int);", 0, 0, 0);
    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);
    char query[200];
    sprintf(query, "SELECT * FROM scores WHERE name='%c'", *name);
    sqlite3_exec(db, query, 0, 0, 0);

    if (getScore(name)){
        sprintf(query, "UPDATE scores SET score=%d WHERE name='%s'", score, name);
        sqlite3_exec(db, query, 0, 0, 0);
    } else {
        sprintf(query, "INSERT INTO scores VALUES ('%s', %d);", name, score);
        sqlite3_exec(db, query, 0, 0, 0);
    }

    sqlite3_exec(db, "END TRANSACTION;", 0, 0, 0);
}

void getTop10(){
    sqlite3_open("scores.db", &db);
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS scores(name text, score int);", 0, 0, 0);
    char query[200];
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT * FROM scores", -1, &stmt, NULL);
    for(int i=0; i<5; i++){
        sqlite3_step(stmt);
        int score = sqlite3_column_int(stmt, 1);
        char name[100];
        name = *sqlite3_column_text(stmt, 0);

        printf("%d -> ", score);
        for (int j=0; j<100; j++){
            printf("%c", name[i]);
        }
        printf("\n");

    }
    sqlite3_finalize(stmt);
}

void createPlayer(char *player1_name, char *player2_name){

    for (int i=0; i<=200; i++){
        player1.name[i] = player1_name[i];
        player2.name[i] = player2_name[i];
    }
    player1.currentScore = 0;
    player1.bestScore = getScore(player1_name);

    player2.currentScore = 0;
    player2.bestScore = getScore(player2_name);

    for (int i=0; i<=50; i++){
        printf("%c", *player1.name);
    }
}
