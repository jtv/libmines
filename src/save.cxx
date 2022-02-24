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

/** We encode our data files in a home-rolled base64--see RFC 3548--with 6 bits
 * of data per character.
 */

#include <cassert>
#include <stdexcept>
#include <string>

#include "save.hxx"

using namespace std;

namespace
{
/// Header at start of saved file--we may change the format later
const string saveheader = "#mines 0.2\n";

unsigned char encode[64], decode[256];
volatile bool encoding_initialized = false;

int init_range(char first, char last, int n)
{
  for (unsigned char i=first; i <= last; ++i, ++n)
  {
    encode[n] = i;
    decode[i] = n;
  }
  return n;
}
}


void initialize_encoding()
{
  if (encoding_initialized) return;

  int n = 0;
  n = init_range('A','Z',n);
  n = init_range('a','z',n);
  n = init_range('0','9',n);
  n = init_range('+','+',n);
  n = init_range('/','/',n);
  assert(n == 64);

  encoding_initialized = true;
}


char *write_header(char *here)
{
  strcpy(here,saveheader.c_str());
  return here + saveheader.size();
}


const char *read_header(const char *here)
{
  if (strncmp(here,saveheader.c_str(),saveheader.size()) != 0)
    throw runtime_error("Saved game not in recognized format");
  return here + saveheader.size();
}


unsigned int extract_char(const char *&here)
{
  const unsigned char c = *here++;
  if (!decode[c] && c != encode[0])
    throw runtime_error("Unexpected character in data block: '" +
	string(here-1,here) + "'");
  return decode[c];
}

char produce_char(unsigned int x)
{
  assert(x < 64);
  assert(encode[x]);
  return encode[x];
}

const char *skip_whitespace(const char *here)
{
  while (*here && isspace(*here)) ++here;
  return here;
}

int read_int(const char key[], const char *&here)
{
  const size_t keylen = 4;
  assert(strlen(key)==keylen);

  here = skip_whitespace(here);
  if (!*here) throw runtime_error("Unexpected end of saved-game buffer");
  if (strncmp(key,here,keylen) != 0)
    throw runtime_error("Invalid saved game format: "
	"no " + string(key) + " field "
	"(found '" + string(here,here+4) + "' instead)");
  here = skip_whitespace(here + keylen);
  const int result = atoi(here);
  while (*here && !isspace(*here)) ++here;
  return result;
}


char *write_int(const char key[], char *here, int val)
{
  const size_t keylen = 4;
  assert(strlen(key) == keylen);

  // TODO: Set locale to "C" first!
  sprintf(here, "%s %d\n", key, val);
  return here + strlen(here);
}


char *write_newline(char *here)
{
  *here = '\n';
  return here + 1;
}


int linepadding(int bitsperline)
{
  return ((bitsperline+7)/8) % 3;
}


namespace
{
inline const char *padbytes(int padding)
{
  static const char pad[3] = "==";

  assert(padding >= 0);
  assert(padding < 3);

  return pad + (2-padding);
}
}


char *write_eol(char *here, int padding)
{
  strcpy(here,padbytes(padding));
  here += padding;
  return write_newline(here);
}


const char *read_eol(const char *here, int padding)
{
  if (padding && strncmp(here,padbytes(padding),padding) != 0)
    throw runtime_error("Saved game format error: incorrect end of data line "
	"('" + string(here,here+padding) + "' instead of "
	"'" + padbytes(padding) + "')");
  here += padding;
  if (!isspace(*here))
  {
    if (!*here) throw runtime_error("Saved game format error: truncated data");
    throw runtime_error("Unespected data at end of line: '" +
	string(here,here+1) + "'");
  }

  return skip_whitespace(here);
}


void terminate(char *here)
{
  *here = '\0';
}


void read_terminator(const char *here)
{
  here = skip_whitespace(here);
  if (*here) throw runtime_error("Saved game format error: "
      "Unexpected data after end of data block");
}

