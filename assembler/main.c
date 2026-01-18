/*
* 4-bit CPU Assembler - Joona Rahm 2026
* 
* Assembles my custom assembly language into 
* machine code for my custom 4-bit CPU
*
* Input: input.ass (assembly source file)
* Output: output.txt (txt file with machine code)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define ROM_SIZE            128
#define RAM_START           0x80
#define RAM_SIZE            16
#define IO_START            0xC0

#define MAX_LABELS          32
#define MAX_CONSTANTS       64
#define MAX_LINE_LENGTH     256

#define NUM_INSTRUCTIONS    16
#define OPCODE_BITS         4

#define COMMENT_CHAR        ';'
#define INPUT_FILE          "input.ass"     // Input assembly source file, can be changed as needed
#define OUTPUT_FILE         "output.txt"    // Output machine code file, can be changed as needed
#define PRINT_ENABLED       1               // Enable (1) or disable (0) printing debug information



/* ===============================================
 * ==================== TYPES ====================
 * =============================================== */

// Instruction opcodes
typedef enum {
    OP_NOP      = 0x0,
    OP_LDI      = 0x1,
    OP_ACC2R0   = 0x2,
    OP_ADDI     = 0x3,
    OP_STA      = 0x4,
    OP_LDA      = 0x5,
    OP_SUBI     = 0x6,
    OP_JMP      = 0x7,
    OP_JZ       = 0x8,
    OP_R02ACC   = 0x9,
    OP_CMPI     = 0xA,
    OP_CALL     = 0xB,
    OP_RET      = 0xC,
    OP_LDMAR    = 0xD,
    OP_STA_MAR  = 0xE,
    OP_LDA_MAR  = 0xF
} Opcode;


// Information about each instruction in the ISA
typedef struct {
    char *mnemonic;         // Assembly mnemonic
    int num_words;          // Total words (nibbles) used by the instruction 
    unsigned char opcode;   // Opcode (4 bits)
} InstructionInfo;

// Stores label information (e.g. "LOOP:" = address 0x10)
typedef struct {
    char *name;             // Label name without the colon
    int address;            // Address in ROM the label points to
} LabelInfo;


// Used to define constants (e.g. "MOL EQU 0x2A")
typedef struct {
    char *name;             // Constant name
    int value;              // Constant value
} Constant;



/* ===============================================
 * =================== GLOBALS ===================
 * =============================================== */

InstructionInfo instruction_table[] = {
    {"NOP",     1,  OP_NOP},
    {"LDI",     2,  OP_LDI},
    {"ACC2R0",  1,  OP_ACC2R0},
    {"ADDI",    2,  OP_ADDI},
    {"STA",     3,  OP_STA},
    {"LDA",     3,  OP_LDA},
    {"SUBI",    2,  OP_SUBI},
    {"JMP",     3,  OP_JMP},
    {"JZ",      3,  OP_JZ},
    {"R02ACC",  1,  OP_R02ACC},
    {"CMPI",    2,  OP_CMPI},
    {"CALL",    3,  OP_CALL},
    {"RET",     1,  OP_RET},
    {"LDMAR",   2,  OP_LDMAR},
    {"STA_MAR", 1,  OP_STA_MAR},
    {"LDA_MAR", 1,  OP_LDA_MAR}
};

// Symbol tables and counters for the two-pass assembly
LabelInfo label_table[MAX_LABELS];
int label_count = 0; 

Constant constant_table[MAX_CONSTANTS];
int constant_count = 0;



/* ===============================================
 * ================ STRING UTILS =================
 * =============================================== */

// Removes leading whitespace from a string
char *ltrim(char *str) {
    while (*str && isspace((unsigned char)*str)) 
    {
        str++;
    }
    return str;
}

// Remove comments(everything after COMMENT_CHAR) from a line
void strip_comments(char *line) {
    char *comment_char = strchr(line, COMMENT_CHAR);
    if (comment_char != NULL) 
    {
        *comment_char = '\0';
    }
}

// Removes trailing colon (":") from a label
void trim_end_colon(char *label){
    size_t len = strlen(label);
    if (len > 0 && label[len - 1] == ':') 
    {
        label[len - 1] = '\0';
    }
}

// Checks if a token is a label (ends with ":")
int token_is_label(char *label) {
    if (label == NULL) 
    {
        return 0;
    }

    size_t len = strlen(label);
    if (len > 0 && label[len - 1] == ':') 
    {
        return 1;
    }
    return 0;
}

/* ===============================================
 * ========== INSTRUCTION TABLE UTILS ============
 * =============================================== */

// Gets the length (in nibbles) of an instruction by its mnemonic
// Returns -1 if the instruction is not found
int get_instruction_length(char *mnemonic) {
    for (int i = 0; i < NUM_INSTRUCTIONS; i++) 
    {
        if (strcmp(instruction_table[i].mnemonic, mnemonic) == 0) { // Match
            return instruction_table[i].num_words;
        }
    }
    return -1; // Not found
}

// Gets the opcode (4 bits) of an instruction by its mnemonic
// Returns OP_NOP (0b0000) if the instruction is not found
int get_instruction_opcode(char *mnemonic) {
    for (int i = 0; i < NUM_INSTRUCTIONS; i++) 
    {
        if (strcmp(instruction_table[i].mnemonic, mnemonic) == 0) { // Match
            return instruction_table[i].opcode;
        }
    }
    return OP_NOP; // Not found
}



/* ===============================================
 * ============= SYMBOL TABLE UTILS ==============
 * =============================================== */

// Gets the address of a label by its name
// Returns 0 if the label is not found
int get_label_address(char* label_name) {    
    for (int i = 0; i < label_count; i++) 
    {
        if (strcmp(label_table[i].name, label_name) == 0) // Match
        { 
            return label_table[i].address;
        }
    }
    return 0; // Not found
}

// Gets the value of a constant by its name
// Returns -1 if the constant is not found
int get_constant_value(char* constant) {
    for (int i = 0; i < constant_count; i++) 
    {
        if (strcmp(constant_table[i].name, constant) == 0) // Match 
        {
            return constant_table[i].value;
        }
    }
    return -1; // Not found
}

// Adds a label to the label table
// Returns 0 on success, 1 on failure
int add_label(char* label_name, int address) {
    if (label_count >= MAX_LABELS) 
    {
        if (PRINT_ENABLED) {
            printf("Error: Maximum number of labels reached.\n");
        }
        return 1;
    }

    label_table[label_count].name = strdup(label_name);
    label_table[label_count].address = address;
    label_count++;
    return 0;
}

// Adds a constant to the symbol table
// Returns 0 on success, 1 on failure
int add_constant(char *constant_name)
{
    char *equ = strtok(NULL, " \t");
    if (equ == NULL || strcmp(equ, "EQU") != 0)
    {
        if (PRINT_ENABLED) {
            printf("Error: Expected EQU in constant definition %s\n", constant_name);
        }
        return 1;
    }

    char *const_value_str = strtok(NULL, " \t\n");
    if (const_value_str == NULL)
    {
        if (PRINT_ENABLED) {
            printf("Error: Missing value for constant %s\n", constant_name);
        }
        return 1;
    }

    if (constant_count >= MAX_CONSTANTS)
    {
        if (PRINT_ENABLED) {
            printf("Error: Too many constants (max %d)\n", MAX_CONSTANTS);
        }
        return 1;
    }

    int value = (int)strtol(const_value_str, NULL, 0);
    constant_table[constant_count].name = strdup(constant_name);
    constant_table[constant_count].value = value;
    constant_count++;
    return 0;
}

// Cleans up allocated memory for labels and constants
void cleanup_tables(void) {
    // 1. Free label names
    for (int i = 0; i < label_count; i++) 
    {
        if (label_table[i].name != NULL) 
        {
            free(label_table[i].name);
        }
    }

    // 2. Free constant names
    for (int i = 0; i < constant_count; i++) 
    {
        if (constant_table[i].name != NULL) 
        {
            free(constant_table[i].name);
        }
    }
}

/* ===============================================
 * ============= INSTRUCTION UTILS ===============
 * =============================================== */

// Splits a 8-bit address into write nibles to two 4-bit values
void split_address(int address, int *high_nibble, int *low_nibble) {
    *high_nibble = (address >> 4) & 0x0F;
    *low_nibble = address & 0x0F;
}

// Encodes a memory address into the binary output
void encode_memory_address(int *bin_output, int addr_counter, char *addr_str) {
    int address = get_constant_value(addr_str);

    if (address == -1)  // Literal address, relative to RAM
    { 
        address = (int)strtol(addr_str, NULL, 0);
        bin_output[addr_counter + 1] = RAM_START >> 4;  // Shift XXXX YYYY to 0000 XXXX to store in high nible
        bin_output[addr_counter + 2] = address;         // The actual RAM addr to read / write
    }
    else                // Absolute address anywhere (ROM, RAM or I/O)
    { 
        int high_nibble, low_nibble;
        split_address(address, &high_nibble, &low_nibble);
        bin_output[addr_counter + 1] = high_nibble;
        bin_output[addr_counter + 2] = low_nibble;
    }
}

// Encodes a jump address into the binary output
void encode_jump_address(int *bin_output, int addr_counter, char *label_name) {
    int address = get_label_address(label_name);
    int high_nibble, low_nibble;
    split_address(address, &high_nibble, &low_nibble);
    bin_output[addr_counter + 1] = high_nibble;
    bin_output[addr_counter + 2] = low_nibble;
}

// Encodes immediate values into the binary output
void encode_immediate(int *bin_output, int addr_counter, int instruction_len) {
    for (int i = 1; i < instruction_len; i++) 
    {
        char* immediate_str = strtok(NULL, " \t\n");
        int immediate = get_constant_value(immediate_str);

        if (immediate == -1) // No matching constant found, treat as literal value
        { 
            immediate = (int)strtol(immediate_str, NULL, 0);
        }
        bin_output[addr_counter + i] = immediate;

    }
}



/* ===============================================
 * ============== OUTPUT FUNCTIONS ===============
 * =============================================== */

// Prints the label table (for debugging)
void print_labels(void) {
    if (!PRINT_ENABLED) {
        return;
    }
    printf("Labels:\n");
    for (int i = 0; i < label_count; i++) 
    {
        printf("%s: %d\n", label_table[i].name, label_table[i].address);
    }
    printf("\n");
}

// Prints a 4-bit binary representation of a value
void print4bit(int value)
{
    if (!PRINT_ENABLED) {
        return;
    }
    for (int i = 3; i >= 0; i--)
    {
        int bit = (value >> i) & 0b0001;
        printf("%d", bit);
    }
    printf("\n");
}

// Prints the binary output in 4-bit format
void print_binary(int *bin_output, int length)
{
    if (!PRINT_ENABLED) {
        return;
    }
    printf("Binary Output:\n");
    for (int i = 0; i < length; i++)
    {
        printf("%3d (0x%02X): ", i, i);
        print4bit(bin_output[i]);
    }
    printf("\n");
}

// Writes the binary output to a file
int write_output_file(const char *filename, int *bin_output) {
    FILE *out = fopen(filename, "w");
    if (out == NULL) 
    {
        if (PRINT_ENABLED) {
            printf("Error: Could not open output file %s\n", filename);
        }
        return 1;
    }

    for (int i = 0; i < ROM_SIZE; i++) {
        fprintf(out, "0x%X ", bin_output[i] & 0xF);
    }

    printf("Machinecode written to %s\n", filename);
    fclose(out);
    return 0;
}



/* ===============================================
 * =================== PASSES ====================
 * =============================================== */

// Handles a single line during the first pass
void handle_first_pass_line(char *line, int *addr_counter) {
    // 1. Clean up the line
    strip_comments(line);
    char *trimmed_line = ltrim(line);

    // 2. Get the first token (mnemonic/label/constant)
    char *token = strtok(trimmed_line, " \t\n");
    if (token == NULL) return; // Skip empty line

    // 3. Check if it's an instruction
    int instruction_len = get_instruction_length(token);

    if (instruction_len == -1)      // Not and instruction, so either a label or a constant
    {
        if (token_is_label(token))  // 4A. Label definition
        { 
            trim_end_colon(token);
            add_label(token, *addr_counter);
        }
        else 
        {                           // 4B. Constant definition
            add_constant(token);
        }
    }
    else // It's an instruction -> Ignore for first pass
    {
        *addr_counter += instruction_len;
    }
}

int first_pass(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) 
    {
        if (PRINT_ENABLED) {
            printf("File not found\n");
        }
        return 1;
    }

    int addr_counter = 0;
    char line[MAX_LINE_LENGTH];

    if (PRINT_ENABLED) {
        printf("First Pass: Finding Labels and Constants...\n\n");
    }

    while (fgets(line, sizeof(line), file)) 
    {
        handle_first_pass_line(line, &addr_counter);
    }

    fclose(file);
    return 0;
}

void handle_second_pass_line(char *line, int *addr_counter, int *bin_output) {
    // 1. Clean up the line
    strip_comments(line);
    char *trimmed_line = ltrim(line);

    // 2. Get the first token (mnemonic/label/constant)
    char *mnemonic = strtok(trimmed_line, " \t\n");
    if (mnemonic == NULL || token_is_label(mnemonic)) // Skip empty lines or labels
    { 
        return;
    }

    // 3. Find instruction length
    int instruction_len = get_instruction_length(mnemonic);
    if (instruction_len == -1) // Most likely a constant definition, skip
    { 
        return;
    }

    // 4. Find and encode instruction
    int opcode = get_instruction_opcode(mnemonic);
    bin_output[*addr_counter] = opcode;

    if (instruction_len > 1) 
    {
        if (opcode == OP_STA || opcode == OP_LDA)
        {
            char *ram_addr_str = strtok(NULL, " \t\n");
            encode_memory_address(bin_output, *addr_counter, ram_addr_str);
        }
        else if (opcode == OP_JMP || opcode == OP_JZ || opcode == OP_CALL)
        {
            char *label_name = strtok(NULL, " \t\n");
            encode_jump_address(bin_output, *addr_counter, label_name);
        }
        else
        { // LDI, ADDI, SUBI, CMPI, LDMAR = operations with immediate values
            encode_immediate(bin_output, *addr_counter, instruction_len);
        }
    }

    *addr_counter += instruction_len;
    
}


int second_pass(const char *filename, int *bin_output)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) 
    {
        if (PRINT_ENABLED) {
            printf("File not found\n");
        }
        return 1;
    }

    int addr_counter = 0;
    char line[MAX_LINE_LENGTH];

    if (PRINT_ENABLED) {
        printf("\nSecond Pass: Generating Code...\n\n");
    }


    while (fgets(line, sizeof(line), file))
    {
        handle_second_pass_line(line, &addr_counter, bin_output);
    }

    fclose(file);
    return addr_counter; // Return total length used
}

int main(void){
    int bin_output[ROM_SIZE] = {0};

    // 1. Map out the symbols
    if (first_pass(INPUT_FILE) != 0) return 1;
    
    // 2. Build the binary
    int total_words = second_pass(INPUT_FILE, bin_output);

    // 3. Output
    if (PRINT_ENABLED) {
        print_labels();
        print_binary(bin_output, total_words);
    }

    write_output_file(OUTPUT_FILE, bin_output);

    // 4. Cleanup
    cleanup_tables();

    return 0;
} 