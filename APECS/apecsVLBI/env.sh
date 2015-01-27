#!/bin/bash

if [[ $PYTHONPATH != *"ply-3.4"* ]]
then
	echo "Adding PLY (Python Lex-Yacc) v3.4 to Python path"
	export PYTHONPATH=$PYTHONPATH:`pwd`/ply/ply-3.4/
fi
