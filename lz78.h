#include "bitio.h"

#define DICTIONARY_SIZE		65536		//Dictionary size (64K entries)
#define	READ				0			//Open file in read mode  (bit_io open)
#define WRITE				1			//Open file in write mode (bit_io open)
#define COLLISION_LIST		2
#define HASH_ENTRY  		3
#define ROOT 0

#define READ_FILE_ERROR		-1			//No such input file
#define WRITE_FILE_ERROR	-2			
#define BITIO_OPEN_ERROR	-3			//Error in executing bit_io open
#define BITIO_CLOSE_ERROR	-4			//Error in executing bit_io open


struct decompressor_entry;


/* 	=== lz78_compressor ===
- 
-
	=== lz78_compressor ===


*/

struct hash_entry;

void build_hash_table();

uint16_t compute_hash(uint16_t parent, uint8_t arc_label);

void insert_in_collision_list(struct hash_entry* hash_entry, uint16_t parent);

void hash_insertion(uint16_t position, uint16_t parent, uint16_t node_id);

void generate_root_childs();

void build_hash_table();

void print_hash_table();

int hash_is_not_full();

void collision_list_destroy();

void rebuild_hash_table();

uint16_t check_for_free_entry_and_add(uint16_t position, uint16_t parent, uint64_t insertion_place);

uint16_t lookup_collision_list(uint16_t position, uint16_t parent);

uint16_t hash_lookup(uint16_t parent, uint8_t arc_label);

void hash_table_destroy();

void hash_init();

/*	COMPRESSION MODE: 
	Dictionary: HASH TABLE 
	Hash entry: Entry of the HASH TABLE */

int lz78_compressor(const char* inputfilename, const char* outputfilename);



/* 	=== lz78_decompressor ===
- core function that implements the lz78 decompression algorithm! 
	=== lz78_decompressor ===
*/
int lz78_decompressor(const char* inputfilename, const char* outputfilename);



/* 	=== init_decompressor_dictionary ===
- utility function to initialize the decompression tree (or dictionary):
- initialize the first 256 node with the ASCII characters and the father to the root fo all the nodes!
	=== init_decompressor_dictionary ===
*/
void init_decompressor_dictionary(struct decompressor_entry * dictionary);



/* 	=== create_decompressor_dictionary ===
- utility function to allocate the memory for the decompression tree (or dictionary): 
- allocating an array of entries implementing the tree!
	=== create_decompressor_dictionary ===
*/
struct decompressor_entry * create_decompressor_dictionary();



/* 	=== emit_decoding ===
-  utility function to emit the sequence encoded in a node:
- raising the tree from the node to the root, storing the symbol associated to each visited node, and emitting in the reverse order!
	=== emit_decoding ===
*/
int emit_decoding(struct bit_io* output, struct decompressor_entry* dictionary, uint16_t node, char* decode_buffer);



/* 	=== ehandle_first_citionary_access===
-  utility function to read first node emit it and update to the next access to the dicitionary: when a new node has to been added
	=== emit_decoding ===
*/
int handle_first_citionary_access(struct bit_io * input, struct bit_io * output, uint64_t * buffer, uint32_t * node,  struct decompressor_entry *dictionary, char * decode_buffer);



/* 	=== get_root_child_symbol ===
- utility function to get the first symbol in an encoded sequence to add to the new node in the tree:
- raising the tree from the node to emit until the root!
	=== get_root_child_symbol ===
*/
uint16_t get_root_child_symbol(struct decompressor_entry* dictionary, uint16_t node);
