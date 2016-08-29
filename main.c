/*
	Implementing Huffman Compressor and Decompressor algorithm for textual characters with ASCII encoding on 8 bits!
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <inttypes.h>
#include "lz78.h"


/* Args to run the program */
struct args 
{

	uint8_t mode; 			// 0 Compression -- 1 Decompression
	char* inputfilename;	// Input file 
	char* outputfilename;	// Output file
};

/* Init the args structure */
void init_args(struct args* arguments) 
{	
	arguments -> mode = 2;
	arguments -> inputfilename = NULL;
	arguments -> outputfilename = NULL;
}

/* Print the help */
void help() 
{	
	printf("\nCorrect Usage:\n");
	printf("./lz78 -opt [args]\n\n");
	printf("Available Options:\n");
	printf("-c\t\tCOMPRESSION MODE\n");	
	printf("-d\t\tDECOMPRESSION MODE\n");
	printf("-i [file]\tINPUT FILE PATH\n");
	printf("-o [file]\tOUTPUT FILE PATH\n\n");
	printf("Examples:\n");
	printf("./lz78 -c -i [file] -o [file]\n");
	printf("./lz78 -d -i [file] -o [file]\n\n");

}

/*Print syntax error and exit the program */
void print_error_and_exit() 
{
	printf("./lz78: invalid syntax\n");
	help();
	exit(EXIT_FAILURE);
}

void check_argc_size(int argc) 
{
	if(argc != 6) 
		print_error_and_exit();
}

/*Verify filename existence */
int file_exists (const char * filename)
{
	struct stat filestat;
	return stat(filename, &filestat);
}

void check_syntax(struct args* arguments) 
{
	if (arguments -> inputfilename == NULL || arguments -> outputfilename == NULL || arguments -> mode == 2)
		print_error_and_exit();
}

int main(int argc, char* argv[]){

	int option = 0;

	struct args* arguments;
	arguments = (struct args*) calloc(1, sizeof(struct args));

	init_args(arguments);

	/*Argc size check */
	check_argc_size(argc);

	/*parsing arguments*/
	while((option = getopt(argc, argv, "cdi:o:")) != -1) 
	{	
		switch (option) 
		{

			/* Compression */
			case 'c': 
						arguments -> mode = 0;
						break;

			/* Decompression */				
			case 'd':
						arguments -> mode = 1;
						break;

			/* Input file path */	
			case 'i':	

						if( file_exists(optarg) != 0) 
						{	
							printf("./lz78: The input file does not exist\n");
							exit(EXIT_FAILURE);
						}
						arguments -> inputfilename = optarg;
						break;

			/* Output file path */				
			case 'o':	
						arguments -> outputfilename = optarg;
						break;


			default:	
						exit(EXIT_FAILURE);

		}
	}

		/*Syntax check */
		check_syntax(arguments);

		if(arguments -> mode == 0)
		{
			/*Starting lz78 Compression algorithm*/
			printf("\nSTARTING LZ78 COMPRESSION........\n");
			if(lz78_compressor(arguments -> inputfilename, arguments -> outputfilename) != 0)
				printf("./lz78: Error during compression\n\n");
			else
				printf("./lz78: Compression terminated\n\n");	
		}
		else
		{	
			/*Starting lz78 Decompression algorithm*/
			printf("\nSTARTING LZ78 DECOMPRESSION........\n");
			if(lz78_decompressor(arguments -> inputfilename, arguments -> outputfilename) != 0)
				printf("./lz78: Error during decompression\n\n");
			else
				printf("./lz78: Decompression terminated\n\n");			
		}

		free(arguments);
		return 0;
}
