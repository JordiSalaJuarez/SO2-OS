#ifndef _CIRCULAR_BUFFER_H_
#define _CIRCULAR_BUFFER_H_

#define N_ITEMS_C_BUFF 20

extern char buff_keyboard[N_ITEMS_C_BUFF];
extern int begin_c_buffer;
extern int end_c_buffer;
extern int len_c_buffer;

void init_c_buff();

char take_next_c_buff();

int append_c_buff(char c);

int has_next_c_buff();


#endif