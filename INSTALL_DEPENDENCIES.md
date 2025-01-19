# Installation of dependencies

Install  software packages:

       - gcc or clang or  build-essential
       - bash-builtins

## Ubuntu or Debian Linux:

     apt-get install build-essential bash-builtins




## MacOSX

    You need https://www.macports.org/

    sudo port selfupdate

    sudo port install  bash

Important: The builtins will work with  the bash version installed by Mac ports  */opt/local/bin/bash*, but not with  */bin/bash*.
You might need to change to the newly installed Bash.
The following shebang line at the top of the script might select the proper Bash version:

     #!/usr/bin/env bash
