#define MAX_LINE_SIZE 1024
#define MAX_BUF_SIZE 16384

#define MAXFILEPATH 255

#define CONFIGFILENAME "bloomutil"
#define CONFIGMAXBUF 16384
#define CONFIGDELIM "="
#define CONFIGMAXFILENAME MAXFILEPATH
#define CONFIGDIRDELIM "/"
#define CONFIGDIRLOC "/etc"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <unistd.h>

#include "bloom.h"

struct bloom bloom;

int opt_init=0;
int opt_ignorecase=0;
int opt_search=0;
int opt_verbose=0;
int opt_debug=0;
int opt_unhex=0;
unsigned int opt_progressitems=1000;
double opt_errorrate=0.01;
char *opt_bloomfile=NULL;

char config_bloomfile[MAXFILEPATH];

int readconfig (char *filename)
{
       char line[CONFIGMAXBUF];

        int i = 0;
	FILE *fp;
	char *cfline;
	size_t size;

	fp=fopen(filename,"r");
	if (fp==NULL) {
#ifdef DEBUG
		fprintf(stderr,"Error opening config file: %s\n", filename);
#endif
		return 0;
	}

	while(fgets(line, sizeof(line), fp) != NULL) {
		size=strlen(line);
		if (line[size-1]=='\n') line[--size]='\0';
		if (line[size-1]=='\r') line[--size]='\0';
		cfline = strstr((char *)line,CONFIGDELIM); 
		if (cfline!=NULL) {
			char *key=line;
			char *value;
			cfline[0]='\0';
			cfline = cfline + strlen(CONFIGDELIM);
			value=cfline;
#ifdef DEBUG
			printf ("Key: %s\n", key);	
			printf ("Value: %s\n", value);	
#endif
				if (strcmp(key,"debug")==0) {
					opt_debug=atoi(value);
				} else if (strcmp(key,"verbose")==0) {
					opt_verbose=atoi(value);
				} else if (strcmp(key,"ignorecase")==0) {
					opt_ignorecase=atoi(value);
				} else if (strcmp(key,"unhex")==0) {
					opt_unhex=atoi(value);
				} else if (strcmp(key,"errorrate")==0) {
					opt_errorrate=atof(value);
				} else if (strcmp(key,"bloomfile")==0) {
					strncpy(config_bloomfile, value, MAXFILEPATH);
					opt_bloomfile=config_bloomfile;
				} 
		} 
	}

	fclose(fp);
	return 1;
}

void loadconfig (void) {
	char filename[CONFIGMAXFILENAME];

	/* read system wide config first */	
	strncpy (filename, CONFIGDIRLOC, CONFIGMAXFILENAME);
	strncat (filename, CONFIGDIRDELIM, CONFIGMAXFILENAME);
	strncat (filename, CONFIGFILENAME, CONFIGMAXFILENAME);
	readconfig(filename);

	/* read user specific config */
	strncpy (filename, getenv("HOME"), CONFIGMAXFILENAME);
	strncat (filename, CONFIGDIRDELIM, CONFIGMAXFILENAME);
	strncat (filename, ".", CONFIGMAXFILENAME);
	strncat (filename, CONFIGFILENAME, CONFIGMAXFILENAME);
	readconfig(filename);
}

char *str2upper(char *src, char *dest) {
	char *p=dest;
	while (*src) {
		*dest++=toupper(*src++);
	}
	*dest='\0';
	return p;
}


void displayhelp (void) {
	FILE *displayto=stdout;

	char src[128]="test\ni\nI";
	char dest[128];

	fprintf(displayto,"bloomutil: utility to query/build set of data. Copyright (C) 2015. Kost\n\n");
	fprintf(displayto,"-h\tDisplay help\n");
	fprintf(displayto,"-b <f>\tUse <f> for name of data set (bloom structure)\n");
	fprintf(displayto,"-c\tCreate data set (bloom structure)\n");
	fprintf(displayto,"-s\tSearch item in data set (bloom structure)\n");
	fprintf(displayto,"-u\tUnhex data first (convert specified hex string to binary)\n");
	fprintf(displayto,"-e\tUse error rate for creating (default: %f)\n",opt_errorrate);
	fprintf(displayto,"-p <i>\tDisplay progress after <i> number of items processed\n");
	fprintf(displayto,"\n");
	fprintf(displayto,"Example: bloomutil mystring\n");
}


size_t hexstr2char (char *src, char *dest, size_t destsize) {
	size_t i=0;
	size_t size=strlen(src);
	char *ptr=dest;
	unsigned int ch;

	/* prevent buffer overflow */	
	if ((size/2)>destsize) size=destsize;

	while (i<size) {
		sscanf(src, "%2x", &ch);
		*dest++ = ch;
		src += 2;
		i+=2;
	}
	return size/2;
}

unsigned long getlinecount (FILE *fp) {
	unsigned long lines = 0;
	int ch;
	unsigned char buffer[MAX_BUF_SIZE];
	size_t blocks, i;

	while(!feof(fp))
	{
		blocks=fread(buffer, 1, MAX_BUF_SIZE, fp);
		for (i=0; i<blocks; i++) {
			if(buffer[i] == '\n')
			{
			    lines++;
			}
		}
	}
	
	return lines;
}

int main (int argc, char *argv[]) {
	int count;
	unsigned long maxitems=0;
	int c;
	int index;
	FILE *fp;
	unsigned long items;
	char line[MAX_LINE_SIZE];
	char pline[MAX_LINE_SIZE];
	char unhex[MAX_LINE_SIZE];
	char *toprocess;
	int size;
	int found=0;	

	/* safe defaults */
	opt_errorrate=0.01;
	opt_bloomfile=NULL;

	/* load config */
	loadconfig();

  while ((c = getopt (argc, argv, "huicp:svde:b:")) != -1)
    switch (c)
      {
      case 'h':
	displayhelp();
	exit(0);
	break;
      case 'u':
	opt_unhex = 1;
	break;
      case 'i':
	opt_ignorecase = 1;
	break;
      case 'c':
        opt_init = 1;
        break;
      case 'p':
	opt_progressitems = atoi(optarg);
	break;
      case 'e':
	opt_errorrate = atof(optarg);
	break;
      case 'b':
        opt_bloomfile = optarg;
        break;
      case 's':
        opt_search = 1;
        break;
		case 'v':
		opt_verbose++;
		break;

		case 'd':
		opt_debug++;
		break;
      case '?':
        if (optopt == 'b')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        return 1;
      default:
        abort ();
      }

	if (opt_debug) {
	  printf ("opt_init = %d, opt_search = %d, opt_bloomfile = %s\n",
		  opt_init, opt_search, opt_bloomfile);

	for (count = 1; count < argc; count++) {
		printf("argv[%d] = %s\n", count, argv[count]);
	}

		for (index = optind; index < argc; index++)
		  printf ("Non-option argument %s\n", argv[index]);
	}

	if (opt_init) {
		for (index = optind; index < argc; index++) {
			if (opt_verbose) fprintf(stderr,"[i] Counting lines for %s\n", argv[index]);
			fp=fopen(argv[index],"r");
			if (fp==NULL) {
				fprintf(stderr,"Error opening %s\n",argv[index]);
				break;	
			}
			items=getlinecount(fp);
			if (opt_verbose) fprintf(stderr,"[i] %s have %lu lines/items\n",argv[index],items);
			maxitems=maxitems+items;
			fclose(fp);
		}
		if (opt_verbose) fprintf(stderr,"[i] Maximum number of items: %lu\n",maxitems);
		bloom_init(&bloom, maxitems, opt_errorrate);

		items=0;
		for (index = optind; index < argc; index++) {
			if (opt_verbose) fprintf(stderr,"[i] Processing %s\n", argv[index]);
			fp=fopen(argv[index],"r");
			if (fp==NULL) {
				fprintf(stderr,"Error opening %s\n",argv[index]);
				break;	
			}
			/* read line by line */
			while (fgets (line, sizeof(line), fp)) {
				toprocess=line;
				size=strlen(line);
				if (line[size-1]=='\n') line[--size]='\0';
				if (line[size-1]=='\r') line[--size]='\0';
				if (opt_debug) fprintf(stderr,"Line (%d): %s \n",size,line);
				if (opt_verbose && (items++ % opt_progressitems==0)) fprintf(stderr,"\r[i] Line %lu of %lu", items, maxitems);

				if (opt_ignorecase) {
					toprocess=str2upper(toprocess,pline);
				}
				if (opt_unhex) {
					size=hexstr2char(toprocess,unhex,MAX_LINE_SIZE);
					toprocess=unhex;
				} 
				bloom_add(&bloom, toprocess, size);
			}
			if (opt_verbose) fprintf(stderr,"\n[i] Done for %s!\n",argv[index]);
			fclose(fp);
		}

		if (opt_bloomfile==NULL) {
			fprintf(stderr,"No bloom file specified for init. Not saving.\n");
		} else {
			if (opt_verbose) fprintf(stderr,"[i] Saving to %s\n",opt_bloomfile);
			bloom_save(&bloom,opt_bloomfile);
			/* if (opt_verbose) bloom_print(&bloom); */
		}
	}

	if (opt_search || (!opt_init)) {
		if (opt_bloomfile==NULL) {
			fprintf(stderr,"No bloom file specified.\n");
		} else {
			if (opt_verbose) fprintf(stderr,"[i] Opening bloom file: %s\n", opt_bloomfile);
			if (bloom_load(&bloom, opt_bloomfile)) {
				fprintf(stderr,"[i] Error loading bloom file: %s\n", opt_bloomfile);
				return (1);
			}
		}

		if (opt_verbose) fprintf(stderr,"[i] Searching patterns\n");

		for (index = optind; index < argc; index++) {
			toprocess=argv[index];
			if (opt_verbose) fprintf(stderr,"[i] Processing %s\n", toprocess);
			size=strlen(toprocess);
			if (opt_ignorecase) {
				toprocess=str2upper(toprocess,pline);
			}
			if (opt_unhex) {
				size=hexstr2char(toprocess,unhex,MAX_LINE_SIZE);
				toprocess=unhex;
			} 
			found=bloom_check(&bloom, toprocess, size);
			if (found) {
				fprintf(stdout,"%s found\n", argv[index]);
			} else {
				fprintf(stdout,"%s not found\n", argv[index]);
			}
		}
	}
}
