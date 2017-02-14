#!/bin/sh

/opt/ejudge/libexec/ejudge/checkers/style_c $1 && \
    ( astyle \
      --add-brackets \
      --pad-oper \
      --pad-header \
      --align-pointer=name \
      < $1 | diff -Z $1 - 1>&2 )

