#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>

/*
 Run as : ./decode <input file> 
  
 File format in 64b words : 
  header 0
  payload 0 : word 0
  ...
  payload 0 : word N-1
  zeros     : word 0
  ...
  zeros     : word M-1
  header 1
  payload 1 : word 0
  ...   
 
 Header is a 64b word, format in 4b nibbles is :  
  A A L L | R R R S | S S S S | S S S S
  -------------------------------------
 Where : 
     b63-b56 : 8b header code 0xAA  
     b55-b48 : 8b payload length in terms of 64b words
     b47-b36 : 12b run number
     b35-b00 : 36b packet number
*/

int
main(int argc, char** argv)
{

  const char * fname = argv[1]; // "./udp_0.dat";
  FILE * f = fopen(fname, "rb");  

  size_t sz;
  char buf[8]; //64b
  char * bufptr = &(buf[0]);

  bool firsttime=true;
  unsigned long seq_prev;
  int bad_seq_cnt = 0;
  while ( !(feof(f)) ) {

    fread(buf,1,8,f); // 8x1B = 64b

    unsigned code = (unsigned)(buf[7])&0xff;

    if( code == 0xaa ) {

      printf("%02x%02x %02x%02x %02x%02x %02x%02x\n",
    	   0xff&buf[7], 0xff&buf[6],
    	   0xff&buf[5], 0xff&buf[4],
    	   0xff&buf[3], 0xff&buf[2],
    	   0xff&buf[1], 0xff&buf[0]);

      unsigned pktlen = buf[6]&0xff;
      unsigned run = (((unsigned)(buf[5])) << 4)&0xff0  |
	((unsigned)(buf[4])>>4)&0xf;
      unsigned long seq = ((unsigned long)buf[0])&0xff |
	((unsigned long)(buf[1])<<8)&0xff00 |
	((unsigned long)(buf[2])<<16)&0xff0000 |
	((unsigned long)(buf[3])<<24)&0xff000000 |
	((unsigned long)(buf[4])<<32)&0xf00000000 ;
      
      printf( "\thdrcode  : 0x%2x\n",      code);
      printf( "\tpktlen   : 0x%2x\n",      pktlen);      
      printf( "\trun#     : 0x%3x\n",      run);      
      printf( "\tseq#     : 0x%09llx\n",   seq);      	


      // check sequence
      if( !firsttime ) { 
	if( seq != seq_prev+1 ) { 
	  printf( "ERROR :: bad sequence : 0x%09llx vs expected 0x%09llx\n", 
		  seq, seq_prev+1);
	  bad_seq_cnt++;
	}
      }
      seq_prev = seq;

      printf("\n   -- data start -- \n");
      for( int wrdrd=0; wrdrd<pktlen; wrdrd++ ) {  
	fread(buf,1,8,f);

	// conversion to unsigned 
	unsigned long val_u64 = 0;
	for( int i=0; i<8; i++ ) 
	  val_u64 |= ((unsigned long)(buf[i])&0xff)<<(i*8ULL);

        // alternatively, cast
	unsigned long cast_u64 = *((unsigned long *)(&(buf[0])));

	printf("%02x : %02x%02x %02x%02x %02x%02x %02x%02x\t0x%08llx\t0x%08llx\n",
	       wrdrd,
	       0xff&buf[7], 0xff&buf[6],
	       0xff&buf[5], 0xff&buf[4],
	       0xff&buf[3], 0xff&buf[2],
	       0xff&buf[1], 0xff&buf[0],
	       val_u64,
	       cast_u64);
      }
      printf("   -- data end -- \n\n\n");

      firsttime = false;
      // now return to main read loop, look again for the hdr code ...
    }
  } //while !eof

  printf ("\ndone\nsequence errors : %d\n\n", bad_seq_cnt);

}
