# 4 Letters to Brainfuck

This is a simple converter/compiler program that converts a program written with a custom language to brainfuck, and takes the brainfuck output and passes into [a brainfuck-C converter by paulkaefer](https://github.com/paulkaefer/bftoc), and finally compiles into machine code using gcc. The custom language consists of a four letter word written with an uncaptilized first letter and different combinations of capitilized/uncapitilized versions of its last three letters. The user is able to choose which four letters make up the word. In conversion, every 4 letters are tokenized and according to the capitilized/uncapitilized combination of the last three letters, each token is matched with an operator in brainfuck (the combinations are explained below). The line indicator in messages thrown out by the first converter may be customized to be written in a 4-letter base-4 number system where your word's letters are matched to 0, 1, 2 and 3, respectively. Calling the program, pass the word in a string (capitilization does not matter) in the following syntax and the program will compile your code just fine. As explained below, you are also able to state your word at the beginning of the source file. Only english alphabet is allowed currently.

    4letbf [filename] [-n] [--word string]
      -n: convert line numbers in the first converter messages to 4-letter base-4 number system 
      -c: stop after converting to brainfuck
      -C: stop after converting to C  
      -o/--output: output binary file name (default: [filename].out)
      -g/--gcc-options: options to be (literally) passed to gcc besides input and output file names
      --word: string of 4 characters that make up the word for the custom language

    
    Arguments: 
      filename: file to be compiled

## The Combinations (C: capitilized letter, U: uncapitilized letter, F: first letter of the word):
>**FCCC**: >   
>**FCCU**: <   
>**FCUU**: +   
>**FCUC**: -    
>**FUCC**: .   
>**FUCU**: ,   
>**FUUC**: [   
>**FUUU**: ]    

## Stating The Word in The Source File
> At the beginning of the file:

    WORD = "word"
