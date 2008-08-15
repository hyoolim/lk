" Vim Syntax File
" Language:    Lk
" Maintainer:  Hyoo Lim <hl@lk-language.com>
" Last Change: 2007 Jun 13
" Location:    http://www.lk-language.com/dl/lk.vim

" sanity check
if version < 600
    syntax clear
elseif exists("b:current_syntax")
    finish
endif

" init lang syntax globals here
syntax case match

" literals
syntax match kcWord /[.a-zA-Z][!?]\?/
syntax match kcWord /[.a-zA-Z][ .a-zA-Z0-9]*[.a-zA-Z0-9][!?]\?/
hi link kcWord Normal
syntax match kcType /[A-Z][!?]\?/
syntax match kcType /[A-Z][ .a-zA-Z0-9]*[.a-zA-Z0-9][!?]\?/
hi link kcType Type
syntax region kcString start=/'/ skip=/\\'/ end=/'/
syntax region kcString start=/"/ skip=/\\"/ end=/"/
syntax match kcString /`[^\s{}\[\]();`'".]*/
hi link kcString String
syntax match kcNumber /[0-9][0-9_]*\(\.[0-9_]*\)\?/
hi link kcNumber Number
syntax match kcCharacter /0c\\\?./
hi link kcCharacter Character

" operators
syntax match kcOperator /[()\[\]{};]/
syntax match kcOperator /[-~!@#$%^&*#%&*=+|:,<>/?]\+/
hi link kcOperator Operator

" special ops
" syntax match kcSpecial /\//
" syntax match kcSpecial /->/
" hi link kcSpecial Special

" padding
syntax region kcComment start='#' end='$'
syntax region kcComment start='#\*' end='\*#'
hi link kcComment Comment

" don't load again
let b:current_syntax = "lk"
