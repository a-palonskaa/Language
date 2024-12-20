push 0
pop hx
call main:
hlt
main:
push hx
push 100
add
pop hx
push 0
pop [hx+1]
push 0
pop [hx+2]
push 0
pop [hx+3]
push 0
pop [hx+4]
in
pop [hx+1]
in
pop [hx+2]
in
pop [hx+3]
push [hx+1]
push 0
je 0:
push [hx+2]
push 2
pow
push 4
push [hx+1]
mul
push [hx+3]
mul
sub
pop [hx+5]
push [hx+5]
push 0
je 1:
push 0
pop [hx+4]
jmp finish_1:
1:
push 0
push [hx+2]
sub
push 2
push [hx+1]
mul
div
pop [hx+4]
push [hx+4]
out
push hx
push 100
sub
pop hx
ret
finish_1:
push [hx+5]
push 0
jb 2:
push [hx+5]
push 0.5
pow
pop [hx+5]
push 0
push [hx+2]
sub
push [hx+5]
sub
push 2
push [hx+1]
mul
div
pop [hx+4]
push [hx+4]
out
push 0
push [hx+2]
sub
push [hx+5]
add
push 2
push [hx+1]
mul
div
pop [hx+4]
push [hx+4]
out
push hx
push 100
sub
pop hx
ret
jmp finish_2:
2:
push 666
sub
pop [hx+4]
push [hx+4]
out
push hx
push 100
sub
pop hx
ret
finish_2:
jmp finish_0:
0:
push [hx+2]
push 0
je 3:
push [hx+3]
push 0
je 4:
push 0
push [hx+2]
sub
pop [hx+4]
push [hx+4]
push [hx+3]
div
pop [hx+4]
push [hx+4]
out
push hx
push 100
sub
pop hx
ret
jmp finish_4:
4:
push 0
pop [hx+4]
push [hx+4]
out
push hx
push 100
sub
pop hx
ret
finish_4:
jmp finish_3:
3:
push [hx+3]
push 0
je 5:
push 666
sub
pop [hx+4]
push [hx+4]
out
push hx
push 100
sub
pop hx
ret
jmp finish_5:
5:
push 666
pop [hx+4]
push [hx+4]
out
push hx
push 100
sub
pop hx
ret
finish_5:
finish_3:
finish_0:
