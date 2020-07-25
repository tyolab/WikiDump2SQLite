#!/bin/bash

home=`dirname $0`/..
libdir=$home/lib
#echo $home
java -cp $home/../WikiLibs/json-java-android.jar:$home/../CommonUtils/bin/commonutils.jar:$home/../WikiSQLite/lib/sqlite-jdbc-3.7.15-M1.jar:$home/../WikiApiLib/bin/wikiapilib.jar:$home/../WikiSQLite/bin/classes:$home/bin au.com.tyo.wiki.utils.WikiPageImporter "$@"

