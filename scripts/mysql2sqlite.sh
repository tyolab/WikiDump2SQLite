#!/bin/sh

home=`dirname $0`

if [ "x$1" == "x" ]; then
   echo "Usage: $0 <dumpname>"
   exit
fi

$home/sqlite_replace.sh $1 > $1.sql

cat $1.sql | sqlite3 $1.db > $1.err
ERRORS=`cat $1.err | wc -l`
if [ $ERRORS == 0 ]; then
  echo "Conversion completed without error. Output file: $1.db"
  #rm $1.sql
  #rm $1.err
   rm tmp
else
   echo "There were errors during conversion.  Please review $1.err and $1.sql for details."
fi
