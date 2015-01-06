/*
 * Determine whether or not a given argument exists on the command line
 * passed into the kernel.
 *
 * Takes a pointer to a NUL terminated string that is the name of
 * the desired argument.
 */
#include <sys/typed.h>
#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif /* TRUE */

extern typed_space boot_gift;

int
arg_exist(arg)
{
    FIFO *gift_ffp;
    typed_space *tp;
    int retval;

    retval = FALSE;

    /* If we can't open the gift, we can't find the argument.  */
    gift_ffp = fifo_open(&boot_gift, 0);

    if (F_NULL != gift_ffp) {
        while (T_NULL != (tp = fifo_read(gift_ffp))) {
	      if (T_STR_ARGF == tp->ts_type) {
		  RETYPE(tp, T_FIFO_SIC);
		  retval = fifo_find_str(tp, arg);
		  break;
	      }
        }
    }

    fifo_close(gift_ffp);
    return(retval);
} /* arg_exist() */


/*
 * Looks for the string "astring" in the fifo "afifo".
 * Returns TRUE if it find the string.
 */
int
fifo_find_str(afifo, astring)
	typed_space *afifo;
	char *astring;
{
    FIFO *command_line;
    typed_space *tp;
    int retval;

    retval = FALSE;

    /* If we can't open the gift, we can't find the argument.  */
    command_line = fifo_open(afifo, 0);
 
    if (F_NULL != command_line) {
        while (T_NULL != (tp = fifo_read(command_line))) {
	      /* Only check those items that are strings.  */
	      if (T_STR_STR == tp->ts_type) {
		  /* Is this the string we've been looking for?  */
		  if (0 == strcmp(tp->ts_data, astring)) {
		  	retval = TRUE;
			break;
		  }
	      }
        }
   }

   fifo_close(command_line);
   return(retval);

} /* fifo_find_str() */
