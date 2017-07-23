" if autocommand expansion is wrong, consider uncommenting set verbose=level
" set verbose=2

augroup project
	autocmd!
	autocmd BufRead,BufNewFile *.h,*.c set filetype=c.doxygen
	autocmd BufRead,BufNewFile *.h,*.c set formatprg=astyle\ --style=allman
augroup END

set makeprg=make
nnoremap <F4> :YcmCompleter FixIt

set tabstop=4
set softtabstop=4
set shiftwidth=4
set noexpandtab

set colorcolumn=110
highlight ColorColumn ctermbg=darkgray

