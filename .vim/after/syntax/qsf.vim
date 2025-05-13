" Vim syntax file
" Language:      LibMozok QSF (Quest Script File)
" Maintainer:    zegalur
" Last Change:   2025-05-13

if exists("b:current_syntax")
  finish
endif


" === Worlds ===
syn region qsfWorldName start="\[" end="\]" oneline
syn region qsfWorldNameC start="\[" end="\]" oneline contained
syn region qsfWorlds start="^worlds\>" end="^projects\>" keepend contains=qsfWorldDef,qsfComment,qsfKeyword
syn match qsfWorldDef "\<\l[a-zA-Z0-9_]*\>" contained


" === Names ===
syn match qsfActionName "\<\u[a-zA-Z0-9_]*\>" contained
syn match qsfObjectName "\<\l[a-zA-Z0-9_]*\>" contained
syn match qsfQuestName "\<\u[a-zA-Z0-9_]*\>" contained
syn match qsfMainQuestName "\<\u[a-zA-Z0-9_]*\>" contained


syn region qsfInitAction start="^\s*\[\l[a-zA-Z0-9_]*\]\s*\u[a-zA-Z0-9_]*\s*(" end="$" oneline contains=qsfWorldNameC,qsfActionName,qsfObjectName


" === Keywords ===
syn keyword qsfKeyword version
syn keyword qsfKeyword script
syn keyword qsfKeyword worlds contained
syn keyword qsfKeyword projects contained
syn keyword qsfKeyword init
syn keyword qsfKeyword debug


" === Status ===
syn keyword qsfStatus UNREACHABLE
syn keyword qsfStatus DONE
syn keyword qsfStatusC UNREACHABLE contained
syn keyword qsfStatusC DONE contained


" === Events ===
syn keyword qsfEvent onInit
syn keyword qsfEventC onNewMainQuest contained
syn keyword qsfEvent onNewQuestStatus
syn keyword qsfEvent onNewSubQuest
syn keyword qsfEvent onSearchLimitReached
syn keyword qsfEvent onSpaceLimitReached
syn keyword qsfEvent onCheck
syn keyword qsfEvent onAction

syn region qsfOnNewMainQuest start="^\s*onNewMainQuest\s*\[" end=":" oneline contains=qsfEventC,qsfMainQuestName,qsfWorldNameC,qsfComment


" === Blocks ===
syn keyword qsfBlock ACT
syn keyword qsfBlock ACT_IF
syn keyword qsfBlock SPLIT
syn keyword qsfBlock ALWAYS


" === Commands ===
syn keyword qsfCmd expect
syn keyword qsfCmdC push contained
syn keyword qsfCmdC pause contained
syn keyword qsfCmdC print contained
syn keyword qsfCmdC exit contained

syn region qsfMsgCmd start="^\s*pause\s*" end="$" oneline contains=qsfCmdC,qsfComment
syn region qsfMsgCmd start="^\s*print\s*" end="$" oneline contains=qsfCmdC,qsfComment
syn region qsfMsgCmd start="^\s*exit\s*" end="$" oneline contains=qsfCmdC,qsfComment
syn region qsfPushCmd start="^\s*push\s*\[" end=")" oneline contains=qsfCmdC,qsfActionName,qsfObjectName,qsfWorldNameC

" === Comments ===
syntax match qsfComment /#.*$/ contains=@Spell
highlight link qsfComment Comment


" === Links ===
hi def link qsfKeyword Keyword
hi def link qsfBlock Keyword
hi def link qsfStatus Keyword
hi def link qsfStatusC Keyword
hi def link qsfWorldName String
hi def link qsfWorldNameC String
hi def link qsfWorldDef String
hi def link qsfObjectName Identifier
hi def link qsfActionName Number
hi def link qsfEvent Special
hi def link qsfEventC Special
hi def link qsfCmd Function
hi def link qsfCmdC Function
hi def link qsfMainQuestName Underlined
hi def link qsfMsgCmd SpecialComment


let b:current_syntax = "qsf"
