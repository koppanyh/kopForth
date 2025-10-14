." Loading boot.fs ... "

: ' BL WORD FIND DROP ;

: LITERAL
  [
    ' (LITERAL) COMPILE,
    ' (LITERAL) ,
  ]
  COMPILE,
  ,
; IMMEDIATE

: CHAR PARSE-NAME DROP C@ ;
: [CHAR] CHAR ; IMMEDIATE

: (
  [CHAR] ) LITERAL
  PARSE 2DROP
; IMMEDIATE

: 2CONSTANT
  CREATE SWAP , ,
  DOES>
  DUP @ SWAP
  1 CELLS + @
;

: WORDS
  LATEST              ( XT )
  DUP >NAME           ( XT C-ADDR )
  COUNT TYPE SPACE    ( XT )
  BEGIN
    >LINK @           ( XT | 0 )
  DUP WHILE
    DUP >NAME         ( XT C-ADDR )
    COUNT TYPE SPACE  ( XT )
  REPEAT
  DROP                (  )
;

." Loaded boot.fs" CR
