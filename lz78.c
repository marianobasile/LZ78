#include "lz78.h"
#include <inttypes.h>

struct hash_entry 
{
	uint16_t parent;			//Index of the parent node
	uint16_t index;				//Node index
	struct hash_entry* next;	//List of entries (because collisions)
};

/*
	======================================================================================================
						COMPRESSOR
	======================================================================================================
*/

struct hash_entry {
	uint16_t parent;							//Index of the parent node
	uint16_t index;								//Node index
	struct hash_entry* next;						//List of entries (because collisions)
};

struct hash_entry *hash_table[DICTIONARY_SIZE];					//DICTIONARY			
uint16_t parent_node;								//Node reached in the tree
uint16_t previous_parent_node;							//Previous node reached in the tree
uint32_t node_id;								//Global counter compressor side
uint32_t counter;								//Global counter decompressor side
	
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

void insert_in_collision_list(struct hash_entry* hash_entry, uint16_t parent)  
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
	hash_table[position] -> parent = parent;
	hash_table[position] -> index = node_id;
}

void generate_root_childs() 
{
	uint8_t arc_label = 0x00;
	uint16_t position;
	node_id = 1;					//node_id = 0 = ROOT 				
	
	hash_insertion(0,ROOT,ROOT);	//insert the ROOT

	while(arc_label <= 0xFF) 
	{	
		position = compute_hash(ROOT,arc_label);

		hash_insertion(position,ROOT,node_id);
		
		node_id++;

		//otherwise overflow arc_label
		if(node_id == 257)		
			break;			 
		else
			arc_label++;
	}
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

	for(i=0; i < DICTIONARY_SIZE; i++)
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
	generate_root_childs();
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
		return check_for_free_entry_and_add(position, parent, COLLISION_LIST);		//insert new symbhol in the collision list

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


int lz78_compressor(const char* inputfilename, const char* outputfilename) 
{

	struct bit_io* bitio_inputfile;
	struct bit_io* bitio_outputfile;
	uint64_t buff = 0;
	uint8_t arc_label = 0;
	const uint64_t END_OF_FILE = 0;

	build_hash_table();
	generate_root_childs();

	
	bitio_inputfile = bitio_open(inputfilename,READ);
	if(bitio_inputfile == NULL)
	{
		printf( "Error opening %s: %s\n", inputfilename, strerror( errno ) );
		return -1;
	}

	bitio_outputfile = bitio_open(outputfilename,WRITE);
	if(bitio_outputfile == NULL)
	{
		printf( "Error opening %s: %s\n", outputfilename, strerror( errno ) );
		return -1;
	}

	parent_node = ROOT;
	previous_parent_node = ROOT;

	while(bitio_read(bitio_inputfile,8,&buff) != -1) 
	{			
			arc_label = (uint8_t)buff; 
			parent_node = hash_lookup(parent_node, arc_label);
	CHECK:
			//child has been found
			if(parent_node != previous_parent_node)										
				previous_parent_node = parent_node;
			
			//New child added so emits
			else																		
			{	
				bitio_write(bitio_outputfile,16,(uint64_t)parent_node);
				parent_node = ROOT;
				previous_parent_node = ROOT;
				parent_node = hash_lookup(parent_node, arc_label);
				goto CHECK;
			}	 
	}

	
	if(bitio_write(bitio_outputfile,16, END_OF_FILE) == -1)
	{
		printf( "Error bitio_write: %s\n", strerror( errno ) );
		return -1;
	}

	if(bitio_close(bitio_inputfile) == -1)
	{
		printf( "Error closing %s: %s\n", inputfilename, strerror( errno ) );
		return -1;
	}

	if(bitio_close(bitio_outputfile) == -1)
	{
		printf( "Error closing %s: %s\n", outputfilename, strerror( errno ) );
		return -1;
	}

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

	dictionary[0].symbol = ROOT;				/* Node 0 used to encode ROOT == EOF*/
	dictionary[0].father = ROOT;
	
	/*initializing the dictionary with the alphabet: the 256 ASCII symbols*/	
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
	uint16_t length=0;
	uint64_t buffer;
	
	struct decompressor_entry* app;
	app = &dictionary[node];
	
	/*creating the decoded sequence*/		
	do{
		decode_buffer[length++] = app->symbol;
		app = &dictionary[app->father];
	}		
	while(app->symbol != ROOT);
	
	/*emitting the decoded sequence in the reverse order*/
	for(i=length-1; i>=0; i--)
	{
		buffer = (uint64_t)decode_buffer[i];

		ret = bitio_write(output, 8, buffer);
		if(ret != 0)
		{
			printf( "Error bitio_write: %s\n", strerror( errno ) );
			return -1;
		}
	}
	
	return 0;
}



uint16_t get_root_child_symbol(struct decompressor_entry* dictionary, uint16_t node)
{
	struct decompressor_entry* app;
	app = &dictionary[node];
	
	/*searching the root child of a decoded sequence*/
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
	uint32_t previous_node;						/*the previous dictionary entry read from the compressed file*/
	uint32_t current_node;						/*the current dictionary entry read from the compressed file*/
	uint64_t buffer;						/*to store the bitio_read data*/
	
	char * decode_buffer;						/*buffer to decode the symbols encoded in the dictionary entry*/	
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
			
	previous_node = current_node;
	
	/*reading the next node*/
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
		/*adding new node (entry in the dictionary) to the previous node*/
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
