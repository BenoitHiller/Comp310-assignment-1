## Intro
The shell is called "shesells" because it is the shesells C shell and yeah.

## Compilation
To compile just fasten your seatbelt and type:
    make
By default the prompt indicator is disabled, this is so that it passes the test script that I didn't write to compile it with the prompt run:
    make withprompt

## Installation
Installing the shesells shell on your computer is as simple as running:
    make install

you will probably afterwards want to adjust your passwd/shadow file to reflect your new favorite shell.
If you do not have permission to do this installation globally the file will be placed in `~/bin`

## Issues
1. I didn't code a prompt at all. It isn't required and it screws up your test file.
2. Unlike in bash "cd something | echo something" works as "expected"...
3. everything after a `${>,<}` is assumed to be a part of the filename, this is only slightly quirkier than the way bash handles it.
4. I did not bother doing any $ substitutions except for $?. This is for the sake of my sanity.
5. There is pretty much no error checking anywhere. In addition to which I think stderr is occasionally swallowed.
6. echo accepts the "-e" flag, no other internal function takes flags of any kind.
7. "`a > b | c`" does not work due to a parser error.
8. Max command length should be 512, that includes the "\n" that I strip off.
9. There is no internal help of any kind, you can still use man though.
10. The code may contain comments denoting broken items that are no longer broken.
11. The exit values reported by functions are "occasionally" strange.
12. This readme file was pastebinned at one point.
13. It doesn't compile on FreeBSD to no one's surprise.
14. There is no way to escape or quote anything.
