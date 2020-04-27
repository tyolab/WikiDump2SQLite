#!/bin/bash

cat $1 |
grep -v -E "^[[:space:]]+KEY" |
grep -v -E "^[[:space:]]+UNIQUE KEY" |
grep -v -E "^[[:space:]]+PRIMARY KEY" |
sed '/^SET/d' |
sed 's/ unsigned / /g' |
sed 's/ auto_increment/ primary key autoincrement/g' |
sed 's/ AUTO_INCREMENT/ primary key autoincrement/g' |
sed 's/ smallint([0-9]*) / integer /g' |
sed 's/ tinyint([0-9]*) / integer /g' |
sed 's/varbinary([0-9]*)/text/g' |
sed 's/ int([0-9]*) / integer /g' |
sed 's/ character set [^ ]* / /g' |
sed 's/ enum([^)]*) / varchar(255) /g' |
sed 's/ on update [^,]*//g' |
sed 's/) ENGINE.*;/);/g' |
perl -e 'local $/;$_=<>;s/,\n\)/\n\)/gs;print "begin;\n";print;print "commit;\n"' |
perl -pe '
  if (/^(INSERT.+?)\(/) {
     $a=$1;
     s/\\'\''/'\'\''/g;
     s/\\n/\n/g;
     s/\),\(/\);\n$a\(/g;
  }
  ' 
