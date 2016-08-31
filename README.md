# LZ78
DESCRIPTION: An LZ78 tree based compressor for textual files.

# Build
Make

# Usage
./lz78 -opt [argument]

Available Options:  
  	-c &emsp;&emsp;&emsp;&emsp;&emsp;COMPRESSION   	
	-d &emsp;&emsp;&emsp;&emsp;&emsp;DECOMPRESSION   
	-i [file] &emsp;&emsp;&emsp; INPUT FILE PATH  
	-o [file] &emsp;&emsp;&emsp;OUTPUT FILE PATH
	
# Examples  
COMPRESSION: ./lz78 -c -i [file] -o [file]  
DECOMPRESSION: ./lz78 -d -i [file] -o [file] 

