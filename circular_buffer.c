#include <circular_buffer.h>

char buff_keyboard[N_ITEMS_C_BUFF];
int begin_c_buffer;
int end_c_buffer;
int len_c_buffer;


void init_c_buff(){
	begin_c_buffer = 0;
	end_c_buffer = begin_c_buffer;
}

char take_next_c_buff(){
	char ret = buff_keyboard[begin_c_buffer];
	begin_c_buffer =  (begin_c_buffer + 1) % N_ITEMS_C_BUFF;
	--len_c_buffer;
	return ret;
}

int append_c_buff(char c){
	if(len_c_buffer == N_ITEMS_C_BUFF) return 0;
	buff_keyboard[end_c_buffer] = c;
	++len_c_buffer;
	end_c_buffer = (end_c_buffer + 1) % N_ITEMS_C_BUFF;
	return 1;
}

int has_next_c_buff(){
	return  begin_c_buffer != end_c_buffer || len_c_buffer == N_ITEMS_C_BUFF; 
}
