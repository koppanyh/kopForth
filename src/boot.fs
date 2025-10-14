." Loading boot.fs ... "

: ( 41 PARSE 2DROP ; IMMEDIATE

: ' BL WORD FIND DROP ;

: 2CONSTANT
  CREATE SWAP , ,
  DOES>
  DUP @ SWAP
  1 CELLS + @
;

." Loaded boot.fs" CR
