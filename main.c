#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>

#define BUFSIZE 1024

int height, width, begin_byte;
int thread_count;
char files[2][BUFSIZE];

int get_nex_value(char *string, int num, int file);
void *thread_function(void *arg); 

int main(int argc, char* argv[])
{
	int image[2];
    int opt;
    while ((opt = getopt(argc, argv, ":f:t:j:")) != -1) {
        switch(opt) {
            case 'f':
                strcpy(files[0], optarg);
                break;
            case 't':
                strcpy(files[1], optarg);
                break;
            case 'j':
                thread_count = atoi(optarg);
                break;
            default:
                break;
        }
    }
    if (thread_count <= 0) thread_count = 1;
    if (thread_count > 100)	thread_count = 100;

    if ((files[0][0] == 0) || (files[1][0] == 0)){
    	printf("Bad args\n");
    	exit(EXIT_FAILURE);
    }

	int res;
	image[0] = open(files[0], O_RDONLY);
	image[1] = open(files[1], O_TRUNC | O_WRONLY | O_CREAT, 0777); 
    if ((image[0] == -1) || (image[1] == -1)){
		perror("File open failed");
		exit(EXIT_FAILURE);
	}

 	char buf[BUFSIZE];
 	if ((res = get_nex_value(buf, BUFSIZE, image[0]))<=0){
 		printf("write error\n");
 		exit(EXIT_FAILURE);
 	}
 	begin_byte += res;
 	if ( strcmp(buf, "P6") && (strcmp(buf, "P4")) && (strcmp(buf, "P1")) ) {
 		printf("Неверный формат изображения. Допустимый: pnm\n");
 		exit(EXIT_FAILURE);
 	}
 	
    buf[strlen(buf)+1] = '\0';buf[strlen(buf)] = '\n';
 	if ((res = write(image[1], buf, strlen(buf)))<=0){
 		printf("write error\n");
 		exit(EXIT_FAILURE);
 	}
 	
 	if ((res = get_nex_value(buf, BUFSIZE, image[0]))<=0){
 		printf("write error\n");
 		exit(EXIT_FAILURE);
 	}
 	begin_byte += res;
 	width = atoi(buf);

    buf[strlen(buf)+1] = '\0';buf[strlen(buf)] = '\n';
 	if ((res = write(image[1], buf, strlen(buf)))<=0){
 		printf("write error\n");
 		exit(EXIT_FAILURE);
 	}

 	if ((res = get_nex_value(buf, BUFSIZE, image[0]))<=0){
 		printf("write error\n");
 		exit(EXIT_FAILURE);
 	}
 	begin_byte += res;
 	height = atoi(buf);
 	
    buf[strlen(buf)+1] = '\0';buf[strlen(buf)] = '\n';
 	if ((res = write(image[1], buf, strlen(buf)))<=0){
 		printf("write error\n");
 		exit(EXIT_FAILURE);
 	} 

 	if ((res = get_nex_value(buf, BUFSIZE, image[0]))<=0){
 		printf("write error\n");
 		exit(EXIT_FAILURE);
 	}
 	begin_byte += res;

    buf[strlen(buf)+1] = '\0';buf[strlen(buf)] = '\n';
 	if ((res = write(image[1], buf, strlen(buf)))<=0){
 		printf("write error\n");
 		exit(EXIT_FAILURE);
 	}
	close(image[0]);
	close(image[1]);

	struct timeval timeval1, timeval2;

	gettimeofday(&timeval1, NULL);

 	int i = 0;
 	time_t work_time = time(NULL);
	pthread_t *thread = malloc(sizeof(pthread_t) * thread_count);
	if (thread == NULL){
		perror("Malloc fail");
		exit(EXIT_FAILURE);
	}
 	int *massiv = malloc(sizeof(int) * thread_count);
 	if (massiv == NULL){
		perror("Malloc fail");
		exit(EXIT_FAILURE);
	}
 	do{
 	    *(massiv+i) = i;
		res = pthread_create(thread+i, NULL, thread_function, massiv+i);
		if (res != 0) {
			perror("Thread creation failed");
			exit(EXIT_FAILURE);
		}
	} while(++i < thread_count);
	i = 0;
	do{
		void *thread_result;
	 	res = pthread_join(*(thread+i), &thread_result); //wait
		if (res != 0) {
			perror("Thread join failed");
			exit(EXIT_FAILURE);
		}
	}while(++i < thread_count);
	work_time = time(NULL) - work_time;

	gettimeofday(&timeval2, NULL);
	long long k1,k2;
	k1 = ((long) timeval1.tv_sec) * 1000000 + ((long) timeval1.tv_usec);
	k2 = ((long) timeval2.tv_sec) * 1000000 + ((long) timeval2.tv_usec);
	printf("Thread count: %d\n", thread_count);
	printf("Time: %fc\n",((double)(k2-k1)/1000000));

	free(thread);
	free(massiv);
	exit(EXIT_SUCCESS);
}

int get_nex_value(char *string, int num, int file)
{	
	char tmp;
 	int i = 1;
 	int r,res = 0;
 	char *s = string;
 	res += read(file, &tmp, 1); 
	if (res <= 0) return -1;
	*(s++) = tmp;
	if (tmp == '#'){
		do{
		 	if (read(file, &tmp, 1) <= 0) return -1;
		 	res++;
		 	*(s++) = (tmp == '\n') ? '\0' : tmp;
 		}while((tmp != '\n') && (++i < num));
	}else if(tmp != '\n'){
		do{
		 	if (read(file, &tmp, 1) <= 0) return -1;
		 	res++;
		 	*(s++) = ((tmp == '\n')||(tmp == ' ')) ? '\0' : tmp;
		}while((tmp != ' ') && (tmp != '\n') && (++i < num));
	}
 	
 	if ((string[0] == '#') || (string[0] == '\n')) {return res + get_nex_value(string, num, file);}
 	
 	return res;
}

void *thread_function(void *arg) 
{
	int my_number = *((int*)arg);	

	int image[2];
	image[0] = open(files[0], O_RDONLY);
	image[1] = open(files[1], O_RDWR, 0777); 
	if ((image[0] == -1) || (image[1] == -1)){
		perror("File open failed");
		exit(EXIT_FAILURE);
	}

	int X, Y;
	int I, J;
	int Sx, Sy, S;
	int piX, piY;
	int N;

	int Mx[3][3] = {
		{-1,0,1},
		{-2,0,2},
		{-1,0,1}
	};
	int My[3][3] = {
		{-1,-2,-1},
		{0,0,0},
		{1,2,1}	
	};

	Sx = 0;
	Sy = 0;
	S = 0;
 		
 	int sh = height / thread_count;
	int y_begin = sh * my_number;
	int y_end = y_begin + sh; 

	if (lseek(image[1], y_begin * width * 3 + begin_byte, SEEK_SET) < 0){
		printf("lseek fail\n");
		exit(1);
	}
	for(Y = y_begin; Y < y_end; Y++){
		for(X = 0; X < width; X++){
			if (Y == 0 || Y - 1 == 0 || Y + 1 == height - 1 || Y == height-1) S = 0;
	            else if (X == 0 || X == width - 3) S = 0;
	            	else {
						Sx = 0;
						Sy = 0;
						S = 0;	
						for (J = -1; J <= 1; J++){
							for (I = -1; I <= 1; I++){	    
							    piX = I + X;
							    piY = J + Y;

								char RGB[3];
							    if (lseek(image[0], piY * width * 3 + (piX * 3) + begin_byte, SEEK_SET) < 0){
							    	printf("lseek fail\n");
									exit(1);
							    }
							    if (read(image[0], RGB, 3) < 0){
							    	printf("read error!\n");
							    	exit(1);
							    }
 
				                N = (RGB[2] + RGB[1] + RGB[0]) / 3;
							    
							    Sx = Sx + N * Mx[I + 1][J + 1];
							    Sy = Sy + N * My[I + 1][J + 1];						    	
					    	}
						}
						S = sqrt(Sx * Sx + Sy * Sy);

						if (S > 255) S = 255;
						if (S < 0) S = 0;
					}

			char res[3];
			res[0] = res[1] = res[2] = (char)S; 

			if (write(image[1], res, 3) <= 0){
				printf("write error\n");
				exit(1);
			}
	     }
	}
	close(image[0]);
	close(image[1]);

	pthread_exit(NULL);
}



