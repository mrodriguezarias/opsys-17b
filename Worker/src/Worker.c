#include "funcionesWorker.h"
int main2(void);

int main(int argc, char* argv[]) {
	process_init();
	//data_open(config_get("RUTA_DATABIN"),mstring_toint(config_get("DATABIN_SIZE")));
	listen_to_master();
	data_close();
	return EXIT_SUCCESS;
}





int main2(void) {

#define SIZE 1024

	int pipe_padreAHijo[2];
	int pipe_hijoAPadre[2];

	pipe(pipe_padreAHijo);
	pipe(pipe_hijoAPadre);
	pid_t pid;
	int status;
	char* buffer = malloc(SIZE);

	if ((pid = fork()) == 0) {
		char * script = "/home/utnso/yama-test1/transformador.sh";
		dup2(LECTURA_PADRE, STDIN_FILENO);
		dup2(ESCRITURA_HIJO, STDOUT_FILENO);
		close(ESCRITURA_PADRE);
		close(LECTURA_HIJO);
		close(ESCRITURA_HIJO);
		close(LECTURA_PADRE);
		char *argv[] = {NULL};
		char *envp[] = {NULL};

		execve(script, argv, envp);
		exit(1);
	} else {
		close( LECTURA_PADRE ); //Lado de lectura de lo que el padre le pasa al hijo.
		close( ESCRITURA_HIJO );//Lado de escritura de lo que hijo le pasa al padre.
		FILE*file = fopen("/home/utnso/yama-test1/WBAN.csv","r");
		char buffer2[2048] ;
		fread(buffer2,sizeof(buffer2),1,file);
		write( ESCRITURA_PADRE,buffer2,sizeof(buffer2));
		close( ESCRITURA_PADRE);
		waitpid(pid,&status,0);
		read( LECTURA_HIJO, buffer, SIZE );
		close( LECTURA_HIJO);
	}

	FILE* fd = fopen("/home/utnso/yatpos/tmp/Master1", "w");
	fputs(buffer, fd);
	fclose(fd);
	free(buffer);
	return 0;
}

