#ifndef NCCRDUMP_H
#define NCCRDUMP_H 1

#define MAGIC_START  "\x43\x44\x46\x53"
#define MAGIC_END    "\xed\xed\xde\xde"
#define MAGIC_HEADER "\xad\xec\xce\xda" 
#define MAGIC_DATA   "\xab\xec\xce\xba"
#define MAGIC_ERR    "\xab\xad\xba\xda"

extern ast_err nccr_dumpheader(Header*);

extern ast_err nccr_data_dump(Data*, Variable*, int bigendian, bytes_t* data);

#endif /*NCCRDUMP_H*/
