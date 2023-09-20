# lockshell

A simple command line tool to temporarily freeze and clear the current shell until a password is entered, at which point
everything will be restored.

## Rationale

Have you ever wanted to stand up and go grab some water? Or a coffee? Or maybe just use the gents (or ladies)? Have you had
an ssh session open with all your stuff on some important server? Have you not wanted to close said important ssh session
because you would lose your command history and cached credentials and sudo login and whatever else you happened to be
working on?

Now you can lock your terminal. Like putting your computer to sleep, you get to keep everything saved, but now no one can simply
walk up to your computer while you aren't looking and type a cheeky `rm -rf ~` or take a peek at the
definitelycodeandnotsomethingunrelated (TM) that you had open at the time.

I'm sure you were hard at work except for the just two
seconds where you only got a tiny bit distracted. Besides, thats why you were getting a coffee, right? To help you focus, right?
