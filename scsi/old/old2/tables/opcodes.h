
/* Define the opcode table structure */
struct _opcode_info {
	char status;
	char *desc;
};
typedef struct _opcode_info OPCODE_INFO;
#define OPCODE_INFO_SIZE sizeof(struct _opcode_info)

/* Define the opcode status */
#define OPCODE_INVALID   'I'
#define OPCODE_MANDITORY 'M'
#define OPCODE_OPTIONAL  'O'
#define OPCODE_VENDOR    'V'
#define OPCODE_RESERVED  'R'
#define OPCODE_OBSOLETE  'Z'

/* Define the opcode table */
#define OPCODE_MAX 256
#ifndef __HAVE_TABLES
extern OPCODE_INFO opcodes[SDT_MAX][OPCODE_MAX];
#endif /* __HAVE_TABLES */
