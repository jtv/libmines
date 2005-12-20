#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
toolarge[] = "<p><em>Playing field too large.  Try something smaller!</em></p>",
youwin[] =
  "<h1>You win.  Congratulations!</h1>",
youlose[] =
  "<h1><em>Boom!</em>  You lose.</h1>",
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


static void seed_randomizer(void)
{
  srand(getpid()^time(NULL)^(unsigned long)getenv("SERVER_SOFTWARE"));
}


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
  char id[idlen*2];
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
    char saved[maxsize+1000];
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
  else if (rows && cols && mines)
  {
    size_t idbytes = 0;

    if (rows*cols > maxsize)
    {
      puts(toolarge);
      puts(footer);
      exit(0);
    }

    if (mines >= rows*cols)
    {
      puts("<p><em>That's too many mines!</em></p>");
      puts(footer);
      exit(0);
    }

    /* We have parameters.  Create new game. */
    seed_randomizer();
    do
    {
      sprintf(id+idbytes,"%*.*x",4,4,rand());
      idbytes = strlen(id);
    } while (idbytes < idlen);
    id[idlen] = '\0';
    F = mines_init(rows,cols,mines);
    if (!F) exit(1);
    if (mines_savesize(F) > maxsize)
    {
      puts(toolarge);
      mines_close(F);
      F = NULL;
    }
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
    char saved[maxsize+100];
    int fd;
    size_t bytes;
    char url[200];
    size_t urlhead;

    if (!mines_togo(F))
    {
      puts(youwin);
      done=1;
    }
    else if (coords_set)
    {
      if (!mines_probe(F, atr, atc, 0))
      {
        puts(youlose);
        /* TODO: Actually stop the game here! */
        done=1;
      }
      else if (!mines_togo(F))
      {
	puts(youwin);
	done=1;
      }
    }

    printf("<p>Moves: %d.  Fields to go: %d</p>\n",
	mines_moves(F), mines_togo(F));
    printf("<form action=\"%s\" method=\"GET\"><table>", scriptname);
    urlhead = sprintf(url, "<td><a href=\"%s?game=%s&atr=", scriptname, id);
    for (r=-1; r<=rows; ++r)
    {
      sprintf(url+urlhead, "%d&atc=", r);

      printf("<tr>");
      for (c=-1; c<=cols; ++c)
      {
	const char x = mines_at(F, r, c);
	if (done || x != '^') printf("<td>%c</td>",x);
	else printf("%s%d\">=</a></td>",url,c);
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

