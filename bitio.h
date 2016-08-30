#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>

struct bit_io;

/* 	=== bitio_write ===
- writes to the bit_io buffer ('data') size bits from the actual buffer 'data';
- returns SUCCESS (0) or FAILURE(-1).
	=== bitio_write ===
*/
int bitio_write(struct bit_io*, uint size, uint64_t data);

/* 	=== bitio_read ===
- reads from  bit_io buffer ('data') at most size bits & stores them into the actual buffer 'result';
- returns the  number of bits that have been read.
	=== bitio_read ===
*/
int bitio_read(struct bit_io*, uint max_size, uint64_t * result);

/* 	=== bitio_open ===
- allocates the bit_io data structure;
- opens the specified file in read (0) or write mode (1);
- initializes the fields inside the bit_io data structure;
- returns the bit_io pointer.
	=== bitio_open ===
*/
struct bit_io * bitio_open(const char * filename, uint mode);

/* 	=== bitio_close ===
- flush data to the file;
- close the file;
- free the memory;
- returns SUCCESS (0) or FAILURE(<0).
	=== bitio_close ===
*/
int bitio_close(struct bit_io*);
