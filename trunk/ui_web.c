#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "c_abi.h"

static const char
header[] =
  "Content-type: text/html\n\n"
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
  "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n"
  "<html xml:lang=\"en\" lang=\"en\">\n"
  "<head>"
  "<meta http-equiv=\"Content-Type\" content=\"text/xhtml+xml; charset=utf-8\" />\n"
  "<title>Minesweeper</title>"
  "</head>"
  "<body>\n",
footer[] =
  "</body></html>\n"
;

static const char
  tag_cols[]="cols=",
  tag_game[]="game=",
  tag_intel[]="intl=",
  tag_mines[]="mines=",
  tag_rows[]="rows=";

enum { idlen=16 };
enum { maxsize=16384 };

static int read_id(const char input[], char buf[])
{
  int i;
  strncpy(buf,input,idlen);
  buf[idlen]='\0';
  for (i=0; i<idlen; ++i) if (!isxdigit(buf[i])) return 0;
  return 1;
}


static void set_filename(char name[], const char gameid[])
{
  sprintf(name, "/var/local/lib/mines/games/%s", gameid);
}


int main(void)
{
  char id[idlen+1];
  char filename[300];
  int rows=0, cols=0, mines=0, intelligence=mines_max_intelligence();
  int atr=0, atc=0, coords_set=0;
  Minefield *F = NULL;
  const char *pos;

  printf("%s", header);

  id[0]='\0';
  for (pos=getenv("QUERY_STRING"); pos; pos=strchr(pos+1,'&'))
  {
    if (*pos == '&') ++pos;
    switch (*pos)
    {
    case 'a':
      if (pos[1]=='t' &&
	  (pos[2]=='c' || pos[2]=='r') &&
	  pos[3]=='=')
      {
	coords_set = 1;
        switch (pos[2])
        {
        case 'c': atc = atoi(pos+4); break;
        case 'r': atr = atoi(pos+4); break;
        }
      }
      break;
    case 'c':
      assert(strlen(tag_cols)==5);
      if (strncmp(pos,tag_cols,5) == 0) cols=atoi(pos+5);
      break;
    case 'g':
      assert(strlen(tag_game)==5);
      if (strncmp(pos,tag_game,5) == 0 && !read_id(pos+5,id))
      {
        puts("<p><em>Invalid game identifier</em></p>");
        puts(footer);
        return 0;
      }
      break;
    case 'i':
      assert(strlen(tag_intel)==5);
      if (strncmp(pos,tag_intel,5) == 0) intelligence = atoi(pos+5);
      break;
    case 'm':
      assert(strlen(tag_mines)==6);
      if (strncmp(pos,tag_mines,6) == 0) mines=atoi(pos+6);
      break;
    case 'r':
      assert(strlen(tag_rows)==5);
      if (strncmp(pos,tag_rows,5) == 0) rows=atoi(pos+5);
      break;
    }
  }

  if (id[0])
  {
    char saved[maxsize+100+1];
    int fd = -1;
    ssize_t bytes = 0, bytesread = 0;

    set_filename(filename, id);
    fd = open(filename, O_RDONLY);
    if (fd == -1)
    {
      const int err = errno;
      perror("Could not open game file for reading");
      if (err == EEXIST)
      {
        puts("<p><em>Game not found</em></p>");
	puts(footer);
      }
      exit(1);
    }
    do
    {
      bytes = read(fd, saved+bytesread, sizeof(saved)-bytesread-1);
      if (bytes > 0) bytesread += bytes;
    } while (bytes > 0 || (bytes == -1 && errno == EINTR));
    if (bytes < 0)
    {
      perror("Error reading game");
      exit(1);
    }
    close(fd);
    F = mines_load(saved);
    if (!F) exit(1);
    rows = mines_rows(F);
    cols = mines_cols(F);
  }
  else if (rows && cols && mines && rows*cols<=maxsize)
  {
    /* We have parameters.  Create new game. */
    /* TODO: Seed randomizer! */
    sprintf(id,"%*.*x",idlen,idlen,rand());
    F = mines_init(rows,cols,mines);
    if (!F) exit(1);
    mines_set_intelligence(F, intelligence); 
  }
  else
  {
    /* TODO: Allow visitor to start a new game! */
  }


  if (F)
  {
    int r, c;
    int done=0;
    const char *scriptname = getenv("SCRIPT_NAME");
    char saved[maxsize+100+1];
    int fd;
    size_t bytes;

    if (!mines_togo(F))
    {
      puts("<h1>You win.  Congratulations!</h1>");
      done=1;
    }
    else if (coords_set && !mines_probe(F, atr, atc, 0))
    {
      puts("<h1>You lose!</h1>");
      /* TODO: Actually stop the game here! */
      done=1;
    }

    printf("<p>Moves: %d.  Fields to go: %d</p>\n",
	mines_moves(F), mines_togo(F));
    printf("<form action=\"%s\" method=\"GET\"><table>", scriptname);
    for (r=-1; r<=rows; ++r)
    {
      printf("<tr>");
      for (c=-1; c<=cols; ++c)
      {
	const char x = mines_at(F, r, c);
	printf("<td>");
	if (done || x != '^') printf("%c", x);
	else printf("<a href=\"%s?game=%s&atr=%d&atc=%d\">=</a>",
	      scriptname,id,r,c);
	printf("</td>");
      }
      puts("</tr>");
    }
    puts("</form></table>");

    bytes = mines_save(F,saved);
    set_filename(filename, id);
    fd = open(filename, O_WRONLY|O_CREAT|O_TRUNC, 0744);
    if (fd == -1)
    {
        perror("Could not open game file for writing");
        exit(1);
    }
    if (write(fd, saved, bytes) < 0)
    {
        perror("Could not write game file");
	unlink(filename);
	exit(1);
    }
    close(fd);
    mines_close(F);
  }

  printf("%s", footer);
  return 0;
}

