set nocompatible
set number
filetype on
syntax on
set tabstop=4
set shiftwidth=4
set softtabstop=4
set expandtab
set autoindent
set smartindent

autocmd BufRead *.py set smartindent cinwords=if,elif,else,for,while,try,except,finally,def,class

autocmd BufRead *.c set cindent
autocmd BufRead *.cpp set cindent

set showmatch
set ruler
set incsearch
set nohls

"窗口操作快捷键
"开启水平窗口，关闭水平窗口
nmap wv <C-w>v 
nmap wc <C-w>c
nmap ws <C-w>s

"设置配色方案
colorscheme evening
set mouse=a

"C编译
map <F5> :call CompileRunGcc()<CR>
func! CompileRunGcc()
exec "w"
exec "!gcc % -o %<"
exec "! ./%<"
endfunc
"C++编译
map <F6> :call CompileRunGpp()<CR>
func! CompileRunGpp()
exec "w"
exec "!g++ % -o %<"
exec "! ./%<"
endfunc

inoremap " ""<ESC>i
inoremap ( ()<ESC>i
inoremap [ []<ESC>i
