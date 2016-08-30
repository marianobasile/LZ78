#include "bitio.h"

#define DICTIONARY_SIZE		65536		//Dictionary size (64K entries)
#define ROOT 				0			//ROOT node id			
#define	READ				0			//Open file in read mode  (bit_io open)
#define WRITE				1			//Open file in write mode (bit_io open)
#define COLLISION_LIST		2 			//Insert node in COLLISION LIST
#define HASH_ENTRY  		3			//Insert node as ENTRY in the HASH TABLE


struct decompressor_entry;

struct hash_entry;



/* 	=== build_hash_table ===
-	To create the dictionary as an hash table (array of hash entries).
-	Hash table entries are of type hash_entry.
	=== build_hash_table ===
*/
void build_hash_table();



/* 	=== compute_hash ===
-	To compute the index in the array in which lookup.
	Returns the index.
	=== compute_hash ===
*/
uint16_t compute_hash(uint16_t parent, uint8_t arc_label);



/* 	=== insert_in_collision_list ===
-	Collision occurs. Insertion of a new node in the collision list of a given (by compute_hash) hash_entry.
	=== insert_in_collision_list ===
*/
void insert_in_collision_list(struct hash_entry* hash_entry, uint16_t parent);



/* 	=== hash_insertion ===
-	NO Collision occurs. Insertion of a new node in a given (by compute_hash) hash_entry.
	=== hash_insertion ===
*/
void hash_insertion(uint16_t position, uint16_t parent, uint16_t node_id);



/* 	=== generate_root_childs ===
-	To Fill the dictionary (root node) by adding all childs between 1 and 256 whose labels go from 0x00 to 0xff.
	=== generate_root_childs ===
*/
void generate_root_childs();



/* 	=== print_hash_table ===
-	To print the dictionary.
	=== print_hash_table ===
*/
void print_hash_table();



/* 	=== hash_is_not_full ===
-	To check if the dictionary is full or not.
	=== hash_is_not_full ===
*/
int hash_is_not_full();



/* 	=== collision_list_destroy ===
-	Destroy collision list for each hash_entry of the dictionary.
	Returns 0 if not full, 1 otherwise.
	=== collision_list_destroy ===
*/
void collision_list_destroy();



/* 	=== hash_init ===
-	Each hash table entry is initiliaze as the following:
	index:	0  
	next:	NULL
	=== hash_init ===
*/
void hash_init();



/* 	=== rebuild_hash_table ===
-	Dictionary is full:
	1)Call collision_list_destroy()
	2)Call hash_init()
	3)Call generate_root_childs()
	=== rebuild_hash_table ===
*/
void rebuild_hash_table();



/* 	=== check_for_free_entry_and_add ===
-	Insert a new node in the dictionary if possible otherwise call eebuild_hash_table().
	Returns the parent node id to emit.
	=== check_for_free_entry_and_add ===
*/
uint16_t check_for_free_entry_and_add(uint16_t position, uint16_t parent, uint64_t insertion_place);



/* 	=== lookup_collision_list ===
-	Collision occurs. Lookup node id in the collision list.
	If the node is present return the child node id, otherwise calls check_for_free_entry_and_add() and returns the parent node id. 
	=== lookup_collision_list ===
*/
uint16_t lookup_collision_list(uint16_t position, uint16_t parent);



/* 	=== hash_lookup ===
-	Lookup for a node in the dictionary.
	If not found calls check_for_free_entry_and_add(). 
	If found and no collision occurs return the child node id.
	Otherwise calls lookup_collision_list().
	Returns the parent node id in the first case, otherwise the value returned by lookup_collision_list().
	=== hash_lookup ===
*/
uint16_t hash_lookup(uint16_t parent, uint8_t arc_label);



/* 	=== hash_table_destroy ===
-	Calls collision_list_destroy().
	Free all hash table entries.
	=== hash_table_destroy ===
*/
void hash_table_destroy();



/*	=== lz78_compressor ===
- core function that implements the lz78 compression algorithm!
  Returns 0 for success, -1 otherwise.	
	=== lz78_compressor ===
*/
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
