.global timer_handler_wrapper
.global keyboard_handler_wrapper

timer_handler_wrapper:
	pusha 							   #push all caller saved registers to the stack
	call timer_handler     #call the function
	popa								   #pop all caller saved registers to restore them
	iret								   #return

keyboard_handler_wrapper:
	pusha									 #push all caller saved registers to the stack
	call keyboard_handler  #call the function
	popa 									 #pop all caller saved registers to restore them
	iret                   #return


