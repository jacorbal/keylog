Very simple keylogger — `keylog`
================================

**Lightweight keylogger for Linux written in C.**  It also includes two
tools: `klserver` to receive data through a TCP connection, and `kljson`
to convert standard log output to JSON format.

  - Current version: 1.0.4 (2026-02-13)

Features
--------

  - **Keyboard detection.**  Automatic detection of keyboard event file.
  - **Network support.**  Transmit keystrokes over TCP to a specified
    server.
  - **JSON conversion.**  Convert logs to JSON format for easier
    processing.
  - **User-friendly.**  Minimal setup required to start logging.

Building
--------

Download the repository:

    git clone https://github.com/jacorbal/keylog.git

Compile the project and generate the `keylog`, `klserver`, and `kljson`
executables in the `bin/` directory.

    cd keylog/
    make all

or, separately:

    cd keylog/
    make keylog
    make klserver
    make kljson

The programs `klserver` and `kljson` are optional, although useful.

**Note.**  In order to build `kljson` binary, the dependency
`libcjson-dev` must be installed on the system.

Usage
-----

### Client mode

#### Save keystrokes to a file

Log all keystrokes to the specified file during runtime:

    cd keylog/bin/
    sudo ./keylog -f <output_file>

#### Send keystrokes over the network

Transmit all keystrokes to a server running on `<hostname>`:

    cd keylog/bin/
    sudo ./keylog -n <hostname>

### Server mode

#### Output keystrokes to `stdout`

Display received keystrokes directly in the terminal:

    cd keylog/bin/
    ./klserver

#### Save keystrokes to a file

Log all received keystrokes into the specified output file:

    cd keylog/bin/
    ./klserver -f <output_file>

### Data processing

#### Convert output file to JSON format

Convert standard log output to JSON for easier data handling:

    cd keylog/bin/
    sudo ./keylog -f <output_file>
    ...^C
    ./kljson -f <output_file> >output.json

Output format
-------------

For each log line, first, the timestamp is presented in format
`YYYY-MM-DD HH:MM:SS.mmm +HHMM`.  Following the timestamp, there are two
spaces, after which each key press is recorded as a string representing
the key.  This is followed by a hyphen surrounded by spaces, then the
action (either "pressed" or "released"), and concludes with a newline.

### Example

Typing "hello world" and quitting (`Ctrl-C`) will produce:

    2026-02-13 10:45:40.338 +0100  KEY_H - pressed
    2026-02-13 10:45:40.418 +0100  KEY_H - released
    2026-02-13 10:45:40.450 +0100  KEY_E - pressed
    2026-02-13 10:45:40.538 +0100  KEY_E - released
    2026-02-13 10:45:40.890 +0100  KEY_L - pressed
    2026-02-13 10:45:40.962 +0100  KEY_L - released
    2026-02-13 10:45:41.058 +0100  KEY_L - pressed
    2026-02-13 10:45:41.138 +0100  KEY_L - released
    2026-02-13 10:45:41.234 +0100  KEY_O - pressed
    2026-02-13 10:45:41.322 +0100  KEY_O - released
    2026-02-13 10:45:42.490 +0100  KEY_SPACE - pressed
    2026-02-13 10:45:42.578 +0100  KEY_SPACE - released
    2026-02-13 10:45:42.826 +0100  KEY_W - pressed
    2026-02-13 10:45:42.922 +0100  KEY_W - released
    2026-02-13 10:45:43.186 +0100  KEY_O - pressed
    2026-02-13 10:45:43.282 +0100  KEY_O - released
    2026-02-13 10:45:43.506 +0100  KEY_R - pressed
    2026-02-13 10:45:43.594 +0100  KEY_R - released
    2026-02-13 10:45:43.930 +0100  KEY_L - pressed
    2026-02-13 10:45:44.010 +0100  KEY_L - released
    2026-02-13 10:45:44.354 +0100  KEY_D - pressed
    2026-02-13 10:45:44.498 +0100  KEY_D - released
    2026-02-13 10:45:46.362 +0100  KEY_ENTER - pressed
    2026-02-13 10:45:46.450 +0100  KEY_ENTER - released
    2026-02-13 10:45:46.642 +0100  KEY_LEFTCTRL - pressed
    2026-02-13 10:45:46.714 +0100  KEY_C - pressed
    2026-02-13 10:45:46.786 +0100  KEY_C - released

However, this format is intended primarily for user readability.  The
`kljson` program converts this output into a JSON file designed for data
processing.

**Note.**  The main program `keylog` could omit the timestamp by using
the `-t` option.  While this output is easier and more user-friendly to
read on the screen, it will not be compatible for parsing with `kljson`.

    KEY_H - pressed
    KEY_H - released
    KEY_E - pressed
    KEY_E - released
    KEY_L - pressed
    KEY_L - released
    KEY_L - pressed
    KEY_L - released
    KEY_O - pressed
    KEY_O - released
    KEY_LEFTCTRL - pressed
    KEY_C - pressed
    KEY_C - released

Disclaimer
----------

This project was developed purely for educational purposes, exploring
Linux keyboard event handling.  **Never** use this tool on machines
where you do not have explicit permission to log keystrokes.
**Unauthorized surveillance is illegal and unethical.**

License
--------

This software is licensed under the 'ISC License'.
Read the [`COPYING`](COPYING) file on this repository, or gather more
information on [ISC Open Source Software
Licenses](https://www.isc.org/licenses/).

Copyright (c) 2026, J. A. Corbal.
