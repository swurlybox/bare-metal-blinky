/*
    Header file which provides an API for navigating the FatFs on the
    attached microSD card. The filesystem object must be mounted with
    FatFs module's f_mount function, in order for the following API
    to work. All of these functions depend on the filesystem object.
*/

#ifndef FF_NAV_H
#define FF_NAV_H

#include <stdint.h>

/*  Lists the files in the current working directory.
    Prints the contents to some sort of user interface (pos. dep. injection)
    Returns the count of items in the directory.
    */
uint32_t list_directory(const char *path);

#endif
