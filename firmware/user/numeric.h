#define MAX_NICK_SIZE 16

typedef struct {
    int16_t current_value;
    int16_t max_value;
    int16_t min_value;
    uint8_t resume_blink;
    uint8_t offset;
    uint8_t steps;
} numeric_data_s;

void numeric_setup(void);
void numeric_teardown(void);
