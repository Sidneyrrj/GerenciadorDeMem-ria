/*Nome do grupo: Jéssica Genta dos Santos - DRE: 111031073.
		 Pedro Caldas Coutinho - DRE: 111380919. 
		 Sidney Ribeiro Ramos Junior - DRE: 110162342.
Trabalho 2 de Sistema Operacionais
*/ 

#define MULTIPLUS 1 //multiplicador para alterar proporcionalmente o tamanho das threads e memorias
#define SLEEP_TIME 500000 //500000/2 //3000000 /*casos de testes diferentes, o que você quiser usar, basta só descomentá-lo e colocá-lo para frente e comentar o resto*/

#include "memoria.h"
	
int main( int argc, char *argv[ ] ){
	reset_main_memory();
 	reset_virtual_memory();

	//inicializa a exclusão mútua (mutex)
	if (pthread_mutex_init(&memory_lock, NULL) != 0) {
        printf("\n mutex init failed\n");
        return 1;
    }
    if (pthread_mutex_init(&process_list_lock, NULL) != 0) {
        printf("\n mutex init failed\n");
        return 1;
    }

    	//inicializa as threads
	int i;
	for(i = 0; i < THREAD_LIMIT; i++){
		pthread_create(&thread[i], NULL, execute_process, create_process());
		usleep(SLEEP_TIME);
	}

	for(i = 0; i < THREAD_LIMIT; i++){
		pthread_join(thread[i], NULL);
	}

	pthread_mutex_destroy(&memory_lock);

	//imprime a situação final
	system("clear");
	print_memories();

	return 0;
}

void* execute_process(int id){
	int i;
	for(i = 0; i < PAGE_LIMIT; i++){
		pthread_mutex_lock(&memory_lock);
		usleep(SLEEP_TIME);
		system("clear");
		printf("--->Entrando com PID: %d e Pagina: %d\n", id, i);
		request_page(id, i);
		print_memories();
		pthread_mutex_unlock(&memory_lock);

		usleep(0);//troca de contexto
	}

	pthread_mutex_lock(&memory_lock);
	system("clear");
	printf(ANSI_COLOR_RED"\n\t---->Parando processo com PID: %d\n"ANSI_COLOR_RESET, id);
	stop_process(id);
	print_memories();
	pthread_mutex_unlock(&memory_lock);
}

int create_process(){
	int i;
	struct Process _process;
	pthread_mutex_lock(&process_list_lock);
	_process.id = number_of_process;
	process_list[_process.id] = _process;
	number_of_process++;
	running_process[running_process_index] = _process.id;
	running_process_index++;
	for(i=0;i<PAGE_LIMIT;i++) process_list[_process.id].works.frames[i] = -1;
	pthread_mutex_unlock(&process_list_lock);
	initialize_page_list_of_process(PAGE_LIMIT, number_of_process - 1);

	return _process.id;
}

void print_memories(){
	running_processes(); 

	int i;
	number_of_non_free_frames = 0;
	number_of_free_frames = 0;

	printf("\tMEMORIA PRINCIPAL\t\t\t\t\t\t\tMEMORIA SECUNDARIA\n");
		
	printf("______________________________________\t\t_______________________________________________________________________________________\n");

	for (i = 0; i < FRAME_LIMIT; i++){
		//memoria principal 0-15
		if(main_memory[i].process_id > -1){
			printf("Frame: %2d -> Processo: %2d -> Page: %2d.\t\t", i, main_memory[i].process_id, main_memory[i].number);
			number_of_non_free_frames++;
		}
		else{
			printf("Frame: %2d Vazio\t\t\t\t\t", i);
			number_of_free_frames++;
		}
		
		//memoria virtual 0-15
		if(virtual_memory[i].process_id > -1)  printf("Frame: %2d -> Processo: %2d -> Page: %2d.\t\t", i, virtual_memory[i].process_id, virtual_memory[i].number);
		else  printf("Frame: %2d Vazio\t\t\t\t\t", i);

		//memoria virtual 16-31
		if (FRAME_LIMIT+i>=VIRTUAL_MEMORY_SIZE)	printf("--\t\t\t\t\t\t");
		else if(virtual_memory[FRAME_LIMIT+i].process_id > -1)  printf("Frame: %2d -> Processo: %2d -> Page: %2d.\t\t", FRAME_LIMIT+i, virtual_memory[FRAME_LIMIT+i].process_id, virtual_memory[FRAME_LIMIT+i].number);
		else printf("Frame: %2d Vazio\t\t\t\t\t", FRAME_LIMIT+i);
		printf("\n");
	}

	if ( (number_of_non_free_frames + number_of_free_frames) != FRAME_LIMIT) { printf("Erro em  qtdade frames"); exit(0);}
	print_LRUF();
 
	//print_queue_details();//debug

	printf("___________________________________Numero da Pagina______");
	for (i = 0; i < PAGE_LIMIT; i++)	printf("_%2i",i);
	printf("_\n");
	for(i=0;i<THREAD_LIMIT;i++) print_workingset(i);	
}

void initialize_page_list_of_process(int size, int process_id){
	int i;
	for(i = 0; i < size; i++)
	{
		process_list[process_id].page_list[i].process_id = process_id;
		process_list[process_id].page_list[i].number = i;
	}
}

void stop_process(int process_id){
	int i;
	stopped_process[stopped_process_index] = process_id;
	stopped_process_index++;
	for (i=0; i<THREAD_LIMIT;i++) if (running_process[i] == process_id) running_process[i] = -1;

	for (i = 0; i < FRAME_LIMIT; i++){
		if(main_memory[i].process_id == process_id){
			main_memory[i].process_id = -1;
		}
	}

	for (i = 0; i < PAGE_LIMIT; i++) process_list[process_id].works.frames[i]=-1;
}

void reset_main_memory(){
	int i;
	for (i = 0; i < FRAME_LIMIT; i++){
		main_memory[i].process_id = -1;
	}
}

void reset_virtual_memory(){
	int i;
	for (i = 0; i < VIRTUAL_MEMORY_SIZE; i++){
		virtual_memory[i].process_id = -1;
	}
}

void memory_overflow(){

	printf("*MEMORIA ESTOURADA - Como devemos tratar?\n");//tratamento: parar e gravar a memoria toda em um arquivo, apos isso reinicializar a memoria
}

void add_page_to_queue(int newPage){
	shift_queue(0);
	page_queue[FRAME_LIMIT - 1] = newPage;
}

void refresh_queue(int page){
	int offSet = get_queue_offset(page);
	shift_queue(offSet);
	page_queue[FRAME_LIMIT - 1] = page;
}

void shift_queue(int offSet){
	int i;
	for (i = offSet; i < FRAME_LIMIT - 1; i++) page_queue[i] = page_queue[i+1];	
}

int get_queue_offset(int page){
	int i;
	for (i = 0; i < FRAME_LIMIT; i++) 
	{   
	    if(page_queue[i] == page)
	    	return i;
	}
}

void running_processes(){
	int i;
	printf(ANSI_COLOR_YELLOW "Estamos executando os processos: \n"ANSI_COLOR_RESET);
	for (i=0; i<THREAD_LIMIT;i++)
		if (running_process[i] > -1) 
			printf(ANSI_COLOR_YELLOW "%i "ANSI_COLOR_RESET,running_process[i]);
	printf("\n");
	printf(ANSI_COLOR_GREEN "Os seguintes processos terminaram: \n" ANSI_COLOR_RESET);
	for (i=0; i<THREAD_LIMIT;i++)
		if (stopped_process[i] >-1) 
			printf(ANSI_COLOR_GREEN "%i " ANSI_COLOR_RESET,stopped_process[i]);

	printf("\n");
}

void print_queue(){
	int i;
	printf(ANSI_COLOR_CYAN"\nFila:  {process,page}\n");
	for(i=0;i<FRAME_LIMIT;i++) 	printf("[%d,%d] \v", page_queue[i]/PAGE_LIMIT, page_queue[i]%PAGE_LIMIT);
	printf("\n"ANSI_COLOR_RESET);
}

void print_queue_details(){
	int i;
		for (i = 0; i < FRAME_LIMIT; i++){
			if(i == 0) 
				printf("Fila:\tSai ----> %d: Processo: %d -> Page: %d.\n", i, page_queue[i]/PAGE_LIMIT, page_queue[i]%PAGE_LIMIT);
			else if(i == FRAME_LIMIT -1)
				printf("    |\tEntra --> %d: Processo: %d -> Page: %d.\n", i, page_queue[i]/PAGE_LIMIT, page_queue[i]%PAGE_LIMIT);
			else
				printf("    |\t--------> %d: Processo: %d -> Page: %d.\n", i, page_queue[i]/PAGE_LIMIT, page_queue[i]%PAGE_LIMIT);
		}
}

void print_LRUF(){
	int i;
	printf(ANSI_BOLD_ON"LRUF - Last Recent Used Frames: [new .. old]"ANSI_COLOR_RESET);
	printf("\nRecente ->" );
	printf("  "ANSI_INVERSE_ON "%i" ANSI_COLOR_RESET, recent_frame[0]);
	for(i=1;i<FRAME_LIMIT;i++) printf("  %i", recent_frame[i]);
	printf(" -> Proximo a ser removido \n");	
}

void print_workingset(int process_id){
	int i;
	printf("Paginas do processo %2i esta alocado nos seguintes frames: ",process_id);
	for (i = 0; i < PAGE_LIMIT; i++) if (process_list[process_id].works.frames[i]==-1) printf(" %2i",process_list[process_id].works.frames[i]);
									else printf(ANSI_INVERSE_ON" %2i"ANSI_COLOR_RESET,process_list[process_id].works.frames[i]);
	printf("\n");
}

bool using_all_working_set(int process_id){
	int i;
	for (i = 0; i < PAGE_LIMIT; i++) if (process_list[process_id].works.frames[i]==-1){
		return false;
	}
	return true;
}

void request_page(int process_id, int page_number){
	int frame=FRAME_LIMIT-1;//remocao do ultimo frame da lista para a virtual como padrao
	int freeframes = free_frames();

	if( workingset_is_full(process_id) ){
		printf("... O working set do processo %i esta cheio\n",process_id);
		frame=insert_pag_full_workingset(process_id, page_number);
	}
	//existencia de frames vazios?
	else if ( freeframes> 0){
		printf("... Ainda existem frames vazios\n");
		frame=insert_pag_empty_frames(process_id, page_number);
	}

	else{
		printf("... A memoria esta cheia\n");
		frame=insert_pag_full_memory(process_id, page_number);
	}

	//adiciona o frame da nova pagina ao workingsetlimit, pode-se usar um dos 2 metodos. O que voce quiser usar, basta descomentar um e comentar o outro
	//process_list[ main_memory[frame].process_id ].works.frames[ main_memory[frame].number ]=frame;
	process_list[process_id].works.frames[page_number]=frame;

	//insere na memoria principal
	main_memory[frame] = process_list[process_id].page_list[page_number];
	add_page_to_queue(PAGE_LIMIT * process_id + main_memory[frame].number);
}

int insert_pag_empty_frames(int process_id, int page_number){
	int i;
	int frame=FRAME_LIMIT-1;//remocao do ultimo frame da lista para a virtual como padrao
	int randompage = rand()%FRAME_LIMIT;//sorteia uma pagina
	srand(time(NULL));

	for (i = randompage; i < FRAME_LIMIT; i++){
		if(main_memory[i].process_id == -1){
			frame=refresh_LRUF(i);//atribui a variavel frame o valor do frame vazio
			return frame;
		}
	}

	for (i = 0; i < randompage; i++){
		if(main_memory[i].process_id == -1){
			frame=refresh_LRUF(i);//atribui a variavel frame o valor do frame vazio
			return frame;
		}
	}
}

int insert_pag_full_memory(int process_id, int page_number){
	int i;
	int frame=FRAME_LIMIT-1;//remocao do ultimo frame da lista para a virtual como padrao
	int last = FRAME_LIMIT-1;//ultimo da fila torna-se o primeiro da fila, ele ira para o inicio dela

	//atualiza processos na virtual
	if (virtual_memory[0].process_id != -1) {
		if(virtual_memory[VIRTUAL_MEMORY_SIZE-1].process_id != -1) memory_overflow();
		for (i = VIRTUAL_MEMORY_SIZE-1; i > 0; i--) virtual_memory[i] = virtual_memory[i-1]; 
		
	}
	
	//movimenta o LRUF
	if(recent_frame[last] != -1) { 
		frame=refresh_LRUF(recent_frame[last] );

		//copia para memoria virtual o frame que saira
		virtual_memory[0] = main_memory[ recent_frame[0] ];
	}	

	//remove a pagina do workingset
	//Frame: 		frame
	//Processo: 	main_memory[frame].process_id
	//Page: 		main_memory[frame].number
	process_list[main_memory[frame].process_id].works.frames[main_memory[frame].number]=-1;

	return frame;
}

int insert_pag_full_workingset(int process_id, int page_number){
	int i;
	int remover = FRAME_LIMIT-1;//coloca na ultima posicao por padrao 
	int recent = -1;
	int index = -1;
	int randompage = rand()%PAGE_LIMIT;//sorteia uma pagina
	srand(time(NULL));

	//sorteia dentre o workingset qual pagina/frame saira da memoria
	for (i = 0; i < PAGE_LIMIT; i++) if (process_list[process_id].works.frames[i] != -1) {
		remover = process_list[process_id].works.frames[i];
		//se encontrar uma pagina alocada antes da sorteada, continuar procurando, caso encontrar, usar depois aquele frame para substituicao
		if (i > randompage) break; 
	}
	printf("Removendo processo do Frame %i\n", remover);

	//atualizacao dos processos na memoria virtual
	if (virtual_memory[0].process_id != -1) {
		if(virtual_memory[VIRTUAL_MEMORY_SIZE-1].process_id != -1) memory_overflow();
		for (i = VIRTUAL_MEMORY_SIZE-1; i > 0; i--) virtual_memory[i] = virtual_memory[i-1];
	}

	for (i = 0; i<FRAME_LIMIT; i++) {
		if ( recent_frame[i] == remover ){
			recent = recent_frame[i];
			index=i;
			break ;
		}
	}

	if (recent == -1 || index ==-1)
	{
		printf("Erro**\n%i %i", recent, index);
		exit(-1);
	}

	refresh_LRUF(remover);
	
	//copia para a memoria virtual o frame que saira
	virtual_memory[0] = main_memory[ remover ];

	//atualiza o valor do frame a ser retirado da memoria principal
	//frame = remover;

	//remove a pagina do workingset
	//Frame: 		frame
	//Processo: 	main_memory[frame].process_id
	//Page: 		main_memory[frame].number
	process_list[ main_memory[remover].process_id ].works.frames[main_memory[remover].number]=-1;

	return remover;
}

bool workingset_is_full(int process_id){
	int i,workingset =0;

	for (i = 0; i < PAGE_LIMIT; i++) if (process_list[process_id].works.frames[i] != -1) workingset++;

	if (workingset == WORKSET_LIMIT) return true;
	else if (workingset < WORKSET_LIMIT) return false;
	else if (workingset > WORKSET_LIMIT) { 
		printf(ANSI_BG_RED ":::: WORKSET_LIMIT Estourado pelo processo %i ::::\n", process_id); 
		
		// printf("Remover FRAME %i \n", insert_pag_full_workingset(process_id, NULL));//debug
		//print_memories();//debug
		exit(-1);
	}
}

int free_frames(){
	int i;
	number_of_non_free_frames = 0;
	number_of_free_frames = 0;

	for (i = 0; i < FRAME_LIMIT; i++){
		if(main_memory[i].process_id > -1) number_of_non_free_frames++;
		else number_of_free_frames++;
	}

	if ( (number_of_non_free_frames + number_of_free_frames) != FRAME_LIMIT) { 
		printf("f=%i nf=%i\nErro2 em  qtdade frames\n",number_of_non_free_frames , number_of_free_frames); 
		exit(0);
	}
	return number_of_free_frames;
}

int refresh_LRUF(int old_frame_in_memory){
	int i,j;
	int ToDo=0;
	int recent=-1;

	for (i = 0; i < FRAME_LIMIT; i++){
		if (recent_frame[i] == old_frame_in_memory){
			//frame ja esta referenciado no LRUF
			ToDo++;
			break;
		}
	}

	//insere na LRUF pela primeira vez
	if (ToDo == 0){
		for (i = FRAME_LIMIT; i > 0; i--) { 
			recent_frame[i] = recent_frame[i-1];
		}
		recent_frame[0] = old_frame_in_memory;
	}

	//reinsere no LRUF
	else if (ToDo == 1){
		for (i = 0; i<FRAME_LIMIT; i++) {
			if(recent_frame[i]== old_frame_in_memory ){//atualiza o LRUF e remove as copias da fila antes de inserir
				for (j = i; j > 0; j--) { 
					recent_frame[j]=recent_frame[j-1];
				}
				recent_frame[0]=old_frame_in_memory;//o ultimo vira o primeiro
			}
		}
	}

	//verificacao de erros
	else if ( ToDo >1){
		printf("Erro ***\n r %i t %i",recent,ToDo);
		exit(-1);
	}

	return recent_frame[0];
}