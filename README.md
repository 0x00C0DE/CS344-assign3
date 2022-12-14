# CS344-assign3
cs344-osu-smallsh

Braden Lee
Oregon State University

This program is intended to mimic a simple basic shell. Very detailed description of this shell can be found via comments in the code. 
This basic shell will be able to:

1.Provide a prompt for running commands

2.Handle blank lines and comments, which are lines beginning with the # character

3.Provide expansion for the variable $$

4.Execute 3 commands exit, cd, and status via code built into the shell

5.Execute other commands by creating new processes using a function from the exec family of functions

6.Support input and output redirection

7.Support running commands in foreground and background processes

Implement custom handlers for 2 signals, SIGINT and SIGTSTP
---------------
INSTRUCTIONS to compile & run
---------------
1. chmod +x ./p3testscript

2. make sure the "p3testscript" file is in the same directory as the "ass3_smallsh.c" file, on the os1 server

3. to compile "gcc -std=gnu99 -Wall ass3_smallsh.c -o smallsh"

4. to run the executable "./p3testscript" or "valgrind --leak-check=yes p3testscript"
---------------
