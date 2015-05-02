" Vim syntax file
" Language: Source Virtual Machine
" Maintainer: Alexander Jones
" Latest Revision: 4/8/2015
"
if exists("b:current_syntax")
  finish
endif

syn match operator '+\|-\|/\|\*\|%\|&\|\^\|!\||\|\.\|<\|>\|=\|?'
syn match function '#[^( ]*'
syn match stringConstant '"[^"]*"'
syn match numberConstant '\d\+\(.\d\+\)\?'
syn match setVariable '@[^@ ]*@'
syn match getVariable '\$[^$ ]*\$'

let b:current_syntax = "svm"

hi def link operator		Operator
hi def link function		Function
hi def link stringConstant	String
hi def link numberConstant	Number
hi def link setVariable		Keyword
hi def link getVariable		Keyword
