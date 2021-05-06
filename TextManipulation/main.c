
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "mpi.h"


#define SIZE 100000
#define MAX_P 4

char *text;
char **tasks;
int fs;
int nr_tasks;
int crt_task;
pthread_mutex_t lock;
pthread_barrier_t bar;



void comedy(char* par){
	size_t n = strlen(par);
	int i = 0;
	int i_word = 0;
	for(i = 0; i < n; i++) {
		i_word++;
		if(par[i] == ' ' || par[i] == '\n' || par[i] == '.' || par[i] == '.' || par[i] == '!' || par[i] == '?'){
			i_word = 0;
			continue;
		}
		
		if(par[i] >= 'a' && par[i] <= 'z' && i_word % 2 == 0){
			par[i] -= 32;
		}
	}
}

void fantasy(char *par) {
	size_t n = strlen(par);
	int i = 0;
	bool change = true;
	for(i = 0; i < n; i++) {
		if(change && (par[i] >= 'a' && par[i] <= 'z')) {
			change = false;
			par[i] -= 32;
		} else if(!change && (par[i] == ' ' || par[i] == '\n' || par[i] == ',' || par[i] == '.' || par[i] == '!' || par[i] == '?' || par[i] == ';')){
			change = true;
		} else if(change && (par[i] >= 'A' && par[i] <= 'Z')) {
			change = false;
		}
	}
}

void reverse(char *x, int begin, int end)
{
   char c;

   if (begin >= end)
      return;

   c          = *(x+begin);
   *(x+begin) = *(x+end);
   *(x+end)   = c;

   reverse(x, ++begin, --end);
}

void science(char *par) {
	char word[30];
	size_t n = strlen(par);

	int i = 0;
	int nr = 0;


	for(i = 0; i < n; i++) {
		if(par[i] == '\n') {
			nr = 0;
		}
		else if(!(par[i] == ' ' || par[i] == '\n' || par[i] == ',' || par[i] == '.' || par[i] == '!' || par[i] == '?' || par[i] == ';')){
			nr++;
			if(nr % 7 != 0){
				while(!(par[i] == ' ' || par[i] == '\n' || par[i] == ',' || par[i] == '.' || par[i] == '!' || par[i] == '?' || par[i] == ';')) {
					i++;
				}
				i--;
			}
			else {
				int j = 0;
				int k = i;
				while(!(par[k] == ' ' || par[k] == '\n' || par[k] == ',' || par[k] == '.' || par[k] == '!' || par[k] == '?' || par[k] == ';')) {
					word[j] = par[k];
					j++;
					k++;
				}
				reverse(word, 0, j-1);
				for(k = 0; k < j; k++, i++) {
					par[i] = word[k];
				}
			}
		}
	}
} 

bool is_consonant(char c){
	return !(c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u') && 
	!(c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U') &&
	((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

char *horror(char* par){
	size_t size = strlen(par);
	char* tmp = (char*) malloc(2 * size);
    int i;
    int j;

    for(i = 0, j = 0; i < size; i++) {
    	if(is_consonant(par[i])) {
    		tmp[j] = par[i];
    		if(par[i] >= 'A' && par[i] <= 'Z'){
    			tmp[j+1] = par[i] + 32;
    		}
    		else tmp[j+1] = par[i];
    		j += 2;
    	} else {
    		tmp[j] = par[i];
    		j++;
    	}
    }
    return tmp;
}

void *citire(void *arg){

	int id = *(int *)arg;

	long long start = id * (double)fs / 4;
    long long end = (id + 1) * (double)fs / 4 < fs ?
    (id + 1) * (double)fs / 4 : fs;

	FILE *fin = fopen("in.txt", "r");

	fseek(fin, start, SEEK_SET);

	fread(text + start, end - start, 1, fin);

	pthread_exit(NULL);

}


void *rezolve_tasks(void* arg){
	int id = *(int *)arg;
	int mpi_rank = id + 1;
	int i = 0;
	int count;
	pthread_mutex_lock(&lock);
	MPI_Status stat;
 

	i = crt_task;
	crt_task += 1;

	pthread_mutex_unlock(&lock);

	while(i < nr_tasks) {


		count = strlen(tasks[i]);
		count++;

		printf("thread #%d info -> %s\n\n", id,tasks[i]);

		MPI_Send(&count, 1, MPI_INT, mpi_rank, i, MPI_COMM_WORLD);

		MPI_Send(tasks[i], count ,MPI_BYTE, mpi_rank, i, MPI_COMM_WORLD);

		int new_count;
		
		MPI_Recv(&new_count, 1, MPI_INT, mpi_rank, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);

		if(new_count != count) {
			tasks[i] = (char*) malloc(new_count * sizeof(char));
		}

		MPI_Recv(tasks[i], new_count, MPI_BYTE, mpi_rank, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);


		pthread_mutex_lock(&lock); 

		i = crt_task;
		crt_task += 1;

		pthread_mutex_unlock(&lock);		

	} 
	count = 5;
	MPI_Send(&count, 1 ,MPI_INT, mpi_rank, i, MPI_COMM_WORLD);
	MPI_Send("done\0", 5 ,MPI_CHAR, mpi_rank, i, MPI_COMM_WORLD);

	pthread_exit(NULL);



}

void tokenize(char *text) {
	int i = 0;
	nr_tasks = 1;
	size_t task_size = 1000;
	tasks = (char**) malloc(task_size * sizeof(char*));

	tasks[0] = text;
	for(i = 0; i < fs - 1; i++) {
		if(text[i] == '\n' && text[i + 1] == '\n') {
			text[i] = '\0';
			text[i + 1] = '\0';
			i++;
			tasks[nr_tasks] = text + i + 1;
			nr_tasks++; 
		}
	}
}


char** edited_pars;

char** argv_g;
int argc_g;
int n;


void *master_threads(void *arg){
	int id = *(int *)arg;
	int mpi_rank = id + 1;

	int nr_per_gen = 0;

	FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    int pr_number = 0;

    int base_size = 1500;
    char* par = (char*)malloc(base_size * sizeof(char));

    fp = fopen(argv_g[1], "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        if(!strcmp("horror\n", line) && id == 0) {
        	nr_per_gen++;
        	par[0] = '\0';
        	int size = 0;

        	while(((read = getline(&line, &len, fp)) != -1) && strcmp("\n", line)){
        		size += strlen(line) + 1;

        		if(base_size < size) {
        			char *tmp = realloc(par, 2 * base_size * sizeof(char));
        			if(tmp) {
        				par = tmp;
        			} else {
        				printf("eroare la realocare :(\n");		//make it a function
        				exit(1);
        			}
        			base_size *= 2;
        		}
        		strcat(par, line);
        	}

        	MPI_Send(&size, 1 ,MPI_INT, mpi_rank, mpi_rank, MPI_COMM_WORLD);
        	MPI_Send(&pr_number, 1 ,MPI_INT, mpi_rank, mpi_rank, MPI_COMM_WORLD);

			MPI_Send(par, size ,MPI_BYTE, mpi_rank, mpi_rank, MPI_COMM_WORLD);

        	if(strcmp("\n", line))pr_number++;

        }
        if(!strcmp("fantasy\n", line) && id == 1) {
        	nr_per_gen++;
        	int size = 0;
        	par[0] = '\0';
        	while(((read = getline(&line, &len, fp)) != -1) && strcmp("\n", line)){
        		size += strlen(line) + 1;
        		if(base_size < size) {
        			char *tmp = realloc(par, 2 * base_size * sizeof(char));
        			if(tmp) {
        				par = tmp;
        			} else {
        				printf("eroare la realocare :(\n");		//make it a function
        				exit(1);
        			}
        			base_size *= 2;
        		}
        		strcat(par, line);
        	}
        	MPI_Send(&size, 1 ,MPI_INT, mpi_rank, mpi_rank, MPI_COMM_WORLD);
        	MPI_Send(&pr_number, 1 ,MPI_INT, mpi_rank, mpi_rank, MPI_COMM_WORLD);

			MPI_Send(par, size ,MPI_BYTE, mpi_rank, mpi_rank, MPI_COMM_WORLD);
        	if(strcmp("\n", line))pr_number++;
        }
        if(!strcmp("comedy\n", line) && id == 2) {
        	nr_per_gen++;
        	int size = 0;
        	par[0] = '\0';
        	while(((read = getline(&line, &len, fp)) != -1) && strcmp("\n", line)){
        		size += read + 1;
        		if(base_size < size) {
        			char *tmp = realloc(par, 2 * base_size * sizeof(char));
        			if(tmp) {
        				par = tmp;
        			} else {
        				printf("eroare la realocare :(\n");		//make it a function
        				exit(1);
        			}
        			base_size *= 2;
        		}
        		strcat(par, line);
        	}
        	MPI_Send(&size, 1 ,MPI_INT, mpi_rank, mpi_rank, MPI_COMM_WORLD);
        	        	MPI_Send(&pr_number, 1 ,MPI_INT, mpi_rank, mpi_rank, MPI_COMM_WORLD);

			MPI_Send(par, size ,MPI_BYTE, mpi_rank, mpi_rank, MPI_COMM_WORLD);
        	if(strcmp("\n", line))pr_number++;
        }
        if(!strcmp("science-fiction\n", line) && id == 3) {
        	nr_per_gen++;
        	int size = 0;
        	par[0] = '\0';
        	while(((read = getline(&line, &len, fp)) != -1) && strcmp("\n", line)){
        		size += strlen(line) + 1;
        		if(base_size < size) {
        			char *tmp = realloc(par, 2 * base_size * sizeof(char));
        			if(tmp) {
        				par = tmp;
        			} else {
        				printf("eroare la realocare :(\n");		//make it a function
        				exit(1);
        			}
        			base_size *= 2;
        		}
        		strcat(par, line);
        	}
        	MPI_Send(&size, 1 ,MPI_INT, mpi_rank, mpi_rank, MPI_COMM_WORLD);
        	MPI_Send(&pr_number, 1 ,MPI_INT, mpi_rank, mpi_rank, MPI_COMM_WORLD);

			MPI_Send(par, size ,MPI_BYTE, mpi_rank, mpi_rank, MPI_COMM_WORLD);
        	if(strcmp("\n", line))pr_number++;
        }
        if(!strcmp("\n", line)){
        	pr_number++;
        }
    }

    len = 5;
    char fin[5] = "done\0";
    MPI_Send(&len, 1 ,MPI_INT, mpi_rank, mpi_rank, MPI_COMM_WORLD);
    MPI_Send(&pr_number, 1 ,MPI_INT, mpi_rank, mpi_rank, MPI_COMM_WORLD);

	MPI_Send(fin, len ,MPI_BYTE, mpi_rank, mpi_rank, MPI_COMM_WORLD);

    free(par);
    free(line);

    fclose(fp);

    if(id == 0) {
    	n = pr_number;

    	edited_pars = (char**) malloc(pr_number * sizeof(char*));
    }
    pthread_barrier_wait(&bar);

    MPI_Status stat;
    int count;

    while(nr_per_gen > 0) {
    	MPI_Recv(&count, 1, MPI_INT, mpi_rank, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
    	int index = stat.MPI_TAG;
    	edited_pars[index] = (char*)malloc(count * sizeof(char));
    	MPI_Recv(edited_pars[index], count, MPI_BYTE, mpi_rank, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);

    	nr_per_gen--;

    }

}

int nr_pars = 0;

typedef struct
{
	char **max20;
	int nr_tasks;
	int index;
	int next_task;
	int char_count;
}TPar;

TPar *paragraphs;
bool done;


TPar* split(char *message, int index, int count){ 
	TPar *p;

	p = (TPar*)malloc(sizeof(TPar));
	
	p->index = index;
	p->next_task = 0;
	p->nr_tasks = 0;
	p->char_count = count;
	int size = 10;
	p->max20 = (char**)malloc(10 * sizeof(char*));

	int crt = 0;
	int i;
	int nr_newlines = 0;

	char *crt_20 = message;

	for(i = 0; message[i] != '\0'; i++) {
		if(message[i] == '\n') nr_newlines++;
		if(message[i] == '\n' && nr_newlines == 20){
			nr_newlines = 0;
			
			p->max20[p->nr_tasks] = crt_20;
			crt_20 = (message + i + 1);
			message[i] = '\0';

			p->nr_tasks++;
			
			if(p->nr_tasks > size){
				char **tmp = realloc(p->max20, 2 * size* sizeof(char*));
				size *= 2;
				if(tmp){
					p->max20 = tmp;
				} else {
					printf("eroare la realocare :(\n");
					return NULL;
				}
			}
		}
	}
	p->max20[p->nr_tasks] = crt_20;
	p->nr_tasks++;


	/*printf("AICIICICI%d\n", p->nr_tasks);

	for(i = 0; i < p->nr_tasks; i++){
		printf("%s\n", p->max20[i]);
	}*/

	return p;

}

int rank;
TPar **my_pars;
int my_pars_size;

int crt_par = 0;



char *rebuild(TPar *p) {
	if(rank == 1) p->char_count *= 2;
	p->char_count += 20;
	char *result = (char*) malloc(p->char_count);

	if(rank == 1) {
		strcpy(result, "horror\n");
	} else if(rank == 2) {
		strcpy(result, "fantasy\n");
	} else if(rank == 3) {
		strcpy(result, "comedy\n");
	} else {
		strcpy(result, "science-fiction\n");
	}

	int i = 0;
	for(i = 0; i < p->nr_tasks; i++) {
		strcat(result, p->max20[i]);
		strcat(result, "\n");
	}

	return result;
}


void *edit_pars(void * arg) {
	
	int id = *(int *)arg;

	


	if(id == 0) {
		MPI_Status stat;
		my_pars_size = 10000;

		my_pars = (TPar**)malloc(my_pars_size * sizeof(TPar*));

		while(true) {

			int count;
			int index;
			MPI_Recv(&count, 1, MPI_INT, 0, rank, MPI_COMM_WORLD, &stat);
			MPI_Recv(&index, 1, MPI_INT, 0, rank, MPI_COMM_WORLD, &stat);
			char *message = (char*) malloc(count * sizeof(char));
			MPI_Recv(message, count, MPI_BYTE, 0, rank, MPI_COMM_WORLD, &stat);

			if(!strcmp(message, "done\0")) {
				done = true;
				break;
			}


			TPar *p = split(message, index, count);
			my_pars[nr_pars] = p;



			
			nr_pars++;

			if(nr_pars > my_pars_size) {
		        TPar** tmp = realloc(my_pars, 2 * my_pars_size * sizeof(TPar *));
		        if (tmp)
		        {
		            my_pars = tmp;
		        } else {
		        	printf("eroare :(\n");
		        }
		        my_pars_size *= 2;
		    }

		}
	} else {
		TPar *my_job;
		int line_index;

		while(true) {
			pthread_mutex_lock(&lock); 
			if(crt_par < nr_pars){
				if(my_pars[crt_par]->next_task == my_pars[crt_par]->nr_tasks) {
					crt_par ++;
					pthread_mutex_unlock(&lock);
					continue;
				} else{
					my_job = my_pars[crt_par];
					line_index = my_job->next_task;
					my_pars[crt_par]->next_task++;
				}
			}
			else if(crt_par == nr_pars){
				if(done){
					pthread_mutex_unlock(&lock);
					break;
				}
				else {
					pthread_mutex_unlock(&lock);
					continue;
				}
			}
			pthread_mutex_unlock(&lock);

			if(rank == 1) {
				char* tmp = horror(my_job->max20[line_index]);
				my_job->max20[line_index] = tmp;
			} else if(rank == 2) {
				fantasy(my_job->max20[line_index]);

			} else if(rank == 3){
				comedy(my_job->max20[line_index]);

			} else {
				science(my_job->max20[line_index]);
			}
		}
	}
	pthread_barrier_wait(&bar);


	if(id == 0){
		int i;
		for(i = 0; i < nr_pars; i++) {
			char* to_send = rebuild(my_pars[i]);
			MPI_Send(&(my_pars[i]->char_count), 1, MPI_INT, 0, my_pars[i]->index, MPI_COMM_WORLD);
			MPI_Send(to_send, my_pars[i]->char_count ,MPI_BYTE, 0, my_pars[i]->index, MPI_COMM_WORLD);
		}
	}
}




int main (int argc, char *argv[])
{

	argc_g = argc;
	argv_g = argv;

    int  numtasks;
    MPI_Status stat;

    int provided;
	MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    if (pthread_mutex_init(&lock, NULL) != 0) { 
	        printf("mutex_init failed\n"); 
	        return 0; 
   	}


	int err =  pthread_barrier_init(&bar, NULL, 4);

	if(err) {
		printf("err stop\n" );
		return 0;
	}

    if(rank == 0){

		int i, r;
		void *status;
		pthread_t threads[4];
		int arguments[4];



		for (i = 0; i < MAX_P; i++) {
			arguments[i] = i;
			r = pthread_create(&threads[i], NULL, master_threads, &arguments[i]);

			if (r) {
				printf("Eroare la crearea thread-ului %d\n", i);
				exit(-1);
			}
		}

		for (i = 0; i < MAX_P; i++) {
			r = pthread_join(threads[i], &status);

			if (r) {
				printf("Eroare la asteptarea thread-ului %d\n", i);
				exit(-1);
			}
		}

		FILE *fp;
		fp = fopen(argv_g[2], "w");

		for(i = 0; i < n; i++) {
			fprintf(fp, "%s", edited_pars[i]);
		}
		

		fclose(fp);



		/*char *token;
	   
		token = strtok(text, "\n\n");
		

		while(token != NULL) {
		    tasks[nr_tasks] = token;
			nr_tasks ++;
		    token = strtok(NULL, "\n\n");

		    if(nr_tasks > task_size - 1) {
		    	char** tmp = (char**) realloc(tasks, 2 * task_size * sizeof(char*));
		    	if(tmp) {
		    		tasks = tmp;
		    	} else{
		    		printf("eroare la realocare tasks :(\n");
		    	}
		    	task_size *= 2;
		    }

		}*/


		

    


	} else {

				void *status;
				int r;
				int i;

		pthread_t threads[4];
		int arguments[4];



		for (i = 0; i < 4; i++) {
			arguments[i] = i;
			r = pthread_create(&threads[i], NULL, edit_pars, &arguments[i]);

			if (r) {
				printf("Eroare la crearea thread-ului %d\n", i);
				exit(-1);
			}
		}

		for (i = 0; i < 4; i++) {
			r = pthread_join(threads[i], &status);

			if (r) {
				printf("Eroare la asteptarea thread-ului %d\n", i);
				exit(-1);
			}
		}


		

	}

	MPI_Finalize();

}
