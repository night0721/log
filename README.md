# noted

noted is a minimalistic command line note manager written in C.

## Preview
![](https://r2.e-z.host/3c62bb3a-a8a9-43f6-afd6-553646f51dc4/lzwqimtg.png)

## Dependencies
- gcc

## Building
```sh
$ make
$ sudo make install
```

## Features
- Uses EDITOR environment variable to open notes
- Respect XDG variables
- Search notes by name with interactive prompt
- Add, remove, and list notes
- Customizable
- Supports org, txt, markdown and html format
- Generate boilerplate for notes for different formats

## Before you get started
noted will use the editor defined in the `EDITOR` environment variable to open notes. If `EDITOR` is not defined, you will need to define it in your shell configuration file.

Default file extension is `.md` and it can be changed in `config.h` file as you like. Once you change the extension, you need to recompile the program to apply the changes and the new notes will have different boilerplates. However, all commands will not work for old notes with the previous extension.

When adding note, you can add a slash to indicate a subdirectory. For example, `noted -I maths/ch1` will create a note called `ch1` in `maths` folder. **Note**: You can't create a note in a non-existing folder. You need to create the folder first manually.

Unlike pass, you don't need to use `noted init` to initialise the store, it is automatically created once you use any command. The store is either in HOME or XDG_CONFIG_HOME if it is defined.

## Usage
```
Usage: ./noted [-l] [-i] [-v] [-h]
       ./noted [-I|-Q|-R|-S] [filename]
noted is a minimalistic command line note manager written in C.
Options:
  -l		    Lists the notes in the directory in a tree format.
  -i		    Prints the directory where the notes are stored.
  -v		    Prints the version of the program.
  -h		    Show the help message.
  -I [filename]	Creates a new note with the name specified in the filename.
  -Q [filename]	Searches for the note with specified name and prints its path to the stdout.
  -R [filename]	Removes the note specified in the filename.
  -S [filename]	Prints the content of the note specified in the filename.
```

# Contributions
Contributions are welcomed, feel free to open a pull request.

# License
This project is licensed under the GNU Public License v3.0. See [LICENSE](https://github.com/night0721/noted/blob/master/LICENSE) for more information.
