/*
This file is part of libmines.

Copyright (C) 2005-2022, Jeroen T. Vermeulen.

libmines is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

libmines is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
libmines; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA  02111-1307  USA
*/

enum { patchesperchar = 3 };

/// Call this to initialize encoding tables (lazy; should be threadsafe)
void initialize_encoding();

char *write_header(char *);
const char *read_header(const char *);

/// Write a newline to output buffer
char *write_newline(char *);
/// Find first non-whitespace character at or after given location
const char *skip_whitespace(const char *);

/// Write key string with integer value to output buffer
char *write_int(const char key[], char *here, int val);
/// Read key string with integer value from input buffer
int read_int(const char key[], const char *&here);

/// End-of-line padding mandated by base64
int linepadding(int bitsperline);

/// End a line of base64 data (write terminator plus newline)
char *write_eol(char *, int padding);
/// Parse a base64 line terminator
const char *read_eol(const char *, int padding);

/// Extract a character's worth of ASCII-encoded binary data
unsigned int extract_char(const char *&);
/// Convert a binary value to a single encoding character
char produce_char(unsigned int);

/// Write terminating zero to output buffer
void terminate(char *);
/// Verify that end-of-file happens where we expect it
void read_terminator(const char *);

