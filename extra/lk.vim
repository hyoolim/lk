" Vim Syntax File
" Language:    Kinetic
" Maintainer:  Hyoo Lim <hl@lk-language.com>
" Last Change: 2004 Nov 06
" Location:    http://www.lk-language.com/dl/lk.vim

" sanity check
if version < 600
    syntax clear
elseif exists("b:current_syntax")
    finish
endif

" init lang syntax globals here
syntax case match

" normal literals
syntax match kcWord /[.a-zA-Z][-.a-zA-Z0-9]*[!?]\?/
hi link kcWord Normal
" lang does not strictly enforce capitalized type names,
" but it is convention
syntax match kcType /[A-Z][-.a-zA-Z0-9]*[!?]\?/
hi link kcType Type
" syntax keyword kcConstant false inf nil nl true
syntax match kcConstant /[A-Z][-.A-Z0-9]\+[!?]\?/
hi link kcConstant Constant
syntax region kcString start=/'/ skip=/\\'/ end=/'/
syntax region kcString start=/"/ skip=/\\"/ end=/"/
syntax match kcString /`[^\s{}\[\]();`'".]*/
hi link kcString String
syntax match kcNumber /[0-9][0-9_]*\(\.[0-9_]*\)\?/
hi link kcNumber Number
syntax match kcCharacter /0c\\\?./
hi link kcCharacter Character
syntax match kcBoolean /\(false\|inf\|nil\|nl\|true\|std\(in\|out\|err\)\)$/
syntax match kcBoolean /\(false\|inf\|nil\|nl\|true\|std\(in\|out\|err\)\)[^-.a-zA-Z0-9!?]/me=e-1
hi link kcBoolean Boolean

" punc messages
" there are no keywords in lang but some of the more
" frequently used functions are listed as keywords here
syntax keyword kcDelimiter do end
syntax match kcOperator /[()\[\]{};]/
syntax match kcOperator /[-~!@#$%^&*#%&*=+|:,<>/?]\+/
hi link kcOperator Operator

" special ops
syntax match kcSpecial /\//
syntax match kcSpecial /->/
hi link kcSpecial Special

" padding/whitespace
syntax region kcComment start='\\\\' end='$'
syntax region kcComment start='\\\*' end='\*\\'
hi link kcComment Comment

" don't load again
let b:current_syntax = "lk"
