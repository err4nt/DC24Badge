#define MAX_NICK_SIZE 16

typedef struct {
    char current_text[MAX_NICK_SIZE+1];
    uint8_t cursor_pos;
    uint8_t resume_blink;
    uint8_t offset;
    uint8_t steps;
} entry_data_s;

void entry_setup(void);
void entry_teardown(void);
