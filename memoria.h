/*Nome do grupo: Jéssica Genta dos Santos - DRE: 111031073.
		 Pedro Caldas Coutinho - DRE: 111380919. 
		 Sidney Ribeiro Ramos Junior - DRE: 110162342.
Trabalho 2 de Sistema Operacionais
*/ 

#include	<stdio.h>
#include 	<stdlib.h>
#include	<pthread.h>
#include 	<sched.h>
#include 	<stdbool.h>
#include 	<time.h>
  
//printf(ANSI_COLOR_RED "This text is RED!" ANSI_COLOR_RESET "\n");//exemplo de impressao colorida
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_BOLD_ON    "\x1b[1m"
#define ANSI_INVERSE_ON "\x1b[7m"
#define ANSI_BOLD_OFF   "\x1b[22m"
#define ANSI_FG_BLACK   "\x1b[30m"
#define ANSI_FG_RED     "\x1b[31m"
#define ANSI_FG_GREEN   "\x1b[32m"
#define ANSI_FG_YELLOW  "\x1b[33m"
#define ANSI_FG_BLUE    "\x1b[34m"
#define ANSI_FG_MAGENTA "\x1b[35m"
#define ANSI_FG_CYAN    "\x1b[36m"
#define ANSI_FG_WHITE   "\x1b[37m"
#define ANSI_BG_RED     "\x1b[41m"
#define ANSI_BG_GREEN   "\x1b[42m"
#define ANSI_BG_YELLOW  "\x1b[43m"
#define ANSI_BG_BLUE    "\x1b[44m"
#define ANSI_BG_MAGENTA "\x1b[45m"
#define ANSI_BG_CYAN    "\x1b[46m"
#define ANSI_BG_WHITE   "\x1b[47m"

//Escolhe um dos casos abaixo. O que voce quiser, basta descomentar e o outro comentar:

#define FRAME_LIMIT 24 //64*MULTIPLUS //main_memory_size //64
#define MAIN_MEMORY_SIZE FRAME_LIMIT //64
#define VIRTUAL_MEMORY_SIZE 2*FRAME_LIMIT //NAOSEIQTO
#define THREAD_LIMIT 15 //20*MULTIPLUS //20
#define PAGE_LIMIT 10 //50*MULTIPLUS //50
#define WORKSET_LIMIT 5 //PAGE_LIMIT //PARA TESTES //4

//#define FRAME_LIMIT 64
//#define MAIN_MEMORY_SIZE FRAME_LIMIT
//#define VIRTUAL_MEMORY_SIZE FRAME_LIMIT
//#define THREAD_LIMIT 120
//#define PAGE_LIMIT 50
//#define WORKSET_LIMIT 5

struct Page{
	int process_id;
    int number;
    int value;
};

typedef union{
	int ids[PAGE_LIMIT];
	int frames[PAGE_LIMIT];
} WorkingSet;

struct Process{
    int id;
    struct Page page_list[PAGE_LIMIT];
    WorkingSet works;
};

int number_of_process = 0;
struct Process process_list[THREAD_LIMIT];

int running_process[THREAD_LIMIT] = { [0 ... THREAD_LIMIT-1 ] = -1 }; 
int running_process_index =0;
int stopped_process[THREAD_LIMIT] = { [0 ... THREAD_LIMIT-1 ] = -1 }; 
int stopped_process_index=0;

//LRU 
int recent_frame[FRAME_LIMIT] = { [0 ... FRAME_LIMIT-1 ] = -1 }; 
int number_of_free_frames = FRAME_LIMIT;
int number_of_non_free_frames = 0;

struct Page main_memory[FRAME_LIMIT];
struct Page virtual_memory[VIRTUAL_MEMORY_SIZE];

pthread_t thread[THREAD_LIMIT];
pthread_mutex_t memory_lock;
pthread_mutex_t process_list_lock;

int page_queue[FRAME_LIMIT];

//Gerenciador de memoria
void print_memories();
void reset_main_memory();
void reset_virtual_memory();
int free_frames();
void memory_overflow();

//Process functions
void request_page(int process_id, int page_number);
int create_process();
void* execute_process(int id);
void initialize_page_list_of_process(int size, int process_id);
void running_processes();
void stop_process(int process_id);
void print_workingset(int process_id);
bool using_all_working_set(int process_id);
int insert_pag_empty_frames(int process_id, int page_number);
int insert_pag_full_memory(int process_id, int page_number);
int insert_pag_full_workingset(int process_id, int page_number);
bool workingset_is_full(int process_id);

//Queue functions
void add_page_to_queue(int newPage);
void refresh_queue(int page);
void shift_queue(int offSet);
void print_queue();
void print_queue_details();
int get_queue_offset(int page);
void print_LRUF();
int refresh_LRUF(int old_frame_in_memory);