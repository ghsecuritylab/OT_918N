
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "libdwP.h"


int
dwarf_lineblock (Dwarf_Line *line, bool *flagp)
{
  if (line == NULL)
    return -1;

  *flagp =  line->basic_block;

  return 0;
}
