#!/bin/bash

# This is a simple wrapper around "ln" that defaults to using 
# symbolic links instead of hard links.

/usr/bin/ln --symbolic --relative ${@}

