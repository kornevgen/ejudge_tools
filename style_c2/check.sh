#!/bin/sh

/opt/ejudge/libexec/ejudge/checkers/style_c $1 && \
    ( astyle \
      --keep-one-line-blocks \
      --keep-one-line-statements \
      < $1 | diff -Z $1 - 1>&2 )

