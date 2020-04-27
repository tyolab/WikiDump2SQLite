#!/bin/bash

ln -sf wikipedia.db wikipedia0.db

redirect=0

if [ "$1" -eq 1 ]; then
	redirect=1
fi

v=0; for i in {0..22}; do num=`sqlite wikipedia${i}.db "select count(*) from articles where redirect=${redirect}"`; v=`expr $v + $num`; done; echo $v

