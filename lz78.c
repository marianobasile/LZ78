#include "lz78.h"
#include <inttypes.h>

struct hash_entry {
	uint16_t parent;			//Index of the parent node
	uint16_t index;				//Node index
	struct hash_entry* next;	//List of entries (because collisions)
};

struct hash_entry *hash_table[DICTIONARY_SIZE];

const uint16_t ROOT_ID = 0;
uint16_t parent_node;
uint16_t previous_parent_node;
uint32_t node_id;

/*the global counter*/
uint32_t counter;

void hash_init() 
{
	int i;

	for(i=0; i < DICTIONARY_SIZE; i++)
	{
		hash_table[i] -> index = 0;
		hash_table[i] -> next = NULL;
	}

}

uint16_t compute_hash(uint16_t parent, uint8_t arc_label) 
{
	return ((((parent << 8) | arc_label)+1) % DICTIONARY_SIZE);
}

void insert_in_collision_list(struct hash_entry* hash_entry, uint16_t parent)  //collision occurs
{
	while(hash_entry -> next != NULL)
		hash_entry = hash_entry -> next;

	hash_entry -> next = (struct hash_entry*) calloc(1,sizeof(struct hash_entry));

	hash_entry -> next -> parent = parent;
	hash_entry -> next -> index = node_id;
	hash_entry -> next -> next = NULL;

	node_id++;
}

void hash_insertion(uint16_t position, uint16_t parent, uint16_t node_id)
{
		//printf("%"PRIu16"\n",node_id);
		hash_table[position] -> parent = parent;
		hash_table[position] -> index = node_id;
}

void generate_root_childs() 
{
	uint8_t arc_label = 0x00;
	uint16_t position;
	node_id = 1;					//node_id = 0 = ROOT_ID 				
	
	hash_insertion(0,0,0);			//insert the ROOT

	while(arc_label <= 0xFF) 
	{	
		position = compute_hash(ROOT_ID,arc_label);

		hash_insertion(position,ROOT_ID,node_id);
		
		node_id++;

		if(node_id == 257)		
			break;			//otherwise overflow arc_label 
		else
			arc_label++;
	}

	//printf("%"PRIu16"\n",node_id);
	//printf("%"PRIu8"\n",arc_label);
}

void build_hash_table() 
{
	int i;

	for(i=0; i < DICTIONARY_SIZE; i++)
		hash_table[i] = (struct hash_entry*) calloc(1,sizeof(struct hash_entry));

	hash_init();
	
}

void print_hash_table() 
{

	int i;

	for(i=0; i < 300; i++)
	{
		printf("\nENTRY:%d",i);

		printf("\tPARENT: ");
		printf("%"PRIu16,hash_table[i]->parent);

		printf("\tINDEX: ");
		printf("%"PRIu16,hash_table[i]->index);

		if(hash_table[i]->next == NULL) 
			printf("\tNEXT: VUOTO");	
	}
	printf("\n");
}

int hash_is_not_full() 
{

	return (node_id <= DICTIONARY_SIZE -1) ? 0 : 1;
}

void collision_list_destroy() 
{
	int i;
	struct hash_entry* temp;
	struct hash_entry* app;

	for(i=0; i < DICTIONARY_SIZE; i++)
	{
		temp = hash_table[i];
		app = hash_table[i] -> next;
		hash_table[i] -> next = NULL;

		while(app != NULL)
		{
			temp = app;
			app = app -> next;
			free(temp);
		}

	}
}	

void rebuild_hash_table() 
{
	collision_list_destroy();
	hash_init();
	//print_hash_table();
	generate_root_childs();
	//print_hash_table();
}
void verify_outside_range() {
	printf("\nVerifica outside range in corso....");
	int i,j;
	j=0;
	for(i=0; i < DICTIONARY_SIZE; i++)
	{
		if(hash_table[i] -> index >= DICTIONARY_SIZE){
			j=1;
			printf("ERRORE!!");
		}
	}

	if(j==0)
		printf("TUTTO OK!!\n");
}

uint16_t check_for_free_entry_and_add(uint16_t position, uint16_t parent, uint64_t insertion_place) 
{
	if(hash_is_not_full() == 0 && insertion_place == HASH_ENTRY)
	{	
		hash_insertion(position,parent,node_id);
		node_id++;
	}
	else if(hash_is_not_full() == 0 && insertion_place == COLLISION_LIST) 
		insert_in_collision_list(hash_table[position],parent);		
	else
	{
		//verify_outside_range();
		node_id = 0;
		rebuild_hash_table();
	}
		

	return parent;
}



uint16_t lookup_collision_list(uint16_t position, uint16_t parent) 
{	
	uint16_t child_id = 0;
	struct hash_entry * hash_entry = hash_table[position];

	while(hash_entry -> next != NULL)
	{
		if(hash_entry -> parent == parent) 
		{
			child_id = hash_entry -> index;
			break;
		}

		hash_entry = hash_entry -> next;
	}

	if(child_id == 0)
		return check_for_free_entry_and_add(position, parent, COLLISION_LIST);		//insert new symbhol in the collision list of entry pointed by position	

	return child_id;
}




uint16_t hash_lookup(uint16_t parent, uint8_t arc_label)
{
	uint16_t position = compute_hash(parent,arc_label);

	if(hash_table[position] -> index != 0)		
	{
		if(hash_table[position] -> parent == parent)
			return hash_table[position] -> index;

		return lookup_collision_list(position,parent);
	}
	
	return check_for_free_entry_and_add(position,parent,HASH_ENTRY);
}

void hash_table_destroy()
{	
	int i;
	
	collision_list_destroy();

	for(i=0; i < DICTIONARY_SIZE; i++){
		free(hash_table[i]);
	}
}


int lz78_compressor(const char* inputfilename, const char* outputfilename) {

	struct bit_io* bitio_inputfile;
	struct bit_io* bitio_outputfile;
	uint64_t buff = 0;
	uint8_t arc_label = 0;

	build_hash_table();
	generate_root_childs();

	//print_hash_table();
	
	bitio_inputfile = bitio_open(inputfilename,READ);
	if(bitio_inputfile == NULL)
		return BITIO_OPEN_ERROR;

	bitio_outputfile = bitio_open(outputfilename,WRITE);
	if(bitio_outputfile == NULL)
		return BITIO_OPEN_ERROR;

	parent_node = ROOT_ID;
	previous_parent_node = ROOT_ID;

	while(bitio_read(bitio_inputfile,8,&buff) != -1) 
	{			
				//printf("Letto:");
				//printf("%"PRIu64"\n",buff);
				arc_label = (uint8_t)buff; 

				parent_node = hash_lookup(parent_node, arc_label);
		CHECK:
				if(parent_node != previous_parent_node)										//child has been found
					previous_parent_node = parent_node;
				else																		//	new child has been added so we emit
				{	
					//printf("Scritto:");
					//printf("%"PRIu64"\n",(uint64_t)parent_node);
					if(parent_node >= DICTIONARY_SIZE)
						printf("WARNING!!!");	
					//printf("Scritto:");
					//printf("%"PRIu16"\n",parent_node);
					bitio_write(bitio_outputfile,16,(uint64_t)parent_node);
					parent_node = ROOT_ID;
					previous_parent_node = ROOT_ID;
					//printf("Letto char:");
					//printf("%"PRIu8"\n",arc_label);
					parent_node = hash_lookup(parent_node, arc_label);
					goto CHECK;
				}	 
	}

	//print_hash_table();
	const uint64_t END_OF_FILE = 0;
	bitio_write(bitio_outputfile,16, END_OF_FILE);
	//printf("%"PRIu64"\n",END_OF_FILE);

	if(bitio_close(bitio_inputfile) == -1)
		return BITIO_CLOSE_ERROR;

	if(bitio_close(bitio_outputfile) == -1)
		return BITIO_CLOSE_ERROR;

	hash_table_destroy();

	return 0;
}

/*
	======================================================================================================
												DECOMPRESSOR
	======================================================================================================
*/

/*the dictionary is an array of decompresso_entry implementing the encoding tree*/
struct decompressor_entry
{
	uint16_t father;
	uint8_t symbol;
};



void init_decompressor_dictionary(struct decompressor_entry * dictionary)
{
	int i;
	/*initializing the dictionary with the alphabet: the 256 ASCII symbols*/
	dictionary[0].symbol = ROOT;												/* Node 0 --> ROOT & end the decoding*/
	dictionary[0].father = ROOT;
	
	for(i=1; i<257; i++)
	{		
		dictionary[i].symbol = i-1;
		dictionary[i].father = ROOT;	
	}

	counter=257;
}



struct decompressor_entry * create_decompressor_dictionary()
{
	struct decompressor_entry *dictionary;
	
	dictionary = (struct decompressor_entry *)calloc(DICTIONARY_SIZE, sizeof(struct decompressor_entry));
	if(dictionary == NULL) 
	{
		errno = ENOMEM;
		return NULL; 
	}
	
	init_decompressor_dictionary(dictionary);
	
	return dictionary;
}



int emit_decoding(struct bit_io* output, struct decompressor_entry* dictionary, uint16_t node, char *decode_buffer){

	int i, ret;
	int length=0;
	uint64_t buffer;
	
	struct decompressor_entry* app;
	app = &dictionary[node];
			
	do{
		decode_buffer[length++] = app->symbol;
		app = &dictionary[app->father];
	}		
	while(app->symbol != ROOT);
	
	for(i=length-1; i>=0; i--)
	{
		buffer = (uint64_t)decode_buffer[i];

		ret = bitio_write(output, 8, buffer);
		if(ret != 0)
		{
			printf( "Error bitio_write: %s\n", strerror( errno ) );
			return -1;
		}

		//printf( "\nLunghezza: %d - Scrivo: %c\n", i+1, (char)buffer );												/*_______________________controllo scrittura decodifica*/
	}
	
	return 0;
}



uint16_t get_root_child_symbol(struct decompressor_entry* dictionary, uint16_t node)
{
	struct decompressor_entry* app;
	app = &dictionary[node];
	
	while(app->father != ROOT)
		app = &dictionary[app->father];
		
	return app->symbol;
}



int handle_first_citionary_access(struct bit_io * input, struct bit_io * output, uint64_t * buffer, uint32_t * node,  struct decompressor_entry *dictionary, char * decode_buffer){
	
	int ret;
	
	ret = bitio_read(input,16, buffer);
	if(ret < 0)
	{
		printf( "Error bitio_read: %s\n", strerror( errno ) );
		return -1;
	}
	*node = (uint32_t)*buffer;

	emit_decoding(output, dictionary, *node, decode_buffer);
	
	return 0;
}


int lz78_decompressor(const char* inputfilename, const char* outputfilename) 
{
	int ret;
	uint32_t previous_node;			/*the previous dictionary entry read from the compressed file*/
	uint32_t current_node;			/*the current dictionary entry read from the compressed file*/
	uint64_t buffer;				/*to store the bitio_read data*/
	
	char * decode_buffer;			/*buffer to decode the symbols encoded in the dictionary entry*/
	
	decode_buffer = (char *)calloc(DICTIONARY_SIZE, sizeof(*decode_buffer));

	if(decode_buffer == NULL) 
	{
		errno = ENOMEM;
		printf( "Error allocating decoding buffer: %s\n", strerror( errno ) );
		return -1; 
	}
	
	/*dictionary creation*/
	struct decompressor_entry *dictionary;
		
	dictionary = create_decompressor_dictionary();
	
	if(dictionary == NULL)
	{
		printf( "Error creating the dictionary: %s\n", strerror( errno ) );
		return -1;	
	}
	
	/*files opening*/
	struct bit_io *input;
	struct bit_io *output;
	
	input = bitio_open(inputfilename, READ);
	if(input == NULL)
	{
		printf( "Error opening %s: %s\n", inputfilename, strerror( errno ) );
		return -1;
	}
	
	output = bitio_open(outputfilename, WRITE);
	if(output == NULL)
	{
		printf( "Error opening %s: %s\n", outputfilename, strerror( errno ) );
		return -1;
	}

	/*handling first dictionary access*/
	ret = handle_first_citionary_access(input, output, &buffer, &current_node, dictionary, decode_buffer);
	if(ret == -1)
		return ret;
	
	/*reading the next node*/		
	previous_node = current_node;
	
	ret = bitio_read(input, 16, &buffer);
	if(ret < 0)
	{
		printf( "Error bitio_read: %s\n", strerror( errno ) );
		return -1;
	}
	current_node = (uint32_t)buffer;

	/*from the second index in the compressed file*/
	while(current_node != ROOT)	
	{
		dictionary[counter].father = previous_node;
		dictionary[counter].symbol = get_root_child_symbol(dictionary, current_node);

		/*emitting the decoding*/
		ret = emit_decoding(output, dictionary, current_node, decode_buffer);
		if(ret != 0)
		{
			printf( "Error decoding : %s\n", strerror( errno ) );
			return -1;
		}
		
		counter++;

		/*checking if dictionary is full*/
		if(counter >= DICTIONARY_SIZE)
		{
			init_decompressor_dictionary(dictionary);
						
			ret = handle_first_citionary_access(input, output, &buffer, &current_node, dictionary, decode_buffer);
			if(ret == -1)
				return ret;
		}

		previous_node = current_node;

		/*reading next node*/
		buffer = 0;
		ret = bitio_read(input, 16, &buffer);

		if(ret < 0)
		{
			printf( "\nError bitio_read: %s\n", strerror( errno ) );
			return -1;
		}
		current_node = (uint32_t)buffer;

	}
	

	/*closing bit_io structures*/
	ret = bitio_close(input);
	if(ret != 0)
	{
		printf( "Error closing %s: %s\n", inputfilename, strerror( errno ) );
		return -1;
	}
	
	ret = bitio_close(output);
	if(ret != 0)
	{
		printf( "Error closing %s: %s\n", outputfilename, strerror( errno ) );
		return -1;
	}
	
	/*deallocating memory*/	
	bzero(decode_buffer, DICTIONARY_SIZE*sizeof(*decode_buffer));
	free(decode_buffer);
	
	bzero(dictionary, DICTIONARY_SIZE*sizeof(*dictionary));
	free(dictionary);
	
	return 0;
}
