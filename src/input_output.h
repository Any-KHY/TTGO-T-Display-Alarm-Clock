
/* network setting from TTGO demo */

#include <esp_timer.h>
#include <rom/ets_sys.h>
#define maxval(x,y) (((x) >= (y)) ? (x) : (y))
typedef enum {
    NO_KEY,
    LEFT_DOWN,
    LEFT_UP,
    RIGHT_DOWN,
    RIGHT_UP
} key_type;

void input_output_init();
key_type get_input();
void get_string(char *title, char *original, int len);
int storage_read_int(char *name, int def);
void storage_write_int(char *name, int val);
void storage_read_string(char *name, char *def, char *dest, int len);
void storage_write_string(char *name, char *val);
void edit_stored_string(char *name, char *prompt);