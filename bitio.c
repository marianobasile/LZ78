#include "bitio.h"
#include <inttypes.h>

struct bit_io
{
	FILE *f;
	uint64_t data;
	uint wp;
	uint rp;
	uint mode;
};

struct bit_io * bitio_open(const char* filename, uint mode) 
{	
/* filename: name of the file to open, mode: 0 READ, 1 WRITE, return: bit_io object dynamically allocated */
	struct bit_io *b;

	if(filename == NULL || filename[0]=='\0' || mode > 1) 
	{
		errno = EINVAL;
		return NULL;
	}

	b = (struct bit_io *)calloc(1,sizeof(struct bit_io));

	if(b == NULL) 
	{
		errno = ENOMEM;
		return NULL; 
	}

	b->f = fopen(filename,(mode == 0?"r":"w"));

	if(b->f == NULL) 
	{
		free(b);
		return NULL;
	}

	b->mode = mode;
	/* wp,rp already initialized */
	return b;
}

int bitio_close (struct bit_io* b) 
{/*return: 0 SUCCESS, -1 FAILURE*/

	/*return value of the function */
	int ret = 0; 
	if(b == NULL) 
	{
		errno = EINVAL;
		ret = -1;
		return ret;
	}

	if(b->mode == 1 && b->wp != 0) 
	{
		if(fwrite(&b->data,1,((b->wp)+7)/8,b->f) != (b->wp+7)/8 )
			ret = -1;			
	}	
	
	fclose(b->f);
	bzero(b,sizeof(*b));
	free(b);
	return ret;
}

int bitio_write(struct bit_io* b, uint size, uint64_t data) 
{	
	/* available space in the buffer*/
	uint space; 
	if(b == NULL || b->mode != 1 || size > 64) 
	{
		errno = EINVAL;
		return -1;	
	}

	if (size == 0)
		return 0;

	space = 64 - b->wp;

	/*clear the higher part of the variable data */
	data &= ((1UL << size) -1); 

	/* there's enough space for the new block in the buffer */
	if(size <=  space) 
	{
		/*shift data of wp position & copy into b->data */
		b->data |= data << b->wp; 
		b->wp += size;
	} 
	else 
	{	
		//printf("space: %lu",(1UL<<space)-1);
		//printf("\ndata: %"PRIu64, ();
		//printf("\nb->data prima: %"PRIu64, data&((1UL<<space)-1));
		//(data&((1UL<<space)-1));
		//b->data |= (data&((1UL<<space)-1))<< b->wp; 
		//printf("\nb->data dopo: %"PRIu64, b->data);
		//printf("%"PRIu64"\n",b->data);	
		if(fwrite(&(b->data),1,8,b->f) <= 0 ) 
		{	
			errno = ENOSPC;
			return -1;	
		}

		/*fill the buffer b->data with the the bits which did not fit the first time */
		b->data = data >> space;	
		b->wp = size - space;		
	}

	return 0;
}

int bitio_read(struct bit_io* b, uint max_size, uint64_t * result) 
{
	uint space;
	int ret;
	if(b == NULL || b->mode != 0 || max_size > 64) 
	{
		errno = EINVAL;
		return -1;	
	}

	*result = 0;

	space = b->wp - b->rp;

	if(max_size == 0)
		return 0;

	if(max_size <= space) 
	{	
		/*first throw away the already read part, then put to 0 the bits after max_size */
		*result = (b->data >> b->rp) & ((1UL << max_size)-1); 
		b->rp += max_size;
		return max_size;
	}
	else
	{
		*result = (b->data >> b->rp);
		ret = fread(&b->data,1,8,b->f);
		if(ret <= 0) 
		{
			errno = ENODATA;
			return -1;
		}
		/* b->wp = # of elements successfully read * 8 since each element is 8 bits long */
		b->wp = ret * 8; 

		/* shift b->data to left by space position (the quantity already used to fill it) & copy in result */
	

		/* At this point we need to copy the remaining chunk of the data (max_size - space) */
		/* We may have read from the file less than (max_size - space) bits */
		if(b->wp >= max_size - space) 
		{	//printf("\nLeggo result: %"PRIu64, *result);
			*result^=*result;
			//printf("\ndopo result: %"PRIu64, *result);

			//printf("\ndopo 2 result: %"PRIu16, (uint16_t)b->data);
			*result |= b->data << space;
			//printf("\ndopo 2 result after: %"PRIu64, *result);
			/*need to clear extra bits copied in the previous operation (max_size)*/
			*result &= ((1UL << max_size) - 1); 
			//printf("\ndopo 3 result: %"PRIu64, *result);

			b->rp = max_size - space;
			return max_size;
		}

		*result |= b->data << space;
		/*need to clear extra bits copied in the previous operation (space + b->wp)*/
		*result &= ((1UL << (b->wp + space)) - 1); 
		b->rp = b->wp; 
		return b->wp + space;			
	}
}
