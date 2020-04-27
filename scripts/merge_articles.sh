#!/bin/bash

for i in {10..19..3}; do echo $i; db=wikipedia; v1=`expr $i + 1`; v2=`expr $i + 2`; cmd="/home/monfee/drives/code/WikiDump2SQLite/merge_articles ../$db$i.db ../$db$v1.db $db$i-$v1.db 250000"; cmd2="/home/monfee/drives/code/WikiDump2SQLite/merge_articles ../$db$v2.db ../$db$v1.db $db$v1-$v2.db +250000"; echo $cmd; echo $cmd2; done

