" Vim syntax file
" Language:      LibMozok Quest File
" Maintainer:    zegalur
" Last Change:   2025-04-07

if exists("b:current_syntax")
  finish
endif


" === Types ===
syn region questTypeDef start="^\s*type\>\s\+" end="$" contains=questTypeName,questComment,questKeywordType oneline 
syn match questTypeName "\<\u[a-zA-Z0-9_]*\>" contained
syn keyword questKeywordType type contained


" === Quests ===
syn region questMainQuestDef start="^\s*main_quest\>\s\+" end="$" contains=questMainQuestName,questComment,questKeywordMainQuest oneline
syn region questActionObjectList start="^\s*actions\>" end="^\s*subquests\>" keepend contains=questObjectName,questActionName,questComment,questQuestParam
syn keyword questKeywordMainQuest main_quest contained
syn match questMainQuestName "\<\u[a-zA-Z0-9_]*\>" contained


" === Objects ===
syn region questObjectDef start="^\s*object\>\s\+" end="$" contains=questObjectName,questTypeName,questComment,questKeywordObject oneline
syn region questArgDef start="^\s*\l[a-zA-Z0-9_]*\s*:\s*\u" end="$" contains=questObjectName,questTypeName,questComment oneline
syn match questObjectName "\<\l[a-zA-Z0-9_]*\>" contained
syn keyword questKeywordObject object contained


" === Relations ===
syn region questRelDef start="^\s*rel\>\s\+" end="\ze(" contains=questRelName,questKeywordRel oneline
syn region questRListDef start="^\s*rlist\>\s\+" end="\ze:" contains=questRelName,questKeywordRList oneline
syn region questRelCall start="\<\u" end="\ze(" contains=questRelName oneline
syn region questRelArgs start="(" end="\ze)" contains=questTypeName,questObjectName oneline
syn keyword questKeywordRel rel contained
syn keyword questKeywordRList rlist contained
syn match questRelName "\<\u[a-zA-Z0-9_]*\>" contained

" === Actions ===
syn region questActionDef start="^\s*action\>\s\+" end="\ze:" contains=questActionName,questNA,questKeywordAction oneline
syn keyword questKeywordAction action contained
syn match questActionName "\<\u[a-zA-Z0-9_]*\>" contained


" === Other Keywords ===
syn keyword questSpecial version project 
syn keyword questMacro include 
syn match questNA /N\/A/ contained
syn keyword questStatus ACTIVE INACTIVE DONE UNREACHABLE PARENT
syn keyword questHeuristic SIMPLE HSP
syn keyword questStrategy ASTAR DFS
syn keyword questKeywordQuest quest
syn keyword questQuestParam preconditions goal actions objects subquests 
syn keyword questQuestParam options searchLimit spaceLimit omega status 
syn keyword questQuestParam heuristic use_atree strategy
syn keyword questActionBlock pre add rem


" === Comments ===
syntax match questComment /#.*$/ contains=@Spell
highlight link questComment Comment


" === Links ===

hi def link questSpecial        Special
hi def link questMacro          Macro
hi def link questNA             String
hi def link questStatus         String
hi def link questHeuristic      String
hi def link questStrategy       String
hi def link questKeyword        Keyword
hi def link questKeywordType    Keyword
hi def link questKeywordObject  Keyword
hi def link questKeywordRel     Keyword
hi def link questKeywordRList   Keyword
hi def link questKeywordAction  Keyword
hi def link questKeywordQuest     String
hi def link questKeywordMainQuest String
hi def link questQuestParam     Keyword
hi def link questActionBlock    Keyword
hi def link questTypeName       Type
hi def link questObjectName     Identifier
hi def link questRelName        Function
hi def link questActionName     Number
hi def link questMainQuestName  Underlined

let b:current_syntax = "quest"
