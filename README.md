simple-LR-Parser
================
Since the given grammer rules can not be reduced to LL grammer format by using
Left Recursion Removing Algorithms, LR Parser System is used as Parser Engine.

The SLR(1) parsing table is constructed and feeded to the Parser Engine.

Files :
input.txt   : Sample character sequence, which the given grammer definitions can recognize.
debug.txt   : All the working process is buffered to thie file. Initially it is empty.
grammar.txt : Grammer rules by use of BNF rules
parser.h    : header file of the parser
parser.cpp  : Implementation of header file
main.cpp    : Simple usage scenario.

BNF Rules
terminals "", 
non-terminals <> 
or |.
