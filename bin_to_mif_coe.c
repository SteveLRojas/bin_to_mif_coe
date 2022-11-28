#include <stdio.h> 
#include <stdlib.h> 
#include <stdint.h>
#include <string.h>

static char MIF_STRING[] = ".mif";
static char COE_STRING[] = ".coe";

void replace_file_extension(char* new_ext, char* out_name, char* new_name)
{
	uint8_t count = 0;
	uint8_t count_copy;
	//copy string and get its length
	do
	{
		new_name[count] = out_name[count];
		++count;
	}while(out_name[count]);
	count_copy = count;
	//search for a '.' starting from the end
	while(count)
	{
		count = count - 1;
		if(new_name[count] == '.')	//replace it and what follows with .mif
		{
			for(unsigned int d = 0; d < 5; ++d)
			{
				new_name[count] = new_ext[d];
				count = count + 1;
			}
			break;
		}
	}
	//check if the file name had no '.'
	if(count == 0)
	{
		count = count_copy;
		for(unsigned int d = 0; d < 5; ++d)
		{
			new_name[count] = new_ext[d];
			count = count + 1;
		}
	}
}

void write_mif(char* out_name, uint8_t* out_data, unsigned long prg_size, unsigned int word_size_bytes)
{
	char new_name[32];
	replace_file_extension(MIF_STRING, out_name, new_name);

	FILE* mif_file = fopen(new_name, "w");
	if(mif_file == NULL)
	{
		printf("Error creating mif file!");
		exit(1);
	}
	
	unsigned int num_words = prg_size / word_size_bytes;
	unsigned int word_size_bits = word_size_bytes * 8;
	
	// HEADER
	fprintf(mif_file, "DEPTH = %u;\n", num_words);
	fprintf(mif_file, "WIDTH = %u;\n", word_size_bits);
	fprintf(mif_file, "ADDRESS_RADIX = HEX;\n");
	fprintf(mif_file, "DATA_RADIX = HEX;\n");
	fprintf(mif_file, "CONTENT\nBEGIN\n");
	
	// DATA
	for(unsigned long current_word = 0; current_word < num_words; ++current_word)
	{
		unsigned long address = (current_word * word_size_bytes);
		
		fprintf(mif_file, "%lX : ", current_word);
		for(int current_byte = (word_size_bytes - 1); current_byte >= 0; --current_byte)
		{
			fprintf(mif_file,"%02X", out_data[address + (unsigned long)current_byte]); 
		}
		fprintf(mif_file, ";\n");
	}

	// FOOTER 
	fprintf(mif_file, "END;\n");
	fclose(mif_file);
}

void write_coe(char* out_name, uint8_t* out_data, unsigned long prg_size, unsigned int word_size_bytes)
{
	char new_name[32];
	replace_file_extension(COE_STRING, out_name, new_name);

	FILE* coe_file = fopen(new_name, "w");
	if(coe_file == NULL)
	{
		printf("Error creating coe file!");
		exit(1);
	}
	
	unsigned int num_words = prg_size / word_size_bytes;
	
	// HEADER
	fprintf(coe_file, "memory_initialization_radix=16;\n");
	fprintf(coe_file, "memory_initialization_vector=\n");
	
	// DATA
	for(unsigned long current_word = 0; current_word < num_words; ++current_word)
	{
		unsigned long address = (current_word * word_size_bytes);
		
		for(int current_byte = (word_size_bytes - 1); current_byte >= 0; --current_byte)
		{
			fprintf(coe_file,"%02X", out_data[address + (unsigned long)current_byte]); 
		}
		
		if ((current_word + 1) != num_words)
		{
			fprintf(coe_file, ",\n");
		}
		else 
		{
			// Only for last entry
			fprintf(coe_file, ";\n");
		}
	}

	fclose(coe_file);
}

int main(int argc, char** argv)
{
	//Assert proper number of program arguments
	if(argc < 2)
	{
		fprintf(stderr, "Specify input file!\n");
		//exit(1);
	}
	if(argc < 3)
	{
		fprintf(stderr, "Specify output file!\n");
		//exit(1);
	}
	if(argc < 4)
	{
		fprintf(stderr, "Specify word size in units of bytes!\n");
		exit(1);
	}

	FILE* fp_in = fopen(argv[1], "rb");
	if(fp_in == NULL)
	{
		printf("Failed to open input file: %s\n", argv[1]);
		exit(1);
	}
	
	// Open file
	fseek(fp_in, 0L, SEEK_END);
	int rom_len = ftell(fp_in);
	fseek(fp_in, 0L, SEEK_SET);

	// Read file
	uint8_t* rom_data = (uint8_t*)malloc(rom_len);
	size_t num_read = fread(rom_data, sizeof(uint8_t), rom_len, fp_in);
	if(num_read != rom_len)
	{
		printf("Failed to load input file properly, exiting...\n");
		exit(1);
	}
	fclose(fp_in);
	
	// Create new files and write to them
	char* word_size_arg = argv[3];
	int word_size_bytes = atoi(word_size_arg);
	if(word_size_bytes == 0)
	{
		printf("Failed to parse word size argument (unit bytes), [ %s ] , exiting...\n", word_size_arg);
		exit(1);	
	}
	else
	{
		write_mif(argv[2], rom_data, rom_len, word_size_bytes);
		write_coe(argv[2], rom_data, rom_len, word_size_bytes);
	}
		
	free(rom_data);
	return 0;
}
