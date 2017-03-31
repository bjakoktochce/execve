# execve

Loadable Kernel Module designed for OpenBSD 4.4 and OpenBSD 4.5. Testes under OpenBSD 4.4 and 4.5. 

After loading this module takes control over system call execve() and executes it's own version of execve().
Function logs the execution details to the system logs and then gives back control to the old execve() function.
