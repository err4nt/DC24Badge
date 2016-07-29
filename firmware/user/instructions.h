#define MAX_NICK_SIZE 16

typedef void (*instruction_end_handler_f)(void);

typedef struct {
    char instruction_text[7][8];
    uint8_t delay;
    uint8_t steps;
    uint8_t instruction;
    uint8_t loop;
    instruction_end_handler_f end_handler; 
} instruction_data_s;

void instructions_setup(uint8_t delay);
void instruction_set(uint8_t index, char *text);
void instructions_set_end_handler(instruction_end_handler_f handler);
void instructions_teardown(void);
