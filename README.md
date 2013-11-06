
JojoDiff - diff utility for binary files

Copyright © 2002-2011 Joris Heirbaut
This software is hosted by: <http://sourceforge.net>


    1. Purpose

    *JDIFF * is a program that outputs the differences between two
    (binary) files.
    *JPTCH* can then be used to reconstruct the second file from the
    first file.
    For example:
      o *jdiff* archive0000.tar archive0001.tar archive0001.jdf
      o *jptch* archive0000.tar archive0001.jdf archive0001b.tar
    will create a file archive0001b.tar which is identical to
    archive0001.tar.

    Possible applications include:
      o incremental backups,
      o synchronising files between two computers over a slow network
        (see *JSYNC*).

    *JDIFF* tries to find a minimal set of differences between two files
    using a heuristic algorithm with constant space and linear time
    complexity. This means that accuracy is traded over speed. *JDIFF*
    will therefore, in general, not always find the smallest set of
    differences, but will try to be fast and will use a fixed amount of
    memory.

    JDIFF does not compress the generated patchfile. It is recommended
    to do so with any compression tool you like. See below for an
    example using ZIP.

    Download these utilities from the Jojo's Binary Diff Download Page
    <http://sourceforge.net/projects/jojodiff/>.


    2. Version and history

    The current version of this utility is bèta 0.8 dating from
    September 2011. The modification history is as follows:
        v0.1 	June 2002 	Insert/delete algorithm.
        v0.2a 	June 2002 	Optimized patch files.
        v0.2b 	July 2002 	Bugfix on code-length of 252.
        v0.2c 	July 2002 	Bugfix on divide-by-zero in verbose mode.
        v0.3a 	July 2002 	Copy/insert algorithm.
        v0.4a 	September 2002 	Select "best" of multiple matches.
        v0.4b 	October 2002 	Optimize matches.
        v0.4c 	January 2003 	Rewrote selection algorithm between
                                multiple matches.
        v0.6 	April 2005 	Support files larger than 2GB.
        v0.7 	November 2009 	Optimizations for files larger than 2GB.
        v0.8 	September 2011 	Conversion to C++ classes that should be
        easier to reuse.


    3. Installation

    On Windows systems:
      o Compiled executables are within the "win32" directory. You can
        run them from a command prompt.

    On GCC/Linux systems:
      o Compiled ELF binaries are within the "linux" directory.
      o You may also compile the source by running "make" within the
        "src" directory.
      o Copy the resulting binaries to your /usr/local/bin.
      o Within the bash directory, you can find an example BASH script,
        *JSYNC*, which I use for synchronizing files between two
        computers connected over a slow network.


    4. Usage

*jdiff* [options] original_file new_file [output_file]

    *Options:*
        -v 	Verbose (greeting, results and tips).
        -vv 	Verbose (debug info).
        -h 	Help (this text).
        -l 	List byte by byte (ascii output).
        -lr 	List groups of bytes (ascii output).
        -b 	Try to be better (using more memory).
        -f 	Try to be faster: using less memory, no out of buffer compares.
        -ff 	Try to be faster: no out of buffer compares, no prescanning.
        -m size 	Size (in kB) for look-ahead buffers (default 128).
        -bs size 	Block size (in bytes) for reading from files (default
        4096).
        -s size 	Number of samples in mega (default 8 mega samples).

    *Principles:*
        *JDIFF* tries to find equal regions between two binary files
        using a heuristic hash algorithm and outputs the differences
        between both files. Heuristics are generally used for improving
        performance and memory usage, at the cost of accuracy.
        Therefore, this program may not find a minimal set of
        differences between files.
    *Notes:*
      o Options -m and -s should be used after -b, -f or -ff.
      o Accuracy may be improved by increasing the number of samples.
      o Speed may be increased with option -f or -ff (lower accuracy).
      o Sample size is always lowered to the largest n-bit prime (n < 32)
      o Original and new files must be random access files.
      o Output is sent to standard output if output file is missing.
    *Important:*
        Do not use jdiff directly on compressed files, such as zip,
        gzip, rar, because compression programs tend to increase the
        difference between files ! Instead use jdiff on uncompressed
        archives, such as tar, cpio or zip -0, and then compress the
        files afterwards, including the jdiff patch file. Afterwards, do
        not forget to uncompress the files before using jpatch. For
        example:
        *zip* -0 archive0000.zip mydir/* 	put mydir in an archive
        *zip* -0 archive0001.zip mydir/* 	some time later
        *jdiff* archive0000.zip archive0001.zip archive0001.jdf
        difference between archives
        *zip* -9 archive0001.jdf.zip 	send compressed difference file to
        a friend
        *zip* -9 archive0000.zip.zip archive0000.zip 	compress the
        archive before sending to a friend
        *...*
        *unzip* archive0000.zip.zip 	restore uncompressed zip file
        *unzip* archive0000.jdf.zip 	restore uncompressed jdf file
        *jpatch* archive0000.zip archive0001.jdf archive0001b.zip
        recreate archive001.zip
        *unzip* archive0001b.zip	restore mydir

        You may also replace zip -0 by tar and zip -9 by gzip, or any
        other archiving and/or compression utility you like.

*jpatch* [options] original_file patch_file [output_file]

    *Options:*
        -v 	Verbose (greeting, results and tips).
        -vv 	Verbose (debug info).
        -vvv 	Verbose (more debug info).
        -h 	Help (this text).

    *Principles:*
        *JPATCH* reapplies a diff file, generated by jdiff, to the
        original file, restoring the new file.


    5. Contacts and remarks

    Author: 	Joris Heirbaut
    Contact me via sourceforge <http://sourceforge.net/projects/jojodiff>

    If you like this program, please let me know ! If you reuse the
    source code for your personal (open source) applications, it would
    be great to let me know too.


        6. Acknowledgements

    Earlier versions of this software have been developed within the
    Cygwin/GNU environment. More recently, development has been done in
    Eclipse/CDT using GCC and MinGW/GCC.
