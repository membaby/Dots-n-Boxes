#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// @desc Logs playing history into a text file for debugging purposes.
// @inputs
// `type` log type -> String (CRITICAL / ERROR / WARNING / INFO / DEBUG)
// `text` log text -> String (...)
void logging(char *type, char *text){
    char *fileName = "logging.txt";
    FILE *fp = fopen(fileName, "a");
    time_t rawtime;
    struct tm *timeinfo;
    time (&rawtime);
    timeinfo = localtime(&rawtime);
    char *time = asctime(timeinfo);
    time[strlen(time) - 1] = 0;
    printf("[%s] [%s]: %s\n", time, type, text);
    fprintf(fp, "[%s] [%s]: %s\n", time, type, text);
    fclose(fp);
}
