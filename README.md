# bash_builtin_file_utils

# Summary

For obtaining file attributes like the last-modified-time or the file-size in Bash scripts the UNIX tool
*/usr/bin/stat* is usually used.  This takes a few milliseconds because */usr/bin/stat* is an
external program. With this Bash extension the same can be done faster with  built-in commands.

Status: Experimental




# Installation

First [Install dependencies](./INSTALL_DEPENDENCIES.md)

Compile:

    compile_C.sh  bashbuiltin_file_utils.c

# Usage

The compiled builtins must be loaded once.
The compile script generates  a script which can be sourced.

    source load_this_builtin.sh

This file contains the list of commands.
To obtain help for the builtin commands type for example

    help cg_fileSize
