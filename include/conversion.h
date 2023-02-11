#ifndef COMP8005_PROJECT_CONVERSION_H
#define COMP8005_PROJECT_CONVERSION_H

#include <errno.h>
#include <limits.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdlib.h>

in_port_t parse_port(const char *buff, int radix);

#endif // COMP8005_PROJECT_CONVERSION_H
