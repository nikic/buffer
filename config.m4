dnl $Id$
dnl config.m4 for extension buffer

PHP_ARG_ENABLE(buffer, whether to enable buffer support,
Make sure that the comment is aligned:
[  --enable-buffer           Enable buffer support])

if test "$PHP_BUFFER" != "no"; then
  PHP_NEW_EXTENSION(buffer, buffer.c, $ext_shared)
fi
